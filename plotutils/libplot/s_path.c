/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, 2009, Free Software
   Foundation, Inc.

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

/* This version of paint_path() is for SVGPlotters.  It renders a libplot
   path in terms of SVG shapes:
   path/rect/circle/ellipse/line/polyline/polygon. */

#include "sys-defines.h"
#include "extern.h"

/* SVG join styles, i.e., stroke-linejoin attribute, indexed by internal
   number (miter/rd./bevel/triangular) */
static const char * const svg_join_style[PL_NUM_JOIN_TYPES] =
{ "miter", "round", "bevel", "round" };

/* SVG cap styles, i.e., stroke-linecap attribute, indexed by internal
   number (butt/rd./project/triangular) */
static const char * const svg_cap_style[PL_NUM_CAP_TYPES] =
{ "butt", "round", "square", "round" };

/* SVG fill rule styles, i.e., fill-rule attribute, indexed by internal
   number (evenodd/nonzero winding number) */
static const char * const svg_fill_style[PL_NUM_FILL_RULES] =
{ "evenodd", "nonzero" };

static const double identity_matrix[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

/* forward references */
static void write_svg_path_data (plOutbuf *page, const plPath *path);
static void write_svg_path_style (plOutbuf *page, const plDrawState *drawstate, bool need_cap, bool need_join);

void
_pl_s_paint_path (S___(Plotter *_plotter))
{
  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	bool closed, lines_only;
	int i;

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
	
	/* determine which sort of SVG primitive shape this should be:
	   line/polyline/polygon, or general path */

	lines_only = true;
	for (i = 1; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    plPathSegmentType element_type;
	    
	    element_type = _plotter->drawstate->path->segments[i].type;
	    if (element_type != S_LINE)
	      {
		lines_only = false;
		break;
	      }
	  }
	
	if (lines_only && _plotter->drawstate->path->num_segments == 2)
	  /* SVG line */
	  {
	    sprintf (_plotter->data->page->point, "<line ");
	    _update_buffer (_plotter->data->page);

	    _pl_s_set_matrix (R___(_plotter) identity_matrix); 

	    sprintf (_plotter->data->page->point,
		     "x1=\"%.5g\" y1=\"%.5g\" x2=\"%.5g\" y2=\"%.5g\" ",
		     _plotter->drawstate->path->segments[0].p.x,
		     _plotter->drawstate->path->segments[0].p.y,
		     _plotter->drawstate->path->segments[1].p.x,
		     _plotter->drawstate->path->segments[1].p.y);
	    _update_buffer (_plotter->data->page);

	    write_svg_path_style (_plotter->data->page, _plotter->drawstate, 
				   true, false);

	    sprintf (_plotter->data->page->point, "/>\n");
	    _update_buffer (_plotter->data->page);
	  }
	
	else if (lines_only && !closed)
	  /* SVG polyline */
	  {
	    sprintf (_plotter->data->page->point, "<polyline ");
	    _update_buffer (_plotter->data->page);

	    _pl_s_set_matrix (R___(_plotter) identity_matrix); 

	    sprintf (_plotter->data->page->point,
		     "points=\"");
	    _update_buffer (_plotter->data->page);
	    for (i = 0; i < _plotter->drawstate->path->num_segments; i++)
	      {
		plPoint p;

		p = _plotter->drawstate->path->segments[i].p;
		sprintf (_plotter->data->page->point,
			 "%.5g,%.5g ",
			 p.x, p.y);
		_update_buffer (_plotter->data->page);
	      }
	    sprintf (_plotter->data->page->point,
		     "\" ");
	    _update_buffer (_plotter->data->page);

	    write_svg_path_style (_plotter->data->page, _plotter->drawstate,
				   true, true);

	    sprintf (_plotter->data->page->point,
		     "/>\n");
	    _update_buffer (_plotter->data->page);
	  }
	
	else if (lines_only && closed)
	  /* SVG polygon */
	  {
	    sprintf (_plotter->data->page->point, "<polygon ");
	    _update_buffer (_plotter->data->page);

	    _pl_s_set_matrix (R___(_plotter) identity_matrix); 

	    sprintf (_plotter->data->page->point,
		     "points=\"");
	    _update_buffer (_plotter->data->page);
	    for (i = 0; i < _plotter->drawstate->path->num_segments - 1; i++)
	      {
		plPoint p;

		p = _plotter->drawstate->path->segments[i].p;
		sprintf (_plotter->data->page->point,
			 "%.5g,%.5g ",
			 p.x, p.y);
		_update_buffer (_plotter->data->page);
	      }
	    sprintf (_plotter->data->page->point,
		     "\" ");
	    _update_buffer (_plotter->data->page);

	    write_svg_path_style (_plotter->data->page, _plotter->drawstate,
				   false, true);

	    sprintf (_plotter->data->page->point,
		     "/>\n");
	    _update_buffer (_plotter->data->page);
	  }

	else
	  /* general SVG path */
	  {
	    sprintf (_plotter->data->page->point, "<path ");
	    _update_buffer (_plotter->data->page);

	    _pl_s_set_matrix (R___(_plotter) identity_matrix); 

	    sprintf (_plotter->data->page->point,
		     "d=\"");
	    _update_buffer (_plotter->data->page);
	    
	    /* write SVG path data string */
	    write_svg_path_data (_plotter->data->page, 
				  _plotter->drawstate->path);

	    sprintf (_plotter->data->page->point,
		     "\" ");
	    _update_buffer (_plotter->data->page);

	    write_svg_path_style (_plotter->data->page, _plotter->drawstate,
				   true, true);

	    sprintf (_plotter->data->page->point,
		     "/>\n");
	    _update_buffer (_plotter->data->page);
	  }
      }
      break;
      
    case (int)PATH_BOX:
      {
	plPoint p0, p1;
	double xmin, ymin, xmax, ymax;

	p0 = _plotter->drawstate->path->p0;
	p1 = _plotter->drawstate->path->p1;
	xmin = DMIN(p0.x, p1.x);
	ymin = DMIN(p0.y, p1.y);
	xmax = DMAX(p0.x, p1.x);
	ymax = DMAX(p0.y, p1.y);

	sprintf (_plotter->data->page->point, "<rect ");
	_update_buffer (_plotter->data->page);

	_pl_s_set_matrix (R___(_plotter) identity_matrix); 

	sprintf (_plotter->data->page->point,
		 "x=\"%.5g\" y=\"%.5g\" width=\"%.5g\" height=\"%.5g\" ",
		 xmin, ymin, xmax - xmin, ymax - ymin);
	_update_buffer (_plotter->data->page);

	write_svg_path_style (_plotter->data->page, _plotter->drawstate, 
			       false, true);
	sprintf (_plotter->data->page->point,
		 "/>\n");
	_update_buffer (_plotter->data->page);
      }
      break;

    case (int)PATH_CIRCLE:
      {
	plPoint pc;
	double radius = _plotter->drawstate->path->radius;

	sprintf (_plotter->data->page->point, "<circle ");
	_update_buffer (_plotter->data->page);

	_pl_s_set_matrix (R___(_plotter) identity_matrix); 

	pc = _plotter->drawstate->path->pc;
	sprintf (_plotter->data->page->point,
		 "cx=\"%.5g\" cy=\"%.5g\" r=\"%.5g\" ",
		 pc.x, pc.y, radius);
	_update_buffer (_plotter->data->page);

	write_svg_path_style (_plotter->data->page, _plotter->drawstate, 
			       false, false);

	sprintf (_plotter->data->page->point,
		 "/>\n");
	_update_buffer (_plotter->data->page);
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	plPoint pc;
	double rx = _plotter->drawstate->path->rx;
	double ry = _plotter->drawstate->path->ry;
	double angle = _plotter->drawstate->path->angle;	
	double local_matrix[6];

	sprintf (_plotter->data->page->point, "<ellipse ");
	_update_buffer (_plotter->data->page);

	pc = _plotter->drawstate->path->pc;
	local_matrix[0] = cos (M_PI * angle / 180.0);
	local_matrix[1] = sin (M_PI * angle / 180.0);
	local_matrix[2] = -sin (M_PI * angle / 180.0);
	local_matrix[3] = cos (M_PI * angle / 180.0);
	local_matrix[4] = pc.x;
	local_matrix[5] = pc.y;
	_pl_s_set_matrix (R___(_plotter) local_matrix);

	sprintf (_plotter->data->page->point, "rx=\"%.5g\" ry=\"%.5g\" ",
		 rx, ry);
	_update_buffer (_plotter->data->page);

	write_svg_path_style (_plotter->data->page, _plotter->drawstate, 
			       false, false);

	sprintf (_plotter->data->page->point, "/>\n");
	_update_buffer (_plotter->data->page);
      }
      break;

    default:			/* shouldn't happen */
      break;
    }
}

