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

/* This file also contains the internal path_is_flushable() method, which
   is invoked after any path segment is added to the segment list, provided
   (0) the segment list has become greater than or equal to the
   `max_unfilled_path_length' Plotter parameter, (1) the path isn't to be
   filled.  In most Plotters, this operation simply returns true. */

/* This file also contains the internal maybe_prepaint_segments() method.
   It is called immediately after any segment is added to a path.  Some
   Plotters, at least under some circumstances, treat endpath() as a no-op.
   Instead, they plot the segments of a path in real time.  */

/**********************************************************************/

/* This version of paint_path() is for XDrawablePlotters (and XPlotters).
   By construction, for such Plotters our path buffer always contains
   either a segment list, or an ellipse object.  If it's a segment list, it
   contains either (1) a sequence of line segments, or (2) a single
   circular or elliptic arc segment.  Those are all sorts of path that X11
   can handle.  (For an ellipse or circular/elliptic arc segment to have
   been added to the path buffer, the map from user to device coordinates
   must preserve axes.) */

/* If the line style is "solid" and the path has zero width, it's actually
   drawn in real time, before endpath() and paint_path() are called; see
   the maybe_prepaint_segments() method further below in this file.  So if the
   path doesn't need to be filled, we don't do anything in paint_path().
   If it does, we fill it, and then redraw it. */

/* Note that when filling a polyline, we look at
   _plotter->drawstate->path->primitive to determine which X11 rendering
   algorithm to use.  Our default algorithm is "Complex" (i.e. generic),
   but when drawing polygonal approximations to ellipse and rectangle
   primitives, which we know must be convex, we specify "Convex" instead,
   to speed up rendering. */

#include "sys-defines.h"
#include "extern.h"

/* number of XPoint structs we can store on the stack, for speed, without
   invoking malloc */
#define MAX_NUM_POINTS_ON_STACK 128

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

void
_pl_x_paint_path (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	bool closed;		/* not currently used */
	int is_a_rectangle;
	int i, polyline_len;
	plPoint p0, p1, pc;
	XPoint *xarray, local_xarray[MAX_NUM_POINTS_ON_STACK];
	bool heap_storage;
	double xu_last, yu_last;
	bool identical_user_coordinates;
	
	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /*shouldn't happen */
	  break;

	if (_plotter->drawstate->path->num_segments == 2
	    && _plotter->drawstate->path->segments[1].type == S_ARC)
	  /* segment buffer contains a single circular arc, not a polyline */
	  {
	    p0 = _plotter->drawstate->path->segments[0].p;
	    p1 = _plotter->drawstate->path->segments[1].p;
	    pc = _plotter->drawstate->path->segments[1].pc;
	    
	    /* use native X rendering to draw the (transformed) circular
               arc */
	    _pl_x_draw_elliptic_arc (R___(_plotter) p0, p1, pc);

	    break;
	  }

	if (_plotter->drawstate->path->num_segments == 2
	    && _plotter->drawstate->path->segments[1].type == S_ELLARC)
	  /* segment buffer contains a single elliptic arc, not a polyline */
	  {
	    p0 = _plotter->drawstate->path->segments[0].p;
	    p1 = _plotter->drawstate->path->segments[1].p;
	    pc = _plotter->drawstate->path->segments[1].pc;
	    
	    /* use native X rendering to draw the (transformed) elliptic
               arc */
	    _pl_x_draw_elliptic_arc_2 (R___(_plotter) p0, p1, pc);
	    
	    break;
	  }

	/* neither of above applied, so segment buffer contains a polyline,
	   not an arc */

	if ((_plotter->drawstate->path->num_segments >= 3)/*check for closure*/
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.x == _plotter->drawstate->path->segments[0].p.x)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.y == _plotter->drawstate->path->segments[0].p.y))
	  closed = true;
	else
	  closed = false;	/* 2-point ones should be open */
  
	/* Check whether we `pre-drew' the polyline, i.e., drew every line
	   segment in real time.  (See the maybe_prepaint_segments() method
	   further below, which is invoked to do that.)  Our convention: we
	   pre-draw only if pen width is zero, and line style is "solid".
	   Also, we don't do it if we're drawing a polygonalized built-in
	   object (i.e. a rectangle or ellipse).

	   If we pre-drew, we don't do anything here unless there's filling
	   to be done.  If so, we'll fill the polyline and re-edge it.  */

	if ((_plotter->drawstate->pen_type != 0 /* pen is present */
	     && _plotter->drawstate->line_type == PL_L_SOLID
	     && !_plotter->drawstate->dash_array_in_effect /* really solid */
	     && _plotter->drawstate->points_are_connected /* really, really */
	     && _plotter->drawstate->quantized_device_line_width == 0
	     && !_plotter->drawstate->path->primitive) /* not builtin object */
	  /* we pre-drew */
	    &&
	    _plotter->drawstate->fill_type == 0)
	  /* there's no filling to be done, so we're out of here */
	  break;

	/* At this point we know that we didn't pre-draw, or we did, but
	   the polyline was drawn unfilled and it'll need to be re-drawn as
	   filled. */

	/* prepare an array of XPoint structures (X11 uses short ints for
	   these) */
	if (_plotter->drawstate->path->num_segments 
	    <= MAX_NUM_POINTS_ON_STACK)
	  /* store XPoints on stack, for speed */
	  {
	    xarray = local_xarray;
	    heap_storage = false;
	  }
	else
	  /* store XPoints in heap */
	  {
	    xarray = (XPoint *)_pl_xmalloc (_plotter->drawstate->path->num_segments * sizeof(XPoint));
	    heap_storage = true;
	  }
	
	/* convert vertices to device coordinates, removing runs; also keep
	   track of whether or not all points in user space are the same */
	
	polyline_len = 0;
	xu_last = 0.0;
	yu_last = 0.0;
	identical_user_coordinates = true;
	for (i = 0; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    plPathSegment datapoint;
	    double xu, yu, xd, yd;
	    int device_x, device_y;
	    
	    datapoint = _plotter->drawstate->path->segments[i];
	    xu = datapoint.p.x;
	    yu = datapoint.p.y;
	    xd = XD(xu, yu);
	    yd = YD(xu, yu);
	    device_x = IROUND(xd);
	    device_y = IROUND(yd);
	    
	    if (X_OOB_INT(device_x) || X_OOB_INT(device_y))
	      /* point position can't be represented using X11's 2-byte
		 ints, so truncate the polyline right here */
	      {
		_plotter->warning (R___(_plotter) 
				   "truncating a polyline that extends too far for X11");
		break;
	      }
	    
	    if (i > 0 && (xu != xu_last || yu != yu_last))
	      /* in user space, not all points are the same */
	      identical_user_coordinates = false;	
	    
	    if ((polyline_len == 0) 
		|| (device_x != xarray[polyline_len-1].x) 
		|| (device_y != xarray[polyline_len-1].y))
	      /* add point, in integer X coordinates, to the array */
	      {
		xarray[polyline_len].x = device_x;
		xarray[polyline_len].y = device_y;
		polyline_len++;
		
		if (polyline_len >= _plotter->x_max_polyline_len)
		  /* polyline is getting too long for the X server to
		     handle (we determined the maximum X request size when
		     openpl() was invoked), so truncate it right here */
		  {
		    _plotter->warning (R___(_plotter) 
				       "truncating a polyline that's too long for the X display");
		    break;
		  }
	      }
	    
	    xu_last = xu;
	    yu_last = yu;
	  }
	
	/* Is this path a rectangle in device space?  We check this because
	   by calling XFillRectangle (and XDrawRectangle too, if the edging
	   is solid), we can save a bit of time, or at least network
	   bandwidth. */

	/* N.B. This checks only for rectangles traced counterclockwise
	   from the lower left corner.  Should improve this. */

