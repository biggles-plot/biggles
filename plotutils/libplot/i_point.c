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
#include "xmi.h"

void
_pl_i_paint_point (S___(Plotter *_plotter))
{
  double xx, yy;
  int ixx, iyy;
  miGC *pGC;
  miPixel fgPixel, bgPixel, pixels[2];
  miPoint point, offset;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* convert point to floating-point device coordinates */
      xx = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yy = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      
      /* round to integer device (GIF) coordinates */
      ixx = IROUND(xx);
      iyy = IROUND(yy);
      
      /* compute background and foreground color for miGC */
      _pl_i_set_pen_color (S___(_plotter));
      bgPixel.type = MI_PIXEL_INDEX_TYPE;
      bgPixel.u.index = _plotter->drawstate->i_bg_color_index;
      fgPixel.type = MI_PIXEL_INDEX_TYPE;
      fgPixel.u.index = _plotter->drawstate->i_pen_color_index;
      pixels[0] = bgPixel;
      pixels[1] = fgPixel;
      
      /* construct an miGC (graphics context for the libxmi module); copy
	 attributes from the Plotter's GC to it */
      pGC = miNewGC (2, pixels);
      _set_common_mi_attributes (_plotter->drawstate, (void *)pGC);
      
      point.x = ixx;
      point.y = iyy;
      miDrawPoints ((miPaintedSet *)_plotter->i_painted_set, 
		    pGC, MI_COORD_MODE_ORIGIN, 1, &point);
      
      /* deallocate miGC */
      miDeleteGC (pGC);
      
      /* copy from painted set to canvas, and clear */
      offset.x = 0;
      offset.y = 0;
      miCopyPaintedSetToCanvas ((miPaintedSet *)_plotter->i_painted_set, 
				(miCanvas *)_plotter->i_canvas, 
				offset);
      miClearPaintedSet ((miPaintedSet *)_plotter->i_painted_set);
      
      /* something was drawn in frame */
      _plotter->i_frame_nonempty = true;
    }
}
