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

/* This version is for HPGL and PCL Plotters.  By construction, for these
   Plotters our path buffer always contains either a segment list, or a
   rectangle or circle object.  If it's a segment list, it may include an
   arbitrary sequence of line, circular arc, and Bezier elements.  (For
   circular arcs to be included, the map from user to device coordinates
   must be uniform, so that e.g. the angle subtended by the arc will be the
   same in user and device coordinates.) */

#include "sys-defines.h"
#include "extern.h"

#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))

void
_pl_h_paint_path (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	plIntPathSegment *xarray;
	plPoint p0, pp1, pc, savedpoint;
	bool closed, use_polygon_buffer;
	double last_x, last_y;
	int i, polyline_len;
	bool identical_user_coordinates = true;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /* shouldn't happen*/
	  break;

	if ((_plotter->drawstate->path->num_segments >= 3)/*check for closure*/
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.x == _plotter->drawstate->path->segments[0].p.x)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.y == _plotter->drawstate->path->segments[0].p.y))
	  closed = true;
	else
	  closed = false;	/* 2-point ones should be open */
      
        /* convert vertices to integer device coordinates, removing runs */
      
	/* array for points, with positions expressed in integer device coors*/
	xarray = (plIntPathSegment *)_pl_xmalloc (_plotter->drawstate->path->num_segments * sizeof(plIntPathSegment));
	
	/* add first point of path to xarray[] (type field is a moveto) */
	xarray[0].p.x = IROUND(XD(_plotter->drawstate->path->segments[0].p.x, 
				  _plotter->drawstate->path->segments[0].p.y));
	xarray[0].p.y = IROUND(YD(_plotter->drawstate->path->segments[0].p.x, 
				  _plotter->drawstate->path->segments[0].p.y));
	polyline_len = 1;
	/* save user coors of last point added to xarray[] */
	last_x = _plotter->drawstate->path->segments[0].p.x;
	last_y = _plotter->drawstate->path->segments[0].p.y;  
	
	for (i = 1; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    plPathSegment datapoint;
	    double xuser, yuser, xdev, ydev;
	    int device_x, device_y;
	    
	    datapoint = _plotter->drawstate->path->segments[i];
	    xuser = datapoint.p.x;
	    yuser = datapoint.p.y;
	    if (xuser != last_x || yuser != last_y)
	      /* in user space, not all points are the same */
	      identical_user_coordinates = false;	
	    
	    xdev = XD(xuser, yuser);
	    ydev = YD(xuser, yuser);
	    device_x = IROUND(xdev);
	    device_y = IROUND(ydev);
	    
	    if (device_x != xarray[polyline_len-1].p.x
		|| device_y != xarray[polyline_len-1].p.y)
	      /* integer device coor(s) changed, so stash point (incl. type
		 field) */
	      {
		plPathSegmentType element_type;
		int device_xc, device_yc;
		
		xarray[polyline_len].p.x = device_x;
		xarray[polyline_len].p.y = device_y;
		element_type = datapoint.type;
		xarray[polyline_len].type = element_type;
		
		if (element_type == S_ARC)
		  /* an arc element, so compute center, subtended angle too */
		  {
		    double angle;
		    
		    device_xc = IROUND(XD(datapoint.pc.x, datapoint.pc.y));
		    device_yc = IROUND(YD(datapoint.pc.x, datapoint.pc.y));
		    xarray[polyline_len].pc.x = device_xc;
		    xarray[polyline_len].pc.y = device_yc;
		    p0.x = last_x; 
		    p0.y = last_y;
		    pp1 = datapoint.p;
		    pc = datapoint.pc;
		    angle = _angle_of_arc (p0, pp1, pc);

		    /* if user coors -> device coors includes a reflection,
                       flip sign */
		    if (!_plotter->drawstate->transform.nonreflection)
		      angle = -angle;
		    xarray[polyline_len].angle = angle;
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
		
		/* save user coors of last point added to xarray[] */
		last_x = datapoint.p.x;
		last_y = datapoint.p.y;  
		polyline_len++;
	      }
	  }
	
	/* Check first for special subcase: all user-space juncture points
	   in the polyline were mapped to a single integer HP-GL
	   pseudo-pixel.  If (1) they weren't all the same to begin with,
	   or (2) they were all the same to begin with and the cap mode is
	   "round", then draw as a filled circle, of diameter equal to the
	   line width; otherwise draw nothing. */
      
	if (_plotter->drawstate->path->num_segments > 1 && polyline_len == 1)
	  /* all points mapped to a single integer pseudo-pixel */
	  {
	    if (identical_user_coordinates == false
		|| _plotter->drawstate->cap_type == PL_CAP_ROUND)
	      {
		double r = 0.5 * _plotter->drawstate->line_width;
		double device_frame_radius;
		
		/* draw single filled circle, using HP-GL's native
		   circle-drawing facility */
		
		/* move to center of circle */
		savedpoint = _plotter->drawstate->pos;
		_plotter->drawstate->pos = 
		  _plotter->drawstate->path->segments[0].p;
		_pl_h_set_position (S___(_plotter));
		_plotter->drawstate->pos = savedpoint;
		
		/* set fill color to pen color, arrange to do filling; sync
		   attributes too, incl. pen width */
		{
		  /* emit HP-GL directives; select a fill color that's
		     actually the pen color */
		  _pl_h_set_fill_color (R___(_plotter) true);
		  _pl_h_set_attributes (S___(_plotter));
		}

		/* compute radius in device frame */
		device_frame_radius = 
		  sqrt(XDV(r,0)*XDV(r,0)+YDV(r,0)*YDV(r,0));
		
		/* Syncing the fill color may have set the
		   _plotter->hpgl_bad_pen flag (e.g. if optimal pen is #0
		   [white] and we're not allowed to use pen #0 to draw
		   with).  So we test _plotter->hpgl_bad_pen before using
		   the pen. */
		if (_plotter->hpgl_bad_pen == false)
		  /* fill the circle (360 degree wedge) */
		  {
		    sprintf (_plotter->data->page->point, "WG%d,0,360;", 
			     IROUND(device_frame_radius));
		    _update_buffer (_plotter->data->page);
		  }
		/* KLUDGE: in pre-HP-GL/2, our `set_fill_color' function
		   may alter the line type, since it may request *solid*
		   crosshatching; so reset the line type */
		if (_plotter->hpgl_version < 2)
		  _pl_h_set_attributes (S___(_plotter));
	      }
	  
	    /* free our temporary array and depart */
	    free (xarray);
	    break;
	  }
	
	/* At this point, we know we have a nondegenerate path in our
	   pseudo-integer device space. */
	
	/* will draw vectors (or arcs) into polygon buffer if appropriate */
	use_polygon_buffer = (_plotter->hpgl_version == 2
			      || (_plotter->hpgl_version == 1 /* i.e. "1.5" */
				  && (polyline_len > 2
				      || _plotter->drawstate->fill_type)) ? true : false);
	
	/* Sync pen color.  This is needed here only if HPGL_VERSION is 1,
	   but we always do it here so that HP-GL/2 output that draws a
	   polyline, if sent erroneously to a generic HP-GL device, will
	   yield a polyline in the correct color, so long as the color
	   isn't white. */
	_pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);
	
	/* set_pen_color() sets the advisory bad_pen flag if white pen (pen
	   #0) would have been selected, and we can't use pen #0 to draw
	   with.  Such a situation isn't fatal if HPGL_VERSION is "1.5" or
	   "2", since we may be filling the polyline with a nonwhite color,
	   as well as using a white pen to draw it.  But if HPGL_VERSION is
	   "1", we don't fill polylines, so we might as well punt right
	   now. */
	if (_plotter->hpgl_bad_pen && _plotter->hpgl_version == 1)
	  {
	    /* free integer storage buffer and depart */
	    free (xarray);
	    break;
	  }
	
	/* sync attributes, incl. pen width if possible; move pen to p0 */
	_pl_h_set_attributes (S___(_plotter));
	
	savedpoint = _plotter->drawstate->pos;
	_plotter->drawstate->pos = _plotter->drawstate->path->segments[0].p;
	_pl_h_set_position (S___(_plotter));
	_plotter->drawstate->pos = savedpoint;
	
	if (use_polygon_buffer)
	  /* have a polygon buffer, and can use it to fill polyline */
	  {
	    /* enter polygon mode */
	    strcpy (_plotter->data->page->point, "PM0;");
	    _update_buffer (_plotter->data->page);
	  }
	
	if (use_polygon_buffer || _plotter->drawstate->pen_type)
	  /* either (1) we'll be drawing into a polygon buffer, and will be
	     using it for at least one of (a) filling and (b) edging, or
	     (2) we won't be drawing into a polygon buffer, so we won't be
	     filling, but we'll be edging (because pen_type isn't zero) */
	  {
	    /* ensure that pen is down for drawing */
	    if (_plotter->hpgl_pendown == false)
	      {
		strcpy (_plotter->data->page->point, "PD;");
		_update_buffer (_plotter->data->page);
		_plotter->hpgl_pendown = true;
	      }
	    
	    /* loop through points in xarray[], emitting HP-GL instructions */
	    i = 1;
	    while (i < polyline_len)
	      {
		switch ((int)xarray[i].type)
		  {
		  case (int)S_LINE:
		    /* emit one or more pen advances */
		    strcpy (_plotter->data->page->point, "PA");
		    _update_buffer (_plotter->data->page);
		    sprintf (_plotter->data->page->point, "%d,%d", 
			     xarray[i].p.x, xarray[i].p.y);
		    _update_buffer (_plotter->data->page);
		    i++;
		    while (i < polyline_len && xarray[i].type == S_LINE)
		      {
			sprintf (_plotter->data->page->point, 
				 ",%d,%d", xarray[i].p.x, xarray[i].p.y);
			_update_buffer (_plotter->data->page);
			i++;
		      }
		    sprintf (_plotter->data->page->point, ";");
		    _update_buffer (_plotter->data->page);	  
		    break;
		    
		  case (int)S_CUBIC:
		    /* emit one or more cubic Bezier segments */
		    strcpy (_plotter->data->page->point, "BZ");
		    _update_buffer (_plotter->data->page);
		    sprintf (_plotter->data->page->point, "%d,%d,%d,%d,%d,%d",
			     xarray[i].pc.x, xarray[i].pc.y,
			     xarray[i].pd.x, xarray[i].pd.y,
			     xarray[i].p.x, xarray[i].p.y);
		    _update_buffer (_plotter->data->page);
		    i++;
		    while (i < polyline_len && xarray[i].type == S_CUBIC)
		      {
			sprintf (_plotter->data->page->point, ",%d,%d,%d,%d,%d,%d",
				 xarray[i].pc.x, xarray[i].pc.y,
				 xarray[i].pd.x, xarray[i].pd.y,
				 xarray[i].p.x, xarray[i].p.y);
			_update_buffer (_plotter->data->page);
			i++;
		      }
		  sprintf (_plotter->data->page->point, ";");
		  _update_buffer (_plotter->data->page);	  
		  break;
		  
		  case (int)S_ARC:
		    {
		      double degrees;
		      int int_degrees;

		      /* emit an arc, using integer sweep angle if possible */
		      degrees = 180.0 * xarray[i].angle / M_PI;
		      int_degrees = IROUND (degrees);
		      if (_plotter->hpgl_version > 0) 
			/* HPGL_VERSION = 1.5 or 2 */
			{
			  if (degrees == (double)int_degrees)
			    sprintf (_plotter->data->page->point, "AA%d,%d,%d;",
				     xarray[i].pc.x, xarray[i].pc.y,
				     int_degrees);
			  else
			    sprintf (_plotter->data->page->point, "AA%d,%d,%.3f;",
				     xarray[i].pc.x, xarray[i].pc.y,
				     degrees);
			}
		      else
			/* HPGL_VERSION = 1, i.e. generic HP-GL */
			/* note: generic HP-GL can only handle integer
			   sweep angles */
			sprintf (_plotter->data->page->point, "AA%d,%d,%d;",
				 xarray[i].pc.x, xarray[i].pc.y,
				 int_degrees);
		      _update_buffer (_plotter->data->page);
		      i++;
		    }
		    break;
		    
		  default:
		    /* shouldn't happen: unknown type for path segment,
                       ignore */
		    i++;
		    break;
		  }
	      }
	  }
	
	if (use_polygon_buffer)
	  /* using polygon mode; will now employ polygon buffer to do
	     filling (possibly) and edging */
	  {
	    if (!closed)
	      /* polyline is open, so lift pen and exit polygon mode */
	      {
		strcpy (_plotter->data->page->point, "PU;");
		_update_buffer (_plotter->data->page);
		_plotter->hpgl_pendown = false;
		strcpy (_plotter->data->page->point, "PM2;");
		_update_buffer (_plotter->data->page);
	      }
	    else
	      /* polyline is closed, so exit polygon mode and then lift pen */
	      {
		strcpy (_plotter->data->page->point, "PM2;");
		_update_buffer (_plotter->data->page);
		strcpy (_plotter->data->page->point, "PU;");
		_update_buffer (_plotter->data->page);
		_plotter->hpgl_pendown = false;
	      }
	    
	    if (_plotter->drawstate->fill_type)
	      /* polyline should be filled */
	      {
		/* Sync fill color.  This may set the
		   _plotter->hpgl_bad_pen flag (if optimal pen is #0
		   [white] and we're not allowed to use pen #0 to draw
		   with).  So we test _plotter->hpgl_bad_pen before using
		   the pen to fill with. */
		_pl_h_set_fill_color (R___(_plotter) false);
		if (_plotter->hpgl_bad_pen == false)
		  /* fill polyline, specifying nonzero winding rule if
		     necessary */
		  {
		    switch (_plotter->drawstate->fill_rule_type)
		      {
		      case PL_FILL_ODD_WINDING:
		      default:
			strcpy (_plotter->data->page->point, "FP;");
			break;
		      case PL_FILL_NONZERO_WINDING:		  
			if (_plotter->hpgl_version == 2)
			  strcpy (_plotter->data->page->point, "FP1;");
			else	/* pre-HP-GL/2 doesn't support nonzero rule */
			  strcpy (_plotter->data->page->point, "FP;");
			break;
		      }
		    _update_buffer (_plotter->data->page);
		  }
	    /* KLUDGE: in pre-HP-GL/2, our `set_fill_color' function may
	       alter the line type, since it may request *solid*
	       crosshatching; so reset the line type */
		if (_plotter->hpgl_version < 2)
		  _pl_h_set_attributes (S___(_plotter));
	      }
	    
	    if (_plotter->drawstate->pen_type)
	      /* polyline should be edged */
	      {
		/* Sync pen color.  This may set the _plotter->hpgl_bad_pen
		   flag (if optimal pen is #0 and we're not allowed to use
		   pen #0 to draw with).  So we test _plotter->hpgl_bad_pen
		   before using the pen. */
		_pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);
		if (_plotter->hpgl_bad_pen == false)
		  /* select appropriate pen for edging, and edge the
                     polyline */
		  {
		    _pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);
		    strcpy (_plotter->data->page->point, "EP;");
		    _update_buffer (_plotter->data->page);
		  }
	      }
	  }
	
	/* We know where the pen now is: if we used a polygon buffer, then
	   _plotter->hpgl_pos is now xarray[0].p.  If we didn't (as would
	   be the case if we're outputting generic HP-GL), then
	   _plotter->hpgl_pos is now xarray[polyline_len - 1].p.
	   Unfortunately we can't simply update _plotter->hpgl_pos, because
	   we want the generated HP-GL[/2] code to work properly on both
	   HP-GL and HP-GL/2 devices.  So we punt. */
	_plotter->hpgl_position_is_unknown = true;
	
	/* free integer storage buffer and depart */
	free (xarray);
      }
      break;
	
    case (int)PATH_BOX:
      {
	plPoint p0, p1, savedpoint;

	p0 = _plotter->drawstate->path->p0;
	p1 = _plotter->drawstate->path->p1;

	/* sync line attributes, incl. pen width */
	_pl_h_set_attributes (S___(_plotter));

	/* move HP-GL pen to first vertex */
	savedpoint = _plotter->drawstate->pos;
	_plotter->drawstate->pos = p0;
	_pl_h_set_position (S___(_plotter));
	_plotter->drawstate->pos = savedpoint;
	
	if (_plotter->drawstate->fill_type)
	  /* rectangle should be filled */
	  {
	    /* Sync fill color.  This may set the _plotter->hpgl_bad_pen
	       flag (e.g. if optimal pen is #0 [white] and we're not
	       allowed to use pen #0 to draw with).  So we test
	       _plotter->hpgl_bad_pen before using the pen. */
	    _pl_h_set_fill_color (R___(_plotter) false);
	    if (_plotter->hpgl_bad_pen == false)
	      /* fill the rectangle */
	      {
		sprintf (_plotter->data->page->point, "RA%d,%d;", 
			 IROUND(XD(p1.x,p1.y)), IROUND(YD(p1.x,p1.y)));
		_update_buffer (_plotter->data->page);
	      }
	    /* KLUDGE: in pre-HP-GL/2, our `set_fill_color' function may
	       alter the line type, since it may request *solid*
	       crosshatching; so reset it */
	    if (_plotter->hpgl_version < 2)
	      _pl_h_set_attributes (S___(_plotter));
	  }	  
	
	if (_plotter->drawstate->pen_type)
	  /* rectangle should be edged */
	  {
	    /* Sync pen color.  This may set the _plotter->hpgl_bad_pen
	       flag (e.g. if optimal pen is #0 [white] and we're not
	       allowed to use pen #0 to draw with).  So we test
	       _plotter->hpgl_bad_pen before using the pen. */
	    _pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);
	    if (_plotter->hpgl_bad_pen == false)
	      /* edge the rectangle */
	      {
		sprintf (_plotter->data->page->point, "EA%d,%d;", 
			 IROUND(XD(p1.x,p1.y)), IROUND(YD(p1.x,p1.y)));
		_update_buffer (_plotter->data->page);
	      }
	  }
      }
      break;

    case (int)PATH_CIRCLE:
      {
	plPoint pc, savedpoint;
	double r = _plotter->drawstate->path->radius;
	double radius = sqrt(XDV(r,0)*XDV(r,0)+YDV(r,0)*YDV(r,0));

	pc = _plotter->drawstate->path->pc;
	
	/* sync attributes, incl. pen width; move to center of circle */
	_pl_h_set_attributes (S___(_plotter));

	savedpoint = _plotter->drawstate->pos;
	_plotter->drawstate->pos = pc;
	_pl_h_set_position (S___(_plotter));
	_plotter->drawstate->pos = savedpoint;
	
	if (_plotter->drawstate->fill_type)
	  /* circle should be filled */
	  {
	    /* Sync fill color.  This may set the _plotter->hpgl_bad_pen
	       flag (e.g. if optimal pen is #0 [white] and we're not
	       allowed to use pen #0 to draw with).  So we test
	       _plotter->hpgl_bad_pen before using the pen. */
	    _pl_h_set_fill_color (R___(_plotter) false);
	    if (_plotter->hpgl_bad_pen == false)
	      /* fill the circle (360 degree wedge) */
	      {
		sprintf (_plotter->data->page->point, "WG%d,0,360;", 
			 IROUND(radius));
		_update_buffer (_plotter->data->page);
	      }
	    /* KLUDGE: in pre-HP-GL/2, our `set_fill_color' function may
	       alter the line type, since it may request *solid*
	       crosshatching; so reset it */
	    if (_plotter->hpgl_version < 2)
	      _pl_h_set_attributes (S___(_plotter));
	  }
	
	if (_plotter->drawstate->pen_type)
	  /* circle should be edged */
	  {
	    /* Sync pen color.  This may set the _plotter->hpgl_bad_pen
	       flag (e.g. if optimal pen is #0 [white] and we're not
	       allowed to use pen #0 to draw with).  So we test
	       _plotter->hpgl_bad_pen before using the pen. */
	    _pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);
	    if (_plotter->hpgl_bad_pen == false)
	      /* do the edging */
	      {
		sprintf (_plotter->data->page->point, "CI%d;", IROUND(radius));
		_update_buffer (_plotter->data->page);
	      }
	  }
      }
      break;

    default:			/* unrecognized path type, shouldn't happen */
      break;
    }
}

