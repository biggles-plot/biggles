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

/* This file contains the internal path_is_flushable() method, which is
   invoked after any path segment is added to the segment list, provided
   (0) the segment list has become greater than or equal to the
   `max_unfilled_path_length' Plotter parameter, (1) the path isn't to be
   filled.  In most Plotters, this operation simply returns true. */

/* This file also contains the internal maybe_prepaint_segments() method.
   It is called immediately after any segment is added to a path.  Some
   Plotters, at least under some circumstances, treat endpath() as a no-op.
   Instead, they plot the segments of a path in real time.  */

#include "sys-defines.h"
#include "extern.h"

/* for Cohen-Sutherland clipper, see g_clipper.c */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* forward references */
static void _emit_regis_vector (plIntPoint istart, plIntPoint iend, bool skip_null, char *tmpbuf);

void
_pl_r_paint_path (S___(Plotter *_plotter))
{
  char tmpbuf[32];

  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	int i;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /*shouldn't happen */
	  break;

	if (_plotter->drawstate->fill_type)
	  /* fill the path */
	  {
	    bool within_display = true;

	    /* are all juncture points contained within the ReGIS display? */
	    for (i = 0; i < _plotter->drawstate->path->num_segments; i++)
	      {
		double x, y;
		int i_x, i_y;
		
		x = XD(_plotter->drawstate->path->segments[i].p.x,
		       _plotter->drawstate->path->segments[i].p.y);
		y = YD(_plotter->drawstate->path->segments[i].p.x,
		       _plotter->drawstate->path->segments[i].p.y);
		i_x = IROUND(x);
		i_y = IROUND(y);
		if (i_x < REGIS_DEVICE_X_MIN
		    || i_x > REGIS_DEVICE_X_MAX
		    || i_y < REGIS_DEVICE_Y_MIN
		    || i_y > REGIS_DEVICE_Y_MAX)
		  {
		    within_display = false;
		    break;
		  }
	      }

	    if (within_display)
	      /* can fill path using ReGIS primitives */
	      {
		double x, y;
		plIntPoint first, oldpoint, newpoint;

		_pl_r_set_fill_color (S___(_plotter));
		x = XD(_plotter->drawstate->path->segments[0].p.x,
		       _plotter->drawstate->path->segments[0].p.y);
		y = YD(_plotter->drawstate->path->segments[0].p.x,
			     _plotter->drawstate->path->segments[0].p.y);
		first.x = IROUND(x);
		first.y = IROUND(y);
		_pl_r_regis_move (R___(_plotter) first.x, first.y); /* use P[..] */

		_write_string (_plotter->data, "F(");
		_write_string (_plotter->data, "V");

		oldpoint = first;
		for (i = 1; i < _plotter->drawstate->path->num_segments; i++)
		  {
		    x = XD(_plotter->drawstate->path->segments[i].p.x,
			   _plotter->drawstate->path->segments[i].p.y);
		    y = YD(_plotter->drawstate->path->segments[i].p.x,
			   _plotter->drawstate->path->segments[i].p.y);
		    newpoint.x = IROUND(x);
		    newpoint.y = IROUND(y);
		    /* emit vector; omit it if it has zero-length in the
		       integer device frame (unless it's the 1st vector) */
		    _emit_regis_vector (oldpoint, newpoint, 
					i > 1 ? true : false, tmpbuf);
		    _write_string (_plotter->data, tmpbuf);
		    oldpoint = newpoint;
		  }

		/* if path isn't closed, add a vector to close it (ReGIS
		   behaves unreliably if this isn't done) */
		_emit_regis_vector (newpoint, first, true, tmpbuf);
		_write_string (_plotter->data, tmpbuf);

		/* terminate F(V..) command */
		_write_string (_plotter->data, ")\n");
		_plotter->regis_position_is_unknown = true; /* to us */
	      }
	    else
	      /* path extends beyond ReGIS display, so must clip before
		 filling */
	      {
		/* NOT IMPLEMENTED YET */
	      }
	  }

	if (_plotter->drawstate->pen_type)
	  /* edge the path */
	  {
	    bool attributes_set = false;
	    bool path_in_progress = false;

	    for (i = 1; i < _plotter->drawstate->path->num_segments; i++)
	      {
		plPoint start, end; /* endpoints of seg. (in device coors) */
		plIntPoint istart, iend; /* same, quantized to integer */
		int clipval;
		
		/* nominal starting point and ending point for new line
		   segment, in floating point device coordinates */
		start.x = XD(_plotter->drawstate->path->segments[i-1].p.x,
			     _plotter->drawstate->path->segments[i-1].p.y);
		start.y = YD(_plotter->drawstate->path->segments[i-1].p.x,
			     _plotter->drawstate->path->segments[i-1].p.y);
		end.x = XD(_plotter->drawstate->path->segments[i].p.x,
			   _plotter->drawstate->path->segments[i].p.y);
		end.y = YD(_plotter->drawstate->path->segments[i].p.x,
			   _plotter->drawstate->path->segments[i].p.y);

		/* clip line segment to rectangular clipping region in
		   device frame */
		clipval = _clip_line (&start.x, &start.y, &end.x, &end.y,
				      REGIS_DEVICE_X_MIN_CLIP, 
				      REGIS_DEVICE_X_MAX_CLIP,
				      REGIS_DEVICE_Y_MIN_CLIP, 
				      REGIS_DEVICE_Y_MAX_CLIP);
		if (!(clipval & ACCEPTED)) /* line segment is OOB */
		  {
		    if (path_in_progress) /* terminate it */
		      _write_string (_plotter->data, "\n");
		    path_in_progress = false;
		    continue;	/* drop this line segment */
		  }

		if (clipval & CLIPPED_FIRST) /* must move */
		  {
		    if (path_in_progress) /* terminate it */
		      _write_string (_plotter->data, "\n");
		    path_in_progress = false;
		  }

		/* convert clipped starting point, ending point to integer
		   ReGIS coors */
		istart.x = IROUND(start.x);
		istart.y = IROUND(start.y);
		iend.x = IROUND(end.x);
		iend.y = IROUND(end.y);
		
		if (path_in_progress 
		    && istart.x == iend.x && istart.y == iend.y)
		  /* redundant, so drop this line segment */
		  continue;

		if (attributes_set == false)
		  /* will be drawing something, so sync ReGIS line type and
		     set the ReGIS foreground color to be our pen color;
		     this code gets executed the first time we get here */
		  {
		    _pl_r_set_attributes (S___(_plotter));
		    _pl_r_set_pen_color (S___(_plotter));
		    attributes_set = true;
		  }

		if (path_in_progress == false)
		  {
		    /* if necessary, move graphics cursor to first point of
                       line segment, using P command */
		    _pl_r_regis_move (R___(_plotter) istart.x, istart.y);
		    
		    /* emit op code for V command, to begin polyline */
		    _write_string (_plotter->data, "V");

		    if (iend.x != istart.x || iend.y != istart.y)
		      /* emit V[] command: ensure initial pixel is painted */
			_write_string (_plotter->data, "[]");

		    path_in_progress = true;
		  }
		
		_emit_regis_vector (istart, iend, true, tmpbuf);
		_write_string (_plotter->data, tmpbuf);
		
		/* update our notion of ReGIS's notion of position */
		  _plotter->regis_pos.x = iend.x;
		  _plotter->regis_pos.y = iend.y;
	      }

	    /* entire path has been drawn */
	    if (path_in_progress == true)
	      _write_string (_plotter->data, "\n");
	  }
      }
      break;
      
    case (int)PATH_CIRCLE:
      {
	double xd, yd, radius_d;
	int i_x, i_y, i_radius;		/* center and radius, quantized */
	plPoint pc;
	double radius;
	
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
	
	if (i_x - i_radius < REGIS_DEVICE_X_MIN
	    || i_x + i_radius > REGIS_DEVICE_X_MAX
	    || i_y - i_radius < REGIS_DEVICE_Y_MIN
	    || i_y + i_radius > REGIS_DEVICE_Y_MAX)
	  /* circle extends beyond edge of display, so polygonalize and
	     recurse */
	  {
	    plPath *oldpath = _plotter->drawstate->path;
	    
	    _plotter->drawstate->path = _flatten_path (oldpath);
	    _plotter->paint_path (S___(_plotter)); /* recursive invocation */
	    _delete_plPath (_plotter->drawstate->path);
	    _plotter->drawstate->path = oldpath;	    
	  }
	else
	  /* circle contained within display, can draw using ReGIS circle
	     primitive */
	  {
	    if (_plotter->drawstate->fill_type)
	      /* fill the circle */
	      {
		_pl_r_set_fill_color (S___(_plotter));

		_pl_r_regis_move (R___(_plotter) i_x, i_y); /* use P command */
		if (i_radius > 0)
		  {
		    sprintf (tmpbuf, "F(C[+%d])\n", i_radius);
		    _plotter->regis_position_is_unknown = true; /* to us */
		  }
		else
		  sprintf (tmpbuf, "V[]\n");
		_write_string (_plotter->data, tmpbuf);
	      }
	    
	    if (_plotter->drawstate->pen_type)
	      /* edge the circle */
	      {
		_pl_r_set_attributes (S___(_plotter));
		_pl_r_set_pen_color (S___(_plotter));

		_pl_r_regis_move (R___(_plotter) i_x, i_y); /* use P command */
		if (i_radius > 0)
		  {
		    sprintf (tmpbuf, "C[+%d]\n", i_radius);
		    _plotter->regis_position_is_unknown = true; /* to us */
		  }
		else
		  sprintf (tmpbuf, "V[]\n");
		_write_string (_plotter->data, tmpbuf);
	      }
	  }
      }
      break;

    default:			/* shouldn't happen */
      break;
    }
}

