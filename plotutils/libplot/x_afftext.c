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

/* This is the XAffText module, which was originally independent of
   libplot.  The header file that accompanies it is x_afftext.h.

   To use the module independently of libplot, simply do not specify
   "-DLIBPLOT" at compile time.

   The module supplies two external functions, which are generalizations of
   the core X11 function XDrawString: XAffDrawRotString and
   XAffDrawAffString.  They draw, respectively, a rotated text string and
   (more generally) a matrix-transformed text string.  In both cases a
   specified core X font is used.  The rotation angle and transformation
   matrix are specified by the user.  The matrix is passed as a 4-element
   array, with the element ordering convention and sign convention being
   those of the Matrix Extension to the XLFD (X Logical Font Description).

   `XAffText' is an abbreviation of `X11 affinely transformed text'.  The
   module was inspired by Alan Richardson's xvertext module for displaying
   rotated text strings in X11, using the core X fonts.  It works in a
   similar way.  (It retrieves a bitmap from the X server into an XImage,
   transforms the XImage, monochrome pixel by pixel, and sends it back to a
   bitmap on the server, for use as a stipple.)  But it supports arbitrary
   transformation matrices, and pays extra attention to pixel-level
   accuracy.  It uses integer arithmetic when possible. */

#include "x_afftext.h"
#ifdef DEBUG
#ifdef __cplusplus
#include <cstdio>
#else  /* not __cplusplus */
#include <stdio.h>
#endif /* not __cplusplus */
#endif /* DEBUG */

#ifdef LIBPLOT
#include "sys-defines.h"	/* plotutils-specific */
#include "extern.h"		/* libplot-specific */
#else  /* not LIBPLOT */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846264
#endif
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))
#define IROUND(x) ((int) ((x) > 0 ? (x) + 0.5 : (x) - 0.5))
#define _pl_xmalloc malloc
#define _pl_xcalloc calloc
#endif /* not LIBPLOT */

#define XAFF_XPROD(v,a) ((v).x * (a)[0] + (v).y * (a)[2])
#define XAFF_YPROD(v,a) ((v).x * (a)[1] + (v).y * (a)[3])

typedef struct XAffVectorStruct
{
  int x, y;
} XAffVector;

typedef struct XAffRealVectorStruct
{
  double x, y;
} XAffRealVector;

typedef struct XAffSizeStruct
{
  unsigned int x, y;
} XAffSize;

typedef struct XAffAffinedTextStruct
{
  Pixmap bitmap;		/* depth-1 Pixmap, i.e., bitmap */
  XAffSize size;		/* bitmap size */
  XAffVector origin;		/* location of text origin within bitmap */
} XAffAffinedText;

/* forward decls of internal ctors/dtors */

static XImage * XAffCreateXImage (Display *dpy, XAffSize size);
static void XAffFreeXImage (XImage *im);

static XAffAffinedText *XAffCreateAffinedText (Display *dpy, XFontStruct *font, double a[4], const char *text);
static void XAffFreeAffinedText (Display *dpy, XAffAffinedText *afftext);

/* other internal functions */

static int can_use_XDrawString (XFontStruct *font, double a[4], const char *text);
#ifdef DEBUG
static void print_image (const XImage *im_in, XAffSize size);
#endif /* DEBUG */

/**************************************************************************/
/*  Create/destroy a depth-1 XImage object                                */
/**************************************************************************/

static XImage *
XAffCreateXImage (Display *dpy, XAffSize size)
{
  XImage *im;
  char *data;
  
  if (size.x == 0 || size.y == 0)
    return NULL;

  data = (char *)_pl_xcalloc(size.y * ((size.x + 7) / 8), 1);
  if (data == NULL)
    return NULL;
  
  im = XCreateImage (dpy, DefaultVisual(dpy, DefaultScreen(dpy)), 
		     (unsigned int)1, /* depth = 1 */
		     XYBitmap, 
		     0,		/* origin = 0 */
		     data, 
		     size.x, size.y, 
		     8,		/* pad: quantum of each scanline */
		     0);	/* scanlines contigous in memory */
  if (im == NULL)
    return NULL;
  
  im->bitmap_bit_order = MSBFirst;
  im->byte_order = MSBFirst;

  return im;
}

static void 
XAffFreeXImage (XImage *im)
{
  free (im->data);
  XFree (im);
}

/**************************************************************************/
/*  Create/destroy an affinely transformed text string                    */
/**************************************************************************/

