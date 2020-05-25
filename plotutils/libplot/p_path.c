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

/* This version is for PSPlotters.  By construction, for PSPlotters our
   path storage buffer, if it contains a path of segment array type,
   contains a sequence of line segments only (no other path elements such
   as arcs or Beziers are allowed).  That's because our PS driver draws
   objects by calling PS functions defined in the idraw header, and the
   header doesn't include functions that draw arcs or Beziers. */

#include "sys-defines.h"
#include "extern.h"

/* 16-bit brush patterns for idraw (1 = on, 0 = off), indexed by our
   internal numbering of line types, i.e. by L_{SOLID,DOTTED,DOTDASHED,
   SHORTDASHED,LONGDASHED,DOTDOTDASHED,DOTDOTDOTDASHED} */ 
static const long idraw_brush_pattern[PL_NUM_LINE_TYPES] = 
{ 0xffff, 0x8888, 0xfc30, 0xf0f0, 0xffc0, 0xfccc, 0xfdb6 };

/* PS join styles, indexed by internal number (miter/rd./bevel/triangular) */
static const int ps_join_style[PL_NUM_JOIN_TYPES] =
{ PS_LINE_JOIN_MITER, PS_LINE_JOIN_ROUND, PS_LINE_JOIN_BEVEL, PS_LINE_JOIN_ROUND };

/* PS cap styles, indexed by internal number (butt/rd./project/triangular) */
static const int ps_cap_style[PL_NUM_CAP_TYPES] =
{ PS_LINE_CAP_BUTT, PS_LINE_CAP_ROUND, PS_LINE_CAP_PROJECT, PS_LINE_CAP_ROUND };