#define IS_A_RECTANGLE(len,q) \
((len) == 5 && (q)[0].x == (q)[4].x && (q)[0].y == (q)[4].y && \
(q)[0].x == (q)[3].x && (q)[1].x == (q)[2].x && \
(q)[0].y == (q)[1].y && (q)[2].y == (q)[3].y && \
(q)[0].x < (q)[1].x && (q)[0].y > (q)[2].y) /* note flipped-y convention */

	is_a_rectangle = IS_A_RECTANGLE(polyline_len, xarray);
  
	/* N.B.  If a rectangle, upper left corner = q[3], and also, width
	   = q[1].x - q[0].x and height = q[0].y - q[2].y (note flipped-y
	   convention). */

	/* compute the square size, and offset of upper left vertex from
	   center of square, that we'll use when handling the notorious
	   special case: all user-space points in the polyline get mapped
	   to a single integer X pixel */

	/* FIRST TASK: fill the polygon (if necessary). */
	
	if (_plotter->drawstate->fill_type) 
	  /* not transparent, so fill the path */
	  {
	    int x_polygon_type 
	      = (_plotter->drawstate->path->primitive ? Convex : Complex);
	    
	    /* update GC used for filling */
	    _pl_x_set_attributes (R___(_plotter) X_GC_FOR_FILLING);
	    
	    /* select fill color as foreground color in GC used for filling */
	    _pl_x_set_fill_color (S___(_plotter));
		    
	    if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	      {
		if (_plotter->drawstate->path->num_segments > 1 
		      && polyline_len == 1)
		  /* special case: all user-space points in the polyline
		     were mapped to a single integer X pixel */
		  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3,
			      _plotter->drawstate->x_gc_fill,
			      (int)(xarray[0].x), (int)(xarray[0].y));
		else
		  /* general case */
		  {
		    if (is_a_rectangle)
		      /* call XFillRectangle, for speed */
		      XFillRectangle (_plotter->x_dpy, _plotter->x_drawable3,
				      _plotter->drawstate->x_gc_fill,
				      (int)(xarray[3].x), (int)(xarray[3].y),
				      (unsigned int)(xarray[1].x - xarray[0].x),
				      /* flipped y */
				      (unsigned int)(xarray[0].y - xarray[2].y));
		    else
		      /* not a rectangle, call XFillPolygon */
		      XFillPolygon (_plotter->x_dpy, _plotter->x_drawable3,
				    _plotter->drawstate->x_gc_fill,
				    xarray, polyline_len,
				    x_polygon_type, CoordModeOrigin);
		  }
	      }
	    else		/* not double buffering, no `x_drawable3' */
	      {
		if (_plotter->drawstate->path->num_segments > 1 
		      && polyline_len == 1)
		  /* special case: all user-space points in the polyline
		     were mapped to a single integer X pixel. */
		  {
		    if (_plotter->x_drawable1)
		      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1,
				  _plotter->drawstate->x_gc_fill,
				  (int)(xarray[0].x), (int)(xarray[0].y));
		    if (_plotter->x_drawable2)
		      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2,
				  _plotter->drawstate->x_gc_fill,
				  (int)(xarray[0].x), (int)(xarray[0].y));
		  }
		else
		  /* general case */
		  {
		    if (is_a_rectangle)
		      /* call XFillRectangle, for speed */
		      {
			if (_plotter->x_drawable1)
			  XFillRectangle (_plotter->x_dpy, _plotter->x_drawable1, 
					  _plotter->drawstate->x_gc_fill,
					  (int)(xarray[3].x), (int)(xarray[3].y),
					  (unsigned int)(xarray[1].x - xarray[0].x),
					  /* flipped y */
					  (unsigned int)(xarray[0].y - xarray[2].y));
			if (_plotter->x_drawable2)
			  XFillRectangle (_plotter->x_dpy, _plotter->x_drawable2, 
					  _plotter->drawstate->x_gc_fill,
					  (int)(xarray[3].x), (int)(xarray[3].y),
					  (unsigned int)(xarray[1].x - xarray[0].x),
					  /* flipped y */
					  (unsigned int)(xarray[0].y - xarray[2].y));
		      }
		    else
		      /* not a rectangle, call XFillPolygon */
		      {
			if (_plotter->x_drawable1)
			  XFillPolygon (_plotter->x_dpy, _plotter->x_drawable1, 
					_plotter->drawstate->x_gc_fill,
					xarray, polyline_len,
					x_polygon_type, CoordModeOrigin);
			if (_plotter->x_drawable2)
			  XFillPolygon (_plotter->x_dpy, _plotter->x_drawable2, 
					_plotter->drawstate->x_gc_fill,
					xarray, polyline_len,
					x_polygon_type, CoordModeOrigin);
		      }
		  }
	      }
	  }
	
	/* SECOND TASK: edge the polygon (if necessary). */
	
	if (_plotter->drawstate->pen_type) 
	  /* pen is present, so edge the path */
	  {
	    int xloc = 0, yloc = 0;
	    unsigned int sp_size = 1;

	    /* update GC used for drawing */
	    _pl_x_set_attributes (R___(_plotter) X_GC_FOR_DRAWING);
	    
	    /* select pen color as foreground color in GC used for drawing */
	    _pl_x_set_pen_color (S___(_plotter));
	    
	    /* Check first for the special case: all points in the polyline
	       were mapped to a single integer X pixel.  If (1) they
	       weren't all the same to begin with, or (2) they were all the
	       same to begin with and the cap mode is "round", then draw as
	       a filled circle of diameter equal to the line width;
	       otherwise draw nothing.  (If the circle would have diameter
	       1 or less, we draw a point instead.) */
      
	    if (_plotter->drawstate->path->num_segments > 1 
		&& polyline_len == 1)
	      /* this is the special case, so compute quantities needed for
		 drawing the filled circle */
	      {
		int sp_offset;

		sp_size = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
		if (sp_size == 0) 
		  sp_size = 1;
		sp_offset = (_plotter->drawstate->quantized_device_line_width + 1) / 2;
		xloc = xarray[0].x - sp_offset;
		yloc = xarray[0].y - sp_offset;
	      }

	    if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	      /* double buffering, have a `x_drawable3' to draw into */
	      {
		if (_plotter->drawstate->path->num_segments > 1 
		    && polyline_len == 1)
		  /* special case */
		  {
		    if (identical_user_coordinates == false
			|| _plotter->drawstate->cap_type == PL_CAP_ROUND)
		      {
			if (sp_size == 1)
			  /* subcase: just draw a point */
			  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
				      _plotter->drawstate->x_gc_fg, 
				      (int)(xarray[0].x), (int)(xarray[0].y));
			else
			  /* draw filled circle */
			  XFillArc(_plotter->x_dpy, _plotter->x_drawable3,
				   _plotter->drawstate->x_gc_fg, 
				   xloc, yloc, sp_size, sp_size,
				   0, 64 * 360);
		      }
		  }
		else
		  /* general case */
		  /* NOTE: this code is what libplot uses to draw nearly all
		     polylines, in the case when double buffering is used */
		  {
		    if (is_a_rectangle
			&& _plotter->drawstate->dash_array_in_effect == false
			&& _plotter->drawstate->line_type == PL_L_SOLID)
		      /* call XDrawRectangle, for speed */
		      XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable3, 
				      _plotter->drawstate->x_gc_fg, 
				      (int)(xarray[3].x), (int)(xarray[3].y),
				      (unsigned int)(xarray[1].x - xarray[0].x),
				      /* flipped y */
				      (unsigned int)(xarray[0].y - xarray[2].y));
		    else
		      /* can't call XDrawRectangle */
		      XDrawLines (_plotter->x_dpy, _plotter->x_drawable3, 
				  _plotter->drawstate->x_gc_fg, 
				  xarray, polyline_len,
				  CoordModeOrigin);
		  }
	      }
	    else
	      /* not double buffering, have no `x_drawable3' */
	      {
		if (_plotter->drawstate->path->num_segments > 1 
		    && polyline_len == 1)
		  /* special case */
		  {
		    if (identical_user_coordinates == false
			|| _plotter->drawstate->cap_type == PL_CAP_ROUND)
		      {
			if (sp_size == 1)
			  /* subcase: just draw a point */
			  {
			    if (_plotter->x_drawable1)
			      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1,
					  _plotter->drawstate->x_gc_fg, 
					  (int)(xarray[0].x), (int)(xarray[0].y));
			    if (_plotter->x_drawable2)
			      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2,
					  _plotter->drawstate->x_gc_fg, 
					  (int)(xarray[0].x), (int)(xarray[0].y));
			  }
			else
			  /* draw filled circle */
			  {
			    if (_plotter->x_drawable1)
			      XFillArc(_plotter->x_dpy, _plotter->x_drawable1,
				       _plotter->drawstate->x_gc_fg, 
				       xloc, yloc, sp_size, sp_size,
				       0, 64 * 360);
			    if (_plotter->x_drawable2)
			      XFillArc(_plotter->x_dpy, _plotter->x_drawable2,
				       _plotter->drawstate->x_gc_fg, 
				       xloc, yloc, sp_size, sp_size,
				       0, 64 * 360);
			}
		      }
		  }
		else
		  /* general case */
		  /* NOTE: this code is what libplot uses to draw nearly all
		     polylines; at least, if double buffering is not used */
		  {
		    if (is_a_rectangle
			&& _plotter->drawstate->dash_array_in_effect == false
			&& _plotter->drawstate->line_type == PL_L_SOLID)
		      /* call XDrawRectangle, for speed */
		      {
			if (_plotter->x_drawable1)
			  XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable1, 
					  _plotter->drawstate->x_gc_fg, 
					  (int)(xarray[3].x), (int)(xarray[3].y),
					  (unsigned int)(xarray[1].x - xarray[0].x),
					  /* flipped y */
					  (unsigned int)(xarray[0].y - xarray[2].y));
			if (_plotter->x_drawable2)
			  XDrawRectangle (_plotter->x_dpy, _plotter->x_drawable2, 
					  _plotter->drawstate->x_gc_fg, 
					  (int)(xarray[3].x), (int)(xarray[3].y),
					  (unsigned int)(xarray[1].x - xarray[0].x),
					  /* flipped y */
					  (unsigned int)(xarray[0].y - xarray[2].y));
		      }
		    else
		      /* can't use XDrawRectangle() */
		      {
			if (_plotter->x_drawable1)
			  XDrawLines (_plotter->x_dpy, _plotter->x_drawable1, 
				      _plotter->drawstate->x_gc_fg, 
				      xarray, polyline_len,
				      CoordModeOrigin);
			if (_plotter->x_drawable2)
			  XDrawLines (_plotter->x_dpy, _plotter->x_drawable2, 
				      _plotter->drawstate->x_gc_fg, 
				      xarray, polyline_len,
				      CoordModeOrigin);
		      }
		  }
	      }
	  }
	
	/* reset buffer used for array of XPoint structs */
	if (_plotter->drawstate->path->num_segments > 0)
	  {
	    if (heap_storage)
	      free (xarray);		/* free malloc'd array of XPoints */
	  }
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	int ninetymult;
	int x_orientation, y_orientation;
	int xorigin, yorigin;
	unsigned int squaresize_x, squaresize_y;
	plPoint pc;
	double rx, ry, angle;

	pc = _plotter->drawstate->path->pc;
	rx = _plotter->drawstate->path->rx;
	ry = _plotter->drawstate->path->ry;
	angle = _plotter->drawstate->path->angle;	

	/* if angle is multiple of 90 degrees, modify to permit use of
	   X11 arc rendering */
	ninetymult = IROUND(angle / 90.0);
	if (angle == (double) (90 * ninetymult))
	  {
	    angle = 0.0;
	    if (ninetymult % 2)
	      {
		double temp;
		
		temp = rx;
		rx = ry;
		ry = temp;
	      }
	  }
	
	rx = (rx < 0.0 ? -rx : rx);	/* avoid obscure libxmi problems */
	ry = (ry < 0.0 ? -ry : ry);  
	
	/* axes flipped? (by default y-axis is, due to libxmi's flipped-y
           convention) */
	x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
	y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);
	
	/* location of `origin' (upper left corner of bounding rect. for
	   ellipse) and width and height; X11's flipped-y convention
	   affects these values */
	xorigin = IROUND(XD(pc.x - x_orientation * rx, 
			    pc.y - y_orientation * ry));
	yorigin = IROUND(YD(pc.x - x_orientation * rx, 
			    pc.y - y_orientation * ry));
	squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
	squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));  
	/* Because this ellipse object was added to the path buffer, we
	   already know that (1) the user->device frame map preserves
	   coordinate axes, (2) effectively, angle == 0.  These are
	   necessary for the libxmi scan-conversion module to do the
	   drawing. */

	/* draw ellipse (elliptic arc aligned with the coordinate axes, arc
	   range = 64*360 64'ths of a degree) */
	_pl_x_draw_elliptic_arc_internal (R___(_plotter) 
				       xorigin, yorigin, 
				       squaresize_x, squaresize_y, 
				       0, 64 * 360);
      }
      break;

    default:			/* shouldn't happen */
      break;
    }

  /* maybe flush X output buffer and handle X events (a no-op for
     XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));
}
  
/* Use native X rendering to draw what would be a circular arc in the user
   frame on an X display.  If this is called, the map from user to device
   coordinates is assumed to preserve coordinate axes (it may be
   anisotropic [x and y directions scaled differently], and it may include
   a reflection through either or both axes).  So it will be a circular or
   elliptic arc in the device frame, of the sort that X11 supports. */

