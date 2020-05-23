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
_pl_m_begin_page (S___(Plotter *_plotter))
{
  if (_plotter->data->page_number == 1)
    /* emit metafile header, i.e. magic string */
    {
      _write_string (_plotter->data, PL_PLOT_MAGIC);

      /* format type 1 = GNU binary, type 2 = GNU portable */
      if (_plotter->meta_portable_output)
	_write_string (_plotter->data, " 2\n");
      else
	_write_string (_plotter->data, " 1\n");
    }
  
  _pl_m_emit_op_code (R___(_plotter) O_OPENPL);
  _pl_m_emit_terminator (S___(_plotter));

  /* reset page-specific, i.e. picture-specific, dynamic variables */
  _plotter->meta_pos.x = 0.0;
  _plotter->meta_pos.y = 0.0;
  _plotter->meta_position_is_unknown = false;
  _plotter->meta_m_user_to_ndc[0] = 1.0;
  _plotter->meta_m_user_to_ndc[1] = 0.0;
  _plotter->meta_m_user_to_ndc[2] = 0.0;
  _plotter->meta_m_user_to_ndc[3] = 1.0;
  _plotter->meta_m_user_to_ndc[4] = 0.0;
  _plotter->meta_m_user_to_ndc[5] = 0.0;
  _plotter->meta_fill_rule_type = PL_FILL_ODD_WINDING;
  _plotter->meta_line_type = PL_L_SOLID;
  _plotter->meta_points_are_connected = true;  
  _plotter->meta_cap_type = PL_CAP_BUTT;  
  _plotter->meta_join_type = PL_JOIN_MITER;  
  _plotter->meta_miter_limit = PL_DEFAULT_MITER_LIMIT;  
  _plotter->meta_line_width = 0.0;
  _plotter->meta_line_width_is_default = true;
  _plotter->meta_dash_array = (const double *)NULL;
  _plotter->meta_dash_array_len = 0;
  _plotter->meta_dash_offset = 0.0;  
  _plotter->meta_dash_array_in_effect = false;  
  _plotter->meta_pen_type = 1;  
  _plotter->meta_fill_type = 0;
  _plotter->meta_orientation = 1;  
  _plotter->meta_font_name = (const char *)NULL;
  _plotter->meta_font_size = 0.0;
  _plotter->meta_font_size_is_default = true;
  _plotter->meta_text_rotation = 0.0;  
  _plotter->meta_fgcolor.red = 0;
  _plotter->meta_fgcolor.green = 0;
  _plotter->meta_fgcolor.blue = 0;
  _plotter->meta_fillcolor_base.red = 0;
  _plotter->meta_fillcolor_base.green = 0;
  _plotter->meta_fillcolor_base.blue = 0;
  _plotter->meta_bgcolor.red = 65535;
  _plotter->meta_bgcolor.green = 65535;
  _plotter->meta_bgcolor.blue = 65535;

  return true;
}
