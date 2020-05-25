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
_pl_m_paint_point (S___(Plotter *_plotter))
{
  _pl_m_set_attributes (R___(_plotter) 
		     PL_ATTR_TRANSFORMATION_MATRIX 
		     | PL_ATTR_PEN_COLOR | PL_ATTR_PEN_TYPE);
  _pl_m_emit_op_code (R___(_plotter) O_FPOINT);
  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->pos.x);
  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->pos.y);
  _pl_m_emit_terminator (S___(_plotter));

  _plotter->meta_pos = _plotter->drawstate->pos;

  return;
}