void
_pl_x_draw_elliptic_arc (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
{
  double radius;
  double theta0, theta1;
  int startangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  /* radius of circular arc in user frame is distance to p0, and also to p1 */
  radius = DIST(pc, p0);

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  yorigin = IROUND(YD(pc.x - x_orientation * radius, 
		      pc.y - y_orientation * radius));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * radius, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * radius));

  theta0 = _xatan2 (-y_orientation * (p0.y - pc.y), 
		    x_orientation * (p0.x - pc.x)) / M_PI;
  theta1 = _xatan2 (-y_orientation * (p1.y - pc.y), 
		    x_orientation * (p1.x - pc.x)) / M_PI;

  if (theta1 < theta0)
    theta1 += 2.0;		/* adjust so that difference > 0 */
  if (theta0 < 0.0)
    {
      theta0 += 2.0;		/* adjust so that startangle > 0 */
      theta1 += 2.0;
    }

  if (theta1 - theta0 > 1.0)	/* swap if angle appear to be > 180 degrees */
    {
      double tmp;
      
      tmp = theta0;
      theta0 = theta1;
      theta1 = tmp;
      theta1 += 2.0;		/* adjust so that difference > 0 */      
    }

  if (theta0 >= 2.0 && theta1 >= 2.0)
    /* avoid obscure X bug */
    {
      theta0 -= 2.0;
      theta1 -= 2.0;
    }

  startangle = IROUND(64 * theta0 * 180.0); /* in 64'ths of a degree */
  anglerange = IROUND(64 * (theta1 - theta0) * 180.0); /* likewise */

  _pl_x_draw_elliptic_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
}

