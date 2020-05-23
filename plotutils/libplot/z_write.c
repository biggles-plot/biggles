/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* This file contains a special version of the function
   _maybe_output_image, which is called by the BitmapPlotter closepl method
   (see b_closepl.c).  This version is for the PNGPlotter subclass:
   provided that the current page of graphics is the first, it writes out a
   PNG file. */

/* Note: this calls gmtime(), which is not reentrant.  For thread-safety we
   should use gmtime_r() instead, but declaring it portably is a hard
   problem (cf. comments at head of p_defplot.c and c_defplot.c). */

/* Other than this egregious problem, this code is thread-safe (the warning
   and error handlers which we pass to libpng lock and unlock the `message
   mutex' defined in g_error.c).  Since libpng uses callbacks, warning and
   error messages aren't produced simply by calling the functions
   _plotter->warning() and _plotter->error() defined in g_error.c. */

/* Note: we should improve this code to support the use of user-specified
   warning and error handlers for libplot; cf. the code in g_error.c.  But
   their use isn't documented yet. */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

#include <png.h>

/* song and dance to define time_t, and declare both time() and gmtime() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for time_t on some pre-ANSI Unix systems */
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() on some pre-ANSI Unix systems */
#include <time.h>		/* for gmtime() */
#else  /* not TIME_WITH_SYS_TIME, include only one (prefer <sys/time.h>) */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else  /* not HAVE_SYS_TIME_H */
#include <time.h>
#endif /* not HAVE_SYS_TIME_H */
#endif /* not TIME_WITH_SYS_TIME */

/* Mutex for locking the warning/error message subsystem.  Defined in
   g_error.c */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
extern pthread_mutex_t _message_mutex;
#endif
#endif

static const char _short_months[12][4] = 
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/* forward references */
static int _image_type (miPixel **pixmap, int width, int height);
static void _our_error_fn_stdio (png_struct *png_ptr, const char *data);
static void _our_warn_fn_stdio (png_struct *png_ptr, const char *data);
#ifdef LIBPLOTTER
static void _our_IO_flush_fn (png_struct *png_ptr);
static void _our_error_fn_stream (png_struct *png_ptr, const char *data);
static void _our_warn_fn_stream (png_struct *png_ptr, const char *data);
static void _our_write_fn (png_struct *png_ptr, png_byte *data, png_size_t length);
#endif /* LIBPLOTTER */

int
_pl_z_maybe_output_image (S___(Plotter *_plotter))
{
  miPixel **pixmap;		/* pixmap in miCanvas */
  int width, height;
  int image_type, bit_depth, color_type;
  png_struct *png_ptr;
  png_info *info_ptr;
  char time_buf[32], software_buf[64];
  png_text text_ptr[10];
  time_t clock;
  struct tm *tmsp;
  FILE *fp = _plotter->data->outfp;
  FILE *errorfp = _plotter->data->errfp;
  void *error_ptr;
  png_error_ptr error_fn_ptr, warn_fn_ptr;
#ifdef LIBPLOTTER
  ostream *stream = _plotter->data->outstream;
  ostream *errorstream = _plotter->data->errstream;
#endif

#ifdef LIBPLOTTER
  if (fp == (FILE *)NULL && stream == (ostream *)NULL)
    return 0;
#else
  if (fp == (FILE *)NULL)
    return 0;
#endif
  
  /* Output the page as a PNG file, but only if it's page #1, since PNG
     format supports only a single page of graphics. */
  if (_plotter->data->page_number != 1)
    return 0;

  /* work out libpng error handling (i.e. callback functions and data) */
#ifdef LIBPLOTTER
  if (errorstream)
    {
      error_fn_ptr = _our_error_fn_stream;
      warn_fn_ptr = _our_warn_fn_stream;
      error_ptr = (void *)errorstream;
    }
  else if (errorfp)
    {
      error_fn_ptr = _our_error_fn_stdio;
      warn_fn_ptr = _our_warn_fn_stdio;
      error_ptr = (void *)errorfp;
    }
  else
    {
      error_fn_ptr = NULL;
      warn_fn_ptr = NULL;
      error_ptr = (void *)NULL;
    }
#else  /* not LIBPLOTTER */
  if (errorfp)
    {
      error_fn_ptr = _our_error_fn_stdio;
      warn_fn_ptr = _our_warn_fn_stdio;
      error_ptr = (void *)errorfp;
    }
  else
    {
      error_fn_ptr = NULL;
      warn_fn_ptr = NULL;
      error_ptr = (void *)NULL;
    }
#endif /* not LIBPLOTTER */

  /* create png_struct, install error/warning handlers */
  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				     error_ptr, 
				     error_fn_ptr, warn_fn_ptr);
  if (png_ptr == (png_struct *)NULL)
    return -1;

  /* allocate/initialize image information data */
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == (png_info *)NULL)
    {
      png_destroy_write_struct (&png_ptr, (png_info **)NULL);
      return -1;
    }

  /* cleanup after libpng errors (error handler does a longjmp) */
  if (setjmp (png_jmpbuf(png_ptr)))
    {
      png_destroy_write_struct (&png_ptr, (png_info **)NULL);
      return -1;
    }
  
