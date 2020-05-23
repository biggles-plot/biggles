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

/* This file contains the internal paint_path() and paint_paths() methods,
   which the public method endpath() is a wrapper around. */

/* This version is for CGMPlotters.  By construction, for CGMPlotters our
   path storage buffer may include any of the three builtin closed
   primitives (box, circle, ellipse), or a sequence of segments, such as
   line segments.
 
   If the parameter CGM_MAX_VERSION is "1", the only possible contents of
   the segment list is a sequence of line segments, i.e., a polyline.  If
   CGM_MAX_VERSION is "2", the segment list may contain, instead, a single
   elliptic or circular arc.  If CGM_MAX_VERSION is "3", the segment list
   may contain an arbitrary sequence of line segments, arc segments, and
   cubic Beziers too, i.e., an arbitrary `mixed path'.

   These restrictions on the segment list contents are implemented by
   setting internal Plotter parameters at initialization time (e.g.,
   _plotter->data->have_mixed_paths; see c_defplot.c).  The reason for the
   restrictions is obvious: to store in the segment list only those
   primitives that can be represented by a single CGM object, either simple
   or compound.  For example, circular arcs are not stored unless the
   emitting of version-2 CGM primitives is supported.  That's because
   although version-1 CGM's support counterclockwise arcs, clockwise arcs
   are supported only beginning with version 2.  And even though version-2
   CGM's support arbitrary closed mixed paths, at least ones that don't
   contain Beziers ("closed figures"), arbitrary open mixed paths
   ("compound lines") are supported only beginning with version 3. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_c_paint_path (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	bool closed;
	plIntPathSegment *xarray;
	int polyline_len;
	bool draw_as_cgm_compound, path_is_single_polyline;
	int pass;
	plPathSegmentType first_element_type;
	int i, byte_count, data_byte_count, data_len;
	int desired_interior_style;
	const char *desired_interior_style_string;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /*shouldn't happen */
	  break;

	/* check for closure */
	if ((_plotter->drawstate->path->num_segments >= 3)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.x == _plotter->drawstate->path->segments[0].p.x)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.y == _plotter->drawstate->path->segments[0].p.y))
	  closed = true;
	else
	  closed = false;		/* 2-point ones should be open */
	
	/* set CGM pen/fill colors and line attributes, by emitting
           appropriate commands */

	/* N.B. pen color and line attributes don't need to be set if
	   pen_type is zero, signifying an edgeless (presumably filled)
	   path */
	_pl_c_set_pen_color (R___(_plotter)
			  closed ? CGM_OBJECT_CLOSED : CGM_OBJECT_OPEN);
	_pl_c_set_fill_color (R___(_plotter)
			   closed ? CGM_OBJECT_CLOSED : CGM_OBJECT_OPEN);
	_pl_c_set_attributes (R___(_plotter) 
			   closed ? CGM_OBJECT_CLOSED : CGM_OBJECT_OPEN);
      
	/* array for points, with positions expressed in integer device
           coors */
	xarray = (plIntPathSegment *)_pl_xmalloc (_plotter->drawstate->path->num_segments * sizeof(plIntPathSegment));
      
	/* add first point of path to xarray[] (a moveto, presumably) */
	xarray[0].p.x = IROUND(XD(_plotter->drawstate->path->segments[0].p.x, 
				  _plotter->drawstate->path->segments[0].p.y));
	xarray[0].p.y = IROUND(YD(_plotter->drawstate->path->segments[0].p.x, 
				  _plotter->drawstate->path->segments[0].p.y));
	polyline_len = 1;
      
	/* convert to integer CGM coordinates (unlike the HP-GL case [see
	   h_path.c], we don't remove runs, so after this loop completes,
	   polyline_len equals _plotter->drawstate->path->num_segments) */
      
	for (i = 1; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    plPathSegment datapoint;
	    double xuser, yuser, xdev, ydev;
	    int device_x, device_y;
	    
	    datapoint = _plotter->drawstate->path->segments[i];
	    xuser = datapoint.p.x;
	    yuser = datapoint.p.y;
	    xdev = XD(xuser, yuser);
	    ydev = YD(xuser, yuser);
	    device_x = IROUND(xdev);
	    device_y = IROUND(ydev);
	    
	    {
	      plPathSegmentType element_type;
	      int device_xc, device_yc;
	      
	      xarray[polyline_len].p.x = device_x;
	      xarray[polyline_len].p.y = device_y;
	      element_type = datapoint.type;
	      xarray[polyline_len].type = element_type;
	      
	      if (element_type == S_ARC || element_type == S_ELLARC)
		/* an arc or elliptic arc element, so compute center too */
		{
		  device_xc = IROUND(XD(datapoint.pc.x, datapoint.pc.y));
		  device_yc = IROUND(YD(datapoint.pc.x, datapoint.pc.y));
		  xarray[polyline_len].pc.x = device_xc;
		  xarray[polyline_len].pc.y = device_yc;
		}
	      else if (element_type == S_CUBIC)
		/* a cubic Bezier element, so compute control points too */
		{
		  xarray[polyline_len].pc.x 
		    = IROUND(XD(datapoint.pc.x, datapoint.pc.y));
		  xarray[polyline_len].pc.y 
		    = IROUND(YD(datapoint.pc.x, datapoint.pc.y));
		  xarray[polyline_len].pd.x
		    = IROUND(XD(datapoint.pd.x, datapoint.pd.y));
		  xarray[polyline_len].pd.y
		    = IROUND(YD(datapoint.pd.x, datapoint.pd.y));
		}
	      
	      polyline_len++;
	    }
	  }
      
	/* A hack for CGM: if a circular or elliptic arc segment in integer
	   device coordinates looks bogus, i.e. endpoints are the same or
	   either is the same as the center point, replace it by a line
	   segment.  This will allow us to assume, later, that the
	   displacement vectors from the center to the two endpoints are
	   nonzero and unequal. */

	for (i = 1; i < polyline_len; i++)
	  {
	    if (xarray[i].type == S_ARC || xarray[i].type == S_ELLARC)
	      if ((xarray[i-1].p.x == xarray[i].p.x 
		   && xarray[i-1].p.y == xarray[i].p.y)
		  || (xarray[i-1].p.x == xarray[i].pc.x 
		      && xarray[i-1].p.y == xarray[i].pc.y)
		  || (xarray[i].p.x == xarray[i].pc.x 
		      && xarray[i].p.y == xarray[i].pc.y))
		xarray[i].type = S_LINE;
	  }
	
	/* set CGM attributes (differently, depending on whether path is
	   closed or open, because different CGM graphical primitives will be
	   emitted in the two cases to draw the path) */
	
	if (closed)
	  {
	    if (_plotter->drawstate->fill_type == 0)
	      /* won't do filling */
	      {
		desired_interior_style = CGM_INT_STYLE_EMPTY;
		desired_interior_style_string = "empty";
	      }
	    else
	      /* will do filling */
	      {
		desired_interior_style = CGM_INT_STYLE_SOLID;
		desired_interior_style_string = "solid";
	      }
	    
	    if (_plotter->cgm_interior_style != desired_interior_style)
	      /* emit "INTERIOR STYLE" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 22,
					  data_len, &byte_count,
					  "INTSTYLE");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				desired_interior_style,
				data_len, &data_byte_count, &byte_count,
				desired_interior_style_string);
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		/* update interior style */
		_plotter->cgm_interior_style = desired_interior_style;
	      }
	    
	    if (_plotter->drawstate->pen_type)
	      /* should draw the closed path so that edge is visible */
	      {
		if (_plotter->cgm_edge_is_visible != true)
		  /* emit "EDGE VISIBILITY" command */
		  {
		    data_len = 2;	/* 2 bytes per enum */
		    byte_count = data_byte_count = 0;
		    _cgm_emit_command_header (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      CGM_ATTRIBUTE_ELEMENT, 30,
					      data_len, &byte_count,
					      "EDGEVIS");
		    _cgm_emit_enum (_plotter->data->page, false, 
				    _plotter->cgm_encoding,
				    1,
				    data_len, &data_byte_count, &byte_count,
				    "on");
		    _cgm_emit_command_terminator (_plotter->data->page, 
						  _plotter->cgm_encoding,
						  &byte_count);
		    /* update edge visibility */
		    _plotter->cgm_edge_is_visible = true;
		  }
	      }
	    else
	      /* shouldn't edge the closed path */
	      {
		if (_plotter->cgm_edge_is_visible != false)
		  /* emit "EDGE VISIBILITY" command */
		  {
		    data_len = 2;	/* 2 bytes per enum */
		    byte_count = data_byte_count = 0;
		    _cgm_emit_command_header (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      CGM_ATTRIBUTE_ELEMENT, 30,
					      data_len, &byte_count,
					      "EDGEVIS");
		    _cgm_emit_enum (_plotter->data->page, false, 
				    _plotter->cgm_encoding,
				    0,
				    data_len, &data_byte_count, &byte_count,
				    "off");
		    _cgm_emit_command_terminator (_plotter->data->page, 
						  _plotter->cgm_encoding,
						  &byte_count);
		    /* update edge visibility */
		    _plotter->cgm_edge_is_visible = false;
		  }
	      }
	  }
	else
	  /* open! */
	  {
	    if (_plotter->drawstate->fill_type != 0)
	      /* will `fill' the path by first drawing an edgeless
		 solid-filled polygon, or an edgeless solid-filled closed
		 figure; in both cases edge visibility will be turned off */
	      {
		if (_plotter->cgm_interior_style != CGM_INT_STYLE_SOLID)
		  /* emit "INTERIOR STYLE" command */
		  {
		    data_len = 2;	/* 2 bytes per enum */
		    byte_count = data_byte_count = 0;
		    _cgm_emit_command_header (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      CGM_ATTRIBUTE_ELEMENT, 22,
					      data_len, &byte_count,
					      "INTSTYLE");
		    _cgm_emit_enum (_plotter->data->page, false, 
				    _plotter->cgm_encoding,
				    CGM_INT_STYLE_SOLID,
				    data_len, &data_byte_count, &byte_count,
				    "solid");
		    _cgm_emit_command_terminator (_plotter->data->page, 
						  _plotter->cgm_encoding,
						  &byte_count);
		    /* update interior style */
		    _plotter->cgm_interior_style = CGM_INT_STYLE_SOLID;
		  }
		
		if (_plotter->cgm_edge_is_visible)
		  /* emit "EDGE VISIBILITY" command */
		  {
		    data_len = 2;	/* 2 bytes per enum */
		    byte_count = data_byte_count = 0;
		    _cgm_emit_command_header (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      CGM_ATTRIBUTE_ELEMENT, 30,
					      data_len, &byte_count,
					      "EDGEVIS");
		    _cgm_emit_enum (_plotter->data->page, false, 
				    _plotter->cgm_encoding,
				    0,
				    data_len, &data_byte_count, &byte_count,
				    "off");
		    _cgm_emit_command_terminator (_plotter->data->page, 
						  _plotter->cgm_encoding,
						  &byte_count);
		    /* update edge visibility */
		    _plotter->cgm_edge_is_visible = false;
		  }
	      }
	  }
      
	/* Will path be drawn as a CGM compound primitive, containing > 1
	   graphical primitives?  If it contains more than one type of path
	   segment, or if it contains more than a single circular arc
	   segment or elliptic arc segment, answer is `yes'.

	   Because of our policies, implemented elsewhere, on what may be
	   stored in the segment buffer (see above), we'll draw as a
	   compound primitive only if CGM_MAX_VERSION >= 3. */

	draw_as_cgm_compound = false;
	first_element_type = xarray[1].type;
	for (i = 2; i < polyline_len; i++)
	  {
	    if (xarray[i].type == S_ARC || xarray[i].type == S_ELLARC
		|| xarray[i].type != first_element_type)
	      {
		draw_as_cgm_compound = true;
		break;
	      }
	  }
	
	/* is path simply a polyline? */
	{
	  path_is_single_polyline = true;
	  for (i = 1; i < polyline_len; i++)
	    {
	      if (xarray[i].type != S_LINE)
		{
		  path_is_single_polyline = false;
		  break;
		}
	    }
	}
	
	/* Make two passes through segment buffer: (0) draw and fill, if
	   necessary, a closed CGM object, e.g. a `closed figure' [necessary
	   iff path is closed, or is open and filled], and (1) edge an open
	   CGM object, e.g. a `compound line' [necessary iff path is
	   open]. */
	
	for (pass = 0; pass < 2; pass++)
	  {
	    int primitives_emitted;
	    
	    if (pass == 0 && !(closed || _plotter->drawstate->fill_type != 0))
	      /* no drawing of a closed object needed: skip pass 0 */
	      continue;
	    
	    if (pass == 1 
		&& (closed || (!closed && _plotter->drawstate->pen_type == 0)))
	      /* no need for a special `draw edge' pass: skip pass 1 */
	      continue;
	    
	    /* keep track of individual graphical primitives emitted per pass
	       (profile requires <=128 per composite primitive, closed or
	       open) */
	    primitives_emitted = 0;
	    
	    if (pass == 0 && !path_is_single_polyline)
	      /* emit `BEGIN CLOSED FIGURE' command (no parameters); drawing
		 of closed polylines and filling of open ones is handled
		 specially (see below) */
	      {
		data_len = 0;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_DELIMITER_ELEMENT, 8,
					  data_len, &byte_count,
					  "BEGFIGURE");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		/* update CGM version needed for this page */
		_plotter->cgm_page_version = IMAX(2, _plotter->cgm_page_version);
	      }
	    
	    if (pass == 1 && draw_as_cgm_compound)
	      /* emit `BEGIN COMPOUND LINE' command (no parameters) */
	      {
		data_len = 0;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_DELIMITER_ELEMENT, 15,
					  data_len, &byte_count,
					  "BEGCOMPOLINE");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		/* update CGM version needed for this page */
		_plotter->cgm_page_version = IMAX(3, _plotter->cgm_page_version);
	      }
	    
	    /* iterate over path elements, combining runs of line segments
	       into polylines, and runs of Beziers into poly-Beziers, but
	       emitting each circular arc and elliptic arc individually
	       (since CGM doesn't support poly-arcs) */
	    i = 0;
	    while (i + 1 < polyline_len)
	      {
		int j, end_of_run;
		plPathSegmentType element_type;
		
		/* determine `run' (relevant only for lines, Beziers) */
		element_type = xarray[i + 1].type;
		for (j = i + 1; 
		     j < polyline_len && xarray[j].type == element_type; 
		     j++)
		  ;
		end_of_run = j - 1;
	      
		switch ((int)element_type)
		  {
		  case (int)S_LINE:
		    if ((pass == 0 && !path_is_single_polyline) || (pass == 1))
		      /* normal case: emit "POLYLINE" command to draw polyline */
		      /* number of line segments in polyline: end_of_run - i */
		      /* number of points in polyline: 1 + (end_of_run - i) */
		      {
			/* update CGM profile for this page */
			if (1 + (end_of_run - i) > 4096)
			  _plotter->cgm_page_profile = 
			    IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
			
			data_len = 2 * CGM_BINARY_BYTES_PER_INTEGER * (1 + end_of_run - i);
			byte_count = data_byte_count = 0;
			_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
						  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 1,
						  data_len, &byte_count,
						  "LINE");
			/* combine line segments into polyline */
			for ( ; i <= end_of_run; i++)
			  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
					   xarray[i].p.x, xarray[i].p.y,
					   data_len, &data_byte_count, &byte_count);
			_cgm_emit_command_terminator (_plotter->data->page, 
						      _plotter->cgm_encoding,
						      &byte_count);
			primitives_emitted++;
			/* next CGM component object begins at i=end_of_run */
			i--;
		      }
		    else
		      /* Special case: we're running pass 0, and path
			 consists of a single polyline.  So emit "POLYGON"
			 command, omitting the final point if the polyline is
			 closed, to agree with CGM conventions.  */
		      {
			/* update CGM profile for this page */
			if (polyline_len - (closed ? 1 : 0) > 4096)
			  _plotter->cgm_page_profile = 
			    IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
			
			data_len = 2 * CGM_BINARY_BYTES_PER_INTEGER * (polyline_len - (closed ? 1 : 0));
			byte_count = data_byte_count = 0;
			_cgm_emit_command_header (_plotter->data->page, 
						  _plotter->cgm_encoding,
						  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 7,
						  data_len, &byte_count,
						  "POLYGON");
			for (i = 0; i < polyline_len - (closed ? 1 : 0); i++)
			  _cgm_emit_point (_plotter->data->page, 
					   false, _plotter->cgm_encoding,
					   xarray[i].p.x, xarray[i].p.y,
					   data_len, &data_byte_count, &byte_count);
			_cgm_emit_command_terminator (_plotter->data->page, 
						      _plotter->cgm_encoding,
						      &byte_count);
			primitives_emitted++;
			
			/* we've used up the entire segment buffer: no more
			   primitives to emit */
			i = polyline_len - 1;
		      }
		    break;
		    
		  case (int)S_ARC:
		    /* emit "CIRCULAR ARC CENTRE [REVERSED]" command */
		    {
		      int delta0_x = xarray[i].p.x - xarray[i + 1].pc.x;
		      int delta0_y = xarray[i].p.y - xarray[i + 1].pc.y;
		      int delta1_x = xarray[i + 1].p.x - xarray[i + 1].pc.x;
		      int delta1_y = xarray[i + 1].p.y - xarray[i + 1].pc.y;
		      double radius = sqrt((double)delta0_x * (double)delta0_x
					   + (double)delta0_y * (double)delta0_y);
		      int i_radius = IROUND(radius);
		      double dot = ((double)delta0_x * (double)delta1_y 
				    - (double)delta0_y * (double)delta1_x);
		      bool reversed = (dot >= 0.0 ? false : true);
		      
		      /* args: 1 point, 2 vectors, and the radius */
		      data_len = (3 * 2 + 1) * CGM_BINARY_BYTES_PER_INTEGER;
		      byte_count = data_byte_count = 0;
		      if (reversed)
			_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
						  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 20,
						  data_len, &byte_count,
						  "ARCCTRREV");
		      else
			_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
						  CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 15,
						  data_len, &byte_count,
						  "ARCCTR");
		      /* center point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i + 1].pc.x, xarray[i + 1].pc.y,
				       data_len, &data_byte_count, &byte_count);
		      /* vector from center to starting point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       delta0_x, delta0_y,
				       data_len, &data_byte_count, &byte_count);
		      /* vector from center to ending point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       delta1_x, delta1_y,
				       data_len, &data_byte_count, &byte_count);
		      /* radius (distance from center to starting point) */
		      _cgm_emit_integer (_plotter->data->page, 
					 false, _plotter->cgm_encoding,
					 i_radius,
					 data_len, &data_byte_count, &byte_count);
		      _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
						    &byte_count);
		      primitives_emitted++;
		      
		      /* update CGM version needed for this page */
		      if (reversed)
			_plotter->cgm_page_version = 
			  IMAX(2, _plotter->cgm_page_version);
		    }
		    /* on to next CGM component object */
		    i++;
		    break;
		    
		  case (int)S_ELLARC:
		    /* emit "ELLIPTICAL ARC" command to draw quarter-ellipse */
		    {
		      /* args: 3 points, 2 vectors */
		      data_len = 5 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
		      byte_count = data_byte_count = 0;
		      _cgm_emit_command_header (_plotter->data->page, 
						_plotter->cgm_encoding,
						CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 18,
						data_len, &byte_count,
						"ELLIPARC");
		      /* center point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i + 1].pc.x, xarray[i + 1].pc.y,
				       data_len, &data_byte_count, &byte_count);
		      /* starting point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i].p.x, xarray[i].p.y,
				       data_len, &data_byte_count, &byte_count);
		      /* ending point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i + 1].p.x, xarray[i + 1].p.y,
				       data_len, &data_byte_count, &byte_count);
		      /* vector from center to starting point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i].p.x - xarray[i + 1].pc.x,
				       xarray[i].p.y - xarray[i + 1].pc.y,
				       data_len, &data_byte_count, &byte_count);
		      /* vector from center to ending point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i + 1].p.x - xarray[i + 1].pc.x, 
				       xarray[i + 1].p.y - xarray[i + 1].pc.y,
				       data_len, &data_byte_count, &byte_count);
		      _cgm_emit_command_terminator (_plotter->data->page, 
						    _plotter->cgm_encoding,
						    &byte_count);
		      primitives_emitted++;
		    }
		    /* on to next CGM component object */
		    i++;
		    break;
		    
		  case (int)S_CUBIC:
		    /* emit "POLYBEZIER" command */
		    /* number of Bezier segments in path: end_of_run - i */
		    /* number of points in path:  1 + 3 * (end_of_run - i) */
		    /* Note: arguments include also a single `continuity
		       indicator' (a two-byte CGM index) */
		    {
		      /* update CGM profile for this page */
		      if (1 + 3 * (end_of_run - i) > 4096)
			_plotter->cgm_page_profile = 
			  IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
		      
		      data_len = 2 + (2 * CGM_BINARY_BYTES_PER_INTEGER) * (1 + 3 * (end_of_run - i));
		      byte_count = data_byte_count = 0;
		      _cgm_emit_command_header (_plotter->data->page, 
						_plotter->cgm_encoding,
						CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 26,
						data_len, &byte_count,
						"POLYBEZIER");
		      _cgm_emit_index (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       /* poly-Bezier continuity index: `2'
					  means successive Beziers abut, so
					  (after the first) each is
					  specified by only three points;
					  `1' means they don't abut.  Our
					  Beziers are contiguous, so we
					  specify `2'.  We used to specify
					  `1' if there's only one Bezier,
					  but the browser plug-in from
					  SYSDEV didn't like that (it
					  produced a parse error when such
					  a Bezier was the only element of
					  a CGM `closed figure'). */
#if 0
				       (end_of_run - i > 1 ? 2 : 1),
#else
				       (end_of_run - i > 1 ? 2 : 2),
#endif
				       data_len, &data_byte_count, &byte_count);
		      /* starting point */
		      _cgm_emit_point (_plotter->data->page, 
				       false, _plotter->cgm_encoding,
				       xarray[i].p.x, xarray[i].p.y,
				       data_len, &data_byte_count, &byte_count);
		      i++;
		      /* combine Bezier segments into poly-Bezier */
		      for ( ; i <= end_of_run; i++)
			{
			  _cgm_emit_point (_plotter->data->page, 
					   false, _plotter->cgm_encoding,
					   xarray[i].pc.x, xarray[i].pc.y,
					   data_len, &data_byte_count, &byte_count);
			  _cgm_emit_point (_plotter->data->page, 
					   false, _plotter->cgm_encoding,
					   xarray[i].pd.x, xarray[i].pd.y,
					   data_len, &data_byte_count, &byte_count);
			  _cgm_emit_point (_plotter->data->page, 
					   false, _plotter->cgm_encoding,
					   xarray[i].p.x, xarray[i].p.y,
					   data_len, &data_byte_count, &byte_count);
			}
		      _cgm_emit_command_terminator (_plotter->data->page, 
						    _plotter->cgm_encoding,
						    &byte_count);
		      primitives_emitted++;
		      
		      /* update CGM version needed for this page */
		      _plotter->cgm_page_version = 
			IMAX(3, _plotter->cgm_page_version);
		      
		      /* next CGM component object begins at i=end_of_run */
		      i--;
		    }
		    break;
		    
		  default:
		    /* shouldn't happen: unknown path segment type, ignore */
		    i++;
		    break;
		  }
	      }
	    
	    if (pass == 0 && !path_is_single_polyline)
	      /* emit `END CLOSED FIGURE' command (no parameters) */
	      {
		data_len = 0;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_DELIMITER_ELEMENT, 9,
					  data_len, &byte_count,
					  "ENDFIGURE");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		
		/* update CGM version needed for this page */
		_plotter->cgm_page_version = IMAX(2, _plotter->cgm_page_version);
		
		/* update CGM profile for this page */
		if (primitives_emitted > 128)
		  _plotter->cgm_page_profile = 
		    IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
	      }
	  
	    if (pass == 1 && draw_as_cgm_compound)
	      /* emit `END COMPOUND LINE' command (no parameters) */
	      {
		data_len = 0;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_DELIMITER_ELEMENT, 16,
					  data_len, &byte_count,
					  "ENDCOMPOLINE");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		
		/* update CGM version needed for this page */
		_plotter->cgm_page_version = 
		  IMAX(3, _plotter->cgm_page_version);
		
		/* update CGM profile for this page */
		if (primitives_emitted > 128)
		  _plotter->cgm_page_profile = 
		    IMAX(_plotter->cgm_page_profile, CGM_PROFILE_NONE);
	      }
	    
	  } /* end of loop over passes */
      
	/* free arrays of device-frame points */
	free (xarray);
      }
      break;
      
    case (int)PATH_BOX:
      {
	plPoint p0, p1;
	int xd0, xd1, yd0, yd1;	/* in integer device coordinates */
	int byte_count, data_byte_count, data_len;
	int desired_interior_style;
	const char *desired_interior_style_string;

	p0 = _plotter->drawstate->path->p0;
	p1 = _plotter->drawstate->path->p1;

	/* compute corners in device coors */
	xd0 = IROUND(XD(p0.x, p0.y));
	yd0 = IROUND(YD(p0.x, p0.y));  
	xd1 = IROUND(XD(p1.x, p1.y));
	yd1 = IROUND(YD(p1.x, p1.y));  
	
	/* set CGM edge color and attributes, by emitting appropriate
           commands */
	_pl_c_set_pen_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_fill_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_attributes (R___(_plotter) CGM_OBJECT_CLOSED);
	
	if (_plotter->drawstate->fill_type == 0)
	  /* won't do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_EMPTY;
	    desired_interior_style_string = "empty";
	  }
	else
	  /* will do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_SOLID;
	    desired_interior_style_string = "solid";
	  }
	
	if (_plotter->cgm_interior_style != desired_interior_style)
	  /* emit "INTERIOR STYLE" command */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				      CGM_ATTRIBUTE_ELEMENT, 22,
				      data_len, &byte_count,
				      "INTSTYLE");
	    _cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
			    desired_interior_style,
			    data_len, &data_byte_count, &byte_count,
			    desired_interior_style_string);
	    _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					  &byte_count);
	    /* update interior style */
	    _plotter->cgm_interior_style = desired_interior_style;
	  }
	
	if (_plotter->drawstate->pen_type)
	  /* should edge the rectangle */
	  {
	    if (_plotter->cgm_edge_is_visible != true)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				1,
				data_len, &data_byte_count, &byte_count,
				"on");
		_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = true;
	      }
	  }
	else
	  /* shouldn't edge the rectangle */
	  {
	    if (_plotter->cgm_edge_is_visible != false)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				0,
				data_len, &data_byte_count, &byte_count,
				"off");
		_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = false;
	      }
	  }
	
	/* emit "RECTANGLE" command */
	{
	  data_len = 2 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 11,
				    data_len, &byte_count,
				    "RECT");
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   xd0, yd0,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   xd1, yd1,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	}
      }
      break;
	
    case (int)PATH_CIRCLE:
      {
	double xd, yd, radius_d;
	int i_x, i_y, i_radius;		/* center and radius, quantized */
	plPoint pc;
	double radius;
	int byte_count, data_byte_count, data_len;
	int desired_interior_style;
	const char *desired_interior_style_string;
	
	pc = _plotter->drawstate->path->pc;
	radius = _plotter->drawstate->path->radius;

	/* known to be a circle in device frame, so compute center and
           radius in that frame */
	xd = XD(pc.x, pc.y);
	yd = YD(pc.x, pc.y);
	radius_d = sqrt (XDV(radius,0) * XDV(radius,0)
			 + YDV(radius,0) * YDV(radius,0));
	i_x = IROUND(xd);
	i_y = IROUND(yd);
	i_radius = IROUND(radius_d);
	
	/* set CGM edge color and attributes, by emitting appropriate
           commands */
	_pl_c_set_pen_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_fill_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_attributes (R___(_plotter) CGM_OBJECT_CLOSED);
	
	if (_plotter->drawstate->fill_type == 0)
	  /* won't do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_EMPTY;
	    desired_interior_style_string = "empty";
	  }
	else
	  /* will do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_SOLID;
	    desired_interior_style_string = "solid";
	  }
	
	if (_plotter->cgm_interior_style != desired_interior_style)
	  /* emit "INTERIOR STYLE" command */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				      CGM_ATTRIBUTE_ELEMENT, 22,
				      data_len, &byte_count,
				      "INTSTYLE");
	    _cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
			    desired_interior_style,
			    data_len, &data_byte_count, &byte_count,
			    desired_interior_style_string);
	    _cgm_emit_command_terminator (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  &byte_count);
	    /* update interior style */
	    _plotter->cgm_interior_style = desired_interior_style;
	  }
	
	if (_plotter->drawstate->pen_type)
	  /* should edge the circle */
	  {
	    if (_plotter->cgm_edge_is_visible != true)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				1,
				data_len, &data_byte_count, &byte_count,
				"on");
		_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = true;
	      }
	  }
	else
	  {
	    if (_plotter->cgm_edge_is_visible != false)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				0,
				data_len, &data_byte_count, &byte_count,
				"off");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = false;
	      }
	  }
	
	/* emit "CIRCLE" command */
	{
	  data_len = 3 * CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 12,
				    data_len, &byte_count,
				    "CIRCLE");
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   i_x, i_y,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_integer (_plotter->data->page, false, _plotter->cgm_encoding,
			     i_radius,
			     data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	}
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	double xd, yd;		/* center, in device frame */
	int i_x, i_y;		/* center, quantized */
	double theta, costheta, sintheta;
	double cd1_endpoint_x, cd1_endpoint_y; /* conjugate diameter endpts */
	double cd2_endpoint_x, cd2_endpoint_y;
	int i1_x, i1_y, i2_x, i2_y; /* same, quantized */
	plPoint pc;
	double rx, ry, angle;
	int byte_count, data_byte_count, data_len;
	int desired_interior_style;
	const char *desired_interior_style_string;

	pc = _plotter->drawstate->path->pc;
	rx = _plotter->drawstate->path->rx;
	ry = _plotter->drawstate->path->ry;
	angle = _plotter->drawstate->path->angle;	

	/* compute center, in device frame */
	xd = XD(pc.x, pc.y);
	yd = YD(pc.x, pc.y);
	i_x = IROUND(xd);
	i_y = IROUND(yd);
	
	/* inclination angle (radians), in user frame */
	theta = M_PI * angle / 180.0;
	costheta = cos (theta);
	sintheta = sin (theta);
	
	/* perform affine user->device coor transformation, computing
	   endpoints of conjugate diameter pair, in device frame */
	cd1_endpoint_x = XD(pc.x + rx * costheta, pc.y + rx * sintheta);
	cd1_endpoint_y = YD(pc.x + rx * costheta, pc.y + rx * sintheta);
	cd2_endpoint_x = XD(pc.x - ry * sintheta, pc.y + ry * costheta);
	cd2_endpoint_y = YD(pc.x - ry * sintheta, pc.y + ry * costheta);
	i1_x = IROUND(cd1_endpoint_x);
	i1_y = IROUND(cd1_endpoint_y);
	i2_x = IROUND(cd2_endpoint_x);
	i2_y = IROUND(cd2_endpoint_y);
	
	/* set CGM edge color and attributes, by emitting appropriate
           commands */
	_pl_c_set_pen_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_fill_color (R___(_plotter) CGM_OBJECT_CLOSED);
	_pl_c_set_attributes (R___(_plotter) CGM_OBJECT_CLOSED);
	
	if (_plotter->drawstate->fill_type == 0)
	  /* won't do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_EMPTY;
	    desired_interior_style_string = "empty";
	  }
	else
	  /* will do filling */
	  {
	    desired_interior_style = CGM_INT_STYLE_SOLID;
	    desired_interior_style_string = "solid";
	  }
	
	if (_plotter->cgm_interior_style != desired_interior_style)
	  /* emit "INTERIOR STYLE" command */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				      CGM_ATTRIBUTE_ELEMENT, 22,
				      data_len, &byte_count,
				      "INTSTYLE");
	    _cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
			    desired_interior_style,
			    data_len, &data_byte_count, &byte_count,
			    desired_interior_style_string);
	    _cgm_emit_command_terminator (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  &byte_count);
	    /* update interior style */
	    _plotter->cgm_interior_style = desired_interior_style;
	  }
	
	if (_plotter->drawstate->pen_type)
	  /* should edge the ellipse */
	  {
	    if (_plotter->cgm_edge_is_visible != true)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				1,
				data_len, &data_byte_count, &byte_count,
				"on");
		_cgm_emit_command_terminator (_plotter->data->page, 
					      _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = true;
	      }
	  }
	else
	  {
	    if (_plotter->cgm_edge_is_visible != false)
	      /* emit "EDGE VISIBILITY" command */
	      {
		data_len = 2;	/* 2 bytes per enum */
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (_plotter->data->page, 
					  _plotter->cgm_encoding,
					  CGM_ATTRIBUTE_ELEMENT, 30,
					  data_len, &byte_count,
					  "EDGEVIS");
		_cgm_emit_enum (_plotter->data->page, false, _plotter->cgm_encoding,
				0,
				data_len, &data_byte_count, &byte_count,
				"off");
		_cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					      &byte_count);
		/* update edge visibility */
		_plotter->cgm_edge_is_visible = false;
	      }
	  }
	
	/* emit "ELLIPSE" command */
	{
	  data_len = 3 * 2 * CGM_BINARY_BYTES_PER_INTEGER;
	  byte_count = data_byte_count = 0;
	  _cgm_emit_command_header (_plotter->data->page, _plotter->cgm_encoding,
				    CGM_GRAPHICAL_PRIMITIVE_ELEMENT, 17,
				    data_len, &byte_count,
				    "ELLIPSE");
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   i_x, i_y,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   i1_x, i1_y,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_point (_plotter->data->page, false, _plotter->cgm_encoding,
			   i2_x, i2_y,
			   data_len, &data_byte_count, &byte_count);
	  _cgm_emit_command_terminator (_plotter->data->page, _plotter->cgm_encoding,
					&byte_count);
	}
      }
      break;
      
    default:			/* shouldn't happen */
      break;
    }
}

bool
_pl_c_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
