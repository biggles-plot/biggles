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

/* This file contains the move method, which is a standard part of libplot.
   It sets a drawing attribute: the location of the graphics cursor, which
   determines the position of the next object drawn on the graphics
   device. */

#include "sys-defines.h"
#include "extern.h"

int
_API_fmove (R___(Plotter *_plotter) double x, double y)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fmove: invalid operation");
      return -1;
    }

  /* flush path under construction, if any */
  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter));

  _plotter->drawstate->pos.x = x; /* update our notion of position */
  _plotter->drawstate->pos.y = y;

  return 0;
}
