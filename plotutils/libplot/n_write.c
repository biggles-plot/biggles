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
   (see b_closepl.c).  Provided that the current page is the first, this
   version writes out a PNM (i.e., PBM/PGM/PPM) file for it.  */

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

/* line lengths in ASCII PBM/PGM/PPM formats (max. no of pixels per line) */
#define MAX_PBM_PIXELS_PER_LINE 70
#define MAX_PGM_PIXELS_PER_LINE 16
#define MAX_PPM_PIXELS_PER_LINE 5

/* forward references */
static int best_image_type (miPixel **pixmap, int width, int height);

/* do a rapid decimal printf of a nonnegative integer, in range 0..999
   to a character buffer */
#define FAST_PRINT(integer_to_print, linebuf, pos) \
{ \
  int k, hundreds, tens, ones; \
  bool force_tens; \
  \
  k = (integer_to_print); \
  hundreds = k / 100; \
  k -= (100 * hundreds); \
  tens = k / 10; \
  ones = k - (10 * tens); \
  \
  force_tens = false; \
  if (hundreds) \
    { \
      linebuf[pos++] = hundreds + '0'; \
      force_tens = true; \
    } \
  if (force_tens || tens) \
    linebuf[pos++] = tens + '0'; \
  linebuf[pos++] = ones + '0'; \
}

int
_pl_n_maybe_output_image (S___(Plotter *_plotter))
{
  /* Output the page as a PBM/PGM/PPM file, but only if it's page #1, since
     PBM/PGM/PPM format supports only a single page of graphics. */
  if (_plotter->data->page_number == 1)
    /* emit PBM/PGM/PPM file */
    _pl_n_write_pnm (S___(_plotter));

  return true;
}

/* determine which sort of PNM (i.e. PBM/PGM/PPM) file should be output,
   and output it */
void
_pl_n_write_pnm (S___(Plotter *_plotter))
{
  int type;			/* 0,1,2 = PBM/PGM/PPM */
  int width, height;
  miPixel **pixmap;

  width = _plotter->b_xn;
  height = _plotter->b_yn;
  pixmap = ((miCanvas *)(_plotter->b_canvas))->drawable->pixmap;
  type = best_image_type (pixmap, width, height);

  switch (type)
    {
    case 0:			/* PBM */
      _pl_n_write_pbm (S___(_plotter));
      break;
    case 1:			/* PGM */
      _pl_n_write_pgm (S___(_plotter));
      break;
    case 2:			/* PPM */
    default:
      _pl_n_write_ppm (S___(_plotter));
      break;
    }
}