static XAffAffinedText *
XAffCreateAffinedText (Display *dpy, XFontStruct *font, double a[4], const char *text)
{
  XAffAffinedText *afftext = NULL;
  GC gc;
  XCharStruct bounds;
  int direction, font_ascent, font_descent;
  XAffSize size_in, size_out;
  XAffRealVector corner_in[4];
  XAffVector origin_in, origin_out;
  XAffVector mincorner, maxcorner;
  Pixmap bitmap_in, bitmap_out;
  XImage *im_in, *im_out;
  int scanline_len_in, scanline_len_out;
  double aa[4], a_inverse[4], det;
  int i, j;

  /* allocate memory for new instance */
  afftext = (XAffAffinedText *)_pl_xmalloc(sizeof(XAffAffinedText));
  if (!afftext)
    return NULL;
	
  /* as passed, a[] is in the format used in the matrix LFD enhancement,
     which assumes a right-handed coordinate system; so convert it to X11's
     left-hand coordinate system (y grows downward) */
  aa[0] = a[0];
  aa[1] = -a[1];
  aa[2] = -a[2];
  aa[3] = a[3];

  /* invert transformation matrix */
  det = aa[0] * aa[3] - aa[1] * aa[2];
  if (det == 0.0)
    return NULL;		/* don't support singular matrices */
  a_inverse[0] =   aa[3] / det;
  a_inverse[1] = - aa[1] / det;
  a_inverse[2] = - aa[2] / det;
  a_inverse[3] =   aa[0] / det;

  /* to include all pixels in text, how large should bitmap be? */
  XTextExtents (font, text, strlen (text), 
		&direction, &font_ascent, &font_descent, &bounds);

  /* bitmap size, number-of-pixels by number-of-pixels */
  size_in.x = - bounds.lbearing + bounds.rbearing;
  size_in.y = bounds.ascent + bounds.descent;

  /* within this bitmap, where is `origin' of text string? */
  origin_in.x = - bounds.lbearing;
  origin_in.y = bounds.ascent;
  
  /* paranoia */
  if (size_in.x == 0 || size_in.y == 0)
    return NULL;

  /* work around a possible bug: some X displays can't create pixmaps that
     are only one pixel wide or high */
  if (size_in.x == 1)
    size_in.x = 2;
  if (size_in.y == 1)
    size_in.y = 2;

#ifdef DEBUG
  fprintf (stderr, "string \"%s\": lbearing=%hd, rbearing=%hd, ascent=%hd, descent=%hd\n", text, bounds.lbearing, bounds.rbearing, bounds.ascent, bounds.descent);
  fprintf (stderr, "\tsize=(%u,%u), origin=(%d,%d)\n", size_in.x, size_in.y, origin_in.x, origin_in.y);
#endif

  /* create bitmap for text, and lightweight gc */
  bitmap_in = XCreatePixmap (dpy, DefaultRootWindow (dpy), 
			     size_in.x, size_in.y, (unsigned int)1);
  gc = XCreateGC (dpy, bitmap_in, (unsigned long)0, (XGCValues *)NULL);
  XSetBackground (dpy, gc, (unsigned long)0);
  XSetFont (dpy, gc, font->fid);

  /* clear the bitmap */
  XSetForeground (dpy, gc, (unsigned long)0);
  XFillRectangle (dpy, bitmap_in, gc, 0, 0, size_in.x, size_in.y);
  XSetForeground (dpy, gc, (unsigned long)1);
    
  /* draw text onto bitmap */
  XDrawString (dpy, bitmap_in, gc, origin_in.x, origin_in.y, 
	       text, strlen (text));

  /* create local image */
  im_in = XAffCreateXImage (dpy, size_in);
  if (im_in == NULL)
    return NULL;

  /* copy bitmap to it */
  XGetSubImage (dpy, bitmap_in, 0, 0, size_in.x, size_in.y,
		(unsigned long)1, XYPixmap, im_in, 0, 0);
  im_in->format = XYBitmap;

#ifdef DEBUG
  print_image (im_in, size_in);
#endif /* DEBUG */

  /* free now-unneeded bitmap */
  XFreePixmap (dpy, bitmap_in);

  /* vertices of image, in real coordinates, if each pixel is taken to be a
     unit square */
  corner_in[0].x = -0.5;
  corner_in[0].y = -0.5;
  corner_in[1].x = (int)size_in.x - 0.5;
  corner_in[1].y = -0.5;
  corner_in[2].x = (int)size_in.x - 0.5;
  corner_in[2].y = (int)size_in.y - 0.5;
  corner_in[3].x = -0.5;
  corner_in[3].y = (int)size_in.y - 0.5;

  /* compute vertices (in integer coordinates) of a rectangular array of
     pixels that will snugly hold the affinely transformed image */

  mincorner.x = mincorner.y = INT_MAX;
  maxcorner.x = maxcorner.y = INT_MIN;
  for (i = 0; i < 4; i++)
    {
      XAffRealVector v_shifted_in;
      XAffVector corner_out[4];
      
      v_shifted_in.x = corner_in[i].x - origin_in.x;
      v_shifted_in.y = corner_in[i].y - origin_in.y;

      corner_out[i].x = IROUND(XAFF_XPROD(v_shifted_in, aa)) + origin_in.x;
      corner_out[i].y = IROUND(XAFF_YPROD(v_shifted_in, aa)) + origin_in.y;

      mincorner.x = IMIN(mincorner.x, corner_out[i].x);
      mincorner.y = IMIN(mincorner.y, corner_out[i].y);

      maxcorner.x = IMAX(maxcorner.x, corner_out[i].x);
      maxcorner.y = IMAX(maxcorner.y, corner_out[i].y);
    }
  size_out.x = maxcorner.x - mincorner.x + 1;
  size_out.y = maxcorner.y - mincorner.y + 1;
  
  origin_out.x = origin_in.x - mincorner.x;
  origin_out.y = origin_in.y - mincorner.y;

  /* work around a possible bug: some X displays can't create pixmaps that
     are only one pixel wide or high */
  if (size_out.x == 1)
    size_out.x = 2;
  if (size_out.y == 1)
    size_out.y = 2;

#ifdef DEBUG
  fprintf (stderr, "size_in = (%u,%u)\n", size_in.x, size_in.y);
  fprintf (stderr, "size_out = (%u,%u)\n", size_out.x, size_out.y);
  fprintf (stderr, "origin_in = (%d,%d)\n", origin_in.x, origin_in.y);
  fprintf (stderr, "origin_out = (%d,%d)\n", origin_out.x, origin_out.y);
#endif

  /* create 2nd image, to hold affinely transformed text */
  im_out = XAffCreateXImage (dpy, size_out);
  if (im_out == NULL)
    return NULL;
    
  /* copy from 1st image to this new one */

  scanline_len_in = (size_in.x + 7) / 8;
  scanline_len_out = (size_out.x + 7) / 8;

  for (j = 0; j < (int)size_out.y; j++)
    {
      int scanline_hit;
      XAffVector v_in, v_out, v_shifted_out;

      scanline_hit = 0;

      v_out.y = j;
      v_shifted_out.y = v_out.y + mincorner.y - origin_in.y;

      for (i = 0; i < (int)size_out.x; i++)
	{
	  v_out.x = i;
	  v_shifted_out.x = v_out.x + mincorner.x - origin_in.x;

	  v_in.x = IROUND(XAFF_XPROD(v_shifted_out, a_inverse)) + origin_in.x;
	  v_in.y = IROUND(XAFF_YPROD(v_shifted_out, a_inverse)) + origin_in.y;

	  if ((!(v_in.x >= 0)) || (!(v_in.x < (int)size_in.x)) ||
	      (!(v_in.y >= 0)) || (!(v_in.y < (int)size_in.y)))
	    {
	      if (scanline_hit)
		/* will be no more hits; so move to next scanline */
		break;
	      else	       /* move to next position on this scanline */
		continue;
	    }
	  else
	    scanline_hit = 1;
	  
	  if (im_in->data[v_in.y * scanline_len_in + v_in.x / 8] 
	      & (128 >> (v_in.x % 8)))
	    {
	      im_out->data[v_out.y * scanline_len_out + v_out.x / 8] 
		|= (128 >> (v_out.x % 8));
	    }
	}
    }

  /* free now-unneeded 1st image */
  XAffFreeXImage (im_in);

  /* create bitmap to hold transformed text */
  bitmap_out = XCreatePixmap (dpy, DefaultRootWindow (dpy),
			      size_out.x, size_out.y, (unsigned int)1);
  
  /* copy transformed text from 2nd image */
  XPutImage (dpy, bitmap_out, gc, im_out, 
	     0, 0, 0, 0, size_out.x, size_out.y);

#ifdef DEBUG
  print_image (im_out, size_out);
#endif

  /* free 2nd image and GC */
  XAffFreeXImage (im_out);
  XFreeGC (dpy, gc);

  /* fill in data members of instance */
  afftext->bitmap = bitmap_out;
  afftext->size = size_out;
  afftext->origin = origin_out;

  return afftext;
}