/* Use native X rendering to draw what would be a quarter-ellipse in the
   user frame on an X display.  If this is called, the map from user to
   device coordinates is assumed to preserve coordinate axes (it may be
   anisotropic [x and y directions scaled differently], and it may include
   a reflection through either or both axes).  So it will be a
   quarter-ellipse in the device frame, of the sort that the X11 drawing
   protocol supports. */
void
_pl_x_draw_elliptic_arc_2 (R___(Plotter *_plotter) plPoint p0, plPoint p1, plPoint pc)
{
  double rx, ry;
  double x0, y0, x1, y1, xc, yc;
  int startangle, endangle, anglerange;
  int x_orientation, y_orientation;
  int xorigin, yorigin;
  unsigned int squaresize_x, squaresize_y;

  /* axes flipped? (by default y-axis is, due to  X's flipped-y convention) */
  x_orientation = (_plotter->drawstate->transform.m[0] >= 0 ? 1 : -1);
  y_orientation = (_plotter->drawstate->transform.m[3] >= 0 ? 1 : -1);

  xc = pc.x, yc = pc.y;
  x0 = p0.x, y0 = p0.y;
  x1 = p1.x, y1 = p1.y;

  if (y0 == yc && x1 == xc)
    /* initial pt. on x-axis, final pt. on y-axis */
    {
      /* semi-axes in user frame */
      rx = (x0 > xc) ? x0 - xc : xc - x0;
      ry = (y1 > yc) ? y1 - yc : yc - y1;
      /* starting and ending angles; note flipped-y convention */
      startangle = ((x0 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
      endangle = ((y1 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
    }
  else
    /* initial pt. on y-axis, final pt. on x-axis */
    {	
      /* semi-axes in user frame */
      rx = (x1 > xc) ? x1 - xc : xc - x1;
      ry = (y0 > yc) ? y0 - yc : yc - y0;
      /* starting and ending angles; note flipped-y convention */
      startangle = ((y0 > yc ? 1 : -1) * y_orientation == -1) ? 90 : 270;
      endangle = ((x1 > xc ? 1 : -1) * x_orientation == 1) ? 0 : 180;
    }	  

  if (endangle < startangle)
    endangle += 360;
  anglerange = endangle - startangle; /* always 90 or 270 */

  /* our convention: a quarter-ellipse can only be 90 degrees
     of an X ellipse, not 270 degrees, so interchange points */
  if (anglerange == 270)
    {
      int tmp;

      tmp = startangle;
      startangle = endangle;
      endangle = tmp;
      anglerange = 90;
    }
      
  if (startangle >= 360)
    /* avoid obscure X bug */
    startangle -= 360;		/* endangle no longer relevant */

  /* location of `origin' (upper left corner of bounding rect. on display)
     and width and height; X's flipped-y convention affects these values */
  xorigin = IROUND(XD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  yorigin = IROUND(YD(xc - x_orientation * rx, 
		      yc - y_orientation * ry));
  squaresize_x = (unsigned int)IROUND(XDV(2 * x_orientation * rx, 0.0));
  squaresize_y = (unsigned int)IROUND(YDV(0.0, 2 * y_orientation * ry));
      
  /* reexpress in 64'ths of a degree (X11 convention) */
  startangle *= 64;
  anglerange *= 64;

  _pl_x_draw_elliptic_arc_internal (R___(_plotter)
				 xorigin, yorigin, 
				 squaresize_x, squaresize_y, 
				 startangle, anglerange);
}

/* Use native X rendering to draw an elliptic arc on an X display.  Takes
   account of the possible presence of more than one drawable, and the
   possible need for filling.

   The cases squaresize_{x,y} <= 1 are handled specially, since XFillArc()
   and XDrawArc() don't support them in the way we wish.  More accurately,
   they don't support squaresize_{x,y} = 0 (documented), and don't support
   squaresize_{x,y} = 1 in the way we'd like (undocumented). */

void
_pl_x_draw_elliptic_arc_internal (R___(Plotter *_plotter) int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange)
{
  if (X_OOB_INT(xorigin) || X_OOB_INT(yorigin) || X_OOB_UNSIGNED(squaresize_x)
      || X_OOB_UNSIGNED(squaresize_y))
    /* dimensions can't be represented using X11's 2-byte ints, so punt */
    {
      _plotter->warning (R___(_plotter) 
			 "not drawing an arc that extends too far for X11");
      return;
    }

  if (_plotter->drawstate->fill_type)
    /* not transparent, so fill the arc */
    {
      /* update GC used for filling */
      _pl_x_set_attributes (R___(_plotter) X_GC_FOR_FILLING);

      /* select fill color as foreground color in GC used for filling */
      _pl_x_set_fill_color (S___(_plotter));

      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* a special case, which XFillArc() doesn't handle in the way we'd
	   like; just paint a single pixel, irrespective of angle range */
	{
	  if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
			_plotter->drawstate->x_gc_fill, 
			xorigin, yorigin);
	  else
	    {
	      if (_plotter->x_drawable1)
		XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1, 
			    _plotter->drawstate->x_gc_fill, 
			    xorigin, yorigin);
	      if (_plotter->x_drawable2)
		XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2, 
			    _plotter->drawstate->x_gc_fill, 
			    xorigin, yorigin);
	    }
	}
      else
	/* default case, almost always used */
	{
	  if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	    XFillArc(_plotter->x_dpy, _plotter->x_drawable3, 
		     _plotter->drawstate->x_gc_fill, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	  else
	    {
	      if (_plotter->x_drawable1)
		XFillArc(_plotter->x_dpy, _plotter->x_drawable1, 
			 _plotter->drawstate->x_gc_fill, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 startangle, anglerange);
	      if (_plotter->x_drawable2)
		XFillArc(_plotter->x_dpy, _plotter->x_drawable2, 
			 _plotter->drawstate->x_gc_fill, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 startangle, anglerange);
	    }
	}
    }
  
  if (_plotter->drawstate->pen_type)
    /* pen is present, so edge the arc */
    {
      unsigned int sp_size = 0;	/* keep compiler happy */

      /* update GC used for drawing */
      _pl_x_set_attributes (R___(_plotter) X_GC_FOR_DRAWING);
      
      /* select pen color as foreground color in GC used for drawing */
      _pl_x_set_pen_color (S___(_plotter));
      
      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* Won't call XDrawArc in the usual way, because it performs poorly
           when one of these two is zero, at least.  Irrespective of angle
           range, will fill a disk of diameter equal to line width */
	{
	  int sp_offset;

	  sp_size 
	    = (unsigned int)_plotter->drawstate->quantized_device_line_width; 
	  sp_offset
	    = (int)(_plotter->drawstate->quantized_device_line_width + 1) / 2;
	  
	  if (sp_size == 0) 
	    sp_size = 1;
	  xorigin -= sp_offset;
	  yorigin -= sp_offset;	  
	}

      if (squaresize_x <= 1 || squaresize_y <= 1)
	/* special case */
	{
	  if (sp_size == 1)
	    /* special subcase: line width is small too, so just paint a
	       single pixel rather than filling abovementioned disk */
	    {
	      if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
		XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3, 
			    _plotter->drawstate->x_gc_fg, 
			    xorigin, yorigin);
	      else
		{
		  if (_plotter->x_drawable1)
		    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1, 
				_plotter->drawstate->x_gc_fg, 
				xorigin, yorigin);
		  if (_plotter->x_drawable2)
		    XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2, 
				_plotter->drawstate->x_gc_fg, 
				xorigin, yorigin);
		}
	    }
	  else
	    /* normal version of special case: fill a disk of diameter
	       equal to line width */
	    {
	      if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
		XFillArc(_plotter->x_dpy, _plotter->x_drawable3, 
			 _plotter->drawstate->x_gc_fg, 
			 xorigin, yorigin, sp_size, sp_size,
			 0, 64 * 360);
	      else
		{
		  if (_plotter->x_drawable1)
		    XFillArc(_plotter->x_dpy, _plotter->x_drawable1, 
			     _plotter->drawstate->x_gc_fg, 
			     xorigin, yorigin, sp_size, sp_size,
			     0, 64 * 360);
		  if (_plotter->x_drawable2)
		    XFillArc(_plotter->x_dpy, _plotter->x_drawable2, 
			     _plotter->drawstate->x_gc_fg, 
			     xorigin, yorigin, sp_size, sp_size,
			     0, 64 * 360);
		}
	    }
	}
      else
	/* default case, which is what is almost always used */
	{
	  if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	    XDrawArc(_plotter->x_dpy, _plotter->x_drawable3, 
		     _plotter->drawstate->x_gc_fg, 
		     xorigin, yorigin, squaresize_x, squaresize_y,
		     startangle, anglerange);
	  else
	    {
	      if (_plotter->x_drawable1)
		XDrawArc(_plotter->x_dpy, _plotter->x_drawable1, 
			 _plotter->drawstate->x_gc_fg, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 startangle, anglerange);
	      if (_plotter->x_drawable2)
		XDrawArc(_plotter->x_dpy, _plotter->x_drawable2, 
			 _plotter->drawstate->x_gc_fg, 
			 xorigin, yorigin, squaresize_x, squaresize_y,
			 startangle, anglerange);
	    }
	}
    }
}