/* A low-level method for moving the graphics cursor of a ReGIS device to
   agree with the Plotter's notion of what the graphics cursor should be.  */

void
_pl_r_regis_move (R___(Plotter *_plotter) int xx, int yy)
{
  char tmpbuf[32];
  plIntPoint newpoint;
      
  /* sanity check */
  if (xx < REGIS_DEVICE_X_MIN || xx > REGIS_DEVICE_X_MAX
      || yy < REGIS_DEVICE_Y_MIN || yy > REGIS_DEVICE_Y_MAX)
    return;

  newpoint.x = xx;
  newpoint.y = yy;

  if (_plotter->regis_position_is_unknown)
    {
      sprintf (tmpbuf, "P[%d,%d]\n", xx, yy);
      _write_string (_plotter->data, tmpbuf);
    }
  else if (xx != _plotter->regis_pos.x || yy != _plotter->regis_pos.y)
    {
      _write_string (_plotter->data, "P");
      _emit_regis_vector (_plotter->regis_pos, newpoint, false, tmpbuf);
      _write_string (_plotter->data, tmpbuf);
      _write_string (_plotter->data, "\n");
    }

  /* update our knowledge of cursor position */
  _plotter->regis_position_is_unknown = false;
  _plotter->regis_pos = newpoint;
}

