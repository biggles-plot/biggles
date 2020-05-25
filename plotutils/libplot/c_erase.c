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
_pl_c_erase_page (S___(Plotter *_plotter))
{
  int i;

  /* reinitialize `font used' array(s) for this page */
  for (i = 0; i < PL_NUM_PS_FONTS; i++)
    _plotter->data->page->ps_font_used[i] = false;

  /* deallocate table of user-specified line types, if any */
  if (_plotter->data->page->extra)
    {
      plCGMCustomLineType *linetype_ptr = (plCGMCustomLineType *)_plotter->data->page->extra;
      plCGMCustomLineType *old_linetype_ptr;

      while (linetype_ptr)
	{
	  if (linetype_ptr->dash_array_len > 0 /* paranoia */
	      && linetype_ptr->dashes)
	    free (linetype_ptr->dashes);
	  old_linetype_ptr = linetype_ptr;
	  linetype_ptr = linetype_ptr->next;
	  free (old_linetype_ptr);
	}
      _plotter->data->page->extra = (void *)NULL;
    }

  /* reset other page-specific, i.e. picture-specific, CGMPlotter
     variables, as if the page had just been opened */
  _plotter->cgm_page_version = 1;
  _plotter->cgm_page_profile = CGM_PROFILE_WEB;
  _plotter->cgm_page_need_color = false;
	/* colors */
  _plotter->cgm_line_color.red = -1;
  _plotter->cgm_line_color.green = -1;
  _plotter->cgm_line_color.blue = -1;
  _plotter->cgm_edge_color.red = -1;
  _plotter->cgm_edge_color.green = -1;
  _plotter->cgm_edge_color.blue = -1;
  _plotter->cgm_fillcolor.red = -1;
  _plotter->cgm_fillcolor.green = -1;
  _plotter->cgm_fillcolor.blue = -1;
  _plotter->cgm_marker_color.red = -1;
  _plotter->cgm_marker_color.green = -1;
  _plotter->cgm_marker_color.blue = -1;
  _plotter->cgm_text_color.red = -1;
  _plotter->cgm_text_color.green = -1;
  _plotter->cgm_text_color.blue = -1;
  _plotter->cgm_bgcolor.red = -1;
  _plotter->cgm_bgcolor.green = -1;
  _plotter->cgm_bgcolor.blue = -1;
	/* other dynamic variables */
  _plotter->cgm_line_type = CGM_L_SOLID;
  _plotter->cgm_dash_offset = 0.0;
  _plotter->cgm_join_style = CGM_JOIN_UNSPEC;
  _plotter->cgm_cap_style = CGM_CAP_UNSPEC;  
  _plotter->cgm_dash_cap_style = CGM_CAP_UNSPEC;  
  	/* CGM's default line width: 1/1000 times the max VDC dimension */
  _plotter->cgm_line_width = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) / 500;
  _plotter->cgm_interior_style = CGM_INT_STYLE_HOLLOW;
  _plotter->cgm_edge_type = CGM_L_SOLID;
  _plotter->cgm_edge_dash_offset = 0.0;
  _plotter->cgm_edge_join_style = CGM_JOIN_UNSPEC;
  _plotter->cgm_edge_cap_style = CGM_CAP_UNSPEC;  
  _plotter->cgm_edge_dash_cap_style = CGM_CAP_UNSPEC;  
  	/* CGM's default edge width: 1/1000 times the max VDC dimension */
  _plotter->cgm_edge_width = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) / 500;
  _plotter->cgm_edge_is_visible = false;
  _plotter->cgm_miter_limit = 32767.0;
  _plotter->cgm_marker_type = CGM_M_ASTERISK;
  	/* CGM's default marker size: 1/1000 times the max VDC dimension */
  _plotter->cgm_marker_size = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) /500;
  	/* label-related variables */
  _plotter->cgm_char_height = -1; /* impossible (dummy) value */
  _plotter->cgm_char_base_vector_x = 1;
  _plotter->cgm_char_base_vector_y = 0;
  _plotter->cgm_char_up_vector_x = 0;
  _plotter->cgm_char_up_vector_y = 1;
  _plotter->cgm_horizontal_text_alignment = CGM_ALIGN_NORMAL_HORIZONTAL;
  _plotter->cgm_vertical_text_alignment = CGM_ALIGN_NORMAL_VERTICAL;
  _plotter->cgm_font_id = -1;	/* impossible (dummy) value */
  _plotter->cgm_charset_lower = 0; /* dummy value (we use values 1..4) */
  _plotter->cgm_charset_upper = 0; /* dummy value (we use values 1..4) */
  _plotter->cgm_restricted_text_type = CGM_RESTRICTED_TEXT_TYPE_BASIC;

  /* copy the bg color currently in the drawing state to the CGM-specific
     part of the CGMPlotter; it'll be written to the output file at the
     head of the picture */
  _pl_c_set_bg_color (S___(_plotter));

  return true;
}