void
_pl_p_paint_path (S___(Plotter *_plotter))
{
  double granularity;

  if (_plotter->drawstate->pen_type == 0
      && _plotter->drawstate->fill_type == 0)
    /* nothing to draw */
    return;

  /* Compute `granularity': factor by which user-frame coordinates will be
     scaled up, so that when they're emitted as integers (which idraw
     requires), resolution loss won't be excessive.  CTM factors will be
     scaled down by this factor. */
  {
    /* compute norm of user->device affine transformation */
    double norm, min_sing_val, max_sing_val;
    
    /* This minimum singular value isn't really the norm.  But it's the
       nominal device-frame line width divided by the actual user-frame
       line-width (see g_linewidth.c), and that's what we need. */
    _matrix_sing_vals (_plotter->drawstate->transform.m,
		       &min_sing_val, &max_sing_val);
    norm = min_sing_val;
    
    granularity = norm / (PS_MIN_RESOLUTION);
  }

  if (granularity == 0.0)
    /* must have norm = 0, quit now to avoid division by zero */
    return;

  switch ((int)_plotter->drawstate->path->type)
    {
    case (int)PATH_SEGMENT_LIST:
      {
	bool closed, closed_int;
	int i, numpoints, index_start, index_increment;
	int polyline_len;
	plIntPoint *xarray;
  
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
	
	/* scale up each point coordinate by granularity factor and round
	   it to closest integer, removing runs of points with the same
	   scaled integer coordinates */
	xarray = (plIntPoint *)_pl_xmalloc (_plotter->drawstate->path->num_segments * sizeof(plIntPoint));
	polyline_len = 0;
	for (i = 0; i < _plotter->drawstate->path->num_segments; i++)
	  {
	    plPoint datapoint;
	    int x_int, y_int;
	    
	    datapoint = _plotter->drawstate->path->segments[i].p;
	    x_int = IROUND(granularity * datapoint.x);
	    y_int = IROUND(granularity * datapoint.y);
	    
	    if ((polyline_len == 0) 
		|| (x_int != xarray[polyline_len-1].x) 
		|| (y_int != xarray[polyline_len-1].y))
	      /* add point, in integer coordinates, to the array */
	      {
		xarray[polyline_len].x = x_int;
		xarray[polyline_len].y = y_int;
		polyline_len++;
	      }
	  }

	/* Handle awkward cases: due to rounding and elimination of runs,
	   may be only 1 or 2 distinct vertices left in the polyline. */
	if (polyline_len == 1)
	  /* add a second point */
	  {
	    xarray[1] = xarray[0];
	    polyline_len = 2;
	  }
	if (polyline_len == 2)
	  closed_int = false;	/* 2-point ones should be open */
	else
	  closed_int = closed;

	/* number of points to be output, given that we're quantizing
	   coordinates and removing runs */
	numpoints = polyline_len - (closed_int ? 1 : 0);

	/* emit prolog and idraw instructions: start of MLine or Poly */
	if (closed_int)
	  strcpy (_plotter->data->page->point, "Begin %I Poly\n");
	else
	  strcpy (_plotter->data->page->point, "Begin %I MLine\n");
	_update_buffer (_plotter->data->page);
	
	/* emit common attributes: CTM, fill rule, cap and join styles and
	   miter limit, dash array, foreground and background colors, and
	   idraw brush. */
	_pl_p_emit_common_attributes (S___(_plotter));
	
	/* emit transformation matrix (all 6 elements) */
	strcpy (_plotter->data->page->point, "%I t\n["); 
	_update_buffer (_plotter->data->page);
	for (i = 0; i < 6; i++)
	  {
	    if ((i==0) || (i==1) || (i==2) || (i==3))
	      sprintf (_plotter->data->page->point, "%.7g ", _plotter->drawstate->transform.m[i] / granularity);
	    else
	      sprintf (_plotter->data->page->point, "%.7g ", _plotter->drawstate->transform.m[i]);
	    _update_buffer (_plotter->data->page);
	  }
	strcpy (_plotter->data->page->point, "\
] concat\n");
	_update_buffer (_plotter->data->page);
	
	/* emit idraw instruction: number of points in line */
	sprintf (_plotter->data->page->point, "\
%%I %d\n", 
		 numpoints);
	_update_buffer (_plotter->data->page);
	
	/* if polyline is closed, loop through points _backward_, since the
	   `Poly' function in the idraw prologue draws closed polylines in
	   reverse, and we want the dasharray to be interpreted correctly */
	if (closed_int)
	  {
	    index_start = numpoints - 1;
	    index_increment = -1;
	  }
	else
	  {
	    index_start = 0;
	    index_increment = 1;
	  }
	for (i = index_start; 
	     i >= 0 && i <= numpoints - 1; 
	     i += index_increment)
	  {
	    /* output the data point */
	    sprintf (_plotter->data->page->point, "\
%d %d\n",
		     xarray[i].x, xarray[i].y);
	    _update_buffer (_plotter->data->page);
	  }
	
	if (closed_int)
	  sprintf (_plotter->data->page->point, "\
%d Poly\n\
End\n\n", numpoints);
	else
	  sprintf (_plotter->data->page->point, "\
%d MLine\n\
End\n\n", numpoints);
	_update_buffer (_plotter->data->page);

	/* free temporary storage for quantized points */
	free (xarray);

	/* Update bounding box, by iterating over segments in the original
	   segment array (no quantizing, please).  But for consistency,
	   iterate in much the same way as above. */

	/* number of points that we'd have emitted, had we not quantized
	   and removed runs */
	numpoints = 
	  _plotter->drawstate->path->num_segments - (closed ? 1 : 0);

	if (closed)
	  {
	    index_start = numpoints - 1;
	    index_increment = -1;
	  }
	else
	  {
	    index_start = 0;
	    index_increment = 1;
	  }
	for (i = index_start; 
	     i >= 0 && i <= numpoints - 1; 
	     i += index_increment)
	  {
	    if (!closed && ((i == 0) || (i == numpoints - 1)))
	      /* an end rather than a join */
	      {
		int j;
		
		j = (i == 0 ? 1 : numpoints - 2);
		_set_line_end_bbox (_plotter->data->page,
				    _plotter->drawstate->path->segments[i].p.x,
				    _plotter->drawstate->path->segments[i].p.y,
				    _plotter->drawstate->path->segments[j].p.x,
				    _plotter->drawstate->path->segments[j].p.y,
				    _plotter->drawstate->line_width,
				    _plotter->drawstate->cap_type,
				    _plotter->drawstate->transform.m);
	      }
	    else
	      /* a join rather than an end */
	      {
		int a, b, c;
		
		if (closed && i == 0) /* wrap */
		  {
		    a = numpoints - 1;
		    b = 0;
		    c = 1;
		  }
		else		/* normal join */
		  {
		    a = i - 1;
		    b = i;
		    c = i + 1;
		  }
		_set_line_join_bbox(_plotter->data->page,
				    _plotter->drawstate->path->segments[a].p.x,
				    _plotter->drawstate->path->segments[a].p.y,
				    _plotter->drawstate->path->segments[b].p.x,
				    _plotter->drawstate->path->segments[b].p.y,
				    _plotter->drawstate->path->segments[c].p.x,
				    _plotter->drawstate->path->segments[c].p.y,
				    _plotter->drawstate->line_width,
				    _plotter->drawstate->join_type,
				    _plotter->drawstate->miter_limit,
				    _plotter->drawstate->transform.m);
	      }
	  }
      }
      break;

    case (int)PATH_BOX:
      {
	int i;

	/* emit prolog and idraw instructions: start of Rect */
	strcpy (_plotter->data->page->point, "Begin %I Rect\n");
	_update_buffer (_plotter->data->page);
	
	/* emit common attributes: CTM, fill rule, cap and join styles and
	   miter limit, dash array, foreground and background colors, and
	   idraw brush. */
	_pl_p_emit_common_attributes (S___(_plotter));
	
	/* emit transformation matrix (all 6 elements) */
	strcpy (_plotter->data->page->point, "%I t\n["); 
	_update_buffer (_plotter->data->page);
	for (i = 0; i < 6; i++)
	  {
	    if ((i==0) || (i==1) || (i==2) || (i==3))
	      sprintf (_plotter->data->page->point, "%.7g ", _plotter->drawstate->transform.m[i] / granularity);
	    else
	      sprintf (_plotter->data->page->point, "%.7g ", _plotter->drawstate->transform.m[i]);
	    _update_buffer (_plotter->data->page);
	  }
	strcpy (_plotter->data->page->point, "\
] concat\n");
	_update_buffer (_plotter->data->page);
	
	/* output the two defining vertices (preceded by an empty idraw
           instruction), and wind things up */
	sprintf (_plotter->data->page->point, "\
%%I\n\
%d %d %d %d Rect\n\
End\n\n",
		 IROUND(granularity * _plotter->drawstate->path->p0.x),
		 IROUND(granularity * _plotter->drawstate->path->p0.y),
		 IROUND(granularity * _plotter->drawstate->path->p1.x),
		 IROUND(granularity * _plotter->drawstate->path->p1.y));
	_update_buffer (_plotter->data->page);
		 
	/* update bounding box */
	_set_line_join_bbox(_plotter->data->page,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->line_width,
			    _plotter->drawstate->join_type,
			    _plotter->drawstate->miter_limit,
			    _plotter->drawstate->transform.m);
	_set_line_join_bbox(_plotter->data->page,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->line_width,
			    _plotter->drawstate->join_type,
			    _plotter->drawstate->miter_limit,
			    _plotter->drawstate->transform.m);
	_set_line_join_bbox(_plotter->data->page,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->line_width,
			    _plotter->drawstate->join_type,
			    _plotter->drawstate->miter_limit,
			    _plotter->drawstate->transform.m);
	_set_line_join_bbox(_plotter->data->page,
			    _plotter->drawstate->path->p1.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p1.y,
			    _plotter->drawstate->path->p0.x,
			    _plotter->drawstate->path->p0.y,
			    _plotter->drawstate->line_width,
			    _plotter->drawstate->join_type,
			    _plotter->drawstate->miter_limit,
			    _plotter->drawstate->transform.m);
      }
      break;

    case (int)PATH_CIRCLE:
      {
	plPoint pc;
	double radius;

	pc = _plotter->drawstate->path->pc;
	radius = _plotter->drawstate->path->radius;
	
	/* final arg flags this for idraw as a circle, not an ellipse */
	_pl_p_fellipse_internal (R___(_plotter) pc.x, pc.y, radius, radius,
			      0.0, true);
      }
      break;
      
    case (int)PATH_ELLIPSE:
      {
	double x = _plotter->drawstate->path->pc.x;
	double y = _plotter->drawstate->path->pc.y;
	double rx = _plotter->drawstate->path->rx;
	double ry = _plotter->drawstate->path->ry;
	double angle = _plotter->drawstate->path->angle;	

	/* final arg flags this for idraw as an ellipse, not a circle */
	_pl_p_fellipse_internal (R___(_plotter) x, y, rx, ry, angle, false);
      }
      break;

    default:			/* shouldn't happen */
      break;
    }
}
      
