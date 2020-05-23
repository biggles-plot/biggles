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

/* This file contains the Plotter-specific _retrieve_font method, which is
   called by the _set_font() function, which in turn is invoked by the API
   functions alabel() and flabelwidth().  It is called when the font_name,
   font_size, and textangle fields of the current drawing state have been
   filled in.  It retrieves the specified font, and fills in the font_type,
   typeface_index, font_index, font_is_iso8858, true_font_size, and
   font_ascent, and font_descent fields of the drawing state. */

/* This version is for FigPlotters.  It also fills in the fig_point_size
   field of the drawing state. */

/* This Fig-specific version is needed because xfig supports arbitrary
   (non-integer) font sizes for PS fonts only on paper.  The current
   releases (3.1 and 3.2) of xfig round them to integers.  So we quantize
   the user-specified font size in such a way that the font size that xfig
   will see, and use, will be precisely an integer. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_f_retrieve_font (S___(Plotter *_plotter))
{
  double theta;
  double dx, dy, device_dx, device_dy, device_vector_len;
  double pointsize, fig_pointsize, size, quantized_size;
  int int_fig_pointsize;
  double quantization_factor;

  /* sanity check */
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT)
    return false;
  
  if (!_plotter->drawstate->transform.uniform 
      || !_plotter->drawstate->transform.nonreflection)
    /* anamorphically transformed PS font not supported, will use Hershey */
    return false;

  /* text rotation in radians */
  theta = _plotter->drawstate->text_rotation * M_PI / 180.0;

  /* unit vector along which we'll move when printing label */
  dx = cos (theta);
  dy = sin (theta);

  /* convert to device frame, and compute length in fig units */
  device_dx = XDV(dx, dy);
  device_dy = YDV(dx, dy);  
  device_vector_len = sqrt(device_dx * device_dx + device_dy * device_dy);

  /* compute xfig pointsize we should use when printing a string in a PS
     font, so as to match this vector length. */

  size = _plotter->drawstate->font_size; /* in user units */
  pointsize = FIG_UNITS_TO_POINTS(size * device_vector_len);

  /* FIG_FONT_SCALING = 80/72 is a silly undocumented factor that shouldn't
     exist, but does.  In xfig, a `point' is not 1/72 inch, but 1/80 inch!  */
  fig_pointsize = FIG_FONT_SCALING * pointsize;
  /* integer xfig pointsize (which really refers to ascent, not overall size)*/
  int_fig_pointsize = IROUND(fig_pointsize);

  /* Integer font size that xfig will see, in the .fig file.  If this is
     zero, we won't actually emit a text object to the .fig file, since
     xfig can't handle text strings with zero font size.  See f_text.c. */
  _plotter->drawstate->fig_font_point_size = int_fig_pointsize;
 
  /* what size in user units should have been, to make fig_font_point_size
     an integer */
  if (device_vector_len == 0.0)
    quantized_size = 0.0;	/* degenerate case */
  else
    quantized_size = 
      (POINTS_TO_FIG_UNITS((double)int_fig_pointsize / FIG_FONT_SCALING))
      / (device_vector_len);
  _plotter->drawstate->true_font_size = quantized_size;

  /* quantize other fields */
  if (size == 0.0)
    quantization_factor = 0.0;	/* degenerate case */
  else
    quantization_factor = quantized_size / size;
  _plotter->drawstate->font_ascent *= quantization_factor;
  _plotter->drawstate->font_descent *= quantization_factor;
  _plotter->drawstate->font_cap_height *= quantization_factor;

  return true;
}
