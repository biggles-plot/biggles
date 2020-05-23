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

/* This version is for XDrawablePlotters and XPlotters.  It calls
   _maybe_handle_x_events(), which is a no-op for the former but not the
   latter (it flushes the X output buffer and may also check for events).
   Since point() is used mostly by people drawing images, it may be invoked
   a great many times.  To speed things up, the call to
   _maybe_handle_x_events() is performed only once per X_POINT_FLUSH_PERIOD
   invocations of this function. */

#define X_POINT_FLUSH_PERIOD 8

#include "sys-defines.h"
#include "extern.h"

void
_pl_x_paint_point (S___(Plotter *_plotter))
{
  double xx, yy;
  int ix, iy;
  plColor oldcolor, newcolor;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* set pen color as foreground color in GC used for drawing (but
	 first, check whether we can avoid a function call) */
      newcolor = _plotter->drawstate->fgcolor;
      oldcolor = _plotter->drawstate->x_current_fgcolor; /* as stored in gc */
      if (newcolor.red != oldcolor.red 
	  || newcolor.green != oldcolor.green 
	  || newcolor.blue != oldcolor.blue
	  || ! _plotter->drawstate->x_gc_fgcolor_status)
	_pl_x_set_pen_color (S___(_plotter));
      
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      ix = IROUND(xx);
      iy = IROUND(yy);

      if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	/* double buffering, have a `x_drawable3' to draw into */
	XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
		    _plotter->drawstate->x_gc_fg, 
		    ix, iy);
      else
	/* not double buffering, have no `x_drawable3' */
	{
	  if (_plotter->x_drawable1)
	    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1, 
			_plotter->drawstate->x_gc_fg, 
			ix, iy);
	  if (_plotter->x_drawable2)
	    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2, 
			_plotter->drawstate->x_gc_fg, 
			ix, iy);
	}
    }
        
  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  if (_plotter->x_paint_pixel_count % X_POINT_FLUSH_PERIOD == 0)
    _maybe_handle_x_events (S___(_plotter));
  _plotter->x_paint_pixel_count++;
}
