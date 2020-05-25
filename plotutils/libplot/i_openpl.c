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

#include "sys-defines.h"
#include "extern.h"
#include "xmi.h"

bool
_pl_i_begin_page (S___(Plotter *_plotter))
{
  /* With each call to openpl(), we reset the dynamic GIF-specific data
     members of the GIFPlotter.  The data members, and the values that are
     set, are the same as are used in initializing the GIFPlotter (see
     i_defplot.c). */
     
  _plotter->i_painted_set = (void *)NULL;
  _plotter->i_canvas = (void *)NULL;
  _plotter->i_num_color_indices = 0;
  _plotter->i_bit_depth = 0;
  _plotter->i_frame_nonempty = false;
  _plotter->i_pixels_scanned = 0;
  _plotter->i_pass = 0;
  _plotter->i_hot.x = 0;
  _plotter->i_hot.y = 0;  
  _plotter->i_header_written = false;

  /* Create new image, consisting of bitmap and colormap; initialized to
     background color.  First entries in color table will be (1)
     transparent color [if there is one, and we're animating] and (2)
     background color.  May be the same. */
  _pl_i_new_image (S___(_plotter));
  
  /* frame starts empty */
  _plotter->i_frame_nonempty = false;

  /* GIF file header not yet written */
  _plotter->i_header_written = false;

  return true;
}

/* Internal function: Create new image, consisting of bitmap and colormap;
   initialized to background color.  First entries in color table will be
   (1) transparent color [if there is one, and we're animating] and (2)
   background color.  Maybe the same. */
void
_pl_i_new_image (S___(Plotter *_plotter))
{
  int i;
  miPixel pixel;
  
  /* colormap starts empty (unused entries initted to `black'; we may later
     need to output some of the unused entries because GIF colormap lengths
     are always powers of 2) */
  _plotter->i_num_color_indices = 0;
  for (i = 0; i < 256; i++)
    {
      _plotter->i_colormap[i].red = 0;
      _plotter->i_colormap[i].green = 0;
      _plotter->i_colormap[i].blue = 0;
    }      

  /* flag any color indices stored in current drawing state as bogus */
  _plotter->drawstate->i_pen_color_status = false;
  _plotter->drawstate->i_fill_color_status = false;
  _plotter->drawstate->i_bg_color_status = false;

  /* Transparency feature of GIF89a files requires that the index of the
     transparent color be the same for all images in the file.  So if we're
     animating, i.e. writing a multi-image file, we allocate the
     transparent color as the first color index (#0) in all images. */
  if (_plotter->i_transparent && _plotter->i_animation)
    /* allocate color cell in colormap; see i_color.c */
    _pl_i_new_color_index (R___(_plotter) 
			_plotter->i_transparent_color.red,
			_plotter->i_transparent_color.green,
			_plotter->i_transparent_color.blue);

  /* allocate bg color as next color index in colormap (it could well be
     the same as the transparent index); also construct a miPixel for it */
  _pl_i_set_bg_color (S___(_plotter));
  pixel.type = MI_PIXEL_INDEX_TYPE;
  pixel.u.index = _plotter->drawstate->i_bg_color_index;

  /* create libxmi miPaintedSet and miCanvas structs */
  _plotter->i_painted_set = (void *)miNewPaintedSet ();
  _plotter->i_canvas = (void *)miNewCanvas ((unsigned int)_plotter->i_xn, (unsigned int)_plotter->i_yn, pixel);
}
