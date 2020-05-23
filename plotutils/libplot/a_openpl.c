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
_pl_a_begin_page (S___(Plotter *_plotter))
{
  int i;

  /* The following resetting code should duplicate what is done in
     erase_page(). */

  /* initialize `font used' array for this page */
  for (i = 0; i < PL_NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;

  /* With each call to openpl(), we reset our knowledge of Illustrator's
     internal state, i.e. the dynamic AI-specific data members of the
     AIPlotter.  The values are the same as are used in initializing the
     AIPlotter (see a_defplot.c). */
     
  _plotter->ai_pen_cyan = 0.0;
  _plotter->ai_pen_magenta = 0.0;
  _plotter->ai_pen_yellow = 0.0;
  _plotter->ai_pen_black = 1.0;  
  _plotter->ai_fill_cyan = 0.0;
  _plotter->ai_fill_magenta = 0.0;
  _plotter->ai_fill_yellow = 0.0;
  _plotter->ai_fill_black = 1.0;  
  _plotter->ai_cyan_used = false;
  _plotter->ai_magenta_used = false;
  _plotter->ai_yellow_used = false;
  _plotter->ai_black_used = false;
  _plotter->ai_cap_style = AI_LINE_CAP_BUTT;
  _plotter->ai_join_style = AI_LINE_JOIN_MITER;  
  _plotter->ai_miter_limit = 4.0; /* default value for AI */
  _plotter->ai_line_type = PL_L_SOLID;  
  _plotter->ai_line_width = 1.0;    
  _plotter->ai_fill_rule_type = 0; /* i.e. nonzero winding number rule */

  return true;
}