bool
_pl_s_paint_paths (S___(Plotter *_plotter))
{
  int i;

  sprintf (_plotter->data->page->point,
	   "<path ");
  _update_buffer (_plotter->data->page);
  
  _pl_s_set_matrix (R___(_plotter) identity_matrix); 

  sprintf (_plotter->data->page->point,
	   "d=\"");
  _update_buffer (_plotter->data->page);
  
  for (i = 0; i < _plotter->drawstate->num_paths; i++)
    {
      plPath *path = _plotter->drawstate->paths[i];

      switch ((int)path->type)
	{
	case (int)PATH_SEGMENT_LIST:
	  /* write SVG path data string */
	  write_svg_path_data (_plotter->data->page, path);
	  break;
	  
	case (int)PATH_CIRCLE:
	  /* draw as four quarter-circles */
	  {
	    plPoint pc;
	    double radius;
	    
	    pc = path->pc;
	    radius = path->radius;
	    if (path->clockwise == false)
	      /* counter-clockwise */
	      sprintf (_plotter->data->page->point, "\
M%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g Z ",
		       pc.x + radius, pc.y, 
		       radius, radius, 0.0, 0, 1, pc.x, pc.y + radius,
		       radius, radius, 0.0, 0, 1, pc.x - radius, pc.y,
		       radius, radius, 0.0, 0, 1, pc.x, pc.y - radius,
		       radius, radius, 0.0, 0, 1, pc.x + radius, pc.y);
	    else
	      /* clockwise */
	      sprintf (_plotter->data->page->point, "\
M%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g Z ",
		       pc.x + radius, pc.y, 
		       radius, radius, 0.0, 0, 0, pc.x, pc.y - radius,
		       radius, radius, 0.0, 0, 0, pc.x - radius, pc.y,
		       radius, radius, 0.0, 0, 0, pc.x, pc.y + radius,
		       radius, radius, 0.0, 0, 0, pc.x + radius, pc.y);
	    _update_buffer (_plotter->data->page);
	  }
	  break;

	case (int)PATH_ELLIPSE:
	  /* draw as four quarter-ellipses */
	  {
	    plPoint pc;
	    double rx, ry, angle;
	    plVector v1, v2;
	    
	    pc = path->pc;
	    rx = path->rx;
	    ry = path->ry;
	    angle = path->angle;
	    v1.x = rx * cos (M_PI * angle / 180.0);
	    v1.y = rx * sin (M_PI * angle / 180.0);
	    v2.x = -ry * sin (M_PI * angle / 180.0);
	    v2.y = ry * cos (M_PI * angle / 180.0);

	    if (path->clockwise == false)
	      /* counter-clockwise */
	      sprintf (_plotter->data->page->point, "\
M%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g Z ",
		       pc.x + v1.x, pc.y + v1.y, 
		       rx, ry, 0.0, 0, 1, pc.x + v2.x, pc.y + v2.y,
		       rx, ry, 0.0, 0, 1, pc.x - v1.x, pc.y - v1.y,
		       rx, ry, 0.0, 0, 1, pc.x - v2.x, pc.y - v2.y,
		       rx, ry, 0.0, 0, 1, pc.x + v1.x, pc.y + v1.y);
	    else
	      /* clockwise */
	      sprintf (_plotter->data->page->point, "\
M%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g \
A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g Z ",
		       pc.x + v1.x, pc.y + v1.y, 
		       rx, ry, 0.0, 0, 0, pc.x - v2.x, pc.y - v2.y,
		       rx, ry, 0.0, 0, 0, pc.x - v1.x, pc.y - v1.y,
		       rx, ry, 0.0, 0, 0, pc.x + v2.x, pc.y + v2.y,
		       rx, ry, 0.0, 0, 0, pc.x + v1.x, pc.y + v1.y);
	    _update_buffer (_plotter->data->page);
	  }
	  break;

	case (int)PATH_BOX:
	  {
	    plPoint p0, p1;
	    bool x_move_is_first;
	    
	    p0 = path->p0;
	    p1 = path->p1;

	    /* if counterclockwise, would first pen motion be in x
               direction? */
	    x_move_is_first = ((p1.x >= p0.x && p1.y >= p0.y)
			       || (p1.x < p0.x && p1.y < p0.y) ? true : false);

	    if (path->clockwise)
	      /* take complement */
	      x_move_is_first = (x_move_is_first == true ? false : true);

	    if (x_move_is_first)
	      sprintf (_plotter->data->page->point, 
		       "M%.5g,%.5g H%.5g V%.5g H%.5g Z ",
		       p0.x, p0.y, p1.x, p1.y, p0.x);
	    else
	      sprintf (_plotter->data->page->point, 
		       "M%.5g,%.5g V%.5g H%.5g V%.5g Z ",
		       p0.x, p0.y, p1.y, p1.x, p0.y);
	    _update_buffer (_plotter->data->page);
	  }
	  break;

	default:		/* shouldn't happen */
	  break;
	}
    }
  sprintf (_plotter->data->page->point,
	   "\" ");
  _update_buffer (_plotter->data->page);

  write_svg_path_style (_plotter->data->page, _plotter->drawstate, 
			 true, true);

  sprintf (_plotter->data->page->point,
	   "/>\n");
  _update_buffer (_plotter->data->page);

  return true;
}