static void
_emit_regis_vector (plIntPoint istart, plIntPoint iend, bool skip_null, char *tmpbuf)
{
  plIntVector v;
  bool xneg = false, yneg = false;
  char xrelbuf[32], yrelbuf[32], xbuf[32], ybuf[32];
  int xrellen, yrellen, xlen, ylen;
  char *x, *y;

  v.x = iend.x - istart.x;
  v.y = iend.y - istart.y;

  /* trivial case */
  if (v.x == 0 && v.y == 0)
    {
      if (skip_null == false)
	sprintf (tmpbuf, "[]");
      else
	*tmpbuf = '\0';		/* empty string */
      return;
    }

  /* compute length of endpoint in terms of characters, when printed in
     relative and absolute coordinates */

  if (v.x < 0)
    {
      xneg = true;
      v.x = -v.x;
    }
  if (v.y < 0)
    {
      yneg = true;
      v.y = -v.y;
    }

  sprintf (xrelbuf, "%s%d", (xneg ? "-" : "+"), v.x);
  xrellen = strlen (xrelbuf);
  sprintf (yrelbuf, "%s%d", (yneg ? "-" : "+"), v.y);
  yrellen = strlen (yrelbuf);

  sprintf (xbuf, "%d", iend.x);
  xlen = strlen (xbuf);
  sprintf (ybuf, "%d", iend.y);
  ylen = strlen (ybuf);
  
  /* use whichever (relative/absolute) is shorter; prefer relative */
  x = (xrellen <= xlen ? xrelbuf : xbuf);
  y = (yrellen <= ylen ? yrelbuf : ybuf);

  /* draw vector: emit point coordinates */
  if (v.x == 0)
    sprintf (tmpbuf, "[,%s]", y);
  else if (v.y == 0)
    sprintf (tmpbuf, "[%s]", x);
  else
    sprintf (tmpbuf, "[%s,%s]", x, y);
}

bool
_pl_r_paint_paths (S___(Plotter *_plotter))
{
  return false;
}

bool
_pl_r_path_is_flushable (S___(Plotter *_plotter))
{
  return true;
}

void
_pl_r_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
{
}