static void 
XAffFreeAffinedText (Display *dpy, XAffAffinedText *afftext)
{
  XFreePixmap (dpy, afftext->bitmap);
  free (afftext);
}

/**************************************************************************/
/*  Draw an affinely transformed text string                              */
/**************************************************************************/

int 
XAffDrawAffString (Display *dpy, Drawable drawable, GC gc, XFontStruct *font, int x, int y, double a[4], const char *text)
{
  XAffAffinedText *afftext;
  GC our_gc;
    
  if (text == NULL || strlen (text) == 0)
    return 0;
    
  if (can_use_XDrawString (font, a, text))
    /* a[] must be equal to, or near the identity matrix */
    return XDrawString (dpy, drawable, gc, x, y, text, strlen (text));

  /* construct annotated bitmap, containing affinely transformed text */
  afftext = XAffCreateAffinedText (dpy, font, a, text);
  if (afftext == NULL)
    return 0;
    
  /* copy gc from user's gc */
  our_gc = XCreateGC (dpy, drawable, (unsigned long)0, (XGCValues *)NULL);
  XCopyGC (dpy, gc, GCForeground|GCFunction|GCPlaneMask, our_gc);

  /* use stipple drawing technique (screen-door patterning) */
  XSetFillStyle (dpy, our_gc, FillStippled);
  XSetStipple (dpy, our_gc, afftext->bitmap);
  XSetTSOrigin (dpy, our_gc, 
		x - afftext->origin.x, y - afftext->origin.y);
  XFillRectangle (dpy, drawable, our_gc, 
		  x - afftext->origin.x, y - afftext->origin.y, 
		  afftext->size.x, afftext->size.y);
    
  /* free resources */
  XFreeGC (dpy, our_gc);
  XAffFreeAffinedText (dpy, afftext);

  return 0;
}