/* ARGS: circlep = drawn as a circle in user frame? */
void
_pl_p_fellipse_internal (R___(Plotter *_plotter) double x, double y, double rx, double ry, double angle, bool circlep)
{  
  if (_plotter->drawstate->pen_type || _plotter->drawstate->fill_type)
    /* have something to draw */
    {
      double granularity;
      double costheta, sintheta;
      double offcenter_rotation_matrix[6];
      double ellipse_transformation_matrix[6];
      int i;

      /* emit prolog instruction and idraw directive: start of Elli or Circ */
      if (circlep)
	strcpy (_plotter->data->page->point, "Begin %I Circ\n");
      else
	strcpy (_plotter->data->page->point, "Begin %I Elli\n");
      _update_buffer(_plotter->data->page);
      
      /* emit common attributes: CTM, fill rule, cap and join styles and
	 miter limit, dash array, foreground and background colors, and
	 idraw brush. */
      granularity = _pl_p_emit_common_attributes (S___(_plotter));

      /* An affine tranformation must be applied to the ellipse produced by
	 the Elli routine in the idraw prologue, to turn it into the
	 ellipse we want.  The Elli routine produces an ellipse with
	 specified semi-axes, aligned parallel to the coordinate axes in
	 user space, and centered on the point (x,y).  I.e. it produces,
	 symbolically,

	 [unit circle centered on (0,0)] S T

	 where S is a diagonal matrix that scales the unit circle to give
	 the specified semi-axis lengths, and T translates (0,0) to (x,y).
	 This is not what we want, since the ellipse is not rotated (it has
	 zero inclination angle).  What we want is
 
	 [unit circle centered on (0,0)] S R T

	 where R is a rotation matrix.  This may be rewritten as
	 
	 [unit circle centered on (0,0)] S T  (T^{-1} R T)

	 where T^{-1} R T is a so-called offcenter rotation matrix, which
	 rotates about the point (x,y).  So the ellipse transformation
	 matrix we'll place in the PS code will be (T^{-1} R T) times the
	 matrix that transforms from user space to device space. */

      costheta = cos (M_PI * angle / 180.0);
      sintheta = sin (M_PI * angle / 180.0);
      
      offcenter_rotation_matrix[0] = costheta; /* 1st 4 els are those of R */
      offcenter_rotation_matrix[1] = sintheta;
      offcenter_rotation_matrix[2] = - sintheta;
      offcenter_rotation_matrix[3] = costheta;
      offcenter_rotation_matrix[4] = x * (1.0 - costheta) + y * sintheta;
      offcenter_rotation_matrix[5] = y * (1.0 - costheta) - x * sintheta;
  
      _matrix_product (offcenter_rotation_matrix,
		       _plotter->drawstate->transform.m,
		       ellipse_transformation_matrix);
  
      /* emit idraw directive: transformation matrix (all 6 elements) */
      sprintf (_plotter->data->page->point, "%%I t\n[");
      _update_buffer(_plotter->data->page);
      for (i = 0; i < 6; i++)
	{
	  if ((i==0) || (i==1) || (i==2) || (i==3))
	    sprintf (_plotter->data->page->point, "%.7g ", 
		     ellipse_transformation_matrix[i] / granularity);
	  else
	    sprintf (_plotter->data->page->point, "%.7g ", 
		     ellipse_transformation_matrix[i]);
	  _update_buffer(_plotter->data->page);
	}
      sprintf (_plotter->data->page->point, "] concat\n");
      _update_buffer(_plotter->data->page);
      
      /* emit idraw directive: draw Elli, and end Elli (or same for Circ) */
      if (circlep)
	sprintf (_plotter->data->page->point, "%%I\n%d %d %d Circ\nEnd\n\n", 
		 IROUND(granularity * x), IROUND(granularity * y), 
		 IROUND(granularity * rx));
      else
	sprintf (_plotter->data->page->point, "%%I\n%d %d %d %d Elli\nEnd\n\n", 
		 IROUND(granularity * x), IROUND(granularity * y), 
		 IROUND(granularity * rx), IROUND(granularity * ry));
      _update_buffer(_plotter->data->page);
      
      /* update bounding box */
      _set_ellipse_bbox (_plotter->data->page, x, y, rx, ry, costheta, sintheta, 
			 _plotter->drawstate->line_width,
			 _plotter->drawstate->transform.m);
    }
}

