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

/* This version is for FigPlotters.  By construction, for FigPlotters our
   path buffer always contains either a segment list, or a rectangle or
   circle or ellipse object.  If it's a segment list, it consists of either
   (1) a sequence of line segments, or (2) a single circular arc segment.
   Those are the only sorts of path that xfig can handle.  (For the last to
   be included, the map from user to device coordinates must be uniform.)  */

#include "sys-defines.h"
#include "extern.h"

/* subtypes of xfig POLYLINE object type (xfig numbering) */
#define P_OPEN 1
#define P_BOX 2
#define P_CLOSED 3

/* subtypes of xfig ELLIPSE object type (xfig numbering) */
#define SUBTYPE_ELLIPSE 1	/* ellipse defined by radii */
#define SUBTYPE_CIRCLE  3	/* circle defined by radius */

/* Fig's line styles, indexed into by internal line number
   (PL_L_SOLID/PL_L_DOTTED/PL_L_DOTDASHED/PL_L_SHORTDASHED/PL_L_LONGDASHED/PL_L_DOTDOTDASHED) */
const int _pl_f_fig_line_style[PL_NUM_LINE_TYPES] =
{ FIG_L_SOLID, FIG_L_DOTTED, FIG_L_DASHDOTTED, FIG_L_DASHED, FIG_L_DASHED,
    FIG_L_DASHDOUBLEDOTTED, FIG_L_DASHTRIPLEDOTTED };

/* Fig join styles, indexed by internal number (miter/rd./bevel/triangular) */
const int _pl_f_fig_join_style[PL_NUM_JOIN_TYPES] =
{ FIG_JOIN_MITER, FIG_JOIN_ROUND, FIG_JOIN_BEVEL, FIG_JOIN_ROUND };

/* Fig cap styles, indexed by internal number (butt/rd./project/triangular) */
const int _pl_f_fig_cap_style[PL_NUM_CAP_TYPES] =
{ FIG_CAP_BUTT, FIG_CAP_ROUND, FIG_CAP_PROJECT, FIG_CAP_ROUND };

#define FUZZ 0.0000001

