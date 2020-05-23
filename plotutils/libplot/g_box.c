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

/* This file contains the box method, which is a standard part of libplot.
   It draws an object: a box, or upright rectangle with diagonal corners
   x0,y0 and x1,y1.  Boxes are one of the three types of primitive closed
   path that libplot supports, along with circles and ellipses. */

/* Most Plotters obviously require that the map from the user frame to the
   device frame preserve coordinate axes, in order to draw a box as a
   primitive (if it isn't, what results will be rotated or skewed).  The
   constraints on the user frame -> device frame map (e.g., it must
   preserve axes) are specified by the internal `allowed_box_scaling'
   parameter, which this code checks.  */

#include "sys-defines.h"
#include "extern.h"

int
_API_fbox (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
{
  double xnew, ynew;
  plPoint p0, p1;
  bool clockwise;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fbox: invalid operation");
      return -1;
    }

  /* If a simple path is under construction (so that endsubpath() must not
      have been invoked), flush out the whole compound path.  (It may
      include other, previously drawn simple paths.) */
  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter));

  /* begin a new path */
  _plotter->drawstate->path = _new_plPath ();

  p0.x = x0; 
  p0.y = y0;
  p1.x = x1; 
  p1.y = y1;
  clockwise = _plotter->drawstate->orientation < 0 ? true : false;

  if (!_plotter->drawstate->points_are_connected)
    /* line type is `disconnected': just plot a pseudo-box consisting of
       four line segments, which will ultimately be displayed (see
       g_endpath.c) as four filled circles (one at each vertex) */
    _add_box_as_lines (_plotter->drawstate->path, 
		       p0, p1, clockwise);

  else
    /* general case */
    {
      /* Add box to path buffer, as a primitive only if (1) this Plotter
	 supports box primitives, and (2) it isn't to be edged, or if it's
	 to be edged solid (not dashed).  The latter restriction is to
	 prevent confusion over the starting point, along the edge of box,
	 for the dashing pattern. */

      if ((_plotter->drawstate->pen_type == 0 /* not edged */
	   ||
	   (_plotter->drawstate->dash_array_in_effect == false
	    && _plotter->drawstate->line_type == PL_L_SOLID)) /* solid edge */
	  &&
	  ((_plotter->data->allowed_box_scaling == AS_ANY)
	   ||
	   (_plotter->data->allowed_box_scaling == AS_AXES_PRESERVED
	    && _plotter->drawstate->transform.axes_preserved)))
	/* add box as a primitive */
	_add_box (_plotter->drawstate->path, 
		  p0, p1, clockwise);
      else
	/* add box as a polyline */
	_add_box_as_lines (_plotter->drawstate->path, 
			   p0, p1, clockwise);

      if (_plotter->drawstate->path->type == PATH_SEGMENT_LIST)
	/* pass all the newly added segments to the Plotter-specific
	   function maybe_paint_segments(), since some Plotters plot paths
	   in real time, i.e., prepaint them, rather than waiting until
	   endpath() is called */
	_plotter->maybe_prepaint_segments (R___(_plotter) 0);
    }
      
  /* move to center (libplot convention) */
  xnew = 0.5 * (x0 + x1);
  ynew = 0.5 * (y0 + y1);
  _plotter->drawstate->pos.x = xnew;
  _plotter->drawstate->pos.y = ynew;

  return 0;
}
