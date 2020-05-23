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

/* This file contains the marker method, which is a GNU extension to
   libplot.  It plots an object: a marker, of specified size, at a
   specified location.  This marker may be one of a list of standard
   symbols, or a single character in the current font.

   The `size' argument is expressed in terms of user coordinates.  If the
   marker is a character, it is the font size (i.e., the em size of the
   font).  If the marker is a symbol, the maximum dimension of the symbol
   will be a fixed fraction of `size'. */

/* This is a generic version.  Currently, it is overridden only by Metafile
   and CGM Plotters. */

#include "sys-defines.h"
#include "extern.h"

/* The maximum dimension of most markers, e.g. the diameter of the circle
   marker (marker #4).  Expressed as a fraction of the `size' argument. */
#define MAXIMUM_MARKER_DIMENSION (5.0/8.0)

/* Line width used while drawing marker symbols.  Expressed as a fraction
   of the `size' argument. */
#define LINE_SCALE (0.05 * MAXIMUM_MARKER_DIMENSION)

/* The diameter of a `dot' (marker #1), as a fraction of the maximum marker
   dimension (see above).  This applies only if the Plotter has a physical
   display, i.e. the PAGESIZE parameter is meaningful for it, or if the
   Plotter has a virtual display that uses real coordinates.  Currently,
   none of our Plotters falls into the 2nd category.

   If the Plotter falls into neither of these two categories, we assume
   that fpoint() draws a single pixel, or something like it, and we use
   fpoint() to draw marker #1.  E.g., we do so on Tektronix, GIF, PNM, X,
   and X Drawable displays.  For them, RELATIVE_DOT_SIZE is ignored. */

#define RELATIVE_DOT_SIZE 0.15

/* Argument for filltype(), when we draw the half-filled marker symbols.
   (This isn't exactly half, but it looks better than half.) */
#define NOMINAL_HALF 0xa000