void
_pl_f_paint_path (S___(Plotter *_plotter))
{
  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	bool closed; 
	const char *format;
	int i, polyline_subtype, line_style;
	double nominal_spacing;
	double device_line_width;
	int quantized_device_line_width;

	/* sanity checks */
	if (_plotter->drawstate->path->num_segments == 0)/* nothing to do */
	  break;
	if (_plotter->drawstate->path->num_segments == 1) /*shouldn't happen */
	  break;
      
	if (_plotter->drawstate->path->num_segments == 2
	    && _plotter->drawstate->path->segments[1].type == S_ARC)
	  /* segment buffer contains a single arc, not a polyline */
	  {
	    double x0 = _plotter->drawstate->path->segments[0].p.x;
	    double y0 = _plotter->drawstate->path->segments[0].p.y;      
	    double x1 = _plotter->drawstate->path->segments[1].p.x;
	    double y1 = _plotter->drawstate->path->segments[1].p.y;      
	    double xc = _plotter->drawstate->path->segments[1].pc.x;
	    double yc = _plotter->drawstate->path->segments[1].pc.y;      
	    
	    _pl_f_draw_arc_internal (R___(_plotter) xc, yc, x0, y0, x1, y1);
	    break;
	  }
	
	if ((_plotter->drawstate->path->num_segments >= 3)/*check for closure*/
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.x == _plotter->drawstate->path->segments[0].p.x)
	    && (_plotter->drawstate->path->segments[_plotter->drawstate->path->num_segments - 1].p.y == _plotter->drawstate->path->segments[0].p.y))
	  closed = true;
	else
	  closed = false;		/* 2-point ones should be open */
	
	if (closed)
	  {
	    polyline_subtype = P_CLOSED;
	    format = "#POLYLINE [CLOSED]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
	  }
	else
	  {
	    polyline_subtype = P_OPEN;
	    format = "#POLYLINE [OPEN]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d";
	  }
	
	/* evaluate fig colors lazily, i.e. only when needed */
	_pl_f_set_pen_color (S___(_plotter));
	_pl_f_set_fill_color (S___(_plotter));
	
	/* In a .fig file, the width of a line is expressed as a
	   non-negative integer (a positive integer, if the line is to be
	   visible).  Originally, the width, if positive, was interpreted
	   as a multiple of a fundamental `Fig display unit', namely 1/80
	   inch.  However, the interpretation of the in-file line width was
	   subsequently changed, thus:

	   Width in .fig file 		Width as actually displayed by xfig
	   1				0.5
	   2				1
	   3				2
	   4				3
	   etc.

	   In consequence, our line width in terms of Fig display units
	   usually needs to be adjusted upward, before we round it to the
	   closest integer.  Thanks to Wolfgang Glunz and Bart De Schutter
	   for pointing this out.  (See the addition of 0.75 below, which
	   is what they recommend.)
	*/

	device_line_width =
	  FIG_UNITS_TO_FIG_DISPLAY_UNITS(_plotter->drawstate->device_line_width);
	if (device_line_width > 0.75)
	  device_line_width += 1.0;

	/* round xfig's notion of the line width to the closest integer;
	   but never round it down to 0 (which would yield an invisible
	   line) if the line width in user coordinates is positive;
	   instead, round it upward to 1  */
	quantized_device_line_width = IROUND(device_line_width);
	if (quantized_device_line_width == 0 && device_line_width > 0.0)
	  quantized_device_line_width = 1;

	/* compute line style (type of dotting/dashing, spacing of
           dots/dashes)*/
	_pl_f_compute_line_style (R___(_plotter) &line_style, &nominal_spacing);
	
	/* update xfig's `depth' attribute */
	if (_plotter->fig_drawing_depth > 0)
	  (_plotter->fig_drawing_depth)--;
	
	sprintf(_plotter->data->page->point,
		format,
		2,		/* polyline object */
		polyline_subtype, /* polyline subtype */
		line_style,	/* Fig line style */
	  			/* thickness, in Fig display units */
		(_plotter->drawstate->pen_type == 0 ? 0 :
		 quantized_device_line_width), 
		_plotter->drawstate->fig_fgcolor, /* pen color */
		_plotter->drawstate->fig_fillcolor, /* fill color */
		_plotter->fig_drawing_depth, /* depth */
		0,		/* pen style, ignored */
		_plotter->drawstate->fig_fill_level, /* area fill */
		nominal_spacing, /* style val, in Fig display units (float) */
		_pl_f_fig_join_style[_plotter->drawstate->join_type],/*join style */
		_pl_f_fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
		0,		/* radius(of arc boxes, ignored here) */
		0,		/* forward arrow */
		0,		/* backward arrow */
		_plotter->drawstate->path->num_segments /*num points in line */
		);
	_update_buffer (_plotter->data->page);
	
	for (i=0; i<_plotter->drawstate->path->num_segments; i++)
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
	    
	    if ((i%5) == 0)
	      sprintf (_plotter->data->page->point, "\n\t");/* make human-readable */
	    else
	      sprintf (_plotter->data->page->point, " ");
	    _update_buffer (_plotter->data->page);
	    
	    sprintf (_plotter->data->page->point, "%d %d", device_x, device_y);
	    _update_buffer (_plotter->data->page);
	  }
	sprintf (_plotter->data->page->point, "\n");
	_update_buffer (_plotter->data->page);
      }
      break;
	
    case (int)PATH_BOX:
      {
	plPoint p0, p1;

	p0 = _plotter->drawstate->path->p0;
	p1 = _plotter->drawstate->path->p1;

	_pl_f_draw_box_internal (R___(_plotter) p0, p1);
      }
      break;

    case (int)PATH_CIRCLE:
      {
	double x = _plotter->drawstate->path->pc.x;
	double y = _plotter->drawstate->path->pc.y;
	double r = _plotter->drawstate->path->radius;

	_pl_f_draw_ellipse_internal (R___(_plotter) 
				  x, y, r, r, 0.0, SUBTYPE_CIRCLE);
      }
      break;

    case (int)PATH_ELLIPSE:
      {
	double x = _plotter->drawstate->path->pc.x;
	double y = _plotter->drawstate->path->pc.y;
	double rx = _plotter->drawstate->path->rx;
	double ry = _plotter->drawstate->path->ry;
	double angle = _plotter->drawstate->path->angle;	

	_pl_f_draw_ellipse_internal (R___(_plotter) 
				  x, y, rx, ry, angle, SUBTYPE_ELLIPSE);
      }
      break;

    default:			/* shouldn't happen */
      break;
    }
}

/* Emit Fig code for an arc.  This is called if the segment buffer contains
   not a polyline, but a single circular arc.  If an arc was placed there,
   we can count on the map from the user frame to the device frame being
   isotropic (so the arc will be circular in the device frame too), and we
   can count on the arc not being of zero length.  See g_arc.c. */

