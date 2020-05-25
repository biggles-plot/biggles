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

/* This file contains the space method, which is a standard part of
   libplot.  It sets the mapping from user coordinates to display
   coordinates.  On the display device, the drawing region is a fixed
   rectangle (usually a square).  The arguments to the space method are the
   lower left and upper right vertices of a `window' (a drawing rectangle),
   in user coordinates.  This window, whose axes are aligned with the
   coordinate axes, will be mapped affinely onto the drawing region on the
   display device.

   Equivalently, the space method sets the transformation matrix attribute
   that will be used for graphical objects that are subsequently drawn.
   Any transformation matrix produced by invoking space() will necessarily
   preserve coordinate axes.

   This file also contains the space2 method, which is a GNU extension to
   libplot.  The arguments to the space2 method are the vertices of an
   `affine window' (a drawing parallelogram), in user coordinates.  (The
   specified vertices are the lower left, the lower right, and the upper
   left.)  This window will be mapped affinely onto the drawing region on
   the display device.  Transformation matrices produced by invoking
   space() do not necessarily preserve coordinate axes.

   space and space2 are simply wrappers around the fsetmatrix() method. */

/* This file also contains the fsetmatrix method, which is a GNU extension
   to libplot.  Much as in Postscript, it sets the transformation matrix
   from user coordinates to NDC (normalized device coordinates).  This, in
   turn, determines the map from user coordinates to device coordinates.
   The resulting transformation matrix will be used as an attribute of
   objects that are subsequently drawn on the graphics display. */

/* This file also contains the fconcat method, which is a GNU extension to
   libplot.  fconcat is simply a wrapper around fsetmatrix.  As in
   Postscript, it left-multiplies the transformation matrix from user
   coordinates to NDC coordinates by a specified matrix.  That is, it
   modifies the affine transformation from user coordinates to NDC and
   hence to device coordinates, by requiring that the transformation
   currently in effect be be preceded by a specified affine
   transformation. */

/* N.B. Invoking fsetmatrix causes the default line width and default font
   size, which are expressed in user units, to be recomputed.  That is
   because those two quantities are specified as a fraction of the size of
   the display: in device terms, rather than in terms of user units.  The
   idea is that no matter what the arguments of fsetmatrix are, switching
   later to the default line width or default font size, by passing an
   out-of-bounds argument to linewidth() or fontsize(), should yield a
   reasonable result. */

#include "sys-defines.h"
#include "extern.h"

/* potential roundoff error (absolute, for defining boundary of display) */
#define ROUNDING_FUZZ 0.0000001

/* potential roundoff error (relative, used for checking isotropy etc.) */
#define OTHER_FUZZ 0.0000001

/* The vertices of the parallelogram in user space have coordinates (going
   counterclockwise) (x0,y0), (x1,y1), (x1,y1)+(x2,y2)-(x0,y0), and
   (x2,y2). */

int
_API_fspace2 (R___(Plotter *_plotter) double x0, double y0, double x1, double y1, double x2, double y2)
{
  double s[6];
  double v0x, v0y, v1x, v1y, v2x, v2y;
  double cross;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fspace2: invalid operation");
      return -1;
    }

  /* Compute affine transformation from user frame to NDC [normalized
     device coordinates] frame.  The parallelogram in the user frame is
     mapped to the square [0,1]x[0,1] in the NDC frame.  */

  v0x = x0;
  v0y = y0;
  v1x = x1 - x0;
  v1y = y1 - y0;
  v2x = x2 - x0;
  v2y = y2 - y0;
  cross = v1x * v2y - v1y * v2x;

  if (cross == 0.0) 
    {
      _plotter->error (R___(_plotter) "the requested singular affine transformation cannot be performed");
      return -1;
    }

  /* linear transformation */  
  s[0] = v2y / cross;
  s[1] = -v1y / cross;
  s[2] = -v2x / cross;
  s[3] = v1x / cross;

  /* translation */
  s[4] = - (v0x * v2y - v0y * v2x) / cross;
  s[5] = (v0x * v1y - v0y * v1x) / cross;
  
  return _API_fsetmatrix (R___(_plotter) 
			  s[0], s[1], s[2], s[3], s[4], s[5]);
}

int
_API_fspace (R___(Plotter *_plotter) double x0, double y0, double x1, double y1)
{
  return _API_fspace2 (R___(_plotter) x0, y0, x1, y0, x0, y1);
}