/**********************************************************************/

/* This version of the internal method path_is_flushable() is for XDrawable
   and X Plotters, which will under some circumstances `pre-draw', i.e.,
   draw line segments in real time.  (This requires a zero line width and a
   "solid" line style.).  In that case, this returns false; otherwise it
   will return true. */

bool
_pl_x_path_is_flushable (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type != 0 /* pen is present */
      && _plotter->drawstate->line_type == PL_L_SOLID
      && !_plotter->drawstate->dash_array_in_effect /* really solid */
      && _plotter->drawstate->points_are_connected /* really, really */
      && _plotter->drawstate->quantized_device_line_width == 0
      && !_plotter->drawstate->path->primitive) /* not a builtin */
    /* we're pre-drawing rather than drawing when endpath() is finally
       invoked, so flushing out a partially drawn path by invoking
       endpath() early would be absurd */
    return false;
  else
    /* endpath() will be invoked */
    return true;
}

/**********************************************************************/

/* This version of the internal method maybe_prepaint_segments() is for
   XDrawable and X Plotters.  It will draw line segments in real time if
   the line width is zero, the line style is "solid", and there's no
   filling to be done.  Also, it requires that the polyline being drawn be
   unfilled, and not be one one of the polygonalized convex closed
   primitives (box/circle/ellipse).

   This hack makes it possible, after doing `graph -TX' (which has a
   default line width of zero), to type in points manually, and see the
   corresponding polyline drawn in real time.  The `-x' and `-y' options
   must of course be specified too, to set the axis limits in advance. */

