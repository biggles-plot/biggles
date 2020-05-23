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

/* This file contains the arc method, which is a standard part of libplot.
   It draws an object: a circular arc from x0,y0 to x1,y1, with center at
   xc,yc.  If xc,yc does not lie on the perpendicular bisector between the
   other two points as it should, it is adjusted so that it does.

   This file also contains the ellarc method, which is a GNU extension to
   libplot.  It draws an object: an arc of an ellipse, from p0=(x0,y0) to
   p1=(x1,y1).  The center of the ellipse will be at pc=(xc,yc).

   These conditions do not uniquely determine the elliptic arc (or the
   ellipse of which it is an arc).  We choose the elliptic arc so that it
   has control points p0, p1, and p0 + p1 - pc, where the third control
   point p0 + p1 - pc is simply the reflection of pc through the line
   determined by p0 and p1.  This means that the arc passes through p0 and
   p1, is tangent at p0 to the line segment joining p0 to p0 + p1 - pc, and
   is tangent at p1 to the line segment joining p1 to p0 + p1 - pc.  So it
   fits snugly into a triangle, the vertices of which are the three control
   points.
   
   This sort of elliptic arc is called a `quarter-ellipse', since it is an
   affinely transformed quarter-circle.  Specifically, it is an affinely
   transformed version of the first quadrant of a unit circle, with the
   affine transformation mapping (0,0) to pc, (0,1) to p0, (1,0) to p1, and
   (1,1) to the control point p0 + p1 - pc. */

#include "sys-defines.h"
#include "extern.h"

#define COLLINEAR(p0, p1, p2) \
	((p0.x * p1.y - p0.y * p1.x - p0.x * p2.y + \
	  p0.y * p2.x + p1.x * p2.y - p1.y * p2.x) == 0.0)

int
_API_farc (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
{
  int prev_num_segments;
  plPoint p0, p1, pc; 

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "farc: invalid operation");
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
  pc.x = xc; pc.y = yc;      

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
     from (x0,y0) to (x1,y1).  Only the endpoints will appear on the
     display. */
  if (!_plotter->drawstate->points_are_connected)
    _add_line (_plotter->drawstate->path, p1);

  /* Another trivial case: treat a zero-length arc as a line segment */
  else if (x0 == x1 && y0 == y1)
    _add_line (_plotter->drawstate->path, p1);

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

      /* add new circular arc to the path buffer */

      /* adjust location of pc if necessary, to place it on the bisector */
      pc = _truecenter (p0, p1, pc);
      
      /* add new circular arc (either real or fake) to path buffer */

      if (((_plotter->data->have_mixed_paths == false
	    && _plotter->drawstate->path->num_segments == 1) /* i.e. moveto */
	   || _plotter->data->have_mixed_paths == true)
	  && (_plotter->data->allowed_arc_scaling == AS_ANY
	      || (_plotter->data->allowed_arc_scaling == AS_UNIFORM
		  && _plotter->drawstate->transform.uniform)
	      || (_plotter->data->allowed_arc_scaling == AS_AXES_PRESERVED
		  && _plotter->drawstate->transform.axes_preserved)))
	/* add circular arc as an arc element, since it's allowed */
	_add_arc (_plotter->drawstate->path, pc, p1);
      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* add a cubic Bezier that approximates the circular arc (allowed
	   since this Plotter supports cubic Beziers) */
	_add_arc_as_bezier3 (_plotter->drawstate->path, pc, p1);
      else
	/* add a polygonal approximation */
	_add_arc_as_lines (_plotter->drawstate->path, pc, p1);
    }

  /* move to endpoint */
  _plotter->drawstate->pos = p1;

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

int
_API_fellarc (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
{
  int prev_num_segments;
  plPoint pc, p0, p1;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "fellarc: invalid operation");
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
  pc.x = xc; pc.y = yc;      

  if (_plotter->drawstate->path == (plPath *)NULL)
    /* begin a new path, of segment list type */
    {
      _plotter->drawstate->path = _new_plPath ();
      prev_num_segments = 0;
      _add_moveto (_plotter->drawstate->path, p0);
    }
  else
    prev_num_segments = _plotter->drawstate->path->num_segments;

  if (!_plotter->drawstate->points_are_connected)
    /* Trivial case: if linemode is "disconnected", just plot a line
       segment from (x0,y0) to (x1,y1).  Only the endpoints will appear on
       the display. */
    _add_line (_plotter->drawstate->path, p1);

  else if (x0 == x1 && y0 == y1)
    /* Another trivial case: treat a zero-length arc as a line segment */
    _add_line (_plotter->drawstate->path, p1);

  else if (COLLINEAR (p0, p1, pc))
    /* yet another trivial case, collinear points: draw line segment
       from p0 to p1 */
    _add_line (_plotter->drawstate->path, p1);

  else
    /* standard (nontrivial) case */
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
      
      /* add new elliptic arc (either real or fake) to path buffer */

      if (((_plotter->data->have_mixed_paths == false
	    && _plotter->drawstate->path->num_segments == 1) /* i.e. moveto */
	   || _plotter->data->have_mixed_paths == true)
	  && (_plotter->data->allowed_ellarc_scaling == AS_ANY
	      || (_plotter->data->allowed_ellarc_scaling == AS_UNIFORM
		  && _plotter->drawstate->transform.uniform)
	      || (_plotter->data->allowed_ellarc_scaling == AS_AXES_PRESERVED
		  && _plotter->drawstate->transform.axes_preserved
		  && ((y0 == yc && x1 == xc) || (x0 == xc && y1 == yc)))))
	/* add elliptic arc to the path buffer as an arc element, since
	   it's allowed (note that we interpret the AS_AXES_PRESERVED
	   constraint to require also that the x and y coors for arc
	   endpoints line up) */
	_add_ellarc (_plotter->drawstate->path, pc, p1);
      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* add a cubic Bezier that approximates the elliptic arc (allowed
	   since this Plotter supports cubic Beziers) */
	_add_ellarc_as_bezier3 (_plotter->drawstate->path, pc, p1);
      else
	/* add a polygonal approximation to the elliptic arc */
	_add_ellarc_as_lines (_plotter->drawstate->path, pc, p1);
    }

  /* move to endpoint */
  _plotter->drawstate->pos = p1;

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
