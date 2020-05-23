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

/* The paint_marker method, which is an internal function that is called
   when the marker() method is invoked.  It plots an object: a marker of a
   specified type, at a specified size, at the current location.

   If this returns `false', marker() will construct the marker from other
   libplot primitives, in a generic way. */

/* This implementation is for CGM Plotters.  CGM format supports the first
   few of our marker types, as CGM primitive objects. */

#include "sys-defines.h"
#include "extern.h"

/* The maximum dimension of most markers, e.g. the diameter of the circle
   marker (marker #4).  Expressed as a fraction of the `size' argument. */
#define MAXIMUM_MARKER_DIMENSION (5.0/8.0)

bool
_pl_c_paint_marker (R___(Plotter *_plotter) int type, double size)
{
  int desired_marker_type, desired_marker_size;
  double xd, yd, size_d;
  int i_x, i_y;

  switch (type)
    {
    case M_DOT:
      desired_marker_type = CGM_M_DOT;
      break;
    case M_PLUS:
      desired_marker_type = CGM_M_PLUS;
      break;
    case M_ASTERISK:
      desired_marker_type = CGM_M_ASTERISK;
      break;
    case M_CIRCLE:
      desired_marker_type = CGM_M_CIRCLE;
      break;
    case M_CROSS:
      desired_marker_type = CGM_M_CROSS;
      break;
    default:
      return false;		/* can't draw it in CGM format */
      break;
    }

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      if (_plotter->cgm_marker_type != desired_marker_type)
	/* emit "MARKER TYPE" command */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = 2;		/* number of bytes per index */
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 6,
				    data_len, &byte_count,
				    "MARKERTYPE");
	  _cgm_emit_index (_plotter->data->page, false, _plotter->cgm_encoding,
			   desired_marker_type,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  
	  /* update marker type */
	  _plotter->cgm_marker_type = desired_marker_type;
	}
      
      /* compute size of marker in device frame */
      size_d = sqrt(XDV(size,0)*XDV(size,0)+YDV(size,0)*YDV(size,0));
      desired_marker_size = IROUND(MAXIMUM_MARKER_DIMENSION * size_d);
      
      if (desired_marker_type != CGM_M_DOT 
	  && _plotter->cgm_marker_size != desired_marker_size)
	/* emit "MARKER SIZE" command (for a dot we don't bother, since
	   dots are meant to be drawn as small as possible) */
	{
	  int byte_count, data_byte_count, data_len;
	  
	  data_len = CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_ATTRIBUTE_ELEMENT, 7,
				    data_len, &byte_count,
				    "MARKERSIZE");
	  _cgm_emit_integer (_plotter->data->page, false, _plotter->cgm_encoding,
			     desired_marker_size,
			     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	  
	  /* update marker size */
	  _plotter->cgm_marker_size = desired_marker_size;
	}
  
      /* set CGM marker color */
      _pl_c_set_pen_color (R___(_plotter) CGM_OBJECT_MARKER);
      
      /* compute location in device frame */
      xd = XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      yd = YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y);
      i_x = IROUND(xd);
      i_y = IROUND(yd);
      
      /* emit "POLYMARKER" command, to draw a single marker */
      {
	int byte_count, data_byte_count, data_len;
	
	data_len = 1 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 3,
				  data_len, &byte_count,
				  "MARKER");
	_cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			 i_x, i_y,
			 data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
				      &byte_count);
      }
    }

  return true;
}