#define DIST(p1, p2) sqrt( ((p1).x - (p2).x) * ((p1).x - (p2).x) \
			  + ((p1).y - (p2).y) * ((p1).y - (p2).y))

void
_pl_f_draw_arc_internal (R___(Plotter *_plotter) double xc, double yc, double x0, double y0, double x1, double y1)
{
  plPoint p0, p1, pc, pb;
  plVector v, v0, v1;
  double cross, radius, nominal_spacing;
  int line_style, orientation;
  double device_line_width;
  int quantized_device_line_width;

  pc.x = xc, pc.y = yc;
  p0.x = x0, p0.y = y0;
  p1.x = x1, p1.y = y1;

  /* vectors from pc to p0, and pc to p1 */
  v0.x = p0.x - pc.x;
  v0.y = p0.y - pc.y;
  v1.x = p1.x - pc.x;
  v1.y = p1.y - pc.y;

  /* cross product, zero means points are collinear */
  cross = v0.x * v1.y - v1.x * v0.y;

  /* Compute orientation.  Note libplot convention: if p0, p1, pc are
     collinear then arc goes counterclockwise from p0 to p1. */
  orientation = (cross >= 0.0 ? 1 : -1);

  radius = DIST(pc, p0);	/* radius is distance to p0 or p1 */

  v.x = p1.x - p0.x;		/* chord vector from p0 to p1 */
  v.y = p1.y - p0.y;
      
  _vscale(&v, radius);
  pb.x = pc.x + orientation * v.y; /* bisection point of arc */
  pb.y = pc.y - orientation * v.x;
      
  /* evaluate fig colors lazily, i.e. only when needed */
  _pl_f_set_pen_color (S___(_plotter));
  _pl_f_set_fill_color (S___(_plotter));
  
  /* xfig expresses the width of a line as an integer number of `Fig
     display units', so convert the width to those units */
  device_line_width =
    FIG_UNITS_TO_FIG_DISPLAY_UNITS(_plotter->drawstate->device_line_width);

  /* the interpretation of line width in a .fig file is now more
     complicated (see comments in _pl_f_paint_path() above), so this value
     must usually be incremented */
  if (device_line_width > 0.75)
    device_line_width += 1.0;

  /* round xfig's notion of the line width to the closest integer; but
     never round it down to 0 (which would yield an invisible line) if the
     line width in user coordinates is positive; instead, round it upward
     to 1  */
  quantized_device_line_width = IROUND(device_line_width);
  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;

  /* compute line style (type of dotting/dashing, spacing of dots/dashes) */
  _pl_f_compute_line_style (R___(_plotter) &line_style, &nominal_spacing);

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  /* compute orientation in NDC frame */
  orientation *= (_plotter->drawstate->transform.nonreflection ? 1 : -1);

  if (orientation == -1)
    /* interchange p0, p1 (since xfig insists that p0, pb, p1 must appear
       in counterclockwise order around the arc) */
    {
      plPoint ptmp;
      
      ptmp = p0;
      p0 = p1;
      p1 = ptmp;
    }

  sprintf(_plotter->data->page->point,
	  "#ARC\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %.3f %.3f %d %d %d %d %d %d\n",
	  5,			/* arc object */
	  1,			/* open-ended arc subtype */
	  line_style,		/* Fig line style */
	  			/* thickness, in Fig display units */
	  (_plotter->drawstate->pen_type == 0 ? 0 :
	   quantized_device_line_width), 
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  nominal_spacing,	/* style val, in Fig display units (float) */
	  _pl_f_fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  1,			/* counterclockwise */
	  0,			/* no forward arrow */
	  0,			/* no backward arrow */
	  XD(pc.x, pc.y),	/* center_x (float) */
	  YD(pc.x, pc.y),	/* center_y (float) */
	  IROUND(XD(p0.x, p0.y)), /* 1st point user entered (p0) */
	  IROUND(YD(p0.x, p0.y)), 
	  IROUND(XD(pb.x, pb.y)), /* 2nd point user entered (bisection point)*/
	  IROUND(YD(pb.x, pb.y)),
	  IROUND(XD(p1.x, p1.y)), /* last point user entered (p1) */
	  IROUND(YD(p1.x, p1.y)));
  _update_buffer (_plotter->data->page);
}