#ifdef LIBPLOTTER
  if (stream)
    {
      /* use custom write and flush functions, defined below */
      png_set_write_fn (png_ptr, 
			(void *)stream,
			(png_rw_ptr)_our_write_fn, 
			(png_flush_ptr)_our_IO_flush_fn);
    }
  else
    /* must have fp!=NULL, so use default stdio-based output */
    png_init_io (png_ptr, fp);

#else  /* not LIBPLOTTER */
    /* use default stdio-based output */
    png_init_io (png_ptr, fp);
#endif /* not LIBPLOTTER */

  /* extract pixmap (2D array of miPixels) from miCanvas */
  pixmap = ((miCanvas *)(_plotter->b_canvas))->drawable->pixmap;

  /* what is best image type that can be used?  0/1/2 = mono/gray/rgb */
  width = _plotter->b_xn;
  height = _plotter->b_yn;
  image_type = _image_type (pixmap, width, height);
  switch (image_type)
    {
    case 0:			/* mono */
      bit_depth = 1;
      color_type = PNG_COLOR_TYPE_GRAY;
      break;
    case 1:			/* gray */
      bit_depth = 8;
      color_type = PNG_COLOR_TYPE_GRAY;
      break;
    case 2:			/* rgb */
    default:
      bit_depth = 8;
      color_type = PNG_COLOR_TYPE_RGB;
      break;
    }

  /* Set image information in file header.  Width and height are up to
     2^31, bit_depth is one of 1, 2, 4, 8, or 16, but valid values also
     depend on the color_type selected. color_type is one of
     PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA,
     PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, or
     PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
     PNG_INTERLACE_ADAM7.  compression_type and filter_type MUST currently
     be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE.  */
  png_set_IHDR (png_ptr, info_ptr, 
		(unsigned int)width, (unsigned int)height, 
		bit_depth, color_type,
		_plotter->z_interlace ? PNG_INTERLACE_ADAM7 
		                      : PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  
  /* set transparent color (if user specified one) */
  if (_plotter->z_transparent)
    {
      plColor transparent_color = _plotter->z_transparent_color;
      bool transparent_color_ok = true;
      png_color_16 trans_value;
      
      switch (image_type)
	{
	case 0:			/* mono */
	  if ((transparent_color.red != 0 && transparent_color.red != 0xffff)
	      || 
	      (transparent_color.green != 0 && transparent_color.green != 0xffff)
	      || 
	      (transparent_color.blue != 0 && transparent_color.blue != 0xffff)
	      ||
	      (transparent_color.red != transparent_color.green
	       || transparent_color.red != transparent_color.blue))
	    /* user-specified transparent color isn't monochrome */
	    transparent_color_ok = false;
	  else
	    trans_value.gray = (png_uint_16)transparent_color.red;
	  break;
	case 1:			/* gray */
	  if (transparent_color.red != transparent_color.green
	      || transparent_color.red != transparent_color.blue)
	    /* user-specified transparent color isn't grayscale */
	    transparent_color_ok = false;
	  else
	    trans_value.gray = (png_uint_16)transparent_color.red;
	  break;
	case 2:			/* rgb */
	default:
	  trans_value.red = (png_uint_16)transparent_color.red;
	  trans_value.green = (png_uint_16)transparent_color.green;
	  trans_value.blue = (png_uint_16)transparent_color.blue;
	  break;
	}
      if (transparent_color_ok)
	png_set_tRNS (png_ptr, info_ptr, (png_byte *)NULL, 1, &trans_value);
    }

  /* add some comments to file header */
  text_ptr[0].key = (char *)"Title";
  text_ptr[0].text = (char *)"PNG plot";
  text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
  text_ptr[1].key = (char *)"Creation Time";
  time (&clock);
  tmsp = gmtime (&clock);
  sprintf (time_buf, 
	   "%d %s %d %02d:%02d:%02d +0000", /* RFC 1123 date */
	   (tmsp->tm_mday) % 31, 
	   _short_months[(tmsp->tm_mon) % 12], 1900 + tmsp->tm_year, 
	   (tmsp->tm_hour) % 24, (tmsp->tm_min) % 60, (tmsp->tm_sec) % 61);
  text_ptr[1].text = time_buf;
  text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
  text_ptr[2].key = (char *)"Software";
  sprintf (software_buf, "GNU libplot drawing library %s",
	   PL_LIBPLOT_VER_STRING);
  text_ptr[2].text = software_buf;
  text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;

  png_set_text(png_ptr, info_ptr, text_ptr, 3);
  
  /* write out PNG file header */
  png_write_info (png_ptr, info_ptr);
  
  /* Write out image data, a row at a time; support multiple passes over
     image if interlacing.  We don't simply call png_write_image() because
     the image in the miCanvas's pixmap is a 2-D array of miPixels, and
     sizeof(miPixel) > 4 is possible.  Instead we copy each miPixel in a
     row into a local row buffer, and write out the row buffer. */
  {
    png_byte *rowbuf;
    int num_passes, pass;

    switch (image_type)
      {
      case 0:			/* mono */
	rowbuf = (png_byte *)_pl_xmalloc(((width + 7)/8) * sizeof(png_byte));
	break;
      case 1:			/* gray */
	rowbuf = (png_byte *)_pl_xmalloc(width * sizeof(png_byte));	
	break;
      case 2:			/* rgb */
      default:
	rowbuf = (png_byte *)_pl_xmalloc(3 * width * sizeof(png_byte));
	break;
      }

    if (_plotter->z_interlace)
      /* turn on interlace handling; if interlacing, need >1 pass over image */
      num_passes = png_set_interlace_handling (png_ptr);
    else
      num_passes = 1;

    for (pass = 0; pass < num_passes; pass++)
      {
	int i, j;

	for (j = 0; j < height; j++)
	  {
	    png_byte *ptr;

	    /* fill row buffer with 3 bytes per miPixel (RGB), or 1 byte
	       (gray), or 1 bit (mono) */
	    ptr = rowbuf;
	    for (i = 0; i < width; i++)
	      {
		switch (image_type)
		  {
		  case 0:	/* mono */
		    if (i % 8 == 0)
		      {
			if (i != 0)
			  ptr++;
			*ptr = (png_byte)0;
		      }
		    if (pixmap[j][i].u.rgb[0]) /* white pixel */
		      *ptr |= (1 << (7 - (i % 8)));
		    break;
		  case 1:	/* gray */
		    *ptr++ = (png_byte)pixmap[j][i].u.rgb[0];
		    break;
		  case 2:	/* rgb */
		  default:
		    *ptr++ = (png_byte)pixmap[j][i].u.rgb[0];
		    *ptr++ = (png_byte)pixmap[j][i].u.rgb[1];
		    *ptr++ = (png_byte)pixmap[j][i].u.rgb[2];
		    break;
		  }
	      }
	    
	    /* write out row buffer */
	    png_write_rows (png_ptr, &rowbuf, 1);
	  }
      }

    free (rowbuf);
  }

  /* write out PNG file trailer (could add more comments here) */
  png_write_end (png_ptr, (png_info *)NULL);

  /* tear down */
  png_destroy_write_struct (&png_ptr, (png_info **)NULL);

  return true;
}