void
_pl_x_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
{
  int i;
  bool something_drawn = false;

  /* sanity check */
  if (_plotter->drawstate->path->num_segments < 2)
    return;

  if (_plotter->drawstate->path->num_segments == prev_num_segments)
    /* nothing to paint */
    return;

  /* Our criteria for pre-drawing line segments: zero-width solid line, and
     we're not drawing this line segment as part of a polygonalized
     built-in object (i.e. a rectangles or ellipse).  If the criteria
     aren't met, we wait until endpath() is invoked, or in general until
     the path is flushed out, before painting it.

     If we pre-draw, we don't also draw when endpath() is invoked.  One
     exception: if the polyline is to be filled.  In that case, at endpath
     time, we'll fill it and re-edge it. */

  if (!(_plotter->drawstate->pen_type != 0 /* pen is present */
	&& _plotter->drawstate->line_type == PL_L_SOLID
	&& !_plotter->drawstate->dash_array_in_effect /* really solid */
	&& _plotter->drawstate->points_are_connected /* really, really */
	&& _plotter->drawstate->quantized_device_line_width == 0
	&& !_plotter->drawstate->path->primitive)) /* not a built-in object */
    /* we're not pre-drawing */
    return;

  /* An X/XDrawable Plotter's segment list, at painting time, will only
     contain a polyline (i.e. a sequence of line segments) or a single
     circular or elliptic arc.  That's because X/XDrawable Plotters can't
     handle `mixed paths'.

     Since they can't handle mixed paths, any single arc that's placed in a
     previously empty segment list will need to be replaced by a polygonal
     approximation, when and if additional segments need to be added.  The
     maybe_replace_arc() function, which is invoked in the base Plotter
     code in several places, takes care of that.

     Because of this replacement procedure, maybe_prepaint_segments() may
     be invoked on a segment list that consists of a single moveto-arc or
     moveto-ellarc pair.  We don't prepaint such things.  Of course if the
     arc is subsequently replaced by a polygonal approximation, then we'll
     prepaint the polygonal approximation, at that time. */

  if (prev_num_segments == 0 && 
      _plotter->drawstate->path->num_segments == 2
      && _plotter->drawstate->path->segments[0].type == S_MOVETO
      && (_plotter->drawstate->path->segments[1].type == S_ARC
	  || _plotter->drawstate->path->segments[1].type == S_ELLARC))
    return;

  if (prev_num_segments == 0)
    /* first segment of path; this must be a `moveto' */
    {
      /* update GC used for drawing */
      _pl_x_set_attributes (R___(_plotter) X_GC_FOR_DRAWING);
      
      /* select pen color as foreground color in GC used for drawing */
      _pl_x_set_pen_color (S___(_plotter));
    }
  
  /* Iterate over all segments to be painted.  Because X/XDrawable Plotters
     can't handle `mixed paths', this function ends up being called only on
     sequences of line segments. */

  for (i = IMAX(1, prev_num_segments); 
       i < _plotter->drawstate->path->num_segments;
       i++)
    {
      /* use same variables for points #1 and #2, since reusing them works
	 around an obscure bug in gcc 2.7.2.3 that rears its head if -O2 is
	 used */
      double xu, yu, xd, yd;
      double x, y;
      int x1, y1, x2, y2;

      /* starting and ending points for zero-width line segment: (xu,yu)
	 and (x,y) respectively */
      xu = _plotter->drawstate->path->segments[i-1].p.x;
      yu = _plotter->drawstate->path->segments[i-1].p.y;
      x = _plotter->drawstate->path->segments[i].p.x;
      y = _plotter->drawstate->path->segments[i].p.y;
  
      /* convert to integer X11 coordinates */
      xd = XD(xu, yu);
      yd = YD(xu, yu);
      x1 = IROUND(xd);
      y1 = IROUND(yd);
      xd = XD(x,y);
      yd = YD(x,y);
      x2 = IROUND(xd);
      y2 = IROUND(yd);
      
      if (x1 != x2 || y1 != y2)
	/* line segment has nonzero length, so draw it */
	{
	  if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	    /* double buffering, have a `x_drawable3' to draw into */
	    XDrawLine (_plotter->x_dpy, _plotter->x_drawable3, 
		       _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	  else
	    {
	      if (_plotter->x_drawable1)
		XDrawLine (_plotter->x_dpy, _plotter->x_drawable1, 
			   _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	      if (_plotter->x_drawable2)
		XDrawLine (_plotter->x_dpy, _plotter->x_drawable2, 
			   _plotter->drawstate->x_gc_fg, x1, y1, x2, y2);
	    }
	  
	  something_drawn = true;
	}
      else
	/* line segment in terms of integer device coordinates has zero
	   length; but if it has nonzero length in user coordinates, draw
	   it as a single pixel unless cap type is "butt" */
	if (!(_plotter->drawstate->cap_type == PL_CAP_BUTT
	      && xu == x && yu == y))
	  {
	    if (_plotter->x_double_buffering != X_DBL_BUF_NONE)
	      /* double buffering, have a `x_drawable3' to draw into */
	      XDrawPoint (_plotter->x_dpy, _plotter->x_drawable3,
			  _plotter->drawstate->x_gc_fg, 
			  x1, y1);
	    else
	      /* not double buffering */
	      {
		if (_plotter->x_drawable1)
		  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable1,
			      _plotter->drawstate->x_gc_fg, 
			      x1, y1);
		if (_plotter->x_drawable2)
		  XDrawPoint (_plotter->x_dpy, _plotter->x_drawable2,
			      _plotter->drawstate->x_gc_fg, 
			      x1, y1);
	      }

	    something_drawn = true;
	  }
    }
        
  if (something_drawn)
    /* maybe flush X output buffer and handle X events (a no-op for
       XDrawablePlotters, which is overridden for XPlotters) */
  _maybe_handle_x_events (S___(_plotter));
}

bool
_pl_x_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
