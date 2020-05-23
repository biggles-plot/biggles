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

/* This file contains the XDrawablePlotter (and XPlotter) version of the
   low-level paint_text_string() method, which is called to plot a label in
   the current (non-Hershey) font, at the current fontsize and textangle.
   The label is just a string: no control codes (font switching or
   sub/superscripts).  The width of the string in user units is returned.

   This version not does support center-justification and right
   justification; only the default left-justification.  That is all
   right, since justification is handled at a higher level. */

/* This file also contains the XDrawablePlotter (and XPlotter) version of
   the flabelwidth_other() method, which is called to compute the width, in
   user coordinates, of a label. */

#include "sys-defines.h"
#include "extern.h"

/* When this is called in g_alabel.c, the X font has already been
   retrieved, in whole or in part (by calling "_set_font()", which in turn
   calls "_plotter->retrieve_font()", which is bound to the
   _pl_x_retrieve_font() routine in x_retrieve.c).  I.e., whatever portion of
   the X font was required to be retrieved in order to return font metrics,
   has previously been retrieved.

   To retrieve a larger part, we call _pl_x_retrieve_font() again.  But this
   time, we pass the label to be rendered to _pl_x_retrieve_font() as a
   "hint", i.e., as the x_label data member of (the driver-specific part
   of) the drawing state.  That tells _pl_x_retrieve_font how much more of the
   font to retrieve.  This scheme is a hack, but it works (and doesn't
   violate layering).  */

#include "x_afftext.h"

double
_pl_x_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  const char *saved_font_name;
  char *temp_font_name;
  bool ok;
  double x, y;
  double width = 0.0;		/* width of string in user units */
  double rot[4];		/* user-frame rotation matrix */
  double a[4];		   /* transformation matrix for XAffDrawString() */
  int i, ix, iy;
  
  /* sanity check; this routine supports only baseline positioning */
  if (v_just != PL_JUST_BASE)
    return 0.0;

  /* similarly for horizontal justification */
  if (h_just != PL_JUST_LEFT)
    return 0.0;

  if (*s == (unsigned char)'\0')
    return 0.0;

  /* Do retrieval, fill in the X-specific field x_font_struct of the
     drawing state.  (We've previously retrieved a small subset of the
     font, to obtain metrics used for text positioning, as mentioned above;
     so retrieving a larger portion should go smoothly.)   

     We retrieve not `font_name' but rather `true_font_name', because the
     latter may have been what was retrieved, if a default X font had to be
     substituted; see g_retrieve.c. */

  if (_plotter->drawstate->true_font_name == NULL) /* shouldn't happen */
    return 0.0;

  saved_font_name = _plotter->drawstate->font_name;
  temp_font_name = 
    (char *)_pl_xmalloc (strlen (_plotter->drawstate->true_font_name) + 1);
  strcpy (temp_font_name, _plotter->drawstate->true_font_name);
  _plotter->drawstate->font_name = temp_font_name;

  _plotter->drawstate->x_label = s; /* pass label hint */
  ok = _pl_x_retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore label hint to default */

  _plotter->drawstate->font_name = saved_font_name;
  free (temp_font_name);

  if (!ok)			/* shouldn't happen */
    return 0.0;

  /* set font in GC used for drawing (the other GC, used for filling, is
     left alone) */
  XSetFont (_plotter->x_dpy, _plotter->drawstate->x_gc_fg,
	    _plotter->drawstate->x_font_struct->fid);

  /* select our pen color as foreground color in X GC used for drawing */
  _pl_x_set_pen_color (S___(_plotter));
  
  /* compute position in device coordinates */
  x = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  y = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
  
  /* X11 protocol OOB check */
  ix = IROUND(x);
  iy = IROUND(y);
  if (X_OOB_INT(ix) || X_OOB_INT(iy))
    {
      _plotter->warning (R___(_plotter) 
			 "not drawing a text string that is positioned too far for X11");
      return 0.0;
    }
    
  /* Draw the text string by calling XAffDrawString() in x_afftext.c, which
     operates by affinely transform a bitmap generated by XDrawString() in
     the following way: it pulls it back from the server as an image,
     transforms the image, and then sends the image back to the server. */
  
  /* First, compute a 2x2 matrix a[] that would, in the jargon of the
     matrix extension to the XLFD (X Logical Font Description) scheme, be
     called a `pixel matrix'.  It specifies how XAffDrawAffString should
     `anamorphically transform' the text bitmap produced by XDrawString(),
     to yield the bitmap we want.  It's essentially the product of (i) the
     user-frame text rotation matrix, and (ii) the user_space->device_space
     transformation matrix.  But see additional comments below. */

  /* user-frame rotation matrix */
  rot[0] = cos (M_PI * _plotter->drawstate->text_rotation / 180.0);
  rot[1] = sin (M_PI * _plotter->drawstate->text_rotation / 180.0);
  rot[2] = - sin (M_PI * _plotter->drawstate->text_rotation / 180.0);
  rot[3] = cos (M_PI * _plotter->drawstate->text_rotation / 180.0);
  
  /* Compute matrix product.  But note flipped-y convention affecting a[1]
     and a[3].  Sign flipping is because the pixel matrix (as used in the
     XLFD matrix extension and hence, for consistency, by our code by
     XAffDrawAffString()) is expressed with respect to a right-handed
     coordinate system, in which y grows upward, rather than X11's default
     left-handed coordinate system, in which y grows downward. */

  a[0] =  (rot[0] * _plotter->drawstate->transform.m[0] 
	   + rot[1] * _plotter->drawstate->transform.m[2]);
  a[1] =  - (rot[0] * _plotter->drawstate->transform.m[1] 
	     + rot[1] * _plotter->drawstate->transform.m[3]);
  a[2] =  (rot[2] * _plotter->drawstate->transform.m[0] 
	   + rot[3] * _plotter->drawstate->transform.m[2]);
  a[3] =  - (rot[2] * _plotter->drawstate->transform.m[1] 
	     + rot[3] * _plotter->drawstate->transform.m[3]);
  
  /* Apply an overall scaling.  We want the text string to appear at a
     certain font size in the user frame; and the font that XDrawString
     will use was retrieved at a certain pixel size in the device frame.
     So we compensate on both sides, so to speak.  We multiply by
     true_font_size / x_font_pixel_size, where the numerator refers to the
     user frame, and the denominator to the device frame. */

  for (i = 0; i < 4; i++)
    a[i] = a[i] 
      * (_plotter->drawstate->true_font_size / _plotter->drawstate->x_font_pixel_size);
    
  if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
    /* double buffering, have a `x_drawable3' to draw into */
    XAffDrawAffString (_plotter->x_dpy, _plotter->x_drawable3, 
		       _plotter->drawstate->x_gc_fg, 
		       _plotter->drawstate->x_font_struct,
		       ix, iy, a, (char *)s);
  else
    {
      /* not double buffering, have no `x_drawable3' */
      if (_plotter->x_drawable1)
	XAffDrawAffString (_plotter->x_dpy, _plotter->x_drawable1, 
			   _plotter->drawstate->x_gc_fg, 
			   _plotter->drawstate->x_font_struct,
			   ix, iy, a, (char *)s);
      if (_plotter->x_drawable2)
	XAffDrawAffString (_plotter->x_dpy, _plotter->x_drawable2, 
			   _plotter->drawstate->x_gc_fg, 
			   _plotter->drawstate->x_font_struct,
			   ix, iy, a, (char *)s);
    }
    
  /* compute width of just-drawn string in user units */
  width = (((XTextWidth (_plotter->drawstate->x_font_struct, 
			 (char *)s, 
			 (int)(strlen((char *)s)))
	     *_plotter->drawstate->true_font_size))
	   / _plotter->drawstate->x_font_pixel_size);
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return width;
}

