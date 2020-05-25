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

/* This file contains all of libplot's low-level code for constructing and
   manipulating paths, both simple and compound.  It includes functions
   that construct polygonal and Bezier approximations to a given path.
   They are used by Plotters whose output format does not support all of
   libplot's graphics primitives. */

/* E.g., _fakearc() draws polygonal approximations to circular and elliptic
   quarter-arcs.  Each polygonal approximation will contain
   2**NUM_ARC_SUBDIVISIONS line segments.
   
   Similarly, polygonal approximations to quadratic and cubic Beziers will
   contain no more than 2**MAX_NUM_BEZIER2_SUBDIVISIONS and
   2**MAX_NUM_BEZIER3_SUBDIVISIONS line segments.  However, each bisection
   algorithm used for drawing a Bezier normally usually its recursion based
   on a relative flatness criterion (see below). */

#include "sys-defines.h"
#include "extern.h"
#include "g_arc.h"		/* for chord table */

/* Number of times a circular arc or quarter-ellipse will be recursively
   subdivided.  Two raised to this power is the number of line segments
   that the polygonalization will contain. */

/* NOTE: the maximum allowed value for NUM_ARC_SUBDIVISIONS is
   TABULATED_ARC_SUBDIVISIONS (i.e., the size of the chordal deviation
   table for a quarter-circle or quarter-ellipse, defined in g_arc.h). */
#define NUM_ARC_SUBDIVISIONS 5

/* Maximum number of times quadratic and cubic Beziers will be recursively
   subdivided.  For Beziers we use an adaptive algorithm, in which
   bisection stops when a specified relative flatness has been reached.
   But these parameters provide a hard cutoff, which overrides the relative
   flatness end condition. */
#define MAX_NUM_BEZIER2_SUBDIVISIONS 6
#define MAX_NUM_BEZIER3_SUBDIVISIONS 7

/* The relative flatness parameters. */
#define REL_QUAD_FLATNESS 5e-4
#define REL_CUBIC_FLATNESS 5e-4

#define DATAPOINTS_BUFSIZ PL_MAX_UNFILLED_PATH_LENGTH
#define DIST(p0,p1) (sqrt( ((p0).x - (p1).x)*((p0).x - (p1).x) \
			  + ((p0).y - (p1).y)*((p0).y - (p1).y)))

#define MIDWAY(x0, x1) (0.5 * ((x0) + (x1)))

/* forward references */
static void _prepare_chord_table (double sagitta, double custom_chord_table[TABULATED_ARC_SUBDIVISIONS]);
static void _fakearc (plPath *path, plPoint p0, plPoint p1, int arc_type, const double *custom_chord_table, const double m[4]);

/* ctor for plPath class; constructs an empty plPath, with type set to
   PATH_SEGMENT_LIST (default type) */
plPath *
_new_plPath (void)
{
  plPath *path;
  
  path = (plPath *)_pl_xmalloc (sizeof (plPath));

  path->type = PATH_SEGMENT_LIST;
  path->segments = (plPathSegment *)NULL;
  path->segments_len = 0;	/* number of slots allocated */
  path->num_segments = 0;	/* number of slots occupied */

  path->primitive = false;
  path->llx = DBL_MAX;
  path->lly = DBL_MAX;
  path->urx = -(DBL_MAX);
  path->ury = -(DBL_MAX);

  return path;
}
  
/* dtor for plPath class */
void
_delete_plPath (plPath *path)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type == PATH_SEGMENT_LIST 
      && path->segments_len > 0) /* number of slots allocated */
    free (path->segments);
  free (path);
}

/* reset function for plPath class */
void
_reset_plPath (plPath *path)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type == PATH_SEGMENT_LIST 
      && path->segments_len > 0) /* number of slots allocated */
    free (path->segments);
  path->segments = (plPathSegment *)NULL;
  path->segments_len = 0;
  path->type = PATH_SEGMENT_LIST; /* restore to default */
  path->num_segments = 0;
  
  path->primitive = false;
  path->llx = DBL_MAX;
  path->lly = DBL_MAX;
  path->urx = -(DBL_MAX);
  path->ury = -(DBL_MAX);
}

void
_add_moveto (plPath *path, plPoint p)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  /* empty, so allocate a segment buffer */
  path->segments = (plPathSegment *) 
    _pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
  path->segments_len = DATAPOINTS_BUFSIZ;
  
  path->segments[0].type = S_MOVETO;
  path->segments[0].p = p;
  path->num_segments = 1;
  
  path->llx = p.x;
  path->lly = p.y;
  path->urx = p.x;
  path->ury = p.y;
}

void
_add_line (plPath *path, plPoint p)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)
    /* empty, so allocate a segment buffer */
    {
      path->segments = (plPathSegment *) 
	_pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
      path->segments_len = DATAPOINTS_BUFSIZ;
    }
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_LINE;
  path->segments[path->num_segments].p = p;
  path->num_segments++;
  
  path->llx = DMIN(path->llx, p.x);
  path->lly = DMIN(path->lly, p.y);
  path->urx = DMAX(path->urx, p.x);
  path->ury = DMAX(path->ury, p.y);
}

void
_add_closepath (plPath *path)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)	/* meaningless */
    return;
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_CLOSEPATH;
  path->segments[path->num_segments].p = path->segments[0].p;
  path->num_segments++;
}

void
_add_bezier2 (plPath *path, plPoint pc, plPoint p)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)
    /* empty, so allocate a segment buffer */
    {
      path->segments = (plPathSegment *) 
	_pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
      path->segments_len = DATAPOINTS_BUFSIZ;
    }
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_QUAD;
  path->segments[path->num_segments].p = p;
  path->segments[path->num_segments].pc = pc;
  path->num_segments++;
}

void
_add_bezier3 (plPath *path, plPoint pc, plPoint pd, plPoint p)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)
    /* empty, so allocate a segment buffer */
    {
      path->segments = (plPathSegment *) 
	_pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
      path->segments_len = DATAPOINTS_BUFSIZ;
    }
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_CUBIC;
  path->segments[path->num_segments].p = p;
  path->segments[path->num_segments].pc = pc;
  path->segments[path->num_segments].pd = pd;
  path->num_segments++;
}

void
_add_arc (plPath *path, plPoint pc, plPoint p1)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)
    /* empty, so allocate a segment buffer */
    {
      path->segments = (plPathSegment *) 
	_pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
      path->segments_len = DATAPOINTS_BUFSIZ;
    }
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_ARC;
  path->segments[path->num_segments].p = p1;
  path->segments[path->num_segments].pc = pc;
  path->num_segments++;
}

void
_add_ellarc (plPath *path, plPoint pc, plPoint p1)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  if (path->num_segments == 0)
    /* empty, so allocate a segment buffer */
    {
      path->segments = (plPathSegment *) 
	_pl_xmalloc (DATAPOINTS_BUFSIZ * sizeof(plPathSegment));
      path->segments_len = DATAPOINTS_BUFSIZ;
    }
  
  if (path->num_segments == path->segments_len)
    /* full, so reallocate */
    {
      path->segments = (plPathSegment *) 
	_pl_xrealloc (path->segments, 
			2 * path->segments_len * sizeof(plPathSegment));
      path->segments_len *= 2;
    }
  
  path->segments[path->num_segments].type = S_ELLARC;
  path->segments[path->num_segments].p = p1;
  path->segments[path->num_segments].pc = pc;
  path->num_segments++;
}

void
_add_box (plPath *path, plPoint p0, plPoint p1, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  path->type = PATH_BOX;
  path->p0 = p0;
  path->p1 = p1;
  path->clockwise = clockwise;
  
  path->llx = DMIN(path->llx, p0.x);
  path->lly = DMIN(path->lly, p0.y);
  path->urx = DMAX(path->urx, p0.x);
  path->ury = DMAX(path->ury, p0.y);
  
  path->llx = DMIN(path->llx, p1.x);
  path->lly = DMIN(path->lly, p1.y);
  path->urx = DMAX(path->urx, p1.x);
  path->ury = DMAX(path->ury, p1.y);
}

void
_add_circle (plPath *path, plPoint pc, double radius, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  path->type = PATH_CIRCLE;
  path->pc = pc;
  path->radius = radius;
  path->clockwise = clockwise;
}

void
_add_ellipse (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  path->type = PATH_ELLIPSE;
  path->pc = pc;
  path->rx = rx;
  path->ry = ry;
  path->angle = angle;
  path->clockwise = clockwise;
}

/* Draw a polygonal approximation to the circular arc from p0 to p1, with
   center pc, by calling _fakearc(), which in turn repeatedly calls
   _add_line().  It is assumed that p0 and p1 are distinct.  It is assumed
   that pc is on the perpendicular bisector of the line segment joining
   them, and that the graphics cursor is initially located at p0. */