/* A low-level method for moving the pen position of an HP-GL pen plotter
   to agree with the Plotter's notion of what the graphics cursor position
   should be.  The state of the pen (up vs. down) after calling this
   function is not uniquely determined.  */

void
_pl_h_set_position (S___(Plotter *_plotter))
{
  int xnew, ynew;
  
  /* if plotter's pen position doesn't agree with what it should be,
     adjust it */

  xnew = IROUND(XD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));
  ynew = IROUND(YD(_plotter->drawstate->pos.x, _plotter->drawstate->pos.y));  

  if (_plotter->hpgl_position_is_unknown == true
      || xnew != _plotter->hpgl_pos.x || ynew != _plotter->hpgl_pos.y)
    {
      if (_plotter->hpgl_pendown == true)
	{
	  sprintf (_plotter->data->page->point, "PU;PA%d,%d;", xnew, ynew);
	  _plotter->hpgl_pendown = false;
	}
      else
	sprintf (_plotter->data->page->point, "PA%d,%d;", xnew, ynew);
      _update_buffer (_plotter->data->page);

      /* update our knowledge of pen position */
      _plotter->hpgl_position_is_unknown = false;
      _plotter->hpgl_pos.x = xnew;
      _plotter->hpgl_pos.y = ynew;
    }
}

bool
_pl_h_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