/* Write an SVG path data string that specifies a single simple path.  This
   may be called only on a libplot segment-list path, not on a libplot path
   that consists of a single closed path primitive (box/circle/ellipse). */

static void
write_svg_path_data (plOutbuf *page, const plPath *path)
{
  bool closed;
  plPoint p, oldpoint;
  int i;
  
  /* sanity check */
  if (path->type != PATH_SEGMENT_LIST)
    return;

  if ((path->num_segments >= 3)	/* check for closure */
      && (path->segments[path->num_segments - 1].p.x == path->segments[0].p.x)
      && (path->segments[path->num_segments - 1].p.y == path->segments[0].p.y))
    closed = true;
  else
    closed = false;		/* 2-point ones should be open */
	
  p = path->segments[0].p;	/* initial seg should be a moveto */
  sprintf (page->point, "M%.5g,%.5g ",
	   p.x, p.y);
  _update_buffer (page);
  
  oldpoint = p;
  for (i = 1; i < path->num_segments; i++)
    {
      plPathSegmentType type;
      plPoint pc, pd;
      
      type = path->segments[i].type;
      p = path->segments[i].p;
      pc = path->segments[i].pc;
      pd = path->segments[i].pd;
      
      if (closed
	  && i == path->num_segments - 1
	  && type == S_LINE)
	continue;	/* i.e. don't end with line-as-closepath */
      
      switch ((int)type)
	{
	case (int)S_LINE:
	  if (p.y == oldpoint.y)
	    sprintf (page->point, "H%.5g ",
		     p.x);
	  else if (p.x == oldpoint.x)
	    sprintf (page->point, "V%.5g ",
		     p.y);
	  else
	    sprintf (page->point, "L%.5g,%.5g ",
		     p.x, p.y);
	  break;
	  
	case (int)S_ARC:
	  {
	    double radius;
	    double angle;
	    
	    /* compute angle in radians, range -pi..pi */
	    angle = _angle_of_arc (oldpoint, p, pc);
	    
	    radius = sqrt ((p.x - pc.x)*(p.x - pc.x)
			   + (p.y - pc.y)*(p.y - pc.y));
	    sprintf (page->point, "A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g ",
		     radius, radius, 
		     0.0, /* rotation of x-axis of ellipse */
		     0, /* large-arc-flag, 0/1 = small/large */
		     angle >= 0.0 ? 1 : 0,/* sweep-flag, 0/1 = clock/c'clock */
		     p.x, p.y);
	  }
	  break;
	  
	case (int)S_ELLARC:
	  {
	    double cross, mixing_angle, rx, ry, theta;
	    plVector u, v, semi_axis_1, semi_axis_2;
	    bool clockwise;

	    /* conjugate radial vectors for the quarter-ellipse */
	    u.x = oldpoint.x - pc.x;
	    u.y = oldpoint.y - pc.y;
	    v.x = p.x - pc.x;
	    v.y = p.y - pc.y;
	    cross = u.x * v.y - v.x * u.y;
	    clockwise = cross < 0.0 ? true : false;

	    /* angle by which they should be mixed, to yield vectors along
	       the major and minor axes */
	    mixing_angle = 0.5 * _xatan2 (2.0 * (u.x * v.x + u.y * v.y),
					  u.x * u.x + u.y * u.y 
					  - v.x * v.x + v.y * v.y);
  
	    /* semi-axis vectors */
	    semi_axis_1.x = u.x * cos(mixing_angle) + v.x * sin(mixing_angle);
	    semi_axis_1.y = u.y * cos(mixing_angle) + v.y * sin(mixing_angle);
	    semi_axis_2.x = (u.x * cos(mixing_angle + M_PI_2) 
			     + v.x * sin(mixing_angle + M_PI_2));
	    semi_axis_2.y = (u.y * cos(mixing_angle + M_PI_2) 
			     + v.y * sin(mixing_angle + M_PI_2));
	    
	    /* semi-axis lengths */
	    rx = sqrt (semi_axis_1.x * semi_axis_1.x
		       + semi_axis_1.y * semi_axis_1.y);
	    ry = sqrt (semi_axis_2.x * semi_axis_2.x
		       + semi_axis_2.y * semi_axis_2.y);
	    
	    /* angle of inclination of first semi-axis */
	    theta = _xatan2 (semi_axis_1.y, semi_axis_1.x);

  /* compensate for possible roundoff error: treat a very small inclination
     angle of the 1st semi-axis, relative to the x-axis, as zero */
#define VERY_SMALL_ANGLE 1e-10
  
	    if (theta < VERY_SMALL_ANGLE && theta > -(VERY_SMALL_ANGLE))
	      theta = 0.0;

	    sprintf (page->point, "A%.5g,%.5g,%.5g,%d,%d,%.5g,%.5g ",
		     rx, ry, 
		     theta * 180.0 / M_PI, /* rotation of x-axis of ellipse */
		     0, /* large-arc-flag, 0/1 = small/large */
		     clockwise ? 0 : 1,	/* sweep-flag, 0/1 = clock/c'clock */
		     p.x, p.y);
	  }
	  break;
	  
	case (int)S_QUAD:
	  sprintf (page->point, "Q%.5g,%.5g,%.5g,%.5g ",
		   pc.x, pc.y, p.x, p.y);
	  break;
	  
	case (int)S_CUBIC:
	  sprintf (page->point, "C%.5g,%.5g,%.5g,%.5g,%.5g,%.5g ",
		   pc.x, pc.y, pd.x, pd.y, p.x, p.y);
	  break;
	  
	default:	/* shouldn't happen */
	  break;
	}
      _update_buffer (page);
      
      oldpoint = p;
    }
  
  if (closed)
    {
      sprintf (page->point, "Z ");
      _update_buffer (page);
    }
}

