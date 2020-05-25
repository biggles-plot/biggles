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

/* This file contains the havecap method, which is a GNU extension to
   libplot.  It queries the current plotter to determine whether or not it
   has a specified capability, specified by a string.

   Return value is 0/1/2 (no/yes/maybe).  If the capability is not
   recognized, the return value is 0. */


#include "sys-defines.h"
#include "extern.h"

int
_API_havecap (R___(Plotter *_plotter) const char *s)
{
  if (strcasecmp (s, "WIDE_LINES") == 0)
    return _plotter->data->have_wide_lines;
  else if (strcasecmp (s, "SOLID_FILL") == 0)
    return _plotter->data->have_solid_fill;
  else if (strcasecmp (s, "DASH_ARRAY") == 0)
    return _plotter->data->have_dash_array;
  else if (strcasecmp (s, "EVEN_ODD_FILL") == 0)
    return _plotter->data->have_odd_winding_fill;
  else if (strcasecmp (s, "NONZERO_WINDING_NUMBER_FILL") == 0)
    return _plotter->data->have_nonzero_winding_fill;
  else if (strcasecmp (s, "SETTABLE_BACKGROUND") == 0)
    return _plotter->data->have_settable_bg;
  else if (strcasecmp (s, "HERSHEY_FONTS") == 0)
    return 1;			/* always supported */
  else if (strcasecmp (s, "PS_FONTS") == 0)
    return _plotter->data->have_ps_fonts;
  else if (strcasecmp (s, "PCL_FONTS") == 0)
    return _plotter->data->have_pcl_fonts;
  else if (strcasecmp (s, "STICK_FONTS") == 0)
    return _plotter->data->have_stick_fonts;
  else if (strcasecmp (s, "EXTRA_STICK_FONTS") == 0)
    return _plotter->data->have_extra_stick_fonts;
  else
    return 0;
}
