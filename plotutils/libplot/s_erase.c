/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, 2009, Free Software
   Foundation, Inc.

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
_pl_s_erase_page (S___(Plotter *_plotter))
{
  int i;

  /* reinitialize `font used' array(s) for this page */
  for (i = 0; i < PL_NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;
  for (i = 0; i < PL_NUM_PCL_FONTS; i++)
    _plotter->data->page->pcl_font_used[i] = false;

  /* reset page-specific SVGPlotter variables, as if the page had just been
     opened */
  _plotter->s_matrix[0] = 1.0;/* dummy matrix values */
  _plotter->s_matrix[1] = 0.0;
  _plotter->s_matrix[2] = 0.0;
  _plotter->s_matrix[3] = 1.0;
  _plotter->s_matrix[4] = 0.0;
  _plotter->s_matrix[5] = 0.0;
  _plotter->s_matrix_is_unknown = true;
  _plotter->s_matrix_is_bogus = false;

  /* update our knowledge of what SVG's background color should be (we'll
     use it when we write the SVG page header) */
  _plotter->s_bgcolor = _plotter->drawstate->bgcolor;
  _plotter->s_bgcolor_suppressed = _plotter->drawstate->bgcolor_suppressed;

  return true;
}

