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

/* This file contains the linewidth method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the line width used in subsequent
   drawing operations, in user units.

   It also computes an estimate for the width of lines in device units.
   This quantity is used by display devices that do not support `sheared
   lines'. */

#include "sys-defines.h"
#include "extern.h"

int
_API_flinewidth(R___(Plotter *_plotter) double new_line_width)
{
  double device_line_width, min_sing_val, max_sing_val;
  int quantized_device_line_width;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "flinewidth: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if (new_line_width < 0.0)	/* reset to default */
    {
      new_line_width = _plotter->drawstate->default_line_width;
      _plotter->drawstate->line_width_is_default = true;
    }
  else
    _plotter->drawstate->line_width_is_default = false;
  
  /* set the new linewidth in the drawing state */
  _plotter->drawstate->line_width = new_line_width;
  
  /* Also compute and set the device-frame line width, and a quantized
     (i.e. integer) version of same, which is used by most Plotters that
     use integer device coordinates. */

  _matrix_sing_vals (_plotter->drawstate->transform.m, 
		     &min_sing_val, &max_sing_val);
  device_line_width = min_sing_val * new_line_width;
  quantized_device_line_width = IROUND(device_line_width);

  /* Don't quantize the device-frame line width to 0 if user specified
     nonzero width.  If it has a bitmap display (rendered with libxmi),
     quantizing to 0 might be regarded as OK, since libxmi treats 0-width
     lines as Bresenham lines rather than invisible.  However, the Hershey
     fonts don't look good at small sizes if their line segments are
     rendered as Bresenham lines.  */

  if (quantized_device_line_width == 0 && device_line_width > 0.0)
    quantized_device_line_width = 1;
  
  _plotter->drawstate->device_line_width = device_line_width;
  _plotter->drawstate->quantized_device_line_width 
    = quantized_device_line_width;

  /* flag linewidth as having been invoked on this page (so that fsetmatrix
     will no longer automatically adjust the line width to a reasonable
     value) */
  _plotter->data->linewidth_invoked = true;

  return 0;
}