/**************************************************************************/
/*  Special case: draw a rotated text string                              */
/**************************************************************************/

int 
XAffDrawRotString (Display *dpy, Drawable drawable, GC gc, XFontStruct *font, int x, int y, double angle, const char *text)
{
  double a[4];
  
  /* convert rotation angle to radians */
  angle *= (M_PI / 180.0);

  /* construct transformation matrix (using the XLFD-matrix-extension sign
     convention for the off-diagonal elements) */
  a[0] = + cos (angle);
  a[1] = + sin (angle);
  a[2] = - sin (angle);
  a[3] = + cos (angle);

  return XAffDrawAffString (dpy, drawable, gc, font, x, y, a, text);
}

/**************************************************************************/
/* Can simply use core XDrawString function rather than transforming the  */
/* resulting bitmap?  (Yes, if the matrix a[] is near the identity.)      */
/**************************************************************************/

static int
can_use_XDrawString (XFontStruct *font, double a[4], const char *text)
{
  int direction, font_ascent, font_descent;
  XCharStruct bounds;
  XAffVector corner_in[4], corner_out[4];
  XAffSize size_in;
  XAffVector origin_in;
  int i, can_do_it = 1;
  double aa[4];

  /* as passed, a[] is in the format used in the matrix LFD enhancement,
     which assumes a right-handed coordinate system; so convert it to X11's
     left-hand coordinate system (y grows downward) */
  aa[0] = a[0];
  aa[1] = -a[1];
  aa[2] = -a[2];
  aa[3] = a[3];

  /* to include all pixels in text, how large should bitmap be? */
  XTextExtents (font, text, strlen (text), 
		&direction, &font_ascent, &font_descent, &bounds);

  /* bitmap size, number-of-pixels by number-of-pixels */
  size_in.x = - bounds.lbearing + bounds.rbearing;
  size_in.y = bounds.ascent + bounds.descent;

  /* within this bitmap, where is `origin' of text string? */
  origin_in.x = - bounds.lbearing;
  origin_in.y = bounds.ascent;
  
  /* corners in integer coordinates, relative to origin */
  corner_in[0].x = 0;
  corner_in[0].y = 0;
  corner_in[1].x = size_in.x - 1;
  corner_in[1].y = 0;
  corner_in[2].x = size_in.x - 1;
  corner_in[2].y = size_in.y - 1;
  corner_in[3].x = 0;
  corner_in[3].y = size_in.y - 1;

  /* compute how corners are transformed by a[] */
  for (i = 0; i < 4; i++)
    {
      XAffVector v_shifted_in;
      
      v_shifted_in.x = corner_in[i].x - origin_in.x;
      v_shifted_in.y = corner_in[i].y - origin_in.y;

      corner_out[i].x = IROUND(XAFF_XPROD(v_shifted_in, aa)) + origin_in.x;
      corner_out[i].y = IROUND(XAFF_YPROD(v_shifted_in, aa)) + origin_in.y;

      if (corner_out[i].x != corner_in[i].x
	  || corner_out[i].y != corner_in[i].y)
	/* at least one corner moves, no good, alas */
	{
	  can_do_it = 0;
	  break;
	}
    }

  return can_do_it;
}

/**************************************************************************/
/*  Print an image to stderr (used for debugging, if -DDEBUG is specified)*/
/**************************************************************************/

#ifdef DEBUG
static void 
print_image (const XImage *im, XAffSize size)
{
  int scanline_len;
  int i, j;
  
  scanline_len = (size.x + 7) / 8;

  for (j = 0; j < (int)size.y; j++)
    {
      for (i = 0; i < (int)size.x; i++)
	{
	  if (im->data[j * scanline_len + i / 8] & (128 >> (i % 8)))
	    fprintf (stderr, "*");
	  else
	    fprintf (stderr, " ");
	}
      fprintf (stderr, "\n");
    }
}
#endif /* DEBUG */