/* write output (header plus RGB values) in PBM format */
void
_pl_n_write_pbm (S___(Plotter *_plotter))
{
  int i, j;
  bool portable = _plotter->n_portable_output;
  miPixel **pixmap = ((miCanvas *)(_plotter->b_canvas))->drawable->pixmap;
  int width = _plotter->b_xn;
  int height = _plotter->b_yn;  
  FILE *fp = _plotter->data->outfp;
#ifdef LIBPLOTTER
  ostream *stream = _plotter->data->outstream;
#endif

#ifdef LIBPLOTTER
  if (fp == NULL && stream == NULL)
    return;
#else
  if (fp == NULL)
    return;
#endif

  if (fp)
    {
      if (portable)			/* emit ascii format */
	{
	  unsigned char linebuf[MAX_PBM_PIXELS_PER_LINE];
	  int pos = 0;		/* position in line buffer */

	  fprintf (fp, "\
P1\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n", PL_LIBPLOT_VER_STRING, width, height);
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		if (pixmap[j][i].u.rgb[0] == 0)
		  linebuf[pos++] = '1';	/* 1 = black */
		else
		  linebuf[pos++] = '0';
		if (pos >= MAX_PBM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    fwrite ((void *)linebuf, sizeof(unsigned char), pos, fp);
		    putc ('\n', fp);
		    pos = 0;
		  }
	      }
	}
      else			/* emit binary format */
	{
	  int bitcount, bytecount;
	  unsigned char outbyte, set;
	  unsigned char *rowbuf;
	  
	  fprintf (fp, "\
P4\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n", PL_LIBPLOT_VER_STRING, width, height);
	  
	  /* row buffer contains bytes, each representing up to 8 pixels */
	  rowbuf = (unsigned char *)_pl_xmalloc (((width + 7) / 8) * sizeof (unsigned char));
	  for (j = 0; j < height; j++)
	    {
	      bitcount = 0;
	      bytecount = 0;
	      outbyte = 0;
	      for (i = 0; i < width; i++)
		{
		  set = (pixmap[j][i].u.rgb[0] == 0 ? 1 : 0); /* 1 = black */
		  outbyte = (outbyte << 1) | set;
		  bitcount++;
		  if (bitcount == 8)	/* write byte to row (8 bits) */
		    {
		      rowbuf[bytecount++] = outbyte;
		      outbyte = 0;
		      bitcount = 0;
		    }
		}
	      if (bitcount)	/* write final byte (not completely filled) */
		{
		  outbyte = (outbyte << (8 - bitcount));
		  rowbuf[bytecount++] = outbyte;
		}
	      /* emit row of bytes */
	      fwrite ((void *)rowbuf, sizeof(unsigned char), bytecount, fp);
	    }

	  free (rowbuf);
	}
    }
#ifdef LIBPLOTTER
  else if (stream)
    {
      if (portable)			/* emit ascii format */
	{
	  unsigned char linebuf[MAX_PBM_PIXELS_PER_LINE];
	  int pos = 0;		/* position in line buffer */

	  (*stream) << "\
P1\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n';
	  
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		if (pixmap[j][i].u.rgb[0] == 0)
		  linebuf[pos++] = '1';	/* 1 = black */
		else
		  linebuf[pos++] = '0';
		if (pos >= MAX_PBM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    stream->write ((const char *)linebuf, pos);
		    stream->put ('\n');

		    pos = 0;
		  }
	      }
	}
      else			/* emit binary format */
	{
	  int bitcount, bytecount;
	  unsigned char outbyte, set;
	  unsigned char *rowbuf;

	  (*stream) << "\
P4\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n';
	  
	  
	  /* row buffer contains bytes, each representing up to 8 pixels */
	  rowbuf = (unsigned char *)_pl_xmalloc (((width + 7) / 8) * sizeof (unsigned char));
	  for (j = 0; j < height; j++)
	    {
	      bitcount = 0;
	      bytecount = 0;
	      outbyte = 0;
	      for (i = 0; i < width; i++)
		{
		  set = (pixmap[j][i].u.rgb[0] == 0 ? 1 : 0); /* 1 = black */
		  outbyte = (outbyte << 1) | set;
		  bitcount++;
		  if (bitcount == 8)	/* write byte to row (8 bits) */
		    {
		      rowbuf[bytecount++] = outbyte;
		      outbyte = 0;
		      bitcount = 0;
		    }
		}
	      if (bitcount)	/* write final byte (not completely filled) */
		{
		  outbyte = (outbyte << (8 - bitcount));
		  rowbuf[bytecount++] = outbyte;
		}
	      /* emit row of bytes */
	      stream->write ((const char *)rowbuf, bytecount);
	    }

	  free (rowbuf);
	}
    }
#endif
}