int
_API_fsetmatrix (R___(Plotter *_plotter) double m0, double m1, double m2, double m3, double m4, double m5)
{
  int i;
  double s[6], t[6];
  double norm, min_sing_val, max_sing_val;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fsetmatrix: invalid operation");
      return -1;
    }

  /* linear transformation */
  s[0] = m0;
  s[1] = m1;
  s[2] = m2;
  s[3] = m3;

  /* translation */
  s[4] = m4;
  s[5] = m5;

  /* store new user_frame->NDC_frame map in drawing state */
  for (i = 0; i < 6; i++)
    _plotter->drawstate->transform.m_user_to_ndc[i] = s[i];

  /* compute the user_frame -> device_frame map, as product of this map
     with the following NDC_frame->device_frame map: store in drawing state */
  _matrix_product (s, _plotter->data->m_ndc_to_device, t);
  for (i = 0; i < 6; i++)
    _plotter->drawstate->transform.m[i] = t[i];

  /* for convenience, precompute boolean properties of the
     user_frame->device_frame map: store in drawing state */

  /* Does the user_frame->device_frame map preserve axis directions? */
  _plotter->drawstate->transform.axes_preserved = 
    (t[1] == 0.0 && t[2] == 0.0) ? true : false;

  /* Is the user_frame->device_frame map a uniform scaling (possibly
     involving a rotation or reflection)?  We need to know this because
     it's only uniform maps that map circles to circles, and circular arcs
     to circular arcs.  Also some Plotters, e.g. Fig Plotters, don't
     support non-uniformly transformed fonts. */

#define IS_ZERO(arg) (IS_ZERO1(arg) && IS_ZERO2(arg))
#define IS_ZERO1(arg) (FABS(arg) < OTHER_FUZZ * DMAX(t[0] * t[0], t[1] * t[1]))
#define IS_ZERO2(arg) (FABS(arg) < OTHER_FUZZ * DMAX(t[2] * t[2], t[3] * t[3]))
  /* if row vectors are of equal length and orthogonal... */
  if (IS_ZERO(t[0] * t[0] + t[1] * t[1] - t[2] * t[2] - t[3] * t[3])
      &&
      IS_ZERO(t[0] * t[2] + t[1] * t[3]))
    /* map's scaling is uniform */
    _plotter->drawstate->transform.uniform = true;
  else
    /* map's scaling not uniform */
    _plotter->drawstate->transform.uniform = false; 

  /* Does the user_frame->physical_frame map involve a reflection?  This is
     useful to know because some Plotters, e.g. Fig Plotters, don't support
     reflected fonts, even if they're uniformly transformed.

     This is a tricky question, because it isn't a question about the
     user_frame->device_frame map alone.  There's a sequence of maps:

     	user_frame -> NDC_frame -> device_frame -> physical_frame

     If the device_frame uses `flipped y' coordinates, then by definition,
     the default NDC_frame->device_frame map and the
     device_frame->physical_frame map both include a reflection, so they
     cancel each other out.

     (Though depending on the Plotter, non-default behavior could obtain.
     For example, the PAGESIZE parameter allows the specification of xsize
     and ysize, and if exactly one of these two is negative, the
     NDC_frame->device_frame map will include an extra reflection.)

     What we do is look at the `sign' or orientation-preservingness of the
     user_frame->device_frame map, and flip it if the
     device_frame->physical_frame map is flagged as `flipped y'. */
  {
    double det;
    
    det = t[0] * t[3] - t[1] * t[2];
    _plotter->drawstate->transform.nonreflection 
      = ((_plotter->data->flipped_y ? -1 : 1) * det >= 0) ? true : false;
  }
  
  /* DO SOME OTHER STUFF, ALL RELATED TO LINE WIDTHS AND FONT SIZES */

  /* For scaling purposes, compute matrix norm of linear transformation
     appearing in the affine map from the user frame to the NDC frame. */

  /* This minimum singular value isn't really the norm.  But it's close
     enough. */
  _matrix_sing_vals (s, &min_sing_val, &max_sing_val);
  norm = min_sing_val;

  /* Set new default line width in user frame.  This default value will be
     switched to, later, if the user calls linewidth() with a negative
     (i.e. out-of-bound) argument. */

  if (_plotter->data->display_coors_type 
      == (int)DISP_DEVICE_COORS_INTEGER_LIBXMI)
    /* using libxmi or a compatible rendering algorithm; so set default
       line width to zero (interpreted as specifying a Bresenham line) */
    _plotter->drawstate->default_line_width = 0.0;
  else
    /* not using libxmi or a compatible rendering algorithm; so set default
       line width to a nonzero fraction of the display size */
    {
      if (norm == 0.0)		/* avoid division by 0 */
	_plotter->drawstate->default_line_width = 0.0;
      else
	_plotter->drawstate->default_line_width 
	  = PL_DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE / norm;
    }

  if (_plotter->data->linewidth_invoked == false)
  /* help out lusers who rely on us to initialize the linewidth to a
     reasonable value, as if this were plot(3) rather than GNU libplot */
    {
      /* invoke API function flinewidth(), which computes a nominal
	 device-frame line width, using the transformation matrix;
	 specifying a negative linewidth switches to the default */
      _API_flinewidth (R___(_plotter) -1.0);

      /* pretend we haven't invoked flinewidth() yet, so that the luser can
	 invoke space() and/or fsetmatrix() additional times, each time
	 automatically resetting the linewidth */
      _plotter->data->linewidth_invoked = false;
    }
  else
    /* invoke API function merely to compute a new nominal device-frame
       line width, from the current user-frame line width */
    _API_flinewidth (R___(_plotter) _plotter->drawstate->line_width);

  /* Similarly, set new default font size in user frame.  This default
     value will be switched to, later, if the user calls fontsize() with
     out-of-bound arguments. */

  if (norm == 0.0)		/* avoid division by 0 */
    _plotter->drawstate->default_font_size = 0.0;
  else
    _plotter->drawstate->default_font_size
      = PL_DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE / norm;

  /* Help out users who rely on us to choose a reasonable font size, as if
     this were Unix plot(3) rather than GNU libplot.  We don't wish to
     retrieve an actual font here, so we don't invoke _API_fontsize().
     However, this size will be used by the Plotter-specific method
     _paint_text(), which will first do the retrieval. */
  if (_plotter->data->fontsize_invoked == false)
    _plotter->drawstate->font_size = _plotter->drawstate->default_font_size;
  
  return 0;
}

