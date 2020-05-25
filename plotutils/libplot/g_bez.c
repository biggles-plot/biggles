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

/* This file contains the bezier2 and bezier3 methods, which are GNU
   extensions to libplot.  Each of them draws an object: a quadratic and a
   cubic Bezier path segment, respectively. */

#include "sys-defines.h"
#include "extern.h"

int
_API_fbezier2 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2)
{
  int prev_num_segments;
  plPoint p0, p1, p2;
  
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier2: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->path != (plPath *)NULL
      && (_plotter->drawstate->path->type != PATH_SEGMENT_LIST
	  || 
	  (_plotter->drawstate->path->type == PATH_SEGMENT_LIST
	   && _plotter->drawstate->path->primitive)))
    /* There's a simple path under construction (so that endsubpath() must
       not have been invoked), and it contains a closed primitive
       (box/circle/ellipse).  So flush out the whole compound path.  (It
       may include other, previously drawn simple paths.) */
    _API_endpath (S___(_plotter));

  /* if new segment not contiguous, move to its starting point (first
     flushing out the compound path under construction, if any) */
  if (x0 != _plotter->drawstate->pos.x
      || y0 != _plotter->drawstate->pos.y)
    {
      if (_plotter->drawstate->path)
	_API_endpath (S___(_plotter));
      _plotter->drawstate->pos.x = x0;
      _plotter->drawstate->pos.y = y0;
    }

  p0.x = x0; p0.y = y0;
  p1.x = x1; p1.y = y1;      
  p2.x = x2; p2.y = y2;      

  if (_plotter->drawstate->path == (plPath *)NULL)
    /* begin a new path, of segment list type */
    {
      _plotter->drawstate->path = _new_plPath ();
      prev_num_segments = 0;
      _add_moveto (_plotter->drawstate->path, p0);
    }
  else
    prev_num_segments = _plotter->drawstate->path->num_segments;

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x2,y2).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    _add_line (_plotter->drawstate->path, p2);

  /* Another trivial case: treat a zero-length arc as a line segment */
  else if (x0 == x2 && y0 == y2)
    _add_line (_plotter->drawstate->path, p2);

  else
    /* standard (non-trivial) case */
    {
      /* if segment buffer is occupied by a single arc, replace arc by a
	 polyline if that's called for (Plotter-dependent) */
      if (_plotter->data->have_mixed_paths == false
	  && _plotter->drawstate->path->num_segments == 2)
	{
	  _pl_g_maybe_replace_arc (S___(_plotter));
	  if (_plotter->drawstate->path->num_segments > 2)
	    prev_num_segments = 0;	
	}

      /* add new quadratic Bezier segment to the path buffer */

      if (_plotter->data->allowed_quad_scaling == AS_ANY)
	/* add as a primitive element, since it's allowed */
	_add_bezier2 (_plotter->drawstate->path, p1, p2);

      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* add quadratic Bezier as a cubic Bezier, since it's allowed */
	{
	  /* (control points need to be computed) */
	  plPoint p, pc, pd;

	  p.x = x2;
	  p.y = y2;
	  pc.x = (2.0 * x1 + x0) / 3.0;
	  pc.y = (2.0 * y1 + y0) / 3.0;
	  pd.x = (2.0 * x1 + x2) / 3.0;
	  pd.y = (2.0 * y1 + y2) / 3.0;
	  _add_bezier3 (_plotter->drawstate->path, pc, pd, p);
	}

      else
	/* add quadratic Bezier segment as a polygonal approximation */
	_add_bezier2_as_lines (_plotter->drawstate->path, p1, p2);
    }
      
  /* move to endpoint */
  _plotter->drawstate->pos = p2;

  /* pass all the newly added segments to the Plotter-specific function
     maybe_paint_segments(), since some Plotters plot paths in real time,
     i.e., prepaint them, rather than waiting until endpath() is called */
  _plotter->maybe_prepaint_segments (R___(_plotter) prev_num_segments);

  /* If the path is getting too long (and it doesn't have to be filled),
     flush it out by invoking endpath(), and begin a new one.  `Too long'
     is Plotter-dependent; some don't do this flushing at all. */
  if ((_plotter->drawstate->path->num_segments 
       >= _plotter->data->max_unfilled_path_length)
      && (_plotter->drawstate->fill_type == 0)
      && _plotter->path_is_flushable (S___(_plotter)))
    _API_endpath (S___(_plotter));
  
  return 0;
}

