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

/* This file contains the circle method, which is a standard part of
   libplot.  It draws an object: a circle with center x,y and radius r.
   Circles are one of the three types of primitive closed path that libplot
   supports, along with boxes and ellipses. */

/* Most Plotters obviously require that the map from the user frame to the
   device frame be uniform, in order to draw a circle as a primitive (if it
   isn't, what results will be an ellipse).  The constraints on the user
   frame -> device frame map (e.g., it must be uniform) are specified by
   the internal `allowed_circle_scaling' parameter, which this code checks.  */

#include "sys-defines.h"
#include "extern.h"

int
_API_fcircle (R___(Plotter *_plotter) double x, double y, double r)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fcircle: invalid operation");
      return -1;
    }

  /* If a simple path is under construction (so that endsubpath() must not
      have been invoked), flush out the whole compound path.  (It may
      include other, previously drawn simple paths.) */
  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter));

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected', so do nothing (libplot convention) */
    {
    }

  else
    /* general case */
    {
      plPoint pc;
      bool clockwise;

      /* begin a new path */
      _plotter->drawstate->path = _new_plPath ();

      /* place circle in path buffer */

      pc.x = x;
      pc.y = y;
      clockwise = _plotter->drawstate->orientation < 0 ? true : false;

      if ((_plotter->data->allowed_circle_scaling == AS_ANY)
	  ||
	  (_plotter->data->allowed_circle_scaling == AS_UNIFORM
	   && _plotter->drawstate->transform.uniform))
	/* place circle as a primitive, since this Plotter supports
	   drawing circles as primitives */
	_add_circle (_plotter->drawstate->path, 
			       pc, r, clockwise);
      else if ((_plotter->data->allowed_ellipse_scaling == AS_ANY)
	       ||
	       (_plotter->data->allowed_ellipse_scaling == AS_AXES_PRESERVED
		&& _plotter->drawstate->transform.axes_preserved))
	/* place circle as an ellipse, since this Plotter supports drawing
	   ellipses as primitives */
	_add_ellipse (_plotter->drawstate->path, 
				pc, r, r, 0.0, clockwise);
      else if (_plotter->data->allowed_ellarc_scaling == AS_ANY
	       || (_plotter->data->allowed_ellarc_scaling == AS_AXES_PRESERVED
		   && _plotter->drawstate->transform.axes_preserved))
	/* draw circle by placing four elliptical arcs into path buffer
	   (allowed since this Plotter supports elliptical arcs) */
	_add_circle_as_ellarcs (_plotter->drawstate->path, 
				pc, r, clockwise);
      else if (_plotter->data->allowed_cubic_scaling == AS_ANY)
	/* draw circle by placing four cubic Beziers into path buffer
	   (allowed since this Plotter supports cubic Beziers) */
	_add_circle_as_bezier3s (_plotter->drawstate->path, 
				 pc, r, clockwise);
      else
	/* draw a polygonal approximation to the circle */
	_add_circle_as_lines (_plotter->drawstate->path, 
			      pc, r, clockwise);

      if (_plotter->drawstate->path->type == PATH_SEGMENT_LIST)
	/* pass all the newly added segments to the Plotter-specific
	   function maybe_paint_segments(), since some Plotters plot paths
	   in real time, i.e., prepaint them, rather than waiting until
	   endpath() is called */
	_plotter->maybe_prepaint_segments (R___(_plotter) 0);
    }

  /* move to center (libplot convention) */
  _plotter->drawstate->pos.x = x;
  _plotter->drawstate->pos.y = y;

  return 0;
}