/* Emit the common attributes, for PS and idraw, of any path object, either
   polyline, ellipse, or box.  This includes the CTM, fill rule, cap and
   join styles and miter limit, dash array, foreground and background
   colors, and idraw brush.
   
   Return value is the `granularity': a factor by which user-frame
   coordinates, when emitted to the output file as integers, should be
   scaled up.  This is to avoid loss of precision when using integer
   coordinates.  The CTM emitted here will automatically compensate for the
   granularity factor.

   Note: some of the functions that call this one (see _pl_p_paint_path()
   above) need to compute the granularity themselves, since they can't need
   to quit if the granularity is zero, without calling this function . */

double
_pl_p_emit_common_attributes (S___(Plotter *_plotter))
{
  bool singular_map;
  int i;
  double invnorm = 0.0, granularity = 1.0;
  double linewidth_adjust = 1.0;
  double min_sing_val, max_sing_val, norm;

  /* compute norm of user->device affine transformation */

  /* This minimum singular value isn't really the norm.  But it's the
     nominal device-frame line width divided by the actual user-frame
     line-width (see g_linewidth.c), and that's what we need. */
  _matrix_sing_vals (_plotter->drawstate->transform.m,
		     &min_sing_val, &max_sing_val);
  norm = min_sing_val;

  /* granularity = scaleup factor for user coordinates, so that when
     they're emitted as integers, resolution loss won't be excessive.
     CTM entries will be scaled down by this factor. */
  granularity = norm / (PS_MIN_RESOLUTION);
  if (norm != 0.0)
    {
      /* invnorm is `norm' of device->user coordinate transformation */
      invnorm = 1.0 / norm;
      singular_map = false;
    }
  else
    singular_map = true;

  /* redefine `originalCTM' matrix, which is the CTM applied when the
     polyline is stroked (as opposed to drawn).  We define it to be the
     same as the one in effect when the polyline was drawn. */
  if (singular_map != true)
    {
      int integer_linewidth = _plotter->drawstate->quantized_device_line_width;
      double double_linewidth = _plotter->drawstate->device_line_width;

      /* adjustment to CTM needed, due to our specifying line widths as
         integers */
      if (integer_linewidth != 0)
	linewidth_adjust = double_linewidth / integer_linewidth;
      else
	linewidth_adjust = 1.0;

      strcpy (_plotter->data->page->point, "[");
      _update_buffer (_plotter->data->page);

      for (i = 0; i < 4; i++)
	{
	  sprintf (_plotter->data->page->point, "%.7g ", 
		   linewidth_adjust * invnorm * _plotter->drawstate->transform.m[i]);
	  _update_buffer (_plotter->data->page);
	}
      _update_buffer (_plotter->data->page);
      strcpy (_plotter->data->page->point, "\
0 0 ] trueoriginalCTM originalCTM\n\
concatmatrix pop\n");
      _update_buffer (_plotter->data->page);
    }
  
  /* specify cap style and join style, and miter limit if mitering */
  if (_plotter->drawstate->join_type == PL_JOIN_MITER)
    sprintf (_plotter->data->page->point, "\
%d setlinecap %d setlinejoin %.4g setmiterlimit\n",
	     ps_cap_style[_plotter->drawstate->cap_type], 
	     ps_join_style[_plotter->drawstate->join_type],
	     _plotter->drawstate->miter_limit);
  else
    sprintf (_plotter->data->page->point, "\
%d setlinecap %d setlinejoin\n",
	     ps_cap_style[_plotter->drawstate->cap_type], 
	     ps_join_style[_plotter->drawstate->join_type]);
  _update_buffer (_plotter->data->page);
  
  /* specify fill rule (i.e. whether to use even-odd filling) */
  if (_plotter->drawstate->fill_rule_type == PL_FILL_NONZERO_WINDING)
    sprintf (_plotter->data->page->point, "\
/eoFillRule false def\n");
  else
    sprintf (_plotter->data->page->point, "\
/eoFillRule true def\n");
  _update_buffer (_plotter->data->page);
  
  if (_plotter->drawstate->pen_type != 0)
    /* pen is present, so will brush an outline of the path */
    {
      int num_dashes;
      double scale;
      double *dashbuf, dash_cycle_length, offset;

      if (_plotter->drawstate->dash_array_in_effect)
	/* have user-specified dash array */
	{
	  /* idraw instruction: brush type (spec'd as bit vector, but for now
	     we just use a solid brush */
	  sprintf (_plotter->data->page->point, "\
%%I b %ld\n", 
		   (long int)0xffff);
	  _update_buffer (_plotter->data->page);
	  
	  num_dashes = _plotter->drawstate->dash_array_len;
	  if (num_dashes > 0)
	    dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));
	  else
	    dashbuf = NULL;	/* solid line */
	  /* take the adjustment to the CTM into account */
	  scale = norm / linewidth_adjust;
	  
	  dash_cycle_length = 0.0;
	  for (i = 0; i < num_dashes; i++)
	    {
	      double dashlen;
	      
	      dashlen = _plotter->drawstate->dash_array[i];
	      dash_cycle_length += dashlen;
	      dashbuf[i] = scale * dashlen;
	    }
	  
	  if (dash_cycle_length > 0.0)
	    /* choose an offset in range 0..true_cycle_length */
	    {
	      double true_cycle_length;
	      
	      offset = _plotter->drawstate->dash_offset;
	      true_cycle_length = 
		dash_cycle_length * (num_dashes % 2 == 1 ? 2 : 1);
	      while (offset < 0.0)
		offset += true_cycle_length;
	      offset = fmod (offset, true_cycle_length);
	      offset *= scale;
	    }
	  else
	    offset = 0.0;
	}
      else
	/* have one of the canonical line types */
	{
	  /* idraw brush type (spec'd as bit vector) */
	  sprintf (_plotter->data->page->point, "\
%%I b %ld\n", 
		   idraw_brush_pattern[_plotter->drawstate->line_type]);
	  _update_buffer (_plotter->data->page);
	  
	  if (_plotter->drawstate->line_type == PL_L_SOLID)
	    {
	      num_dashes = 0;
	      dashbuf = NULL;
	      offset = 0.0;
	    }
	  else
	    {
	      const int *dash_array;
	      double display_size_in_points, min_dash_unit;
	      
	      /* compute PS dash array for this line type */
	      dash_array = 
		_pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
	      num_dashes =
		_pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;
	      dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));
	      
	      /* scale the array of integers by line width (actually by
		 floored line width) */
	      display_size_in_points = 
		DMIN(_plotter->data->xmax - _plotter->data->xmin,
		     _plotter->data->ymax - _plotter->data->ymin);
	      min_dash_unit = (PL_MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE 
			       * display_size_in_points);
	      scale = DMAX(min_dash_unit,
			   _plotter->drawstate->device_line_width);
	      /* take the adjustment to the CTM into account */
	      scale /= linewidth_adjust;
	      
	      for (i = 0; i < num_dashes; i++)
		dashbuf[i] = scale * dash_array[i];
	      offset = 0.0;
	    }
	}

      /* PS instruction: SetB (i.e. setbrush), with args
	 LineWidth, LeftArrow, RightArrow, DashArray, DashOffset. */
      /* Note LineWidth must be an integer for idraw compatibility. */
      
      /* emit dash array */
      sprintf (_plotter->data->page->point, "%d 0 0 [ ", 
	       _plotter->drawstate->quantized_device_line_width);
      _update_buffer (_plotter->data->page);
      for (i = 0; i < num_dashes; i++)
	{
	  sprintf (_plotter->data->page->point, "%.3g ", dashbuf[i]);
	  _update_buffer (_plotter->data->page);
	}
      sprintf (_plotter->data->page->point, "] %.3g SetB\n", offset);
      _update_buffer (_plotter->data->page);
      free (dashbuf);
    }
  else
    /* pen_type = 0, we have no pen to draw with (though we may do filling) */
    {
      sprintf (_plotter->data->page->point, "\
%%I b n\n\
none SetB\n");
      _update_buffer (_plotter->data->page);
    }
  
  /* idraw instruction: set foreground color */
  _pl_p_set_pen_color (S___(_plotter)); /* invoked lazily, when needed */
  sprintf (_plotter->data->page->point, "\
%%I cfg %s\n\
%g %g %g SetCFg\n",
	   _pl_p_idraw_stdcolornames[_plotter->drawstate->ps_idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red, 
	   _plotter->drawstate->ps_fgcolor_green, 
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (_plotter->data->page);
  
  /* idraw instruction: set background color */
  _pl_p_set_fill_color (S___(_plotter)); /* invoked lazily, when needed */
  sprintf (_plotter->data->page->point, "\
%%I cbg %s\n\
%g %g %g SetCBg\n",
	   _pl_p_idraw_stdcolornames[_plotter->drawstate->ps_idraw_bgcolor],
	   _plotter->drawstate->ps_fillcolor_red, 
	   _plotter->drawstate->ps_fillcolor_green, 
	   _plotter->drawstate->ps_fillcolor_blue);
  _update_buffer (_plotter->data->page);
  
  /* includes idraw instruction: set fill pattern */
  if (_plotter->drawstate->fill_type == 0)	/* transparent */
    sprintf (_plotter->data->page->point, "\
%%I p\n\
none SetP\n");
  else			/* filled, i.e. shaded, in the sense of idraw */
    sprintf (_plotter->data->page->point, "\
%%I p\n\
%f SetP\n", 
	     _pl_p_idraw_stdshadings[_plotter->drawstate->ps_idraw_shading]);
  _update_buffer (_plotter->data->page);
  
  /* return factor we'll later use to scale up user-frame coordinates */
  return granularity;
}

bool
_pl_p_paint_paths (S___(Plotter *_plotter))
{
  return false;
}