void
_pl_f_draw_box_internal (R___(Plotter *_plotter) plPoint p0, plPoint p1)
{
  int xd0, xd1, yd0, yd1;	/* in device coordinates */
  double nominal_spacing;
  int line_style;
  double device_line_width;
  int quantized_device_line_width;

  /* evaluate fig colors lazily, i.e. only when needed */
  _pl_f_set_pen_color (S___(_plotter));
  _pl_f_set_fill_color (S___(_plotter));
  
  /* xfig expresses the width of a line as an integer number of `Fig
     display units', so convert the width to those units */
  device_line_width =
    FIG_UNITS_TO_FIG_DISPLAY_UNITS(_plotter->drawstate->device_line_width);

  /* the interpretation of line width in a .fig file is now more
     complicated (see comments in _pl_f_paint_path() above), so this value
     must usually be incremented */
  if (device_line_width > 0.75)
    device_line_width += 1.0;

  /* round xfig's notion of the line width to the closest integer; but
     never round it down to 0 (which would yield an invisible line) if the
     line width in user coordinates is positive; instead, round it upward
     to 1  */
  quantized_device_line_width = IROUND(device_line_width);
  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;

  /* compute line style (type of dotting/dashing, spacing of dots/dashes)*/
  _pl_f_compute_line_style (R___(_plotter) &line_style, &nominal_spacing);
  
  /* update xfig's `depth' attribute */
  if (_plotter->fig_drawing_depth > 0)
    (_plotter->fig_drawing_depth)--;
  
  sprintf(_plotter->data->page->point,
	  "#POLYLINE [BOX]\n%d %d %d %d %d %d %d %d %d %.3f %d %d %d %d %d %d\n",
	  2,			/* polyline object */
	  P_BOX,		/* polyline subtype */
	  line_style,		/* Fig line style */
	  			/* thickness, in Fig display units */
	  (_plotter->drawstate->pen_type == 0 ? 0 :
	  quantized_device_line_width), 
	  _plotter->drawstate->fig_fgcolor,	/* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  nominal_spacing,	/* style val, in Fig display units (float) */
	  _pl_f_fig_join_style[_plotter->drawstate->join_type], /* join style */
	  _pl_f_fig_cap_style[_plotter->drawstate->cap_type], /* cap style */
	  0,			/* radius (of arc boxes, ignored here) */
	  0,			/* forward arrow */
	  0,			/* backward arrow */
	  5			/* number of points in line */
	  );
  _update_buffer (_plotter->data->page);
  
  p0 = _plotter->drawstate->path->p0;
  p1 = _plotter->drawstate->path->p1;
  xd0 = IROUND(XD(p0.x, p0.y));
  yd0 = IROUND(YD(p0.x, p0.y));  
  xd1 = IROUND(XD(p1.x, p1.y));
  yd1 = IROUND(YD(p1.x, p1.y));  
  
  sprintf (_plotter->data->page->point, "\t%d %d ", xd0, yd0);
  _update_buffer (_plotter->data->page);
  sprintf (_plotter->data->page->point, "%d %d ", xd0, yd1);
  _update_buffer (_plotter->data->page);
  sprintf (_plotter->data->page->point, "%d %d ", xd1, yd1);
  _update_buffer (_plotter->data->page);
  sprintf (_plotter->data->page->point, "%d %d ", xd1, yd0);
  _update_buffer (_plotter->data->page);
  sprintf (_plotter->data->page->point, "%d %d\n", xd0, yd0);
  _update_buffer (_plotter->data->page);
}