void
_add_arc_as_lines (plPath *path, plPoint pc, plPoint p1)
{
  /* starting point */
  plPoint p0;
  /* bisection point of arc, and midpoint of chord */
  plPoint pb, pm;
  /* rotation matrix */
  double m[4];
  /* other variables */
  plVector v, v0, v1;
  double radius, sagitta;
  double cross, orientation;
  /* handcrafted relative chordal deviation table, for this arc */
  double custom_chord_table[TABULATED_ARC_SUBDIVISIONS];
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
      
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;

  if (p0.x == p1.x && p0.y == p1.y)
    /* zero-length arc, draw as zero-length line segment */
    _add_line (path, p0);

  else
    /* genuine polygonal approximation */
    {
      /* vectors from pc to p0, and pc to p1 */
      v0.x = p0.x - pc.x;
      v0.y = p0.y - pc.y;
      v1.x = p1.x - pc.x;
      v1.y = p1.y - pc.y;
      
      /* cross product, zero if points are collinear */
      cross = v0.x * v1.y - v1.x * v0.y;
      
      /* Compute orientation.  Note libplot convention: if p0, p1, pc are
	 collinear then arc goes counterclockwise from p0 to p1. */
      orientation = (cross >= 0.0 ? 1.0 : -1.0);
      
      radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */
      
      pm.x = 0.5 * (p0.x + p1.x); /* midpoint of chord from p0 to p1 */
      pm.y = 0.5 * (p0.y + p1.y);  
      
      v.x = p1.x - p0.x;	/* chord vector from p0 to p1 */
      v.y = p1.y - p0.y;
      
      _vscale(&v, radius);
      pb.x = pc.x + orientation * v.y; /* bisection point of arc */
      pb.y = pc.y - orientation * v.x;
      
      sagitta = DIST(pb, pm) / radius;
      
      /* fill in entries of chordal deviation table for this user-defined
         sagitta */
      _prepare_chord_table (sagitta, custom_chord_table);
      
      /* call _fakearc(), using for `rotation' matrix m[] a clockwise or
	 counterclockwise rotation by 90 degrees, depending on orientation */

      m[0] = 0.0, m[1] = orientation, m[2] = -orientation, m[3] = 0.0;
      _fakearc (path, p0, p1, USER_DEFINED_ARC, custom_chord_table, m);
    }
}

/* Draw a polygonal approximation to a quarter-ellipse from p0 to p1, by
   calling _fakearc(), which in turn repeatedly calls _add_line().  pc is
   the center of the arc, and p0, p1, pc are assumed not to be collinear.
   It is assumed that the graphics cursor is located at p0 when this
   function is called.

   The control triangle for the elliptic arc will have vertices p0, p1, and
   K = p0 + (p1 - pc) = p1 + (p0 - pc).  The arc will pass through p0 and
   p1, and will be tangent at p0 to the edge from p0 to K, and at p1 to the
   edge from p1 to K. */
void 
_add_ellarc_as_lines (plPath *path, plPoint pc, plPoint p1)
{ 
  plPoint p0;
  plVector v0, v1; 
  double cross;
  double m[4];

  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
      
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  /* cross product */
  cross = v0.x * v1.y - v1.x * v0.y;
  if (FROUND(cross) == 0.0)
    /* collinear points, draw line segment from p0 to p1
       (not quite right, could be bettered) */
    _add_line (path, p1);
  else
    {
      /* `rotation' matrix (it maps v0 -> -v1 and v1 -> v0) */
      m[0] = - (v0.x * v0.y + v1.x * v1.y) / cross;
      m[1] = (v0.x * v0.x + v1.x * v1.x) / cross;
      m[2] = - (v0.y * v0.y + v1.y * v1.y) / cross;
      m[3] = (v0.x * v0.y + v1.x * v1.y) / cross;
      
      /* draw polyline inscribed in the quarter-ellipse */
      _fakearc (path, p0, p1, QUARTER_ARC, (double *)NULL, m);
    }
}

/* A function that approximates a circular arc by a cubic Bezier.  The
   approximation used is a standard one.  E.g., a quarter circle extending
   from (1,0) to (0,1), with center (0,0), would be approximated by a cubic
   Bezier with control points (1,KAPPA) and (KAPPA,1).  Here KAPPA =
   (4/3)[sqrt(2)-1] = 0.552284749825, approximately.  The cubic Bezier will
   touch the quarter-circle along the line x=y.

   For a quarter-circle, the maximum relative error in r as a function of
   theta is about 2.7e-4.  The error in r has the same sign, for all theta. */

/* According to Berthold K. P. Horn <bkph@ai.mit.edu>, the general formula
   for KAPPA, for a radius-1 circular arc (not necessary a quarter-circle),

   KAPPA = (4/3)sqrt[(1-cos H)/(1+cos H)] 
   	 = (4/3)[1-cos H]/[sin H] = (4/3)[sin H]/[1+cosH]

   where H is half the angle subtended by the arc.  H=45 degrees for a
   quarter circle.  This is the formula we use. */

/* Louis Vosloo <support@yandy.com> points out that for a quarter-circle,
   the value 0.55228... for KAPPA is, for some purposes, sub-optimal.  By
   dropping the requirement that the quarter-circle and the Bezier touch
   each other along the symmetry line x=y, one can slightly decrease the
   maximum relative error.  He says 0.5541... is the best possible choice.
   He doesn't have an improved value of KAPPA for a general arc, though.  */

void 
_add_arc_as_bezier3 (plPath *path, plPoint pc, plPoint p1)
{ 
  plPoint p0;
  plVector v0, v1;
		  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
      
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;

  /* vectors to starting, ending points */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;
  
  if ((v0.x == 0.0 && v0.y == 0.0) || (v1.x == 0.0 && v1.y == 0.0)
      || (v0.x == v1.x && v0.y == v1.y))
    /* degenerate case */
    _add_line (path, p1);
  else
    /* normal case */
    {
      double oldangle, newangle, anglerange;
      double cross;
      int orientation;
      
      /* cross product, zero if points are collinear */
      cross = v0.x * v1.y - v1.x * v0.y;
      
      /* Compute orientation.  Note libplot convention: if p0, p1, pc
	 are collinear then arc goes counterclockwise from p0 to p1. */
      orientation = (cross >= 0.0 ? 1 : -1);
	  
      /* compute signed subtended angle */
      oldangle = _xatan2 (v0.y, v0.x);
      newangle = _xatan2 (v1.y, v1.x);
      anglerange = newangle - oldangle;
      if (anglerange > M_PI)
	anglerange -= (2 * M_PI);
      if (anglerange <= -(M_PI))
	anglerange += (2 * M_PI);

      if (FABS(anglerange) > 0.51 * M_PI)
	/* subtended angle > 0.51 * pi, so split arc in two and recurse,
	   since Bezier approximation isn't very good for angles much
	   greater than 90 degrees */
	{
	  double radius;
	  plPoint pb;
	  plVector v;

	  radius = DIST(pc, p0); /* radius is distance to p0 or p1 */
	  
	  v.x = p1.x - p0.x;	/* chord vector from p0 to p1 */
	  v.y = p1.y - p0.y;
	  
	  _vscale(&v, radius);
	  pb.x = pc.x + orientation * v.y; /* bisection point of arc */
	  pb.y = pc.y - orientation * v.x;
	  
	  _add_arc_as_bezier3 (path, pc, pb);
	  _add_arc_as_bezier3 (path, pc, p1);
	}
      else
	/* subtended angle <= 0.51 * pi, so a single Bezier suffices */
	{
	  double halfangle, sinhalf, coshalf, kappa;
	  plPoint pc_bezier3, pd_bezier3;

	  halfangle = 0.5 * FABS(anglerange);
	  sinhalf = sin (halfangle);
	  coshalf = cos (halfangle);
	  /* compute kappa using either of two formulae, depending on
	     numerical stability */
	  if (FABS(sinhalf) < 0.5)
	    kappa = (4.0/3.0) * sinhalf / (1.0 + coshalf);
	  else
	    kappa = (4.0/3.0) * (1.0 - coshalf) / sinhalf;
	  
	  pc_bezier3.x = p0.x - kappa * orientation * v0.y;
	  pc_bezier3.y = p0.y + kappa * orientation * v0.x;
	  pd_bezier3.x = p1.x + kappa * orientation * v1.y;
	  pd_bezier3.y = p1.y - kappa * orientation * v1.x;
	  _add_bezier3 (path, pc_bezier3, pd_bezier3, p1);
	}
    }
}

#define KAPPA_FOR_QUARTER_CIRCLE 0.552284749825

void 
_add_ellarc_as_bezier3 (plPath *path, plPoint pc, plPoint p1)
{ 
  plPoint p0, pc_bezier3, pd_bezier3;
  plVector v0, v1;
		  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
      
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;

  /* vectors to starting, ending points */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;
  
  /* replace by cubic Bezier, with computed control points */
  pc_bezier3.x = p0.x + KAPPA_FOR_QUARTER_CIRCLE * v1.x;
  pc_bezier3.y = p0.y + KAPPA_FOR_QUARTER_CIRCLE * v1.y;
  pd_bezier3.x = p1.x + KAPPA_FOR_QUARTER_CIRCLE * v0.x;
  pd_bezier3.y = p1.y + KAPPA_FOR_QUARTER_CIRCLE * v0.y;
  _add_bezier3 (path, pc_bezier3, pd_bezier3, p1);
}

