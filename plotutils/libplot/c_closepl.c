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

/* This version is used for CGMPlotters, which emit graphics only after all
   pages of graphics have been drawn, and the Plotter is deleted.  Such
   Plotters maintain a linked list of pages (graphics are only written to
   the output stream when a Plotter is deleted, and the appropriate
   `terminate' method is invoked). */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_c_end_page (S___(Plotter *_plotter))
{
  int i, fullstrength, red, green, blue;

  /* update CGM profile for this page to take into account number of
     user-defined line types (nonzero only if output file can include
     version-3 constructs; see c_attribs.c) */
  {
    plCGMCustomLineType *line_type_ptr = (plCGMCustomLineType *)_plotter->data->page->extra;
    int num_line_types = 0;
    bool violates_profile = false;

    while (line_type_ptr != (plCGMCustomLineType *)NULL)
      {
	if (line_type_ptr->dash_array_len > CGM_PL_MAX_DASH_ARRAY_LENGTH)
	  violates_profile = true;
	line_type_ptr = line_type_ptr->next;
	num_line_types++;
      }
    if (num_line_types > CGM_MAX_CUSTOM_LINE_TYPES)
      violates_profile = true;

    if (violates_profile)
      _plotter->cgm_page_profile = 
	IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
  }

  /* update CGM version number for this page to take into account whether
     fonts were used on it; if allowed version is >=3 then we'll emit
     version-3 "FONT PROPERTIES" commands for every font (see c_defplot.c) */
  if (_plotter->cgm_max_version >= 3)
    {
      for (i = 0; i < PL_NUM_PS_FONTS; i++)
	{
	  if (_plotter->data->page->ps_font_used[i] == true)
	    {
	      _plotter->cgm_page_version = IMAX(_plotter->cgm_page_version, 3);
	      break;
	    }
	}
    }
  
  /* update the CGM version number of the output file, and its profile
     type, to take this page into account */
  _plotter->cgm_version = 
    IMAX(_plotter->cgm_version, _plotter->cgm_page_version);
  _plotter->cgm_profile = 
    IMAX(_plotter->cgm_profile, _plotter->cgm_page_profile);

  /* Check whether a color other than black or white has been used on this
     page: check the background color in particular (all other colors have
     already been taken into account). */
  red = _plotter->cgm_bgcolor.red;
  green = _plotter->cgm_bgcolor.green;
  blue = _plotter->cgm_bgcolor.blue;
  fullstrength = (1 << (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT)) - 1;
  if ((red != 0 || green != 0 || blue != 0)
      && (red != fullstrength || green != fullstrength || blue != fullstrength))
    _plotter->cgm_page_need_color = true;

  /* update `color needed' flag to take this page into account */
  if (_plotter->cgm_page_need_color)
    _plotter->cgm_need_color = true;

  /* copy the background color from the CGM Plotter into the `bgcolor'
     element of the plOutbuf for this page (we'll use it when writing the
     page header into the CGM output file, see c_defplot.c) */
  _plotter->data->page->bg_color = _plotter->cgm_bgcolor;
  _plotter->data->page->bg_color_suppressed = 
    _plotter->cgm_bgcolor_suppressed; /* color is really "none"? */

  return true;
}
