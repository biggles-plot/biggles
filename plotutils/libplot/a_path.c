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

/* This version is for AIPlotters.  By construction, for AIPlotters our
   path storage buffer may include only a segment list: a list of line
   segments and/or cubic Bezier segments.  No primitives such as ellipses,
   circles, and boxes are allowed: any such primitive, if drawn, is
   replaced by a segment list at a higher level. */

#include "sys-defines.h"
#include "extern.h"

/* Maximum value for squared sine of angle between two segments, if their
   juncture is to be a `smooth point' rather than a corner point.  (In
   Illustrator, smooth points have only one direction handle; not two.) */
#define MAX_SQUARED_SINE (1e-6)

void
_pl_a_paint_path (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	int i, numpoints;
	bool closed;
	double linewidth;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /*shouldn't happen */
	  break;

	if ((_plotter->drawstate->path->num_segments >= 3)/*check for closure*/
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.x == _plotter->drawstate->path->segments[0].p.x)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.y == _plotter->drawstate->path->segments[0].p.y))
	  closed = true;
	else
	  closed = false;		/* 2-point ones should be open */
	
	/* set fill color and pen color */
	if (_plotter->drawstate->fill_type)
	  /* will be filling the path */
	  _pl_a_set_fill_color (R___(_plotter) false);
	else
	  /* won't be filling the path, but set AI's fill color anyway; in
	     particular, to be the same as the pen color (this is a
	     convenience for AI users who may wish e.g. to switch from
	     stroking to filling) */
	  _pl_a_set_fill_color (R___(_plotter) true);

	_pl_a_set_pen_color (S___(_plotter));
	
	/* update line attributes (cap style, join style, line width), if
	   necessary */
	_pl_a_set_attributes (S___(_plotter));
	
	linewidth = _plotter->drawstate->line_width;
	numpoints = _plotter->drawstate->path->num_segments;
	
	/* loop over segments in path */
	for (i = 0; i < numpoints; i++)
	  {
	    bool smooth_join_point; /* if a path join point, a smooth one? */
	    
	    /* update bounding box to take into account the segment's
	       terminal point (which is either a path join point or a path
	       end point) */

	    if (!closed && (i == 0 || i == numpoints - 1))
	      /* for the path, an end rather than a join */
	      {
		double xcurrent, ycurrent, xother, yother;
		
		smooth_join_point = false;
		
		/* compute path end point, and a nearby point, the vector
		   to which will determine the shape of the path end */
		xcurrent = _plotter->drawstate->path->segments[i].p.x;
		ycurrent = _plotter->drawstate->path->segments[i].p.y;	  
		
		if (i == 0)	/* i = 0, initial end point */
		  {
		    if (_plotter->drawstate->path->segments[i+1].type == S_CUBIC)
		      {
			xother = _plotter->drawstate->path->segments[i+1].pc.x;
			yother = _plotter->drawstate->path->segments[i+1].pc.y;
		      }
		    else	/* line segment */
		      {
			xother = _plotter->drawstate->path->segments[i+1].p.x;
			yother = _plotter->drawstate->path->segments[i+1].p.y;
		      }
		  }
		else		/* i = numpoints - 1, final end point */
		  {
		    if (_plotter->drawstate->path->segments[i].type == S_CUBIC)
		      {
			xother = _plotter->drawstate->path->segments[i].pd.x;
			yother = _plotter->drawstate->path->segments[i].pd.y;
		      }
		    else	/* line segment */
		      {
			xother = _plotter->drawstate->path->segments[i-1].p.x;
			yother = _plotter->drawstate->path->segments[i-1].p.y;
		      }
		  }
		/* take path end into account: update bounding box */
		_set_line_end_bbox (_plotter->data->page,
				    xcurrent, ycurrent, xother, yother,
				    linewidth, _plotter->drawstate->cap_type,
				    _plotter->drawstate->transform.m);
	      }
	    else
	      /* for the path, a join rather than an end */
	      {
		int a, b, c;
		double xcurrent, ycurrent, xleft, yleft, xright, yright;
		
		if (closed && (i == 0 || i == numpoints - 1)) /* wrap */
		  {
		    a = numpoints - 2;
		    b = numpoints - 1;
		    c = 1;
		  }
		else		/* normal join */
		  {
		    a = i - 1;
		    b = i;
		    c = i + 1;
		  }
		
		xcurrent = _plotter->drawstate->path->segments[b].p.x;
		ycurrent = _plotter->drawstate->path->segments[b].p.y;
		
		/* compute points to left and right, vectors to which will
		   determine the shape of the path join */
		switch ((int)_plotter->drawstate->path->segments[b].type)
		  {
		  case (int)S_LINE:
		  default:
		    xleft = _plotter->drawstate->path->segments[a].p.x;
		    yleft = _plotter->drawstate->path->segments[a].p.y;
		    break;
		  case (int)S_CUBIC:
		    xleft = _plotter->drawstate->path->segments[b].pd.x;
		    yleft = _plotter->drawstate->path->segments[b].pd.y;
		    break;
		  }
		switch ((int)_plotter->drawstate->path->segments[c].type)
		  {
		  case (int)S_LINE:
		  default:
		    xright = _plotter->drawstate->path->segments[c].p.x;
		    yright = _plotter->drawstate->path->segments[c].p.y;
		    break;
		  case (int)S_CUBIC:
		    xright = _plotter->drawstate->path->segments[c].pc.x;
		    yright = _plotter->drawstate->path->segments[c].pc.y;
		    break;
		  }
		
		/* take path join into account: update bounding box */
		_set_line_join_bbox(_plotter->data->page,
				    xleft, yleft, xcurrent, ycurrent, xright, yright,
				    linewidth, 
				    _plotter->drawstate->join_type,
				    _plotter->drawstate->miter_limit,
				    _plotter->drawstate->transform.m);
		
		/* is join smooth? */
		{
		  double ux, uy, vx, vy, cross, dot, uselfdot, vselfdot;
		  
		  ux = xleft - xcurrent;
		  uy = yleft - ycurrent;
		  vx = xright - xcurrent;
		  vy = yright - ycurrent;
		  
		  cross = ux * vy - uy * vx;
		  dot = ux * vx + uy * vy;
		  uselfdot = ux * ux + uy * uy;
		  vselfdot = vx * vx + vy * vy;
		  
		  if (cross * cross < MAX_SQUARED_SINE * uselfdot * vselfdot 
		      && dot < 0.0)
		    smooth_join_point = true;
		  else
		    smooth_join_point = false;
		}
	      }
	    
	    /* output to Illustrator the points that define this segment */
	    
	    if (i != 0 
		&& (_plotter->drawstate->path->segments)[i].type == S_CUBIC)
	      /* cubic Bezier segment, so output control points */
	      {
		sprintf (_plotter->data->page->point, 
			 "%.4f %.4f %.4f %.4f ", 
			 XD(_plotter->drawstate->path->segments[i].pc.x,
			    _plotter->drawstate->path->segments[i].pc.y),
			 YD(_plotter->drawstate->path->segments[i].pc.x,
			    _plotter->drawstate->path->segments[i].pc.y),
			 XD(_plotter->drawstate->path->segments[i].pd.x,
		      _plotter->drawstate->path->segments[i].pd.y),
			 YD(_plotter->drawstate->path->segments[i].pd.x,
			    _plotter->drawstate->path->segments[i].pd.y));
		_update_buffer (_plotter->data->page);
		/* update bounding box due to extremal x/y values in device
                   frame */
		_set_bezier3_bbox (_plotter->data->page, 
				   _plotter->drawstate->path->segments[i-1].p.x,
				   _plotter->drawstate->path->segments[i-1].p.y,
				   _plotter->drawstate->path->segments[i].pc.x,
				   _plotter->drawstate->path->segments[i].pc.y,
				   _plotter->drawstate->path->segments[i].pd.x,
				   _plotter->drawstate->path->segments[i].pd.y,
				   _plotter->drawstate->path->segments[i].p.x,
				   _plotter->drawstate->path->segments[i].p.y,
				   _plotter->drawstate->device_line_width,
				   _plotter->drawstate->transform.m);
	      }
	    
	    /* output terminal point of segment */
	    sprintf (_plotter->data->page->point, 
		     "%.4f %.4f ", 
		     XD(_plotter->drawstate->path->segments[i].p.x,
			_plotter->drawstate->path->segments[i].p.y),
		     YD(_plotter->drawstate->path->segments[i].p.x,
			_plotter->drawstate->path->segments[i].p.y));
	    _update_buffer (_plotter->data->page);
	    
	    /* tell Illustrator what sort of path segment this is */
	    if (i == 0)
	      /* start of path, so just move to point */
	      sprintf (_plotter->data->page->point, "m\n");
	    else
	      /* append line segment or Bezier segment to path */
	      switch ((int)_plotter->drawstate->path->segments[i].type)
		{
		case (int)S_LINE:
		default:
		  sprintf (_plotter->data->page->point, 
			   smooth_join_point ? "l\n" : "L\n");
		  break;
		case (int)S_CUBIC:
		  sprintf (_plotter->data->page->point, 
			   smooth_join_point ? "c\n" : "C\n");
		  break;	    
		}
	    _update_buffer (_plotter->data->page);
	    
	  } /* end of loop over segments */
	
	if (_plotter->drawstate->pen_type)
	  /* have a pen to draw with */
	  {
	    /* emit `closepath' if path is closed; stroke and maybe fill */
	    if (_plotter->drawstate->fill_type)
	      {
		if (closed)
		  /* close path, fill and stroke */
		  sprintf (_plotter->data->page->point, "b\n");
		else
		  /* fill and stroke */
		  sprintf (_plotter->data->page->point, "B\n");
	      }
	    else
	      {
		if (closed)
		  /* close path, stroke */
		  sprintf (_plotter->data->page->point, "s\n");
		else
		  /* stroke */
		  sprintf (_plotter->data->page->point, "S\n");
	      }
	  }
	else
	  /* no pen to draw with, but we may do filling */
	  {
	    /* emit `closepath' if path is closed; don't stroke */
	    if (_plotter->drawstate->fill_type)
	      {
		if (closed)
		  /* close path, fill */
		  sprintf (_plotter->data->page->point, "f\n");
		else
		  /* fill */
		  sprintf (_plotter->data->page->point, "F\n");
	      }
	  }
	_update_buffer (_plotter->data->page);
      }
      break;
      
    default:			/* shouldn't happen */
      break;
    }
}

bool
_pl_a_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
