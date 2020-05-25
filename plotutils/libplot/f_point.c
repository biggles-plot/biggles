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

/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

/* So far as a FigPlotter objects goes, a point is a single-vertex polyline
   (that is the only sort of point that xfig supports). */

#include "sys-defines.h"
#include "extern.h"

/* xfig polyline subtypes */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

void
_pl_f_paint_point (S___(Plotter *_plotter))
{
  double x, y;
  
  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* evaluate fig colors lazily, i.e. only when needed */
      _pl_f_set_pen_color (S___(_plotter));
      _pl_f_set_fill_color (S___(_plotter));
      
      /* update xfig's `depth' attribute */
      if (_plotter->fig_drawing_depth > 0)
	(_plotter->fig_drawing_depth)--;
      
      /* get location */
      x = _plotter->drawstate->pos.x;
      y = _plotter->drawstate->pos.x;

      sprintf(_plotter->data->page->point,
	      "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n\t%d %d\n",
	      2,		/* polyline object */
	      P_OPEN,		/* polyline subtype */
	      FIG_L_SOLID,	/* style */
	      1,		/* thickness, in Fig display units */
	      _plotter->drawstate->fig_fgcolor, /* pen color */
	      _plotter->drawstate->fig_fgcolor, /* fill color */
	      _plotter->fig_drawing_depth, /* depth */
	      0,		/* pen style, ignored */
	      20,		/* fig fill level (20 = full intensity) */
	      0.0,		/* style val, ignored (?) */
	      FIG_JOIN_ROUND,	/* join style = round */
	      FIG_CAP_ROUND,	/* cap style = round */
	      0,		/* radius (of arc boxes, ignored) */
	      0,		/* forward arrow */
	      0,		/* backward arrow */
	      1,		/* number of points in polyline */
	      IROUND(XD(x,y)), 
	      IROUND(YD(x,y))
	      );
      
      _update_buffer (_plotter->data->page);
    }
}
