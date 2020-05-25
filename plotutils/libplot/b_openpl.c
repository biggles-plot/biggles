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
_pl_b_begin_page (S___(Plotter *_plotter))
{
  /* create new pixmap of specified size (all pixels of background color) */
  _pl_b_new_image (S___(_plotter));

  return true;
}

/* internal function: create new image, consisting of a bitmap; also fill
   with Plotter's background color */
void
_pl_b_new_image (S___(Plotter *_plotter))
{
  unsigned char red, green, blue;
  miPixel pixel;

  /* compute 24-bit bg color, and construct a miPixel for it */
  red = ((unsigned int)(_plotter->drawstate->bgcolor.red) >> 8) & 0xff;
  green = ((unsigned int)(_plotter->drawstate->bgcolor.green) >> 8) & 0xff;
  blue = ((unsigned int)(_plotter->drawstate->bgcolor.blue) >> 8) & 0xff;  
  pixel.type = MI_PIXEL_RGB_TYPE;
  pixel.u.rgb[0] = red;
  pixel.u.rgb[1] = green;
  pixel.u.rgb[2] = blue;

  /* create libxmi miPaintedSet and miCanvas structs */
  _plotter->b_painted_set = (void *)miNewPaintedSet ();
  _plotter->b_canvas = (void *)miNewCanvas ((unsigned int)_plotter->b_xn, (unsigned int)_plotter->b_yn, pixel);
}