/* Approximate a quadratic Bezier by a polyline: standard deCasteljau
   bisection algorithm.  However, we stop subdividing when an appropriate
   metric of the quadratic Bezier to be drawn is sufficiently small.  If
   (p0,p1,p2) defines the quadratic Bezier, we require that the length of
   p0-2*p1+p2 be less than REL_QUAD_FLATNESS times the distance between the
   endpoints of the original Bezier. */

void
_add_bezier2_as_lines (plPath *path, plPoint pc, plPoint p)
{
  plPoint r0[MAX_NUM_BEZIER2_SUBDIVISIONS + 1], r1[MAX_NUM_BEZIER2_SUBDIVISIONS + 1], r2[MAX_NUM_BEZIER2_SUBDIVISIONS + 1];
  int level[MAX_NUM_BEZIER2_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_NUM_BEZIER2_SUBDIVISIONS */
  int segments_drawn = 0;
  plPoint p0;
  double sqdist, max_squared_length;
      
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;
  
  /* squared distance between p0 and p */
  sqdist = (p.x - p0.x) * (p.x - p0.x) + (p.y - p0.y) * (p.y - p0.y);
  max_squared_length = REL_QUAD_FLATNESS * REL_QUAD_FLATNESS * sqdist;

  r0[0] = p0;
  r1[0] = pc;
  r2[0] = p;
  level[0] = 0;
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      int current_level;
      plPoint q0, q1, q2;
      
      current_level = level[n];
      q0 = r0[n];
      q1 = r1[n];
      q2 = r2[n];
      
      if (current_level >= MAX_NUM_BEZIER2_SUBDIVISIONS) 
	/* to avoid stack overflow, draw as line segment */
	{
	  _add_line (path, q2);
	  segments_drawn++;
	  n--;
	}
      else
	/* maybe bisect the Bezier */
	{
	  plPoint qq0, qq1;
	  plPoint qqq0;
	  plVector vec1;
	  
	  vec1.x = q0.x - 2 * q1.x + q2.x;
	  vec1.y = q0.y - 2 * q1.y + q2.y;
	  
	  if (vec1.x * vec1.x + vec1.y * vec1.y < max_squared_length)
	    /* very flat Bezier, so draw as line segment */
	    {
	      _add_line (path, q2);
	      segments_drawn++;
	      n--;
	    }
	  else
	    /* split Bezier into pair and recurse */
	    /* level[n] >= n is an invariant */
	    {
	      qq0.x = MIDWAY(q0.x, q1.x);
	      qq0.y = MIDWAY(q0.y, q1.y);
	      qq1.x = MIDWAY(q1.x, q2.x);
	      qq1.y = MIDWAY(q1.y, q2.y);
	      
	      qqq0.x = MIDWAY(qq0.x, qq1.x);
	      qqq0.y = MIDWAY(qq0.y, qq1.y);
	      
	      /* first half, deal with next */
	      r0[n+1] = q0;
	      r1[n+1] = qq0;
	      r2[n+1] = qqq0;
	      level[n+1] = current_level + 1;
	      
	      /* second half, deal with later */
	      r0[n] = qqq0;
	      r1[n] = qq1;
	      r2[n] = q2;
	      level[n] = current_level + 1;
	      
	      n++;
	    }
	}
    }
}

/* Approximate a cubic Bezier by a polyline: standard deCasteljau bisection
   algorithm.  However, we stop subdividing when an appropriate metric of
   the cubic Bezier to be drawn is sufficiently small.  If (p0,p1,p2,p3)
   defines the cubic Bezier, we require that the lengths of p0-2*p1+p2 and
   p1-2*p2+p3 be less than REL_CUBIC_FLATNESS times the distance between
   the endpoints of the original Bezier. */

void
_add_bezier3_as_lines (plPath *path, plPoint pc, plPoint pd, plPoint p)
{
  plPoint r0[MAX_NUM_BEZIER3_SUBDIVISIONS + 1], r1[MAX_NUM_BEZIER3_SUBDIVISIONS + 1], r2[MAX_NUM_BEZIER3_SUBDIVISIONS + 1], r3[MAX_NUM_BEZIER3_SUBDIVISIONS + 1];
  int level[MAX_NUM_BEZIER3_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < MAX_NUM_BEZIER3_SUBDIVISIONS */
  int segments_drawn = 0;
  plPoint p0;
  double sqdist, max_squared_length;
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments == 0)
    return;
  
  /* determine starting point */
  p0 = path->segments[path->num_segments - 1].p;
  
  /* squared distance between p0 and p */
  sqdist = (p.x - p0.x) * (p.x - p0.x) + (p.y - p0.y) * (p.y - p0.y);
  max_squared_length = REL_CUBIC_FLATNESS * REL_CUBIC_FLATNESS * sqdist;

  r0[0] = p0;
  r1[0] = pc;
  r2[0] = pd;
  r3[0] = p;
  level[0] = 0;
  
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      int current_level;
      plPoint q0, q1, q2, q3;
      
      current_level = level[n];
      q0 = r0[n];
      q1 = r1[n];
      q2 = r2[n];
      q3 = r3[n];
      
      if (current_level >= MAX_NUM_BEZIER3_SUBDIVISIONS) 
	/* draw line segment, to avoid stack overflow */
	{
	  _add_line (path, q3);
	  segments_drawn++;
	  n--;
	}
      else
	/* maybe bisect the Bezier */
	{
	  plPoint qq0, qq1, qq2;
	  plPoint qqq0, qqq1;
	  plPoint qqqq0;
	  plVector vec1, vec2;
	  
	  vec1.x = q0.x - 2 * q1.x + q2.x;
	  vec1.y = q0.y - 2 * q1.y + q2.y;
	  vec2.x = q1.x - 2 * q2.x + q3.x;
	  vec2.y = q1.y - 2 * q2.y + q3.y;
	  
	  if (vec1.x * vec1.x + vec1.y * vec1.y < max_squared_length
	      && vec2.x * vec2.x + vec2.y * vec2.y < max_squared_length)
	    /* very flat Bezier, so draw as line segment */
	    {
	      _add_line (path, q3);
	      segments_drawn++;
	      n--;
	    }
	  else
	    /* split Bezier into pair and recurse */
	    /* level[n] >= n is an invariant */
	    {
	      qq0.x = MIDWAY(q0.x, q1.x);
	      qq0.y = MIDWAY(q0.y, q1.y);
	      qq1.x = MIDWAY(q1.x, q2.x);
	      qq1.y = MIDWAY(q1.y, q2.y);
	      qq2.x = MIDWAY(q2.x, q3.x);
	      qq2.y = MIDWAY(q2.y, q3.y);
	      
	      qqq0.x = MIDWAY(qq0.x, qq1.x);
	      qqq0.y = MIDWAY(qq0.y, qq1.y);
	      qqq1.x = MIDWAY(qq1.x, qq2.x);
	      qqq1.y = MIDWAY(qq1.y, qq2.y);
	      
	      qqqq0.x = MIDWAY(qqq0.x, qqq1.x);
	      qqqq0.y = MIDWAY(qqq0.y, qqq1.y);
	      
	      /* first half, deal with next */
	      level[n+1] = current_level + 1;
	      r0[n+1] = q0;
	      r1[n+1] = qq0;
	      r2[n+1] = qqq0;
	      r3[n+1] = qqqq0;
	      
	      /* second half, deal with later */
	      level[n] = current_level + 1;
	      r0[n] = qqqq0;
	      r1[n] = qqq1;
	      r2[n] = qq2;
	      r3[n] = q3;
	      
	      n++;
	    }
	}
    }
}
  