/* return best type for writing an image (0=mono, 1=grey, 2=color) */
static int
_image_type (miPixel **pixmap, int width, int height)
{
  int i, j;
  int type = 0;			/* default is mono */
  
  for (j = 0; j < height; j++)
    for (i = 0; i < width; i++)
      {
	unsigned char red, green, blue;
	
	red = pixmap[j][i].u.rgb[0];
	green = pixmap[j][i].u.rgb[1];
	blue = pixmap[j][i].u.rgb[2];
	if (type == 0)		/* up to now, all pixels are black or white */
	  {
	    if (! ((red == (unsigned char)0 && green == (unsigned char)0
		    && blue == (unsigned char)0)
		   || (red == (unsigned char)255 && green == (unsigned char)255
		    && blue == (unsigned char)255)))
	      {
		if (red == green && red == blue)
		  type = 1;	/* need grey */
		else
		  {
		    type = 2;	/* need color */
		    return type;
		  }
	      }
	  }
	else if (type == 1)
	  {
	    if (red != green || red != blue)
	      {
		type = 2;	/* need color */
		return type;
	      }
	  }
      }
  return type;
}

/* custom error and warning handlers (for stdio) */
static void 
_our_error_fn_stdio (png_struct *png_ptr, const char *data)
{
  FILE *errfp;

  errfp = (FILE *)png_get_error_ptr (png_ptr);
  if (errfp)
    {
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* lock the message subsystem */
      pthread_mutex_lock (&_message_mutex);
#endif
#endif
      fprintf (errfp, "libplot: libpng error: %s\n", data);
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* unlock the message subsystem */
      pthread_mutex_unlock (&_message_mutex);
#endif
#endif
    }

  longjmp (png_jmpbuf(png_ptr), 1);
}

