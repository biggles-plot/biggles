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

/* This file contains the point method, which is a standard part of
   libplot.  It plots an object: a point with coordinates x,y. */

#include "sys-defines.h"
#include "extern.h"

int
_API_fpoint (R___(Plotter *_plotter) double x, double y)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fpoint: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* update our notion of position */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  if (_plotter->drawstate->pen_type == 0)
    /* no pen to draw with, so do nothing */
    return 0;

  /* call internal function: draw marker at current location, in a
     Plotter-specific way */
  _plotter->paint_point (S___(_plotter));

  return 0;
}

/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

/* This implementation is for a generic Plotter.  It does nothing. */

void
_pl_g_paint_point (S___(Plotter *_plotter))
{
  return;
}
