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

/* This file contains the line method, which is a standard part of libplot.
   It draws an object: a line segment extending from the point x0,y0 to the
   point x1,y1.  By repeatedly invoking cont(), the user may extend this
   line object to a polyline object.  As implemented, the line method is a
   wrapper around the cont method. */

/* This file also contains the cont method, which is a standard part of
   libplot.  It continues a line from the current position of the graphics
   cursor to the point specified by x and y.

   This method is used in the construction of paths.  By repeatedly
   invoking cont(), the user may construct a polyline of arbitrary length.
   arc() and ellarc() may also be invoked, to add circular or elliptic arc
   elements to the path.  The path will terminate when the user either

     (1) explicitly invokes the endpath() method, or 
     (2) changes the value of one of the relevant drawing attributes, 
          e.g. by invoking move(), linemod(), linewidth(), pencolor(), 
	  fillcolor(), or filltype(), or 
     (3) draws some non-path object, by invoking box(), 
           circle(), point(), label(), alabel(), etc., or 
     (4) invokes restorestate() to restore an earlier drawing state. */

#include "sys-defines.h"
#include "extern.h"

int
_API_fline (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fline: invalid operation");
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

  return _API_fcont (R___(_plotter) x1, y1);
}

int
_API_fcont (R___(Plotter *_plotter) double x, double y)
{
  int prev_num_segments;
  plPoint p0, p1;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcont: invalid operation");
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

  p0 = _plotter->drawstate->pos;
  p1.x = x; p1.y = y;      

  if (_plotter->drawstate->path == (plPath *)NULL)
    /* begin a new path, of segment list type */
    {
      _plotter->drawstate->path = _new_plPath ();
      prev_num_segments = 0;
      _add_moveto (_plotter->drawstate->path, p0);
    }
  else
    prev_num_segments = _plotter->drawstate->path->num_segments;

  /* if segment buffer is occupied by a single arc, replace arc by a
     polyline if that's called for (Plotter-dependent) */
  if (_plotter->data->have_mixed_paths == false
      && _plotter->drawstate->path->num_segments == 2)
    {
      _pl_g_maybe_replace_arc (S___(_plotter));
      if (_plotter->drawstate->path->num_segments > 2)
	prev_num_segments = 0;	
    }

  /* add new line segment to the path buffer */
  _add_line (_plotter->drawstate->path, p1);

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

/* Some Plotters, such as FigPlotters, support the drawing of single arc
   segments as primitives, but they don't allow mixed segment lists to
   appear in the path storage buffer, because they don't know how to handle
   them.  A `mixed segment list' is a sequence of arc segments interspersed
   with line segments, or a path consisting of more than a single
   contiguous arc.  I.e., a mixed path is defined by a segment list that is
   not a pure polyline and has length >1.
   
   For such Plotters, this function may be invoked by fcont() or farc() or
   fellarc() or fbezier2() or fbezier3(), to delete a single arc from the
   buffer and replace it by its polygonal approximation, i.e. by a
   polyline.  I.e., it is invoked to keep the stored path from becoming
   mixed. */

void
_pl_g_maybe_replace_arc (S___(Plotter *_plotter))
{
  /* sanity check */
  if (!(_plotter->data->have_mixed_paths == false
	&& _plotter->drawstate->path->num_segments == 2))
    return;

  switch (_plotter->drawstate->path->segments[1].type)
    {
      plPoint pc, pd, p1;
      
    case S_ARC:
      /* segment buffer contains a single circular arc segment, so remove it */
      pc = _plotter->drawstate->path->segments[1].pc;
      p1 = _plotter->drawstate->path->segments[1].p;
      _plotter->drawstate->path->num_segments = 1;

      /* add polygonal approximation to circular arc to the segment buffer */
      _add_arc_as_lines (_plotter->drawstate->path, pc, p1);
      break;
      
    case S_ELLARC:
      /* segment buffer contains a single elliptic arc segment, so remove it */
      pc = _plotter->drawstate->path->segments[1].pc;
      p1 = _plotter->drawstate->path->segments[1].p;
      _plotter->drawstate->path->num_segments = 1;

      /* add polygonal approximation to elliptic arc to the segment buffer */
      _add_ellarc_as_lines (_plotter->drawstate->path, pc, p1);
      break;

    case S_QUAD:
      /* segment buffer contains a single quad. Bezier segment, so remove it */
      pc = _plotter->drawstate->path->segments[1].pc;
      p1 = _plotter->drawstate->path->segments[1].p;
      _plotter->drawstate->path->num_segments = 1;

      /* add polygonal approximation to quad. Bezier to the segment buffer */
      _add_bezier2_as_lines (_plotter->drawstate->path, pc, p1);
      break;

    case S_CUBIC:
      /* segment buffer contains a single cubic Bezier segment, so remove it */
      pc = _plotter->drawstate->path->segments[1].pc;
      pd = _plotter->drawstate->path->segments[1].pd;
      p1 = _plotter->drawstate->path->segments[1].p;
      _plotter->drawstate->path->num_segments = 1;

      /* add polygonal approximation to cubic Bezier to the segment buffer */
      _add_bezier3_as_lines (_plotter->drawstate->path, pc, pd, p1);
      break;

      default:
	/* other segment type (presumably a line segment, see above); OK */
	break;
    }
}
