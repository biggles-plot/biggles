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

/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of a ReGIS display to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* ReGIS 8-bit `patterns', i.e., line types, indexed into by our internal
   line style number (PL_L_SOLID/PL_L_DOTTED/
   PL_L_DOTDASHED/PL_L_SHORTDASHED/PL_L_LONGDASHED/PL_L_DOTDOTDASHED/PL_L_DOTDOTDOTDASHED).

   ReGIS supports standard patterns P0..P9, and user-specified patterns
   made up of 2 to 8 bits.  If fewer than 8 bits are supplied, ReGIS
   repeats as much of the pattern as possible in what remains of the 8-bit
   segment.  Standard pattern P1 is solid. */

static const char * const regis_line_types[PL_NUM_LINE_TYPES] =
{ "P1", "P1000", "P11100100", "P11110000", "P11111100", "P11101010", "P10" };

void
_pl_r_set_attributes (S___(Plotter *_plotter))
{
  if (_plotter->regis_line_type_is_unknown
      || _plotter->regis_line_type != _plotter->drawstate->line_type)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(%s)\n",
	       regis_line_types[_plotter->drawstate->line_type]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_line_type_is_unknown = false;
      _plotter->regis_line_type = _plotter->drawstate->line_type;
    }
}