static void 
_our_warn_fn_stdio (png_struct *png_ptr, const char *data)
{
  FILE *errfp;

  errfp = (FILE *)png_get_error_ptr (png_ptr);
  if (errfp)
    {
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* lock the message subsystem */
      pthread_mutex_lock (&_message_mutex);
#endif
#endif
      fprintf (errfp, "libplot: libpng: %s\n", data);
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* unlock the message subsystem */
      pthread_mutex_unlock (&_message_mutex);
#endif
#endif
    }
}

#ifdef LIBPLOTTER
/* custom write and flush functions (for streams) */
static void 
_our_write_fn (png_struct *png_ptr, png_byte *data, png_size_t length)
{
  ostream *stream;

  stream = (ostream *)png_get_io_ptr (png_ptr);
  stream->write ((const char *)data, length);
}

static void 
_our_IO_flush_fn (png_struct *png_ptr)
{
  ostream *stream;

  stream = (ostream *)png_get_io_ptr (png_ptr);
  stream->flush ();
}

/* custom error and warning handlers (for streams) */
static void 
_our_error_fn_stream (png_struct *png_ptr, const char *data)
{
  ostream *errstream;

  errstream = (ostream *)png_get_error_ptr (png_ptr);
  if (errstream)
    {
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* lock the message subsystem */
      pthread_mutex_lock (&_message_mutex);
#endif
#endif
      (*errstream) << "libplot: libpng error: " << data << 'n';
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* unlock the message subsystem */
      pthread_mutex_unlock (&_message_mutex);
#endif
#endif
    }

  longjmp (png_ptr->jmpbuf, 1);
}

static void 
_our_warn_fn_stream (png_struct *png_ptr, const char *data)
{
  ostream *errstream;

  errstream = (ostream *)png_get_error_ptr (png_ptr);
  if (errstream)
    {
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* lock the message subsystem */
      pthread_mutex_lock (&_message_mutex);
#endif
#endif
      (*errstream) << "libplot: libpng: " << data << 'n';
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
      /* unlock the message subsystem */
      pthread_mutex_unlock (&_message_mutex);
#endif
#endif
    }
}
#endif /* LIBPLOTTER */