/* write output (header plus RGB values) in PGM format */
void
_pl_n_write_pgm (S___(Plotter *_plotter))
{
  int i, j;
  bool portable = _plotter->n_portable_output;
  miPixel **pixmap = ((miCanvas *)(_plotter->b_canvas))->drawable->pixmap;
  int width = _plotter->b_xn;
  int height = _plotter->b_yn;  
  FILE *fp = _plotter->data->outfp;
#ifdef LIBPLOTTER
  ostream *stream = _plotter->data->outstream;
#endif

#ifdef LIBPLOTTER
  if (fp == NULL && stream == NULL)
    return;
#else
  if (fp == NULL)
    return;
#endif

  if (fp)
    {
      if (portable)			/* emit ascii format */
	{
	  /* allow room for 3 decimal digits, plus a space, per pixel */
	  unsigned char linebuf[4 * MAX_PGM_PIXELS_PER_LINE];
	  int pos = 0;
	  int num_pixels = 0;

	  fprintf (fp, "\
P2\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n\
255\n", PL_LIBPLOT_VER_STRING, width, height);
	  
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		/* emit <=3 decimal digits per grayscale pixel */
		FAST_PRINT (pixmap[j][i].u.rgb[0], linebuf, pos)
		num_pixels++;
		if (num_pixels >= MAX_PGM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    fwrite ((void *)linebuf, sizeof(unsigned char), pos, fp);
		    putc ('\n', fp);
		    num_pixels = 0;
		    pos = 0;
		  }
		else
		  linebuf[pos++] = ' ';
	      }
	}
      else				/* emit binary format */
	{
	  unsigned char *rowbuf;
	  
	  rowbuf = (unsigned char *)_pl_xmalloc (width * sizeof (unsigned char));
	  fprintf (fp, "\
P5\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n\
255\n", PL_LIBPLOT_VER_STRING, width, height);
	  
	  for (j = 0; j < height; j++)
	    {
	      for (i = 0; i < width; i++)
		rowbuf[i] = pixmap[j][i].u.rgb[0];
	      fwrite ((void *)rowbuf, sizeof(unsigned char), width, fp);
	    }
	  free (rowbuf);
	}
    }
#ifdef LIBPLOTTER
  else if (stream)
    {
      if (portable)			/* emit ascii format */
	{
	  /* allow room for 3 decimal digits, plus a space, per pixel */
	  unsigned char linebuf[4 * MAX_PGM_PIXELS_PER_LINE];
	  int pos = 0;
	  int num_pixels = 0;

	  (*stream) << "\
P2\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n'
	         << "255" << '\n';
	  
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		/* emit <=3 decimal digits per grayscale pixel */
		FAST_PRINT (pixmap[j][i].u.rgb[0], linebuf, pos)
		num_pixels++;
		if (num_pixels >= MAX_PGM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    stream->write ((const char *)linebuf, pos);
		    stream->put ('\n');

		    num_pixels = 0;
		    pos = 0;
		  }
		else
		  linebuf[pos++] = ' ';
	      }
	}
      else				/* emit binary format */
	{
	  unsigned char *rowbuf;
	  
	  (*stream) << "\
P5\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n'
	         << "255" << '\n';
	  
	  rowbuf = (unsigned char *)_pl_xmalloc (width * sizeof (unsigned char));
	  for (j = 0; j < height; j++)
	    {
	      for (i = 0; i < width; i++)
		rowbuf[i] = pixmap[j][i].u.rgb[0];
	      stream->write ((const char *)rowbuf, width);
	    }
	  free (rowbuf);
	}
    }
#endif
}

