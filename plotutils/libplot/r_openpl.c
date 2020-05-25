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

/* This version is for ReGIS Plotters. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_r_begin_page (S___(Plotter *_plotter))
{
  /* send graphics initialization commands to output stream */

  /* clear terminal screen */
  _write_string (_plotter->data, "\033[2J");
  /* enter ReGIS graphics */
  _write_string (_plotter->data, "\033P1p");
  /* turn off graphics cursor */
  _write_string (_plotter->data, "S(C0)\n");

  /* copy libplot's background color to internal ReGIS state */
  _pl_r_set_bg_color (S___(_plotter));
  /* erase screen (causing background color to show up) */
  _write_string (_plotter->data, "S(E)\n");

  return true;
}