void
_pl_f_draw_ellipse_internal (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle, int subtype)
{
  const char *format;
  double theta, mixing_angle;
  double ux, uy, vx, vy;
  double semi_axis_1_x, semi_axis_1_y;
  double semi_axis_2_x, semi_axis_2_y;  
  double rx_device, ry_device, theta_device;
  double costheta, sintheta;
  double nominal_spacing;
  int line_style;
  double device_line_width;
  int quantized_device_line_width;

  /* inclination angle (radians), in user frame */
  theta = M_PI * angle / 180.0;
  costheta = cos (theta);
  sintheta = sin (theta);

  /* perform affine user->device coor transformation; (ux,uy) and (vx,vy)
     are forward images of the semiaxes, i.e. they are conjugate radial
     vectors in the device frame */

  ux = XDV(rx * costheta, rx * sintheta);
  uy = YDV(rx * costheta, rx * sintheta);

  vx = XDV(-ry * sintheta, ry * costheta);
  vy = YDV(-ry * sintheta, ry * costheta);

  /* angle by which the conjugate radial vectors should be mixed, in order
     to yield vectors along the major and minor axes in the device frame */
  mixing_angle = 0.5 * _xatan2 (2.0 * (ux * vx + uy * vy),
				ux * ux + uy * uy - vx * vx + vy * vy);
  
  /* semi-axis vectors in device coordinates */
  semi_axis_1_x = ux * cos(mixing_angle) + vx * sin(mixing_angle);
  semi_axis_1_y = uy * cos(mixing_angle) + vy * sin(mixing_angle);  
  semi_axis_2_x = ux * cos(mixing_angle + M_PI_2) 
    + vx * sin(mixing_angle + M_PI_2);
  semi_axis_2_y = uy * cos(mixing_angle + M_PI_2) 
    + vy * sin(mixing_angle + M_PI_2);  

  /* semi-axis lengths in device coordinates */
  rx_device = sqrt (semi_axis_1_x * semi_axis_1_x
		    + semi_axis_1_y * semi_axis_1_y);
  ry_device = sqrt (semi_axis_2_x * semi_axis_2_x
		    + semi_axis_2_y * semi_axis_2_y);

  /* angle of inclination of the first semi-axis, in device frame
     (note flipped-y convention) */
  theta_device = - _xatan2 (semi_axis_1_y, semi_axis_1_x);
  if (theta_device == 0.0)
    theta_device = 0.0;		/* remove sign bit if any */

  if (subtype == SUBTYPE_CIRCLE && 
      IROUND (rx_device) != IROUND (ry_device))
    subtype = SUBTYPE_ELLIPSE;

  /* evaluate fig colors lazily, i.e. only when needed */
  _pl_f_set_pen_color (S___(_plotter));
  _pl_f_set_fill_color (S___(_plotter));
  
  /* xfig expresses the width of a line as an integer number of `Fig
     display units', so convert the width to those units */
  device_line_width =
    FIG_UNITS_TO_FIG_DISPLAY_UNITS(_plotter->drawstate->device_line_width);

  /* the interpretation of line width in a .fig file is now more
     complicated (see comments in _pl_f_paint_path() above), so this value
     must usually be incremented */
  if (device_line_width > 0.75)
    device_line_width += 1.0;

  /* round xfig's notion of the line width to the closest integer; but
     never round it down to 0 (which would yield an invisible line) if the
     line width in user coordinates is positive; instead, round it upward
     to 1  */
  quantized_device_line_width = IROUND(device_line_width);
  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;

  /* compute line style (type of dotting/dashing, spacing of dots/dashes) */
  _pl_f_compute_line_style (R___(_plotter) &line_style, &nominal_spacing);

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  if (subtype == SUBTYPE_CIRCLE)
    format = "#ELLIPSE [CIRCLE]\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";
  else
    format = "#ELLIPSE\n%d %d %d %d %d %d %d %d %d %.3f %d %.3f %d %d %d %d %d %d %d %d\n";

  sprintf(_plotter->data->page->point,
	  format,
	  1,			/* ellipse object */
	  subtype,		/* subtype, see above */
	  line_style,		/* Fig line style */
	  			/* thickness, in Fig display units */
	  (_plotter->drawstate->pen_type == 0 ? 0 :
	   quantized_device_line_width), 
	  _plotter->drawstate->fig_fgcolor,	/* pen color */
	  _plotter->drawstate->fig_fillcolor, /* fill color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _plotter->drawstate->fig_fill_level, /* area fill */
	  nominal_spacing,	/* style val, in Fig display units (float) */
	  1,			/* direction, always 1 */
	  theta_device,		/* inclination angle, in radians (float) */
	  IROUND(XD(x,y)),	/* center_x (not float, unlike arc) */
	  IROUND(YD(x,y)),	/* center_y (not float, unlike arc) */
	  IROUND(rx_device),	/* radius_x */
	  IROUND(ry_device),	/* radius_y */
	  IROUND(XD(x,y)),	/* start_x, 1st point entered */
	  IROUND(YD(x,y)),	/* start_y, 1st point entered */
	  IROUND(XD(x,y)	/* end_x, last point entered */
		 + semi_axis_1_x + semi_axis_2_x),
	  IROUND(YD(x,y)	/* end_y, last point entered */
		 + semi_axis_1_y + semi_axis_2_y) 
	  );			
  _update_buffer(_plotter->data->page);
}

/* compute appropriate Fig line style, and also appropriate value for Fig's
   notion of `dash length/dot gap' (in Fig display units) */