int
_API_fbezier3 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
  int prev_num_segments;
  plPoint p0, p1, p2, p3;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbezier3: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->path != (plPath *)NULL
      && (_plotter->drawstate->path->type != PATH_SEGMENT_LIST
	  || 
	  (_plotter->drawstate->path->type == PATH_SEGMENT_LIST
	   && _plotter->drawstate->path->primitive)))
    /* There's a simple path under construction (so that endsubpath() must
       not have been invoked), and it contains a closed primitive
       (box/circle/ellipse).  So flush out the whole compound path.  (It
       may include other, previously drawn simple paths.) */
    _API_endpath (S___(_plotter));

  /* if new segment not contiguous, move to its starting point (first
     flushing out the compound path under construction, if any) */
  if (x0 != _plotter->drawstate->pos.x
      || y0 != _plotter->drawstate->pos.y)
    {
      if (_plotter->drawstate->path)
	_API_endpath (S___(_plotter));
      _plotter->drawstate->pos.x = x0;
      _plotter->drawstate->pos.y = y0;
    }

  p0.x = x0; p0.y = y0;
  p1.x = x1; p1.y = y1;      
  p2.x = x2; p2.y = y2;      
  p3.x = x3; p3.y = y3;      

  if (_plotter->drawstate->path == (plPath *)NULL)
    /* begin a new path, of segment list type */
    {
      _plotter->drawstate->path = _new_plPath ();
      prev_num_segments = 0;
      _add_moveto (_plotter->drawstate->path, p0);
    }
  else
    prev_num_segments = _plotter->drawstate->path->num_segments;

  /* Trivial case: if linemode is "disconnected", just plot a line segment
     from (x0,y0) to (x2,y2).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    _add_line (_plotter->drawstate->path, p3);

  /* Another trivial case: treat a zero-length arc as a line segment */
  else if (x0 == x3 && y0 == y3)
    _add_line (_plotter->drawstate->path, p3);

  else
    /* standard (non-trivial) case */
    {
      /* if segment buffer is occupied by a single arc, replace arc by a
	 polyline if that's called for (Plotter-dependent) */
      if (_plotter->data->have_mixed_paths == false
	  && _plotter->drawstate->path->num_segments == 2)
	{
	  _pl_g_maybe_replace_arc (S___(_plotter));
	  if (_plotter->drawstate->path->num_segments > 2)
	    prev_num_segments = 0;	
	}

      /* add new cubic Bezier segment to the segment buffer */

      if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* add as a primitive element, since it's allowed */
	_add_bezier3 (_plotter->drawstate->path, p1, p2, p3);
      else
	/* add cubic Bezier segment as a polygonal approximation */
	_add_bezier3_as_lines (_plotter->drawstate->path, p1, p2, p3);
    }
  
  /* move to endpoint */
  _plotter->drawstate->pos = p3;

  /* pass all the newly added segments to the Plotter-specific function
     maybe_paint_segments(), since some Plotters plot paths in real time,
     i.e., prepaint them, rather than waiting until endpath() is called */
  _plotter->maybe_prepaint_segments (R___(_plotter) prev_num_segments);

  /* If the path is getting too long (and it doesn't have to be filled),
     flush it out by invoking endpath(), and begin a new one.  `Too long'
     is Plotter-dependent; some don't do this flushing at all.  */
  if ((_plotter->drawstate->path->num_segments 
       >= _plotter->data->max_unfilled_path_length)
      && (_plotter->drawstate->fill_type == 0)
      && _plotter->path_is_flushable (S___(_plotter)))
    _API_endpath (S___(_plotter));

  return 0;
}
