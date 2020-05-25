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

/* TekPlotter objects display a point as a zero-length line segment. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_t_paint_point (S___(Plotter *_plotter))
{
  double xx, yy;
  int ixx, iyy;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* convert point to floating-point device coordinates */
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      
      /* do nothing if point is outside device clipping rectangle */
      if ((xx < TEK_DEVICE_X_MIN_CLIP)
	  || (xx > TEK_DEVICE_X_MAX_CLIP)
	  || (yy < TEK_DEVICE_Y_MIN_CLIP)
	  || (yy > TEK_DEVICE_Y_MAX_CLIP))
	return;
      
      /* round to integer device (Tektronix) coordinates */
      ixx = IROUND(xx);
      iyy = IROUND(yy);
      
      /* emit an escape sequence if necessary, to switch to POINT mode */
      _pl_t_tek_mode (R___(_plotter) TEK_MODE_POINT);
      
      /* sync Tek's color too (significant only for kermit Tek emulator) */
      _pl_t_set_pen_color (S___(_plotter));

      /* Output the point.  If in fact we were already in POINT mode, this
	 is slightly suboptimal because we can't call
	 _pl_t_tek_vector_compressed() to save (potentially) a few bytes,
	 because we don't know what the last-plotted point was.  Unlike
	 when incrementally drawing a polyline, when plotting points we
	 don't keep track of "where we last were". */
      _pl_t_tek_vector (R___(_plotter) ixx, iyy);
      
      /* update our notion of Tek's notion of position */
      _plotter->tek_pos.x = ixx;
      _plotter->tek_pos.y = iyy;
    }
}