void
_pl_f_compute_line_style (R___(Plotter *_plotter) int *style, double *spacing)
{
  int fig_line_style;
  double fig_nominal_spacing;
    
  if (_plotter->drawstate->dash_array_in_effect
      && _plotter->drawstate->dash_array_len == 2
      && (_plotter->drawstate->dash_array[1]
	  == _plotter->drawstate->dash_array[0]))
    /* special case of user-specified dashing (equal on/off lengths);
       we map this into Fig's `dashed' line type */
    {
      double min_sing_val, max_sing_val;

      /* Minimum singular value is the nominal device-frame line width
	 divided by the actual user-frame line-width (see g_linewidth.c),
	 so it's the user->device frame conversion factor. */
      _matrix_sing_vals (_plotter->drawstate->transform.m,
			 &min_sing_val, &max_sing_val);

      /* desired cycle length in Fig display units */
      fig_nominal_spacing =
	FIG_UNITS_TO_FIG_DISPLAY_UNITS(min_sing_val * 2.0 * _plotter->drawstate->dash_array[0]);
      fig_line_style = FIG_L_DASHED;
    }
  else if (_plotter->drawstate->dash_array_in_effect
	   && _plotter->drawstate->dash_array_len == 2
	   && (_plotter->drawstate->dash_array[1]
	       > (3 - FUZZ) * _plotter->drawstate->dash_array[0])
	   && (_plotter->drawstate->dash_array[1]
	       < (3 + FUZZ) * _plotter->drawstate->dash_array[0]))
    /* special case of user-specified dashing (gap length = 3 * dash length);
       we map this into Fig's `dotted' line type, since it agrees with
       libplot's convention for dashing `dotted' lines (see g_dash2.c) */
    {
      double min_sing_val, max_sing_val;

      _matrix_sing_vals (_plotter->drawstate->transform.m,
			 &min_sing_val, &max_sing_val);

      /* desired cycle length in Fig display units */
      fig_nominal_spacing =
	FIG_UNITS_TO_FIG_DISPLAY_UNITS(min_sing_val * 4.0 * _plotter->drawstate->dash_array[0]);
      fig_line_style = FIG_L_DOTTED;
    }
  else
    /* canonical line type; retrieve dash array from database (in g_dash2.c) */
    {
      int i, num_dashes, cycle_length;
      const int *dash_array;
      double display_size_in_fig_units, min_dash_unit, dash_unit;

      num_dashes =
	_pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;
      dash_array = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
      cycle_length = 0;
      for (i = 0; i < num_dashes; i++)
	cycle_length += dash_array[i];
      /* multiply cycle length of dash array by device-frame line width in
	 Fig display units, with a floor on the latter (see comments at
	 head of file) */
      display_size_in_fig_units = DMIN(_plotter->data->xmax - _plotter->data->xmin, 
				       /* flipped y */
				       _plotter->data->ymin - _plotter->data->ymax);
      min_dash_unit = PL_MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE 
	* FIG_UNITS_TO_FIG_DISPLAY_UNITS(display_size_in_fig_units);
      dash_unit = DMAX(min_dash_unit, 
		       FIG_UNITS_TO_FIG_DISPLAY_UNITS(_plotter->drawstate->device_line_width));

      /* desired cycle length in Fig display units */
      fig_nominal_spacing = cycle_length * dash_unit;
      fig_line_style = _pl_f_fig_line_style[_plotter->drawstate->line_type];
    }
      
  /* compensate for Fig's (or fig2dev's) peculiarities; value stored in Fig
     output file isn't really the cycle length */
  switch (fig_line_style)
    {
    case FIG_L_SOLID:
    default:			/* shouldn't happen */
      break;
    case FIG_L_DOTTED:
      fig_nominal_spacing -= 1.0;
      break;
    case FIG_L_DASHDOTTED:
      fig_nominal_spacing -= 1.0;
      /* fall thru */
    case FIG_L_DASHED:
      fig_nominal_spacing *= 0.5;
      break;
    case FIG_L_DASHDOUBLEDOTTED:
      fig_nominal_spacing -= 2.0;
      fig_nominal_spacing /= (1.9 + 1/3.0); /* really */
      break;
    case FIG_L_DASHTRIPLEDOTTED:
      fig_nominal_spacing -= 3.0;
      fig_nominal_spacing /= 2.4;
      break;
    }
  if (fig_nominal_spacing <= 1.0)
    fig_nominal_spacing = 1.0;

  /* pass back what Fig will need */
  *style = fig_line_style;
  *spacing = fig_nominal_spacing;
}

bool
_pl_f_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