int
_API_fconcat (R___(Plotter *_plotter) double m0, double m1, double m2, double m3, double m4, double m5)
{
  double m[6], s[6];

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fconcat: invalid operation");
      return -1;
    }

  m[0] = m0;
  m[1] = m1;
  m[2] = m2;
  m[3] = m3;
  m[4] = m4;
  m[5] = m5;

  /* compute new user->NDC affine map */
  _matrix_product (m, _plotter->drawstate->transform.m_user_to_ndc, s);

  /* set it in drawing state */
  return _API_fsetmatrix (R___(_plotter) 
			  s[0], s[1], s[2], s[3], s[4], s[5]);
}

/* Compute the affine transformation from the NDC frame to the device
   frame.  This is an internal method, called by any Plotter at
   initialization time, or at latest during the first invocation of
   openpl().

   The square [0,1]x[0,1] in the NDC frame is mapped to the viewport in the
   device frame (a square or rectangular region).  So, the
   NDC_frame->device_frame map preserves coordinate axes.  (Though either
   the x or y axis may be flipped, the latter being more common, because
   some devices' native coordinate system has a flipped-y convention, which
   means that the final device_frame->physical_frame map flips in the y
   direction.)

   There is support for the ROTATION Plotter parameter, which allows the
   NDC frame to be rotated by 90, 180, or 270 degrees, before it is mapped
   to the device frame. */

