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

/* This file contains a low-level routine for repositioning the graphics
   cursor on a Tektronix display, by emitting an escape sequence.

   The reposition command automatically knocks the Tektronix into a
   non-alpha mode.  We choose either PLOT or POINT mode, depending on
   whether the polyline we're drawing is connected, or is simply a set of
   disconnected points.  That information is stored in our drawing state.

   If we are already in PLOT/POINT mode, emitting the escape sequence will
   prevent a line being drawn at the time of the move (the "dark vector"
   concept).  That is just what we want. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_t_tek_move (R___(Plotter *_plotter) int xx, int yy)
{
  int correct_tek_mode = 
    _plotter->drawstate->points_are_connected ? TEK_MODE_PLOT : TEK_MODE_POINT;

  switch (correct_tek_mode)
    {
    case TEK_MODE_POINT:
      /* ASCII FS, i.e. ^\ (enter POINT mode)*/
      _write_byte (_plotter->data, '\034'); 
      break;
    case TEK_MODE_PLOT:
      /* ASCII GS, i.e. ^] (enter PLOT mode) */
      _write_byte (_plotter->data, '\035'); 
      break;
    default:			/* shouldn't happen */
      return;
    }

  /* output location to the Tektronix */
  _pl_t_tek_vector (R___(_plotter) xx, yy);

  /* Tek position is now correct */
  _plotter->tek_pos.x = xx;
  _plotter->tek_pos.y = yy;  
  _plotter->tek_position_is_unknown = false;

  /* Tek is now in correct mode for plotting vectors */
  _plotter->tek_mode_is_unknown = false;
  _plotter->tek_mode = correct_tek_mode;

  /* re-emphasize: on return we'll be in either PLOT or POINT mode. */
  return;
}