/* write output (header plus RGB values) in PPM format */
void
_pl_n_write_ppm (S___(Plotter *_plotter))
{
  int i, j;
  bool portable = _plotter->n_portable_output;
  miPixel **pixmap = ((miCanvas *)(_plotter->b_canvas))->drawable->pixmap;
  int width = _plotter->b_xn;
  int height = _plotter->b_yn;  
  FILE *fp = _plotter->data->outfp;
#ifdef LIBPLOTTER
  ostream *stream = _plotter->data->outstream;
#endif

#ifdef LIBPLOTTER
  if (fp == NULL && stream == NULL)
    return;
#else
  if (fp == NULL)
    return;
#endif

  if (fp)
    {
      if (portable)			/* emit ascii format */
	{
	  /* allow room for 3 decimal digits, plus a space, per pixel */
	  unsigned char linebuf[4 * MAX_PGM_PIXELS_PER_LINE];
	  int pos = 0;
	  int num_pixels = 0;

	  fprintf (fp, "\
P3\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n\
255\n", PL_LIBPLOT_VER_STRING, width, height);
      
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		/* emit <=3 decimal digits per RGB component */
		FAST_PRINT (pixmap[j][i].u.rgb[0], linebuf, pos)
		linebuf[pos++] = ' ';
		FAST_PRINT (pixmap[j][i].u.rgb[1], linebuf, pos)
		linebuf[pos++] = ' ';
		FAST_PRINT (pixmap[j][i].u.rgb[2], linebuf, pos)
		num_pixels++;
		if (num_pixels >= MAX_PPM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    fwrite ((void *)linebuf, sizeof(unsigned char), pos, fp);
		    putc ('\n', fp);
		    num_pixels = 0;
		    pos = 0;
		  }
		else
		  linebuf[pos++] = ' ';
	      }
	}
      else			/* emit binary format */
	{
	  unsigned char *rowbuf;
	  int component;

	  fprintf (fp, "\
P6\n\
# CREATOR: GNU libplot drawing library, version %s\n\
%d %d\n\
255\n", PL_LIBPLOT_VER_STRING, width, height);
      
	  rowbuf = (unsigned char *)_pl_xmalloc (3 * width * sizeof (unsigned char));
	  for (j = 0; j < height; j++)
	    {
	      for (i = 0; i < width; i++)
		for (component = 0; component < 3; component++)
		  rowbuf[3 * i + component] = pixmap[j][i].u.rgb[component];
	      fwrite ((void *)rowbuf, sizeof(unsigned char), 3 * width, fp);
	    }
	  free (rowbuf);
	}
    }
#ifdef LIBPLOTTER
  else if (stream)
    {
      if (portable)			/* emit ascii format */
	{
	  /* allow room for 3 decimal digits, plus a space, per pixel */
	  unsigned char linebuf[4 * MAX_PGM_PIXELS_PER_LINE];
	  int pos = 0;
	  int num_pixels = 0;

	  (*stream) << "\
P3\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n'
	         << "255" << '\n';
      
	  for (j = 0; j < height; j++)
	    for (i = 0; i < width; i++)
	      {
		/* emit <=3 decimal digits per RGB component */
		FAST_PRINT (pixmap[j][i].u.rgb[0], linebuf, pos)
		linebuf[pos++] = ' ';
		FAST_PRINT (pixmap[j][i].u.rgb[1], linebuf, pos)
		linebuf[pos++] = ' ';
		FAST_PRINT (pixmap[j][i].u.rgb[2], linebuf, pos)
		num_pixels++;
		if (num_pixels >= MAX_PPM_PIXELS_PER_LINE || i == (width - 1))
		  {
		    stream->write ((const char *)linebuf, pos);
		    stream->put ('\n');

		    num_pixels = 0;
		    pos = 0;
		  }
		else
		  linebuf[pos++] = ' ';
	      }
	}
      else			/* emit binary format */
	{
	  unsigned char *rowbuf;
	  int component;
	  
	  (*stream) << "\
P6\n\
# CREATOR: GNU libplot drawing library, version " 
		 << PL_LIBPLOT_VER_STRING << '\n'
		 << width << ' ' << height << '\n'
	         << "255" << '\n';
	  
	  rowbuf = (unsigned char *)_pl_xmalloc (3 * width * sizeof (unsigned char));
	  for (j = 0; j < height; j++)
	    {
	      for (i = 0; i < width; i++)
		for (component = 0; component < 3; component++)
		  rowbuf[3 * i + component] = pixmap[j][i].u.rgb[component];
	      stream->write ((const char *)rowbuf, 3 * width);
	    }
	  free (rowbuf);
	}
    }
#endif
}

/* return best type for writing an image (0=mono, 1=grey, 2=color) */
static int
best_image_type (miPixel **pixmap, int width, int height)
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
