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

bool
_pl_i_erase_page (S___(Plotter *_plotter))
{
  /* If we're animating, emit the GIF header, and emit the just-finished
     frame as one of the images in the animated GIF.  But don't do this
     for the zeroth frame unless it's nonempty. */
  if (_plotter->i_animation && _plotter->data->page_number == 1 && _plotter->data->outfp
      && (_plotter->data->frame_number > 0 || _plotter->i_frame_nonempty))
    {
      if (_plotter->i_header_written == false)
	{
	  _pl_i_write_gif_header (S___(_plotter));
	  _plotter->i_header_written = true;
	}
      /* emit image using RLE module (see i_rle.c) */
      _pl_i_write_gif_image (S___(_plotter));
    }

  /* delete image: deallocate frame's libxmi canvas, reset frame's color
     table */
  _pl_i_delete_image (S___(_plotter));

  /* Create new image, consisting of libxmi canvas and colormap;
     initialized to background color.  First entries in the color table
     will be (1) transparent color [if there is one, and we're animating]
     and (2) background color. */
  _pl_i_new_image (S___(_plotter));
  
  /* next frame will start empty */
  _plotter->i_frame_nonempty = false;

  return true;
}