void
_add_box_as_lines (plPath *path, plPoint p0, plPoint p1, bool clockwise)
{
  bool x_move_is_first;
  plPoint newpoint;
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  _add_moveto (path, p0);
  
  /* if counterclockwise, would first pen motion be in x direction? */
  x_move_is_first = ((p1.x >= p0.x && p1.y >= p0.y)
		     || (p1.x < p0.x && p1.y < p0.y) ? true : false);
  
  if (clockwise)
    /* take complement */
    x_move_is_first = (x_move_is_first == true ? false : true);
  
  if (x_move_is_first)
    {
      newpoint.x = p1.x;
      newpoint.y = p0.y;
    }
  else
    {
      newpoint.x = p0.x;
      newpoint.y = p1.y;
    }
  _add_line (path, newpoint);
  
  _add_line (path, p1);
  
  if (x_move_is_first)
    {
      newpoint.x = p0.x;
      newpoint.y = p1.y;
    }
  else
    {
      newpoint.x = p1.x;
      newpoint.y = p0.y;
    }
  _add_line (path, newpoint);
  
  _add_line (path, p0);
  
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_ellipse_as_bezier3s (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise)
{
  plPoint startpoint, newpoint;
  double theta, costheta, sintheta;
  double xc, yc;
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  /* draw ellipse by drawing four elliptic arcs */
  theta = (M_PI / 180.0) * angle; /* convert to radians */
  costheta = cos (theta);
  sintheta = sin (theta);
  
  xc = pc.x;
  yc = pc.y;
  startpoint.x = xc + rx * costheta;
  startpoint.y = yc + rx * sintheta;
  _add_moveto (path, startpoint);
  
  if (clockwise)
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  else
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  _add_ellarc_as_bezier3 (path, pc, newpoint);
  
  newpoint.x = xc - rx * costheta;
  newpoint.y = yc - rx * sintheta;
  _add_ellarc_as_bezier3 (path, pc, newpoint);
  
  if (clockwise)
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  else
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  _add_ellarc_as_bezier3 (path, pc, newpoint);
  
  _add_ellarc_as_bezier3 (path, pc, startpoint);
  
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_ellipse_as_ellarcs (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise)
{
  plPoint startpoint, newpoint;
  double theta, costheta, sintheta;
  double xc, yc;
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  /* draw ellipse by drawing four elliptic arcs */
  theta = (M_PI / 180.0) * angle; /* convert to radians */
  costheta = cos (theta);
  sintheta = sin (theta);
  
  xc = pc.x;
  yc = pc.y;
  startpoint.x = xc + rx * costheta;
  startpoint.y = yc + rx * sintheta;
  _add_moveto (path, startpoint);
  
  if (clockwise)
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  else
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  _add_ellarc (path, pc, newpoint);
  
  newpoint.x = xc - rx * costheta;
  newpoint.y = yc - rx * sintheta;
  _add_ellarc (path, pc, newpoint);
  
  if (clockwise)
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  else
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  _add_ellarc (path, pc, newpoint);
  
  _add_ellarc (path, pc, startpoint);
  
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_ellipse_as_lines (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise)
{
  plPoint startpoint, newpoint;
  double theta, costheta, sintheta;
  double xc, yc;
  
  if (path == (plPath *)NULL)
    return;
  
  if (path->type != PATH_SEGMENT_LIST || path->num_segments > 0)
    return;
  
  /* draw ellipse by drawing four fake elliptic arcs */
  theta = (M_PI / 180.0) * angle; /* convert to radians */
  costheta = cos (theta);
  sintheta = sin (theta);
  
  xc = pc.x;
  yc = pc.y;
  startpoint.x = xc + rx * costheta;
  startpoint.y = yc + rx * sintheta;
  _add_moveto (path, startpoint);
  
  if (clockwise)
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  else
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  _add_ellarc_as_lines (path, pc, newpoint);
  
  newpoint.x = xc - rx * costheta;
  newpoint.y = yc - rx * sintheta;
  _add_ellarc_as_lines (path, pc, newpoint);
  
  if (clockwise)
    {
      newpoint.x = xc - ry * sintheta;
      newpoint.y = yc + ry * costheta;
    }
  else
    {
      newpoint.x = xc + ry * sintheta;
      newpoint.y = yc - ry * costheta;
    }
  _add_ellarc_as_lines (path, pc, newpoint);
  
  _add_ellarc_as_lines (path, pc, startpoint);
  
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_circle_as_bezier3s (plPath *path, plPoint pc, double radius, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  _add_ellipse_as_bezier3s (path, pc, radius, radius, 0.0, clockwise);
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_circle_as_ellarcs (plPath *path, plPoint pc, double radius, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  _add_ellipse_as_ellarcs (path, pc, radius, radius, 0.0, clockwise);
  path->primitive = true;	/* flag as flattened primitive */
}

void
_add_circle_as_lines (plPath *path, plPoint pc, double radius, bool clockwise)
{
  if (path == (plPath *)NULL)
    return;
  
  _add_ellipse_as_lines (path, pc, radius, radius, 0.0, clockwise);
  path->primitive = true;	/* flag as flattened primitive */
}

/* The _fakearc() subroutine below, which is called by _add_arc_as_lines()
   and _add_ellarc_as_lines(), contains a remote descendent of the
   arc-drawing algorithm of Ken Turkowski <turk@apple.com> described in
   Graphics Gems V.  His algorithm is a recursive circle subdivision
   algorithm, which relies on the fact that if s and s' are the (chordal
   deviation)/radius associated to (respectively) an arc and a half-arc,
   then s' is approximately equal to s/4.  The exact formula is s' = 1 -
   sqrt (1 - s/2), which applies for all s in the meaningful range, i.e. 0
   <= s <= 2.

   Ken's algorithm rotates the chord of an arc by 90 degrees and scales it
   to have length s'.  The resulting vector will be the chordal deviation
   vector of the arc, which gives the midpoint of the arc, and hence the
   endpoints of each half of the arc.  So this drawing technique is
   recursive.

   The problem with this approach is that scaling a vector to a specified
   length requires a square root, so there are two square roots in each
   subdivision step.  One can attempt to remove one of them by noticing
   that the chord half-length h always satisfies h = sqrt(s * (2-s))).  So
   one can rotate the chord vector by 90 degrees, and multiply its length
   by s/2h, i.e., s/2sqrt(s * (2-s)), to get the chordal deviation vector.
   This factor still includes a square root though.  Also one still needs
   to compute a square root in order to proceed from one subdivision step
   to the next, i.e. to compute s' from s.

   We can get around the square root problem by drawing only circular arcs
   with subtended angle of 90 degrees (quarter-circles), or elliptic arcs
   that are obtained from such quarter-circles by affine transformations
   (so-called quarter-ellipses).  To draw the latter, we need only replace
   the 90 degree rotation matrix mentioned above by an affine
   transformation that maps v0->-v1 and v1->v0, where v0 = p0 - pc and v1 =
   p1 - pc are the displacement vectors from the center of the ellipse to
   the endpoints of the arc.  If we do this, we get an elliptic arc with p0
   and p1 as endpoints. The vectors v0 and v1 are said lie along conjugate
   diameters of the quarter-ellipse.
   
   So for drawing quarter-ellipses, the only initial value of s we need to
   consider is the one for a quarter-circle, which is 1-sqrt(1/2).  The
   successive values of s/2h that will be encountered, after each
   bisection, are pre-computed and stored in a lookup table, found in
   g_arc.h.

   This approach lets us avoid, completely, any computation of square roots
   during the drawing of quarter-circles and quarter-ellipses.  The only
   square roots we need are precomputed.  We don't need any floating point
   divisions in the main loop either.  

   Of course for other angles than 90 degrees, we precompute a chord table
   and pass it to _fakearc().

   The implementation below does not use recursion (we use a local array,
   rather than the call stack, to store the sequence of generated
   points). */

static void 
_fakearc (plPath *path, plPoint p0, plPoint p1, int arc_type, const double *custom_chord_table, const double m[4])
{
  plPoint p[NUM_ARC_SUBDIVISIONS + 1], q[NUM_ARC_SUBDIVISIONS + 1];
  int level[NUM_ARC_SUBDIVISIONS + 1];
  int n = 0;	/* index of top of stack, < NUM_ARC_SUBDIVISIONS */
  int segments_drawn = 0;
  const double *our_chord_table;

  if (arc_type == USER_DEFINED_ARC)
    our_chord_table = custom_chord_table;
  else				/* custom_chord_table arg ignored */
    our_chord_table = _chord_table[arc_type];

  p[0] = p0;
  q[0] = p1;
  level[0] = 0;
  while (n >= 0)		/* i.e. while stack is nonempty */
    {
      if (level[n] >= NUM_ARC_SUBDIVISIONS) 
	{			/* draw line segment */
	  _add_line (path, q[n]);
	  segments_drawn++;
	  n--;
	}
      
      else			/* bisect line segment */
	{
	  plVector v;
	  plPoint pm, pb;
	  
	  v.x = q[n].x - p[n].x; /* chord = line segment from p[n] to q[n] */
	  v.y = q[n].y - p[n].y;
	  
	  pm.x = p[n].x + 0.5 * v.x; /* midpoint of chord */
	  pm.y = p[n].y + 0.5 * v.y;
	  
	  /* Compute bisection point.  If m=[0 1 -1 0] this just rotates
	     the chord clockwise by 90 degrees, and scales it to yield the
	     chordal deviation vector, which is used as an offset. */
	  
	  pb.x = pm.x + 
	    our_chord_table[level[n]] * (m[0] * v.x + m[1] * v.y);
	  pb.y = pm.y + 
	    our_chord_table[level[n]] * (m[2] * v.x + m[3] * v.y);
	  
	  /* replace line segment by pair; level[n] >= n is an invariant */
	  p[n+1] = p[n];
	  q[n+1] = pb;		/* first half, deal with next */
	  level[n+1] = level[n] + 1;
	  
	  p[n] = pb;
	  q[n] = q[n];		/* second half, deal with later */
	  level[n] = level[n] + 1;
	  
	  n++;
	}
    }
}

/* prepare_chord_table() computes the list of chordal deviation factors
   that _fakearc() needs when it is employed to draw a circular arc of
   subtended angle other than the default angles it supports */
static void
_prepare_chord_table (double sagitta, double custom_chord_table[TABULATED_ARC_SUBDIVISIONS])
{
  double half_chord_length;
  int i;

  half_chord_length = sqrt ( sagitta * (2.0 - sagitta) );
  for (i = 0; i < TABULATED_ARC_SUBDIVISIONS; i++)
    {
      custom_chord_table[i] = 0.5 * sagitta / half_chord_length;
      sagitta = 1.0 - sqrt (1.0 - 0.5 * sagitta);
      half_chord_length = 0.5 * half_chord_length / (1.0 - sagitta);
    }
}

/* Flatten a simple path into a path of segment list type, consisting only
   of line segments.  

   As supplied, the path may be perfectly general: it may be a segment list
   (not all segments necessarily being line segments), or be a closed
   primitive (box/circle/ellipse).  If supplied path already consists only
   of line segments (with an initial moveto and possibly a final
   closepath), it is returned unchanged; this can be tested for by
   comparing pointers for equality.  If a new path is returned, it must be
   freed with _delete_plPath(). */

plPath *
_flatten_path (const plPath *path)
{
  plPath *newpath;
  
  if (path == (plPath *)NULL)
    return (plPath *)NULL;

  switch (path->type)
    {
    case PATH_SEGMENT_LIST:
      {
	bool do_flatten = false;
	int i;

	for (i = 0; i < path->num_segments; i++)
	  {
	    if (path->segments[i].type != S_MOVETO
		&& path->segments[i].type != S_LINE
		&& path->segments[i].type != S_CLOSEPATH)
	      {
		do_flatten = true;
		break;
	      }
	  }
	
	if (do_flatten == false)
	  newpath = (plPath *)path; /* just return original path */
	else
	  {
	    newpath = _new_plPath ();
	    for (i = 0; i < path->num_segments; i++)
	      {
		switch ((int)(path->segments[i].type))
		  {
		  case (int)S_MOVETO:
		    _add_moveto (newpath, path->segments[i].p);
		    break;
		  case (int)S_LINE:
		    _add_line (newpath, path->segments[i].p);
		    break;
		  case (int)S_CLOSEPATH:
		    _add_closepath (newpath);
		    break;

		    /* now, the types of segment we flatten: */

		  case (int)S_ARC:
		    _add_arc_as_lines (newpath, 
				       path->segments[i].pc, 
				       path->segments[i].p);
		    break;
		  case (int)S_ELLARC:
		    _add_ellarc_as_lines (newpath, 
					  path->segments[i].pc, 
					  path->segments[i].p);
		    break;
		  case (int)S_QUAD:
		    _add_bezier2_as_lines (newpath, 
					   path->segments[i].pc, 
					   path->segments[i].p);
		    break;
		  case (int)S_CUBIC:
		    _add_bezier3_as_lines (newpath, 
					   path->segments[i].pc, 
					   path->segments[i].pd, 
					   path->segments[i].p);
		    break;
		  default:	/* shouldn't happen */
		    break;
		  }
	      }
	  }
	break;
      }
    case PATH_CIRCLE:
      newpath = _new_plPath ();
      _add_circle_as_lines (newpath, 
			    path->pc, path->radius, path->clockwise);
      break;
    case PATH_ELLIPSE:
      newpath = _new_plPath ();
      _add_ellipse_as_lines (newpath, 
			     path->pc, path->rx, path->ry, path->angle,
			     path->clockwise);
      break;
    case PATH_BOX:
      newpath = _new_plPath ();
      _add_box_as_lines (newpath, path->p0, path->p1, path->clockwise);
      break;
    default:			/* shouldn't happen */
      newpath = _new_plPath ();
      break;
    }
  return newpath;
}

/**********************************************************************/

/* The code below exports the _merge_paths() function, which munges an
   array of plPath objects and returns a new array.  Heuristic
   "path-merging" of this sort is performed in g_endpath.c when filling a
   compound path (i.e., a multi-plPath path), if the output driver supports
   only the filling of single plPaths.  That is the case for nearly all of
   libplot's output drivers.

   `Child' paths are merged into their `parents', so each location in the
   returned array where a child was present will contain NULL.  Also, any
   non-child that had no children will be returned without modification.

   You should take this into account when freeing the returned array of
   plPaths.  Only the elements that are (1) non-NULL, and (2) differ from
   the corresponding elements in the originally passed array should have
   _delete_plPath() invoked on them.  The array as a whole may be
   deallocated by calling free().

   The _merge_paths() function was inspired by a similar function in
   Wolfgang Glunz's pstoedit, which was originally written by Burkhard
   Plaum <plaum@ipf.uni-stuttgart.de>. */

/* Note: a well-formed plPath has the form:
   { moveto { line | arc }* { closepath }? }

   The _merge_paths function was written to merge _closed_ plPath's,
   i.e. ones whose endpoint is the same as the startpoint (possibly
   implicitly, i.e. closepath is allowed).  However, libplot applies it to
   open paths too, in which case an `implicit closepath' is added to the
   path to close it.

   NOTE: The current release of libplot does not yet support `closepath'
   segments at a higher level.  So we regard a pass-in plPath as `closed'
   if its last defining vertex is the same as the first.  THIS CONVENTION
   WILL GO AWAY. */

/* ad hoc structure for an annotated plPath, in particular one that has
   been flattened into line segments and annotated; used only in this file,
   for merging purposes */

typedef struct subpath_struct
{
  plPathSegment *segments;	/* segment array */
  int num_segments;		/* number of segments */

  struct subpath_struct **parents; /* array of pointers to possible parents */
  struct subpath_struct *parent; /* pointer to parent path */
  struct subpath_struct **children; /* array of pointers to child paths */
  int num_children;		/* number of children */

  int num_outside;		/* number of subpaths outside this one */

  double llx, lly, urx, ury;    /* bounding box of the subpath */
  bool inserted;		/* subpath has been inserted into result? */
} subpath;

/* forward references */

/* 0. ctors, dtors */
static subpath * new_subpath (void);
static subpath ** new_subpath_array (int n);
static void delete_subpath (subpath *s);
static void delete_subpath_array (subpath **s, int n);

/* 1. functions that act on a subpath, i.e. an `annotated path' */
static bool is_inside_of (const subpath *s, const subpath *other);
static double _cheap_lower_bound_on_distance (const subpath *path1, const subpath *path2);
static void linearize_subpath (subpath *s);
static void read_into_subpath (subpath *s, const plPath *path);

/* 2. miscellaneous */
static void find_parents_in_subpath_list (subpath **annotated_paths, int num_paths);
static void insert_subpath (plPathSegment *parent_segments, const plPathSegment *child_segments, int parent_size, int child_size, int parent_index, int child_index);
static void _compute_closest (const plPathSegment *p1, const plPathSegment *p2, int size1, int size2, double *distance, int *index1, int *index2);

/**********************************************************************/

/* ctor for subpath class */
static subpath * 
new_subpath (void)
{
  subpath *s;
  
  s = (subpath *)_pl_xmalloc (sizeof (subpath));

  s->segments = (plPathSegment *)NULL;
  s->num_segments = 0;
  s->parents = (subpath **)NULL;
  s->parent = (subpath *)NULL;
  s->children = (subpath **)NULL;
  s->num_children = 0;
  s->num_outside = 0;
  s->llx = DBL_MAX;
  s->lly = DBL_MAX;
  s->urx = -DBL_MAX;
  s->ury = -DBL_MAX;
  s->inserted = false;

  return s;
}
  
/* corresponding ctor for a subpath array */
static subpath **
new_subpath_array (int n)
{
  int i;
  subpath **s;
  
  s = (subpath **)_pl_xmalloc (n * sizeof (subpath *));
  for (i = 0; i < n; i++)
    s[i] = new_subpath ();

  return s;
}
  
/* dtor for subpath class */
static void
delete_subpath (subpath *s)
{
  if (s)
    {
      if (s->segments)
	free (s->segments);
      if (s->children)
	free (s->children);
      if (s->parents)
	free (s->parents);

      free (s);
    }
}

/* corresponding dtor for a subpath array */
static void
delete_subpath_array (subpath **s, int n)
{
  int i;

  if (s)
    {
      for (i = 0; i < n; i++)
	delete_subpath (s[i]);
      free (s);
    }
}

/* replace every segment in a subpath by a lineto (invoked only on a child
   subpath, i.e. a subpath with an identified parent) */
static void 
linearize_subpath (subpath *s)
{
  /* replace first segment (moveto) with a lineto */
  s->segments[0].type = S_LINE;

  /* if final segment is a closepath, also replace with a lineto, back to
     point #0 */
  if (s->segments[s->num_segments-1].type == S_CLOSEPATH)
    {
      s->segments[s->num_segments-1].type = S_LINE;
      s->segments[s->num_segments-1].p = s->segments[0].p;
    }
}

/* Read a sequence of plPathSegments from a plPath into a previously empty
   annotated subpath.  (This is called only after the plPath has been
   flattened, so the segments include no arcs.)

   Because the way in which _merge_paths() is currently called in libplot,
   we need to handle the possibility that the plPath may not be closed.  If
   it isn't, we add a closepath.

   At present, we allow a final lineto to the start vertex to serve the
   same purpose.  THIS IS A LIBPLOT CONVENTION THAT WILL GO AWAY. */

static void
read_into_subpath (subpath *s, const plPath *path)
{
  bool need_to_close = false;
  int i;
  
  /* sanity check */
  if (path->type != PATH_SEGMENT_LIST)
    return;

  /* allocate space for segment array of subpath; add 1 extra slot for
     manual closure, if needed */
  s->segments = (plPathSegment *)_pl_xmalloc((path->num_segments + 1) * sizeof (plPathSegment));
  s->num_segments = path->num_segments;

  /* sanity check */
  if (path->num_segments == 0)
    return;

  /* Is this path closed?  If not, we'll close manually the annotated path
     that we'll construct.  WE CURRENTLY TREAT FINAL = INITIAL AS
     INDICATING CLOSURE. */
  if (path->segments[path->num_segments - 1].type != S_CLOSEPATH
      &&
      (path->segments[path->num_segments - 1].p.x != path->segments[0].p.x
       || path->segments[path->num_segments - 1].p.y != path->segments[0].p.y))
    need_to_close = true;

  /* copy the segments, updating bounding box to take each juncture point
     into account */
  for (i = 0; i < path->num_segments; i++)
    {
      plPathSegment e;
      
      e = path->segments[i];
      s->segments[i] = e;

      if (e.p.x < s->llx)
	s->llx = e.p.x;
      if (e.p.y < s->lly)
	s->lly = e.p.y;
      if (e.p.x > s->urx)
	s->urx = e.p.x;
      if (e.p.y > s->ury)
	s->ury = e.p.y;
    }

  if (need_to_close)
    {
#if 0
      s->segments[path->num_segments].type = S_CLOSEPATH;
#else  /* currently, use line segment instead of closepath */
      s->segments[path->num_segments].type = S_LINE;
#endif
      s->segments[path->num_segments].p = path->segments[0].p;
      s->num_segments++;
    }
}

/* check if a subpath is inside another subpath */
static bool 
is_inside_of (const subpath *s, const subpath *other)
{
  int inside = 0;
  int outside = 0;
  int i;
  
  /* if bbox fails to lie inside the other's bbox, false */
  if (!((s->llx >= other->llx) && (s->lly >= other->lly) &&
	(s->urx <= other->urx) && (s->ury <= other->ury)))
    return false;
  
  /* otherwise, check all juncture points */
  for (i = 0; i < s->num_segments; i++)
    {
      bool point_is_inside;

      if (s->segments[i].type == S_CLOSEPATH)
	/* should have i = num_segments - 1, no associated juncture point */
	continue;

      /* Check if the vertex s->segments[i].p is inside `other'.  Could be
	 done in a separate function, but we inline it for speed. */
      {
	/* These two factors should be small positive floating-point
	   numbers.  They should preferably be incommensurate, to minimize
	   the probability of a degenerate case occurring: two line
	   segments intersecting at the endpoint of one or the other. */
#define SMALL_X_FACTOR (M_SQRT2 * M_PI)
#define SMALL_Y_FACTOR (M_SQRT2 + M_PI)

	/* argument of the now-inlined function (besides `other') */
	plPoint p;
	/* local variables of the now-inlined function */
	int k, crossings;
	/* (x1,y1) is effectively the point at infinity */
	double x1, y1;
	/* (x2,y2) is specified point */
	double x2, y2;
	
	/* argument of the now-inlined function (besides `other') */
	p = s->segments[i].p;
  
	/* (x1,y1) is effectively the point at infinity */
	x1 = (DMAX(p.x, other->urx) 
	      + SMALL_X_FACTOR * (DMAX(p.x, other->urx) 
				  - DMIN(p.x, other->llx)));
	y1 = (DMAX(p.y, other->ury) 
	      + SMALL_Y_FACTOR * (DMAX(p.y, other->ury) 
				  - DMIN(p.y, other->lly)));
	
	/* (x2,y2) is specified point */
	x2 = p.x;
	y2 = p.y;
	
	crossings = 0;
	for (k = 0; k < other->num_segments; k++)
	  {
	    int j;
	    double x3, y3, x4, y4, det, det1, det2;
	    
	    if (other->segments[k].type == S_CLOSEPATH) /* k > 0 */
	      {
		x3 = other->segments[k-1].p.x;
		y3 = other->segments[k-1].p.y;
	      }
	    else
	      {
		x3 = other->segments[k].p.x;
		y3 = other->segments[k].p.y;
	      }
	    
	    j = (k == other->num_segments - 1 ? 0 : k + 1);
	    if (other->segments[j].type == S_CLOSEPATH)
	      continue;
	    
	    x4 = other->segments[j].p.x;
	    y4 = other->segments[j].p.y;
	    
	    /* (x3,y3)-(x4,y4) is a line segment in the closed path */
	    
	    /* Check whether the line segments (x1,y1)-(x2,y2) and
	       (x3-y3)-(x4,y4) cross each other.
	       
	       System to solve is:
	       
	       [p1 + (p2 - p1) * t1] - [p3 + (p4 - p3) * t2] = 0
	       
	       i.e.
	       
	       (x2 - x1) * t1 - (x4 - x3) * t2 = x3 - x1;
	       (y2 - y1) * t1 - (y4 - y3) * t2 = y3 - y1;
	       
	       Solutions are: t1 = det1/det
	                      t2 = det2/det
	       
	       The line segments cross each other (in their interiors) if
	       0.0 < t1 < 1.0 and 0.0 < t2 < 1.0 */
      
	    det = (x2 - x1) * (-(y4 - y3)) - (-(x4 - x3)) * (y2 - y1);
	    if (det == 0.0)
	      /* line segments are parallel; ignore the degenerate case
		 that they might overlap */
	      continue;
      
	    det1 = (x3 - x1) * (-(y4 - y3)) - (-(x4 - x3)) * (y3 - y1);
	    det2 = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
	    
	    if ((det<0.0 && (det1>0.0 || det2>0.0 || det1<det || det2<det))
		||
		(det>0.0 && (det1<0.0 || det2<0.0 || det1>det || det2>det)))
	      /* solution for at least one of t1 and t2 is outside the
		 interval [0,1], so line segments do not cross */
	      continue;
       
	    /* We ignore the possibility that t1, t2 are both in the interval
	       [0,1], but
	       (t1 == 0.0) || (t1 == 1.0) || (t2 == 0.0) || (t2 == 1.0).

	       t1 == 0.0 should never happen, if p1 is effectively
	       the point at infinity.

	       So this degenerate case occurs only if the line segment
	       (x1,y1)-(x2,y2) goes through either (x3,y3) or (x4,y4), or
	       the specified point (x2,y2) lies on the line segment
	       (x3,y3)-(x4,y4) that is part of the path. */
      
	    crossings++;
	  }
	
	/* our determination of whether the point is inside the path;
	   before we inlined this function, this was the return value */
	point_is_inside = (crossings & 1) ? true : false;
      }

      /* increment inside,outside depending on whether or not the juncture
	 point was inside the other path */
      if (point_is_inside)
	inside++;
      else
	outside++;
    }

  /* make a democratic decision as to whether the path as a whole is inside
     the other path */
  return (inside > outside ? true : false);
}

/* Find parent (if any) of each subpath in a list of subpaths.  When this
   is invoked, each subpath should consist of an initial moveto, at least
   one lineto, and a closepath (not currently enforced). */

static void 
find_parents_in_subpath_list (subpath **annotated_paths, int num_paths)
{
  int i, j;
  subpath *parent;
  
  /* determine for each subpath the subpaths that are nominally outside it */
  for (i = 0; i < num_paths; i++)
    {
      annotated_paths[i]->parents = new_subpath_array (num_paths);
      for (j = 0; j < num_paths; j++)
	{
	  if (j != i)
	    {
	      if (is_inside_of (annotated_paths[i], annotated_paths[j]))
		{
		  annotated_paths[i]->parents[annotated_paths[i]->num_outside] = 
		    annotated_paths[j];
		  annotated_paths[i]->num_outside++;
		}
	    }
	}
    }

  /* Now find the real parent subpaths, i.e. the root subpaths.  A subpath
     is a parent subpath if the number of nominally-outside subpaths is
     even, and is a child subpath only if the number is odd.  An odd
     number, together with a failure to find a suitable potential parent,
     will flag a path as an isolate: technically a parent, but without
     children. */

  for (i = 0; i < num_paths; i++)
    {
      if ((annotated_paths[i]->num_outside & 1) == 0)
	/* an even number of subpaths outside, definitely a parent
	   (i.e. doesn't have a parent itself, may or may not have children) */
	{
	  /* allocate space for children (if any) */
	  annotated_paths[i]->children = new_subpath_array (num_paths);
	}
    }
  
  /* now determine which are children, and link them to their parents */

  for (i = 0; i < num_paths; i++)
    {
      if ((annotated_paths[i]->num_outside & 1) == 0)
	/* even number outside, definitely a parent subpath (whether it has
           children remains to be determined) */
	continue;
      else
	/* odd number outside, possibly a child, so search linearly through
	   possible parents until we get a hit; if so, this is a child; if
	   not, this is an isolate (classed as a parent) */
	{
	  for (j = 0; j < annotated_paths[i]->num_outside; j++)
	    {
	      if (annotated_paths[i]->num_outside == 
		  annotated_paths[i]->parents[j]->num_outside + 1)
		/* number outside is one more than the number outside a
		   potential parent; flag as a child, and add it to the
		   parent's child list */
		{
		  parent = annotated_paths[i]->parents[j];
		  annotated_paths[i]->parent = parent; /* give it a parent */
		  parent->children[parent->num_children] = annotated_paths[i];
		  parent->num_children++;
		  break;
		}
	    }
	}
    }
}

/* Compute closest vertices in two paths.  Indices of closest vertices, and
   (squared) distance between them, are returned via pointers.

   This is invoked in _merge_paths() only on paths that have been
   flattened, and have had the initial moveto and the optional final
   closepath replaced by line segments.  So when this is called, each
   segment type is S_LINE. */

static void
_compute_closest (const plPathSegment *p1, const plPathSegment *p2, int size1, int size2, double *distance, int *index1, int *index2)
{
  int best_i = 0, best_j = 0;	/* keep compiler happy */
  double best_distance = DBL_MAX;
  int ii, jj;
  
  for (ii = 0; ii < size1; ii++)
    {
      plPoint point1;
      
      point1 = p1[ii].p;
      for (jj = 0; jj < size2; jj++)
	{
	  double tmp1, tmp2, distance;
	  plPoint point2;
	  
	  point2 = p2[jj].p;
	  tmp1 = point1.x - point2.x;
	  tmp2 = point1.y - point2.y;
	  distance = tmp1 * tmp1 + tmp2 * tmp2;
	  if (distance < best_distance)
	    {
	      best_distance = distance;
	      best_i = ii;
	      best_j = jj;
	    }
	}
    }
  
  /* return the three quantities */
  *distance = best_distance;
  *index1 = best_i;
  *index2 = best_j;
}

/* Compute a cheap lower bound on the (squared) distance between two
   subpaths, by looking at their bounding boxes. */

static double
_cheap_lower_bound_on_distance (const subpath *path1, const subpath *path2)
{
  double xdist = 0.0, ydist = 0.0, dist;
  
  if (path1->urx < path2->llx)
    xdist = path2->llx - path1->urx;
  else if (path2->urx < path1->llx)
    xdist = path1->llx - path2->urx;    
    
  if (path1->ury < path2->lly)
    ydist = path2->lly - path1->ury;
  else if (path2->ury < path1->lly)
    ydist = path1->lly - path2->ury;    

  dist = xdist * xdist + ydist * ydist;

  return dist;
}

/* Insert a closed child subpath into a closed parent, by connecting the
   two, twice, at a specified vertex of the parent path and a specified
   vertex of the child path.  This is the key function invoked by
   _merge_paths().

   Both paths are supplied as segment lists, and all segments are lines;
   the final segment of each is a line back to the starting point.  I.e.,
   for both subpaths, the final vertex duplicates the start vertex.  So we
   ignore the final vertex of the child (but not that of the parent).
   I.e. if the child vertices are numbered 0..child_size-1, we map the case
   child_index = child_size-1 to child_index = 0.

   The new path has parent_size + child_size + 1 vertices, of which the
   last is a repetition of the first. */

  /* INDEX MAP: 

     PARENT -> NEW PATH
       i --> i                (if 0 <= i < parent_index + 1)
       i --> i+child_size+1   (if parent_index + 1 <= i < parent_size)

     CHILD -> NEW PATH
       i --> i+parent_index+child_size-child_index  (0 <= i < child_index+1)
       i --> i+parent_index-child_index+1   (child_index+1 <= i < child_size-1)
       		(0'th element of the child is same as child_size-1 element)

     NEW PATH                        CONTENTS

     0..parent_index	             
                                     0..parent_index of PARENT
     parent_index+1
                                     child_index of CHILD (i.e. ->join)
     parent_index+2..parent_index+child_size-child_index-1
                                     child_index+1..child_size-2 of CHILD
     parent_index+child_size-child_index..parent_index+child_size
                                     0..child_index of CHILD
     parent_index+child_size+1
                                     parent_index of PARENT (i.e. ->join)
     parent_index+child_size+2..parent_size+child_size
                                     parent_index+1..parent_size-1 of PARENT

  */

/* Macros that map from vertices in the child path and the parent path, to
   vertices in the merged path.  Here the argument `i' is the index in the
   original path, and each macro evaluates to the index in the merger.  */

/* The first macro should not be applied to i=child_size-1; as noted above,
   that vertex is equivalent to i=0, so apply it to i=0 instead. */

#define CHILD_VERTEX_IN_MERGED_PATH(i,parent_index,parent_size,child_index,child_size) ((i) <= (child_index) ? (i) + (parent_index) + (child_size) - (child_index) : (i) + (parent_index) - (child_index) + 1)
#define PARENT_VERTEX_IN_MERGED_PATH(i,parent_index,parent_size,child_index,child_size) ((i) <= (parent_index) ? (i) : (i) + (child_size) + 1)


static void 
insert_subpath (plPathSegment *parent, const plPathSegment *child, int parent_size, int child_size, int parent_index, int child_index)
{
  int i;
  plPathSegment e1, e2;
  int src_index;
  
  /* map case when joining vertex is final vertex of child to case when
     it's the 0'th vertex */
  if (child_index == child_size - 1)
    child_index = 0;

  /* move up: add child_size+1 empty slots to parent path */
  for (i = parent_size - 1; i >= parent_index + 1; i--)
    parent[i + child_size + 1] = parent[i];
  
  /* add a line segment from specified vertex of parent path to specified
     vertex of child path */
  e1 = child[child_index];
  e1.type = S_LINE;		/* unnecessary */
  parent[parent_index + 1] = e1;
  
  /* copy vertices of child into parent, looping back to start in child if
     necessary; note we skip the last (i.e. child_size-1'th) vertex, since
     the 0'th vertex is the same */
  src_index = child_index;
  for (i = 0; i < child_size - 1; i++)
    {
      src_index++;
      if (src_index == child_size - 1)
	src_index = 0;
      parent[parent_index + 2 + i] = child[src_index];
    }
  
  /* add a line segment back from specified vertex of child path to
     specified vertex of parent path */
  e2 = parent[parent_index];
  e2.type = S_LINE;
  parent[parent_index + child_size + 1] = e2;
}

/* The key function exported by this module, which is used by libplot for
   filling compound paths. */

plPath **
_merge_paths (const plPath **paths, int num_paths)
{
  int i;
  subpath **annotated_paths;
  plPath **flattened_paths;
  plPath **merged_paths;

  /* flatten every path to a list of line segments (some paths may come
     back unaltered; will be able to compare pointers to check for that) */
  flattened_paths = (plPath **)_pl_xmalloc (num_paths * sizeof(plPath *));
  for (i = 0; i < num_paths; i++)
    {
      flattened_paths[i] = _flatten_path (paths[i]);
#ifdef DEBUG
      fprintf (stderr, "path %d: %d segments, flattened to %d segments\n",
	       i, paths[i]->num_segments, flattened_paths[i]->num_segments);
#endif
    }

  /* Copy each flattened path into a corresponding annotated path
     (`subpath').  Manual closure, if necessary (see above) is performed,
     i.e. we always add a final closepath to close the path.  At this stage
     bounding boxes are computed. */
  annotated_paths = new_subpath_array (num_paths);
  for (i = 0; i < num_paths; i++)
    read_into_subpath (annotated_paths[i], flattened_paths[i]);

  /* Flattened paths no longer needed, so delete them carefully (some may
     be the same as the original paths, due to _flatten_path() having
     simply returned its argument) */
  for (i = 0; i < num_paths; i++)
    if (flattened_paths[i] != paths[i])
      _delete_plPath (flattened_paths[i]);

  /* determine which subpaths are parents, children */
  find_parents_in_subpath_list (annotated_paths, num_paths);

  /* in each child, replace each moveto/closepath by a lineto */
  for (i = 0; i < num_paths; i++)
    if (annotated_paths[i]->parent != (subpath *)NULL)
      /* child path */
      linearize_subpath (annotated_paths[i]);

  /* create array of merged paths: parent paths will have child paths
     merged into them, and child paths won't appear */

  /* allocate space for new array, to be returned */
  merged_paths = (plPath **)_pl_xmalloc (num_paths * sizeof(plPath *));

  for (i = 0; i < num_paths; i++)
    {
      int j, k, num_segments_in_merged_path;
      subpath *parent;
      plPath *merged_path;
      double *parent_to_child_distances;
      int *child_best_indices, *parent_best_indices;

      if (annotated_paths[i]->parent != (subpath *)NULL)
	/* child path; original path will be merged into parent */
	{
	  merged_paths[i] = (plPath *)NULL;
	  continue;
	}

      if (annotated_paths[i]->num_children == 0)
	/* no parent, but no children either, so no merging done; in output
	   path array, place original unflattened path */
	{
	  merged_paths[i] = (plPath *)paths[i];
	  continue;
	}

      /* this path must be a parent, with one or more children to be merged
	 into it; so create new empty `merged path' with segments array
	 that will hold it, and the merged-in children */
      parent = annotated_paths[i];
      num_segments_in_merged_path = parent->num_segments;
      for (j = 0; j < parent->num_children; j++)
	num_segments_in_merged_path 
	  += (parent->children[j]->num_segments + 1);

      merged_path = _new_plPath ();
      merged_path->segments = (plPathSegment *)_pl_xmalloc(num_segments_in_merged_path * sizeof (plPathSegment));
      merged_path->num_segments = 0;
      merged_path->segments_len = num_segments_in_merged_path;

      /* copy parent path into new empty path, i.e. initialize the merged
         path */
      for (j = 0; j < parent->num_segments; j++)
	merged_path->segments[j] = parent->segments[j];
      merged_path->num_segments = parent->num_segments;
      
      /* Create temporary storage for `closest vertex pairs' and inter-path
	 distances.  We first compute the shortest distance between each
	 child path.  We also keep track of the shortest distance between
	 each child and the merged path being constructed, and update it
	 when any child is added.  */

      parent_to_child_distances = (double *)_pl_xmalloc(parent->num_children * sizeof (double));
      parent_best_indices = (int *)_pl_xmalloc(parent->num_children * sizeof (int));
      child_best_indices = (int *)_pl_xmalloc(parent->num_children * sizeof (int));

      /* compute closest vertices between merged path (i.e., right now, the
	 parent) and any child; these arrays will be updated when any child
	 is inserted into the merged path */
      for (j = 0; j < parent->num_children; j++)
	_compute_closest (parent->segments,
			  parent->children[j]->segments,
			  parent->num_segments,
			  parent->children[j]->num_segments,
			  &(parent_to_child_distances[j]),
			  &(parent_best_indices[j]),
			  &(child_best_indices[j]));
      
      for (k = 0; k < parent->num_children; k++)
	/* insert a child (the closest remaining one!) into the built-up
           merged path; and flag the child as having been inserted so that
           we don't pay attention to it thereafter */
	{
	  double min_distance;
	  int closest = 0; /* keep compiler happy */
	  double *new_parent_to_child_distances;
	  int *new_child_best_indices, *new_parent_best_indices;

	  /* allocate storage for arrays that will be used to update the
	     three abovementioned arrays, with each pass through the loop */
	  new_parent_to_child_distances = (double *)_pl_xmalloc(parent->num_children * sizeof (double));
	  new_parent_best_indices = (int *)_pl_xmalloc(parent->num_children * sizeof (int));
	  new_child_best_indices = (int *)_pl_xmalloc(parent->num_children * sizeof (int));

	  /* initially, they're the same as the current arrays */
	  for (j = 0; j < parent->num_children; j++)
	    {
	      new_parent_to_child_distances[j] = parent_to_child_distances[j];
	      new_parent_best_indices[j] = parent_best_indices[j];
	      new_child_best_indices[j] = child_best_indices[j];
	    }

	  /* find closest child to merged path, which has not yet been
             inserted */
	  min_distance = DBL_MAX;
	  for (j = 0; j < parent->num_children; j++)
	    {
	      if (parent->children[j]->inserted) /* ignore this child */
		continue;
	      
	      if (parent_to_child_distances[j] < min_distance)
		{
		  closest = j;
		  min_distance = parent_to_child_distances[j];
		}
	    }
	  
	  /* closest remaining child has index `closest'; it will be
	     inserted into the current merged path */

	  /* loop over all children, skipping inserted ones and also
	     skipping `closest', the next child to be inserted */
	  for (j = 0; j < parent->num_children; j++)
	    {
	      double inter_child_distance;
	      int inter_child_best_index1, inter_child_best_index2;
	      double lower_bound_on_inter_child_distance;
	      bool compute_carefully;

	      if (parent->children[j]->inserted) /* ignore */
		continue;
	      
	      if (j == closest)	/* ignore */
		continue;
	      
	      /* compute distance (and closest vertex pairs) between
		 `closest' and the j'th child; result is only of interest
		 if the distance is less than parent_to_child_distances[j],
		 so we first compute a cheap lower bound on the result by
		 looking at bounding boxes. */

	      lower_bound_on_inter_child_distance = 
		_cheap_lower_bound_on_distance (parent->children[j],
						parent->children[closest]);
	      compute_carefully = 
		(lower_bound_on_inter_child_distance < 
		 parent_to_child_distances[j]) ? true : false;

	      if (compute_carefully)
		/* compute accurate inter-child distance; also which two
		   vertices yield the minimum distance */
		_compute_closest (parent->children[j]->segments,
				  parent->children[closest]->segments,
				  parent->children[j]->num_segments,
				  parent->children[closest]->num_segments,
				  &inter_child_distance,
				  &inter_child_best_index1, /* vertex in j */
				  &inter_child_best_index2); /* in `closest' */
	      
	      /* fill in j'th element of the new arrays
		 parent_to_child_distances[], parent_best_indices[] and
		 child_best_indices[] so as to take the insertion of the
		 child into account; but we don't update the old arrays
		 until we do the actual insertion */

	      if (compute_carefully &&
		  inter_child_distance < parent_to_child_distances[j])
		/* j'th child is nearer to a vertex in `closest', the child
		   to be inserted, than to any vertex in the current merged
		   path, so all three arrays are affected */
		{
		  int nearest_index_in_closest_child;

		  new_parent_to_child_distances[j] = inter_child_distance;
		  new_child_best_indices[j] = inter_child_best_index1;
		  nearest_index_in_closest_child = inter_child_best_index2;
		  
		  /* Compute new value of parent_best_indices[j], taking
		     into account that `closest' will be inserted into the
		     merged path, thereby remapping the relevant index in
		     `closest'.  The macro doesn't perform correctly if its
		     first arg takes the maximum possible value; so
		     instead, we map that possibility to `0'.  See comment
		     above, before the macro definition. */

		  if (nearest_index_in_closest_child == 
		      parent->children[closest]->num_segments - 1)
		    nearest_index_in_closest_child = 0;
		  new_parent_best_indices[j] = 
		    CHILD_VERTEX_IN_MERGED_PATH(nearest_index_in_closest_child,
						parent_best_indices[closest],
						merged_path->num_segments, 
						child_best_indices[closest],
						parent->children[closest]->num_segments);
		}
	      else
		/* j'th child is nearer to a vertex in the current merged
		   path than to any vertex in `closest', the child to be
		   inserted into the merged path */
		{
		  int nearest_index_in_parent;

		  nearest_index_in_parent = parent_best_indices[j];

		  /* compute new value of parent_best_indices[j], taking
		     into account that `closest' will be inserted into the
		     merged path, thereby remapping the relevant index in
		     the merged path */
		  new_parent_best_indices[j] = 
		    PARENT_VERTEX_IN_MERGED_PATH(nearest_index_in_parent,
						 parent_best_indices[closest],
						 merged_path->num_segments, 
						 child_best_indices[closest],
						 parent->children[closest]->num_segments);
		}
	    }
	  
	  /* do the actual insertion, by adding a pair of lineto's between
             closest vertices; flag child as inserted */
	  insert_subpath (merged_path->segments, 
			  parent->children[closest]->segments, 
			  merged_path->num_segments, 
			  parent->children[closest]->num_segments,
			  parent_best_indices[closest],
			  child_best_indices[closest]);
	  merged_path->num_segments += 
	    (parent->children[closest]->num_segments + 1);
	  parent->children[closest]->inserted = true;

	  /* update the old arrays to take insertion into account: replace
             them by the new ones */
	  for (j = 0; j < parent->num_children; j++)
	    {
	      parent_to_child_distances[j] = new_parent_to_child_distances[j];
	      parent_best_indices[j] = new_parent_best_indices[j];
	      child_best_indices[j] = new_child_best_indices[j];
	    }

	  free (new_parent_to_child_distances);
	  free (new_parent_best_indices);
	  free (new_child_best_indices);
	}
      /* End of loop over all children of parent subpath; all >=1 children
	 have now been inserted into the parent, i.e. into the `merged
	 path' which the parent initialized.  However, the merged path's
	 segments are all lines; so change the first to a moveto. */

      merged_path->segments[0].type = S_MOVETO;
      merged_paths[i] = merged_path;

      /* NOTE: SHOULD ALSO REPLACE LAST LINE SEGMENT BY A CLOSEPATH! */

      /* delete temporary storage for `closest vertex pairs' and inter-path
         distances */
      free (parent_to_child_distances);
      free (parent_best_indices);
      free (child_best_indices);
    }
  /* end of loop over parent subpaths */

  /* no more annotated paths needed */
  delete_subpath_array (annotated_paths, num_paths);

  return merged_paths;
}