bool
_compute_ndc_to_device_map (plPlotterData *data)
{
  double t[6];
  double map_1[6], map_2[6], map_1a[6], map_1b[6], map_1ab[6], map_1c[6];
  double device_x_left, device_x_right, device_y_bottom, device_y_top;
  const char *rotation_s;
  double rotation_angle;
  int i;

  /* begin by computing device coordinate ranges */
  switch (data->display_model_type)
    {
    case (int)DISP_MODEL_PHYSICAL:
      /* Plotter has a physical display, ranges in device coordinates of
	 the viewport are known (they're expressed in inches, and are
	 computed from the PAGESIZE parameter when the Plotter is created,
	 see ?_defplot.c).  E.g., AI, Fig, HPGL, PCL, and PS Plotters. */
      {
	device_x_left = data->xmin;
	device_x_right = data->xmax;
	device_y_bottom = data->ymin;
	device_y_top = data->ymax;
      }
      break;

    case (int)DISP_MODEL_VIRTUAL:
    default:
      /* Plotter has a display, but its size isn't specified in physical
         units such as inches.  E.g., CGM, SVG, GIF, PNM, Tektronix, X, and
         X Drawable Plotters.  CGM and SVG Plotters are hybrids of a sort:
         the PAGESIZE parameter is meaningful for them, as far as nominal
         viewport size goes, but we treat a CGM or SVG display as `virtual'
         because a CGM or SVG viewer or interpreter is free to ignore the
         requested viewport size.  */
      {
	switch ((int)data->display_coors_type)
	  {
	  case (int)DISP_DEVICE_COORS_REAL:
	  default:
	    /* Real-coordinate virtual display device.  E.g., generic and
	       Metafile Plotters; also SVG Plotters. */
	    device_x_left = data->xmin;
	    device_x_right = data->xmax;
	    device_y_bottom = data->ymin;
	    device_y_top = data->ymax;
	    break;
	  case (int)DISP_DEVICE_COORS_INTEGER_LIBXMI:
	  case (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI:
	    /* Integer-coordinate virtual display device, in the sense that
	       we emit integer coordinates only (sometimes by choice).
	       
	       Of the Plotters that have virtual displays (see above), GIF,
	       PNM, X, and X Drawable Plotters use libxmi-compatible scan
	       conversion; Tektronix Plotters and CGM Plotters do not.
	       
	       In both cases, compute device coordinate ranges from imin,
	       imax, jmin, jmax, which are already available (see
	       ?_defplot.c; e.g., for Plotters with adjustable-size
	       displays, they are taken from the BITMAPSIZE parameter).

	       The subtraction/addition of 0.5-ROUNDING_FUZZ, which widens
	       the rectangle by nearly 0.5 pixel on each side, is magic. */
	    {
	      /* test whether NCD_frame->device_frame map reflects in the x
                 and/or y direction */
	      double x_sign = (data->imin < data->imax ? 1.0 : -1.0);
	      double y_sign = (data->jmin < data->jmax ? 1.0 : -1.0);
	      
	      device_x_left = ((double)(data->imin) 
			      + x_sign * (- 0.5 + ROUNDING_FUZZ));
	      device_x_right = ((double)(data->imax) 
			      + x_sign * (0.5 - ROUNDING_FUZZ));
	      device_y_bottom = ((double)(data->jmin)
			      + y_sign * (- 0.5 + ROUNDING_FUZZ));
	      device_y_top = ((double)(data->jmax) 
			      + y_sign * (0.5 - ROUNDING_FUZZ));
	    }
	    break;
	  }
      }
      break;
    }

  /* Device coordinate ranges now known, so work out transformation from
     NDC frame to device frame; take ROTATION parameter into account.

     The (NDC_frame)->(device_frame) map is the composition of two maps:

     (1) a preliminary rotation about (0.5,0.5) in the NDC frame,
     (2) the default (NDC_frame)->(device frame) map,

     in that order.  And the first of these is the composition of three:

     (1a) translate by -(0.5,0.5)
     (1b) rotate by ROTATION degrees about (0,0)
     (1c) translate by +(0.5,0.5).
  */

  /* compute map #1 as product of maps 1a, 1b, 1c */

  rotation_s = (const char *)_get_plot_param (data, "ROTATION");
  if (rotation_s == NULL)
    rotation_s = (const char *)_get_default_plot_param ("ROTATION");

  if (strcmp (rotation_s, "no") == 0)
    rotation_angle = 0.0;	/* "no" means 0 degrees */
  else if (strcmp (rotation_s, "yes") == 0)
    rotation_angle = 90.0;	/* "yes" means 90 degrees */
  else if (sscanf (rotation_s, "%lf", &rotation_angle) <= 0)
    rotation_angle = 0.0;	/* default */

  rotation_angle *= (M_PI / 180.0); /* convert to radians */

  map_1a[0] = map_1a[3] = 1.0;
  map_1a[1] = map_1a[2] = 0.0;
  map_1a[4] = map_1a[5] = -0.5;
  
  map_1b[0] = cos (rotation_angle);
  map_1b[1] = sin (rotation_angle);
  map_1b[2] = - sin (rotation_angle);
  map_1b[3] = cos (rotation_angle);
  map_1b[4] = map_1b[5] = 0.0;
  
  map_1c[0] = map_1c[3] = 1.0;
  map_1c[1] = map_1c[2] = 0.0;
  map_1c[4] = map_1c[5] = 0.5;
  
  _matrix_product (map_1a, map_1b, map_1ab);
  _matrix_product (map_1ab, map_1c, map_1);
  
  /* compute map #2: the default (NDC frame)->(device frame) map */

  /* NDC point (0,0) [lower left corner] gets mapped into this */
  map_2[4] = device_x_left;
  map_2[5] = device_y_bottom;
  /* NDC vector (1,0) gets mapped into this */
  map_2[0] = device_x_right - device_x_left;
  map_2[1] = 0.0;
  /* NDC vector (0,1) gets mapped into this */
  map_2[2] = 0.0;
  map_2[3] = device_y_top - device_y_bottom;
  
  /* compute (NDC_frame)->(device frame) map as a product of maps 1,2 */
  _matrix_product (map_1, map_2, t);

  /* set affine transformation in Plotter */
  for (i = 0; i < 6; i++)
    data->m_ndc_to_device[i] = t[i];

  return true;
}
