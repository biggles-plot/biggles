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
_pl_s_begin_page (S___(Plotter *_plotter))
{
  int i;

  /* initialize `font used' arrays for this page */
  for (i = 0; i < PL_NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;
  for (i = 0; i < PL_NUM_PCL_FONTS; i++)
    _plotter->data->page->pcl_font_used[i] = false;

  /* copy background color to the SVG-specific part of the SVGPlotter; that
     value will be written out at the head of the page (also, it'll be
     updated before that, if erase() is invoked) */
  _plotter->s_bgcolor = _plotter->drawstate->bgcolor;
  _plotter->s_bgcolor_suppressed = _plotter->drawstate->bgcolor_suppressed;

  return true;
}
