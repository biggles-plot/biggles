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

#include "sys-defines.h"
#include "extern.h"

bool
_pl_t_erase_page (S___(Plotter *_plotter))
{
  /* erase: emit ESC C-l, i.e. ^[^l */
  _write_string (_plotter->data, "\033\014");
  _plotter->tek_mode = TEK_MODE_ALPHA; /* erasing enters alpha mode */

  /* Note: kermit Tek emulator, on seeing ESC C-l , seems to enter graphics
     mode, not alpha mode.  Maybe we should specify TEK_MODE_PLOT above,
     instead of TEK_MODE_ALPHA?  The above won't hurt though, because we don't
     use TEK_MODE_ALPHA anyway (we'll have to switch away from it). */

  /* set background color (a no-op unless we're writing to a kermit
     Tektronix emulator, see t_color.c) */
  _pl_t_set_bg_color (S___(_plotter));

  return true;
}

