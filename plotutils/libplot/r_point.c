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

#include "sys-defines.h"
#include "extern.h"

void
_pl_r_paint_point (S___(Plotter *_plotter))
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
      if ((xx < REGIS_DEVICE_X_MIN_CLIP)
	  || (xx > REGIS_DEVICE_X_MAX_CLIP)
	  || (yy < REGIS_DEVICE_Y_MIN_CLIP)
	  || (yy > REGIS_DEVICE_Y_MAX_CLIP))
	return;
      
      /* round to integer device (ReGIS) coordinates */
      ixx = IROUND(xx);
      iyy = IROUND(yy);
      
      /* sync ReGIS's foreground color to be the same as our pen color */
      _pl_r_set_pen_color (S___(_plotter));

      /* output the point, as a single pixel */
      _pl_r_regis_move (R___(_plotter) ixx, iyy);
      _write_string (_plotter->data, "V[]\n");

      /* update our notion of ReGIS's notion of position */
      _plotter->regis_pos.x = ixx;
      _plotter->regis_pos.y = iyy;
    }
}