static void
write_svg_path_style (plOutbuf *page, const plDrawState *drawstate, bool need_cap, bool need_join)
{
  char color_buf[8];		/* enough room for "#ffffff", incl. NUL */

  if (drawstate->pen_type)
    {
      if (drawstate->fgcolor.red != 0
	  || drawstate->fgcolor.green != 0
	  || drawstate->fgcolor.blue != 0)
	/* non-black, i.e. non-default */
	{
	  sprintf (page->point, "stroke=\"%s\" ",
		   _libplot_color_to_svg_color (drawstate->fgcolor, 
						color_buf));
	  _update_buffer (page);
	}
      
      /* should use `px' here to specify user units, per the SVG Authoring
	 Guide, but ImageMagick objects to that */
      sprintf (page->point, "stroke-width=\"%.5g\" ",
	       drawstate->line_width);
      _update_buffer (page);
      
      if (need_cap)
	{
	  if (drawstate->cap_type != PL_CAP_BUTT) /* i.e. not default */
	    {
	      sprintf (page->point, "stroke-linecap=\"%s\" ",
		       svg_cap_style[drawstate->cap_type]);
	      _update_buffer (page);
	    }
	}
      
      if (need_join)
	{
	  if (drawstate->join_type != PL_JOIN_MITER) /* i.e. not default */
	    {
	      sprintf (page->point, "stroke-linejoin=\"%s\" ",
		       svg_join_style[drawstate->join_type]);
	      _update_buffer (page);
	    }
	  
	  if (drawstate->join_type == PL_JOIN_MITER
	      && drawstate->miter_limit != PL_DEFAULT_MITER_LIMIT)
	    {
	      sprintf (page->point, "stroke-miterlimit=\"%.5g\" ",
		       drawstate->miter_limit);
	      _update_buffer (page);
	    }
	}

      if ((drawstate->dash_array_in_effect /* user-specified dash array */
	   && drawstate->dash_array_len > 0)
	  ||
	  (drawstate->dash_array_in_effect == false
	   && drawstate->line_type != PL_L_SOLID)) /* non-solid builtin linetype*/
	/* need to specify stroke-array, maybe stroke-offset too */
	{
	  int i;
	  double *dashbuf, offset;
	  int num_dashes;

	  if (drawstate->dash_array_in_effect)
	    {
	      dashbuf = (double *)(drawstate->dash_array);
	      num_dashes = drawstate->dash_array_len;
	      offset = drawstate->dash_offset;
	    }
	  else
	    /* builtin line type, handcraft a SVG-style dash array for it */
	    {
	      const int *dash_array;
	      double min_sing_val, max_sing_val, min_width, scale;

	      /* compute maximum singular value of user->device coordinate
		 map, which we use as a divisive factor to convert size in
		 NCD frame back to size in the user frame */
	      _matrix_sing_vals (drawstate->transform.m_user_to_ndc,
				 &min_sing_val, &max_sing_val);
	      if (max_sing_val != 0.0)
		min_width = 
		 PL_DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE / max_sing_val;
	      else
		min_width = 0.0;
	      scale = DMAX(drawstate->line_width, min_width);

	      /* take normalized dash array (linemode-specific) from
                 internal table */
	      dash_array = 
		_pl_g_line_styles[drawstate->line_type].dash_array;
	      num_dashes =
		_pl_g_line_styles[drawstate->line_type].dash_array_len;
	      dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));

	      /* scale length of each dash by current line width, unless
		 it's too small (see above computation) */
	      for (i = 0; i < num_dashes; i++)
		dashbuf[i] = scale * dash_array[i];
	      offset = 0.0;	/* true for all builtin line types */
	    }

	  sprintf (page->point, "stroke-dasharray=\"");
	  _update_buffer (page);
	  for (i = 0; i < num_dashes; i++)
	    {
	      sprintf (page->point, "%.5g%s",
		       dashbuf[i],
		       i < num_dashes - 1 ? ", " : "\"");
	      _update_buffer (page);
	    }

	  if (offset != 0.0) /* not default */
	    {
	      /* should use `px' here to specify user units, per the SVG
		 Authoring Guide, but ImageMagick objects to that */
	      sprintf (page->point, "stroke-dashoffset=\"%.5g\" ",
		       offset);
	      _update_buffer (page);
	    }

	  if (drawstate->dash_array_in_effect == false)
	    /* have a handcrafted dash array to free */
	    free (dashbuf);
	}
      else
	/* solid, so don't specify stroke-dasharray or stroke-offset */
	{
	}
    }
  else
    {
      sprintf (page->point, "stroke=\"none\" ");
      _update_buffer (page);
    }

  if (drawstate->fill_type)
    {
      sprintf (page->point, "fill=\"%s\" ",
	       _libplot_color_to_svg_color (drawstate->fillcolor, color_buf));
      _update_buffer (page);

      if (drawstate->fill_rule_type != PL_FILL_ODD_WINDING) /* not default */
	{
	  sprintf (page->point, "fill-rule=\"%s\" ",
		   svg_fill_style[drawstate->fill_rule_type]);
	  _update_buffer (page);
	}
    }
}