int
_API_fmarker (R___(Plotter *_plotter) double x, double y, int type, double size)
{
  bool drawn;
  char label_buf[2];
  double x_dev, y_dev, delta_x_dev, delta_y_dev;
  double delta_x_user = 0.0, delta_y_user = 0.0;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmarker: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  if (_plotter->drawstate->pen_type == 0)
    /* no pen to draw with, so do nothing */
    return 0;

  /* attempt to draw marker in a Plotter-specific way */
  drawn = _plotter->paint_marker (R___(_plotter) type, size);
  if (drawn)
    return 0;

  /* Plotter couldn't do it, so draw the marker in a generic way, by
     constructing it from other libplot primitives. */

  if (type < 0)			/* silently return if marker type < 0 */
    return 0;
  type %= 256;			/* compute marker type mod 256 */

  /* begin by saving drawing attributes we may change */
  _API_savestate (S___(_plotter));

  if (_plotter->data->display_coors_type != (int)DISP_DEVICE_COORS_REAL)
    /* move temporarily to nearest point with integer device coordinates,
       so that marker will be symmetrically positioned */
    {
      x_dev = XD(x, y);
      y_dev = YD(x, y);
      delta_x_dev = IROUND(x_dev) - x_dev;
      delta_y_dev = IROUND(y_dev) - y_dev;
      delta_x_user = XUV(delta_x_dev, delta_y_dev);
      delta_y_user = YUV(delta_x_dev, delta_y_dev);
      
      _plotter->drawstate->pos.x += delta_x_user;
      _plotter->drawstate->pos.y += delta_y_user;
    }
  
  if (type > 31)
    /* plot a single character, in current font */
    {
      _API_pentype (R___(_plotter) 1);
      _API_ffontsize (R___(_plotter) size);
      _API_textangle (R___(_plotter) 0);
      label_buf[0] = (char)type;
      label_buf[1] = '\0';
      _API_alabel (R___(_plotter) 'c', 'c', label_buf);
    }
  else
    /* plot a handcrafted marker symbol */
    {
      _API_pentype (R___(_plotter) 1);
      _API_linemod (R___(_plotter) "solid");
      _API_capmod (R___(_plotter) "butt");
      _API_joinmod (R___(_plotter) "miter");
      _API_flinewidth (R___(_plotter) LINE_SCALE * size);
      _API_fillcolor (R___(_plotter)
		      _plotter->drawstate->fgcolor.red,
		      _plotter->drawstate->fgcolor.green,
		      _plotter->drawstate->fgcolor.blue);

      /* beautification kludge: if display is a bitmap device using libxmi
	 or a compatible scan-conversion scheme, and line thickness is one
	 pixel or less, but not zero, then change it to zero thickess
	 (which will be interpreted as specifying a Bresenham line). */
      if ((_plotter->data->display_coors_type == 
	   (int)DISP_DEVICE_COORS_INTEGER_LIBXMI)
	  && _plotter->drawstate->quantized_device_line_width == 1)
	_API_flinewidth (R___(_plotter) 0.0);	

      size *= (0.5 * MAXIMUM_MARKER_DIMENSION);

      switch (type)
	/* N.B. 5-pointed star should be added some day */
	{
	case (int)M_NONE:		/* no-op */
	default:
	  break;
	case (int)M_DOT:		/* dot, GKS 1 */
	  if (_plotter->data->display_model_type == (int)DISP_MODEL_PHYSICAL
	      || 
	      _plotter->data->display_coors_type ==(int)DISP_DEVICE_COORS_REAL)
	    {
	      _API_filltype (R___(_plotter) 1);
	      _API_fcirclerel (R___(_plotter) 0.0, 0.0, 
			       RELATIVE_DOT_SIZE * size);
	    }
	  else			/* see comment above */
	    _API_fpointrel (R___(_plotter) 0.0, 0.0);
	  break;
	case (int)M_PLUS:	/* plus, GKS 2 */
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) 2 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) 0.0, 2 * size);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  break;
	case (int)M_ASTERISK:	/* asterisk, GKS 3 */
	  {
	    double vert = 0.5 * size;
	    double hori = 0.5 * M_SQRT3 * size;
	    
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	    _API_fcontrel (R___(_plotter) 0.0, 2 * size);
	    _API_fmoverel (R___(_plotter) 0.0, -size);

	    _API_fcontrel (R___(_plotter) hori, vert);
	    _API_fmoverel (R___(_plotter) -hori, -vert);
	    _API_fcontrel (R___(_plotter) -hori, -vert);
	    _API_fmoverel (R___(_plotter) hori, vert);
	    _API_fcontrel (R___(_plotter) hori, -vert);
	    _API_fmoverel (R___(_plotter) -hori, vert);
	    _API_fcontrel (R___(_plotter) -hori, vert);
	    _API_fmoverel (R___(_plotter) hori, -vert);
	  }
	  break;
	case (int)M_CIRCLE:	/* circle, GKS 4 */
	  _API_filltype (R___(_plotter) 0);
	  _API_fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_CROSS:	/* cross, GKS 5 */
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) 2 * size, 2 * size);
	  _API_fmoverel (R___(_plotter) 0.0, - 2 * size);
	  _API_fcontrel (R___(_plotter) -2 * size, 2 * size);
	  _API_fmoverel (R___(_plotter) size, -size);
	  break;
	case (int)M_STAR:	/* star */
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) 2 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) 0.0, 2 * size);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) size, size);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) size, -size);
	  _API_fmoverel (R___(_plotter) -size, size);
	  _API_fcontrel (R___(_plotter) -size, size);
	  _API_fmoverel (R___(_plotter) size, -size);
	  _API_fcontrel (R___(_plotter) -size, -size);
	  _API_fmoverel (R___(_plotter) size, size);
	  break;
	case (int)M_SQUARE:	/* square */
	  _API_filltype (R___(_plotter) 0);
	  _API_fboxrel (R___(_plotter) -size, -size, size, size);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_DIAMOND:	/* diamond */
	  _API_filltype (R___(_plotter) 0);
	  _API_fmoverel (R___(_plotter) size, 0.0);
	  _API_fcontrel (R___(_plotter) -size, size);
	  _API_fcontrel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) size, -size);
	  _API_fcontrel (R___(_plotter) size, size);
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  break;
	case (int)M_TRIANGLE:	/* triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) 0);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_INVERTED_TRIANGLE: /* triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) 0);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_FILLED_SQUARE: /* filled square */
	  _API_filltype (R___(_plotter) 1);
	  _API_fboxrel (R___(_plotter) -size, -size, size, size);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_FILLED_DIAMOND: /* filled diamond */
	  _API_filltype (R___(_plotter) 1);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) size, size);
	  _API_fcontrel (R___(_plotter) -size, size);
	  _API_fcontrel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) size, -size);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_FILLED_TRIANGLE: /* filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) 1);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_FILLED_INVERTED_TRIANGLE: /* filled triangle, vertex down*/
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) 1);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_FILLED_CIRCLE: /* filled circle */
	  _API_filltype (R___(_plotter) 1);
	  _API_fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_STARBURST:	/* starburst */
	  _API_fmoverel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _API_fmoverel (R___(_plotter) size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.0, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, -0.5 * size);
	  break;
	case (int)M_FANCY_PLUS:	/* ornate plus */
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) 2 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) 0.0, 2 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) -size, 0.0);
	  _API_fmoverel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) size, 0.0);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, size);
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  break;
	case (int)M_FANCY_CROSS: /* ornate cross */
	  _API_fmoverel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) 2 * size, 2 * size);
	  _API_fmoverel (R___(_plotter) 0.0, -2 * size);
	  _API_fcontrel (R___(_plotter) -2 * size, 2 * size);
	  _API_fmoverel (R___(_plotter) 2 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  break;
	case (int)M_FANCY_SQUARE: /* ornate square */
	  _API_filltype (R___(_plotter) 0);
	  _API_fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_FANCY_DIAMOND: /* diamond */
	  _API_filltype (R___(_plotter) 0);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_endpath (S___(_plotter));
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -0.5 * size, -size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_FILLED_FANCY_SQUARE: /* filled ornate square */
	  _API_filltype (R___(_plotter) 1);
	  _API_fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_FILLED_FANCY_DIAMOND: /* filled ornate diamond */
	  _API_filltype (R___(_plotter) 1);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_endpath (S___(_plotter));
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -0.5 * size, -size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_HALF_FILLED_SQUARE:	/* half_filled square */
	  _API_filltype (R___(_plotter) NOMINAL_HALF);
	  _API_fboxrel (R___(_plotter) -size, -size, size, size);
	  _API_fmoverel (R___(_plotter) -size, -size);
	  break;
	case (int)M_HALF_FILLED_DIAMOND: /* half_filled diamond */
	  _API_filltype (R___(_plotter) NOMINAL_HALF);
	  _API_fmoverel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) size, size);
	  _API_fcontrel (R___(_plotter) -size, size);
	  _API_fcontrel (R___(_plotter) -size, -size);
	  _API_fcontrel (R___(_plotter) size, -size);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_HALF_FILLED_TRIANGLE: /* half_filled triangle */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) NOMINAL_HALF);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	  }	  
	  break;
	case (int)M_HALF_FILLED_INVERTED_TRIANGLE: /* half_filled triangle, vertex down */
	  {
	    double halfwidth = 0.5 * M_SQRT3 * size;
	    
	    _API_filltype (R___(_plotter) NOMINAL_HALF);
	    _API_fmoverel (R___(_plotter) 0.0, -size);
	    _API_fcontrel (R___(_plotter) halfwidth, 1.5 * size);
	    _API_fcontrel (R___(_plotter) -2 * halfwidth, 0.0);
	    _API_fcontrel (R___(_plotter) halfwidth, -1.5 * size);
	    _API_fmoverel (R___(_plotter) 0.0, size);
	  }	  
	  break;
	case (int)M_HALF_FILLED_CIRCLE:	/* half_filled circle */
	  _API_filltype (R___(_plotter) NOMINAL_HALF);
	  _API_fcirclerel (R___(_plotter) 0.0, 0.0, size);
	  break;
	case (int)M_HALF_FILLED_FANCY_SQUARE:  /* half-filled ornate square */
	  _API_filltype (R___(_plotter) NOMINAL_HALF);
	  _API_fboxrel (R___(_plotter) -0.5 * size, -0.5 * size, 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, -1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) -1.5 * size, 1.5 * size);	  
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fmoverel (R___(_plotter) 1.5 * size, -1.5 * size);	  
	  break;
	case (int)M_HALF_FILLED_FANCY_DIAMOND: /* half-filled ornate diamond */
	  _API_filltype (R___(_plotter) NOMINAL_HALF);
	  _API_fmoverel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_endpath (S___(_plotter));
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, 0.5 * size);
	  _API_fmoverel (R___(_plotter) -0.5 * size, -size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.0);
	  _API_fmoverel (R___(_plotter) size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -0.5 * size);
	  _API_fmoverel (R___(_plotter) 0.0, size);
	  break;
	case (int)M_OCTAGON:	/* octagon */
	  _API_filltype (R___(_plotter) 0);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) size, -0.5 * size);
	  break;
	case (int)M_FILLED_OCTAGON: /* filled octagon */
	  _API_filltype (R___(_plotter) 1);
	  _API_fmoverel (R___(_plotter) -size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, -size);
	  _API_fcontrel (R___(_plotter) 0.5 * size, -0.5 * size);
	  _API_fcontrel (R___(_plotter) size, 0.0);
	  _API_fcontrel (R___(_plotter) 0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) 0.0, size);
	  _API_fcontrel (R___(_plotter) -0.5 * size, 0.5 * size);
	  _API_fcontrel (R___(_plotter) -size, 0.0);
	  _API_fcontrel (R___(_plotter) -0.5 * size, -0.5 * size);
	  _API_fmoverel (R___(_plotter) size, -0.5 * size);
	  break;
	}
    }
  
  if (_plotter->data->display_coors_type != (int)DISP_DEVICE_COORS_REAL)
    /* undo the small repositioning (see above) */
    {
      _plotter->drawstate->pos.x -= delta_x_user;
      _plotter->drawstate->pos.y -= delta_y_user;
    }

  /* restore the original values of all drawing attributes */
  _API_restorestate (S___(_plotter));

  return 0;
}

/* The paint_marker method, which is an internal function that is called
   when the marker() method is invoked.  It plots an object: a marker of a
   specified type, at a specified size, at the current location.

   If this returns `false', marker() will construct the marker from other
   libplot primitives, in a generic way. */

/* Nearly all Plotters use this version, which does nothing but returns
   `false'. */

bool
_pl_g_paint_marker (R___(Plotter *_plotter) int type, double size)
{
  return false;
}