/* Compute width, in user coordinates, of label in the currently selected
   font (no escape sequences!).  Current font is assumed to be a
   non-Hershey font (so we have an X font structure for it).  This is
   installed as an internal class method, invoked if the current font is
   non-Hershey (which means Postscript, PCL, or `other' [i.e. any non-PS,
   non-PCL, retrievable X font].  */

/* When this is called in g_alabel.c, the X font has already been
   retrieved, in whole or in part (by calling "_set_font()", which in turn
   calls "_plotter->retrieve_font()", i.e., which calls the
   _pl_x_retrieve_font() routine in x_retrieve.c).  I.e., whatever portion of
   the X font was required to be retrieved in order to return font metrics,
   has previously been retrieved.

   To retrieve a larger part, we call _pl_x_retrieve_font() again.  But this
   time, we pass the label to be rendered to _pl_x_retrieve_font() as a
   "hint", i.e., as a data member of (the driver-specific part of) the
   drawing state.  That tells _pl_x_retrieve_font how much more of the font to
   retrieve.  This scheme is an ugly hack, but it works (and doesn't
   violate layering). */

double
_pl_x_get_text_width (R___(Plotter *_plotter) const unsigned char *s)
{
  const char *saved_font_name;
  char *temp_font_name;
  bool ok;
  double width;

  /* Do retrieval, but use current `true_font_name' as our font name (see
     above; we've previously retrieved a subset of it). */

  if (_plotter->drawstate->true_font_name == NULL) /* shouldn't happen */
    return 0.0;

  saved_font_name = _plotter->drawstate->font_name;
  temp_font_name = 
    (char *)_pl_xmalloc (strlen (_plotter->drawstate->true_font_name) + 1);
  strcpy (temp_font_name, _plotter->drawstate->true_font_name);
  _plotter->drawstate->font_name = temp_font_name;

  _plotter->drawstate->x_label = s; /* pass label hint */
  ok = _pl_x_retrieve_font (S___(_plotter));
  _plotter->drawstate->x_label = NULL; /* restore label hint to default */

  _plotter->drawstate->font_name = saved_font_name;
  free (temp_font_name);

  if (!ok)			/* shouldn't happen */
    return 0.0;

  /* compute width of string in user units; see above comments on
     `compensating on both sides' */
  width = ((XTextWidth (_plotter->drawstate->x_font_struct, 
			(char *)s, 
			(int)(strlen((char *)s)))
	    *_plotter->drawstate->true_font_size)
	   / _plotter->drawstate->x_font_pixel_size);
  
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));

  return width;
}
