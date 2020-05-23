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

/* This file contains device-specific color database access routines.
   These routines are called by various CGMPlotter methods, before drawing
   objects.

   The CGM output file will use either 24-bit RGB or 48-bit RGB, depending
   on the value of CGM_BINARY_BYTES_PER_COLOR_COMPONENT (set in extern.h; 
   1 or 2, respectively).  This code assumes that the value is 1 or 2, even
   though CGM files allow 3 or 4 as well.  To handle the `4' case, we
   should rewrite this to use unsigned ints rather than signed ints.

   The reason we don't bother with 3 or 4 is that internally, libplot uses
   48-bit color.  So 48-bit RGB in the CGM output file is all we need. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_c_set_pen_color(R___(Plotter *_plotter) int cgm_object_type)
{
  int red_long, green_long, blue_long;
  int red, green, blue;
  int byte_count, data_byte_count, data_len;
  int fullstrength;

  if (_plotter->drawstate->pen_type == 0
      && cgm_object_type != CGM_OBJECT_TEXT)
    /* don't do anything, pen color will be ignored when writing objects */
    return;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->fgcolor.red;
  green_long = _plotter->drawstate->fgcolor.green;
  blue_long = _plotter->drawstate->fgcolor.blue;

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  fullstrength = (1 << (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT)) - 1;
  if ((red != 0 || green != 0 || blue != 0)
      && (red != fullstrength || green != fullstrength || blue != fullstrength))
    _plotter->cgm_page_need_color = true;

  switch (cgm_object_type)
    {
    case CGM_OBJECT_OPEN:
      if (_plotter->cgm_line_color.red != red 
	  || _plotter->cgm_line_color.green != green
	  || _plotter->cgm_line_color.blue != blue)
	/* emit "LINE_COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 4,
				    data_len, &byte_count,
				    "LINECOLR");
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's pen color */
	  _plotter->cgm_line_color.red = red;
	  _plotter->cgm_line_color.green = green;
	  _plotter->cgm_line_color.blue = blue;
	}
      break;
    case CGM_OBJECT_CLOSED:
      if (_plotter->cgm_edge_color.red != red 
	  || _plotter->cgm_edge_color.green != green
	  || _plotter->cgm_edge_color.blue != blue)
	/* emit "EDGE_COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 29,
				    data_len, &byte_count,
				    "EDGECOLR");
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's edge color */
	  _plotter->cgm_edge_color.red = red;
	  _plotter->cgm_edge_color.green = green;
	  _plotter->cgm_edge_color.blue = blue;
	}
      break;
    case CGM_OBJECT_MARKER:
      if (_plotter->cgm_marker_color.red != red 
	  || _plotter->cgm_marker_color.green != green
	  || _plotter->cgm_marker_color.blue != blue)
	/* emit "MARKER COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 8,
				    data_len, &byte_count,
				    "MARKERCOLR");
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's marker color */
	  _plotter->cgm_marker_color.red = red;
	  _plotter->cgm_marker_color.green = green;
	  _plotter->cgm_marker_color.blue = blue;
	}
      break;
    case CGM_OBJECT_TEXT:
      if (_plotter->cgm_text_color.red != red 
	  || _plotter->cgm_text_color.green != green
	  || _plotter->cgm_text_color.blue != blue)
	/* emit "TEXT COLOR" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 14,
				    data_len, &byte_count,
				    "TEXTCOLR");
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)red,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)green,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				     (unsigned int)blue,
				     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  /* update our knowledge of CGM's text color */
	  _plotter->cgm_text_color.red = red;
	  _plotter->cgm_text_color.green = green;
	  _plotter->cgm_text_color.blue = blue;
	}
      break;
    default:
      break;
    }
}

void
_pl_c_set_fill_color(R___(Plotter *_plotter) int cgm_object_type)
{
  int red_long, green_long, blue_long;
  int red, green, blue;
  int fullstrength;
  int byte_count, data_byte_count, data_len;

  if (_plotter->drawstate->fill_type == 0)
    /* don't do anything, fill color will be ignored when writing objects */
    return;

  if (cgm_object_type != CGM_OBJECT_OPEN
      && cgm_object_type != CGM_OBJECT_CLOSED)
    /* don't do anything; won't be filling */
    return;

  /* obtain each RGB as a 16-bit quantity (48 bits in all) */
  red_long = _plotter->drawstate->fillcolor.red;
  green_long = _plotter->drawstate->fillcolor.green;
  blue_long = _plotter->drawstate->fillcolor.blue;

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  fullstrength = (1 << (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT)) - 1;
  if ((red != 0 || green != 0 || blue != 0)
      && (red != fullstrength || green != fullstrength || blue != fullstrength))
    _plotter->cgm_page_need_color = true;

  if (_plotter->cgm_fillcolor.red != red 
      || _plotter->cgm_fillcolor.green != green
      || _plotter->cgm_fillcolor.blue != blue)
    /* emit "FILL COLOR" command */
    {
      data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
      byte_count = data_byte_count = 0;
      _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				CGM_ATTRIBUTE_ELEMENT, 23,
				data_len, &byte_count,
				"FILLCOLR");
      _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				 (unsigned int)red,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				 (unsigned int)green,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_color_component (_plotter->data->page, false, _plotter->cgm_encoding,
				 (unsigned int)blue,
				 data_len, &data_byte_count, &byte_count);
      _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
				    &byte_count);
      /* update our knowledge of CGM's fill color */
      _plotter->cgm_fillcolor.red = red;
      _plotter->cgm_fillcolor.green = green;
      _plotter->cgm_fillcolor.blue = blue;
    }
}

void
_pl_c_set_bg_color(S___(Plotter *_plotter))
{
  int red_long, green_long, blue_long;
  int red, green, blue;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->bgcolor.red;
  green_long = _plotter->drawstate->bgcolor.green;
  blue_long = _plotter->drawstate->bgcolor.blue;

  /* 24-bit or 48-bit RGB (as used in CGMs) */
  switch (CGM_BINARY_BYTES_PER_COLOR_COMPONENT)
    {
    case 1:
      /* 24-bit */
      red = (((unsigned int)red_long) >> 8) & 0xff;
      green = (((unsigned int)green_long) >> 8) & 0xff;
      blue = (((unsigned int)blue_long) >> 8) & 0xff;
      break;
    case 2:
    default:
      /* 48-bit */
      red = red_long;
      green = green_long;
      blue = blue_long;
      break;
    }

  /* update our knowledge of what CGM's background color should be (we'll
     use it only when we write the picture header) */
  _plotter->cgm_bgcolor.red = red;
  _plotter->cgm_bgcolor.green = green;
  _plotter->cgm_bgcolor.blue = blue;

  /* should the just-computed color be ignored, i.e., did the user really
     specify "none" as the background color? */
  _plotter->cgm_bgcolor_suppressed = _plotter->drawstate->bgcolor_suppressed;
}
