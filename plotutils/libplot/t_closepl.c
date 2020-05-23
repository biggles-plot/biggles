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
_pl_t_end_page (S___(Plotter *_plotter))
{
  _pl_t_tek_move (R___(_plotter) 0, 0); /* go to lower left corner in Tek space */
  _pl_t_tek_mode (R___(_plotter) TEK_MODE_ALPHA); /* switch to alpha mode */

  switch (_plotter->tek_display_type) /* exit from Tek mode */
    {
    case TEK_DPY_KERMIT:
      /* use VT340 command to exit graphics mode */
      _write_string (_plotter->data, "\033[?38l");
      /* following command may be an alternative */
      /*
	_write_string (_plotter->data, "\030");
      */
      break;
    case TEK_DPY_XTERM:
      /* ESC C-c, restore to VT102 mode */
      _write_string (_plotter->data, "\033\003"); 
      break;
    case TEK_DPY_GENERIC:
    default:
      break;
    }

  return true;
}
