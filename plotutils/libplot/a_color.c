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

/* We call these routines, which set the Illustrator pen and fill colors,
   lazily i.e. only when needed (just before an object is written to the
   output buffer). */

#include "sys-defines.h"
#include "extern.h"

void
_pl_a_set_pen_color(S___(Plotter *_plotter))
{
  double red, green, blue;
  double cyan, magenta, yellow, black, temp;

  /* convert from RGB to CMYK */
  red = ((double)((_plotter->drawstate->fgcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fgcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fgcolor).blue))/0xFFFF;
  cyan = 1.0 - red;
  magenta = 1.0 - green;
  yellow = 1.0 - blue;
  temp = magenta < yellow ? magenta : yellow;
  black = cyan < temp ? cyan : temp;
  cyan -= black;
  magenta -= black;
  yellow -= black;

  if ((_plotter->ai_pen_cyan != cyan)
      || (_plotter->ai_pen_magenta != magenta)
      || (_plotter->ai_pen_yellow != yellow)
      || (_plotter->ai_pen_black != black))
    /* need to change pen CMYK */
    {
      sprintf (_plotter->data->page->point, "%.4f %.4f %.4f %.4f K\n",
	       cyan, magenta, yellow, black);
      _update_buffer (_plotter->data->page);
      _plotter->ai_pen_cyan = cyan;
      _plotter->ai_pen_magenta = magenta;  
      _plotter->ai_pen_yellow = yellow;
      _plotter->ai_pen_black = black;
    }
  
  /* keep track of which colors AI uses */
  if (_plotter->ai_pen_cyan > 0.0)
    _plotter->ai_cyan_used = true;
  if (_plotter->ai_pen_magenta > 0.0)
    _plotter->ai_magenta_used = true;
  if (_plotter->ai_pen_yellow > 0.0)
    _plotter->ai_yellow_used = true;
  if (_plotter->ai_pen_black > 0.0)
    _plotter->ai_black_used = true;
}

void
_pl_a_set_fill_color(R___(Plotter *_plotter) bool force_pen_color)
{
  double red, green, blue;
  double cyan, magenta, yellow, black, temp;

  if (force_pen_color == false && _plotter->drawstate->fill_type == 0) 
    /* won't be doing filling, so punt */
    return;

  /* get color; if force_pen_color is set, get pen color instead
     of fill color */
  if (force_pen_color)
    {
      red = ((double)((_plotter->drawstate->fgcolor).red))/0xFFFF;
      green = ((double)((_plotter->drawstate->fgcolor).green))/0xFFFF;
      blue = ((double)((_plotter->drawstate->fgcolor).blue))/0xFFFF;
    }
  else
    {
      red = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
      green = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
      blue = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;
    }

  /* convert from RGB to CMYK */
  cyan = 1.0 - red;
  magenta = 1.0 - green;
  yellow = 1.0 - blue;
  temp = magenta < yellow ? magenta : yellow;
  black = cyan < temp ? cyan : temp;
  cyan -= black;
  magenta -= black;
  yellow -= black;

  if ((_plotter->ai_fill_cyan != cyan)
      || (_plotter->ai_fill_magenta != magenta)
      || (_plotter->ai_fill_yellow != yellow)
      || (_plotter->ai_fill_black != black))
    /* need to change AI fill CMYK */
    {
      sprintf (_plotter->data->page->point, "%.4f %.4f %.4f %.4f k\n",
	       cyan, magenta, yellow, black);
      _update_buffer (_plotter->data->page);
      _plotter->ai_fill_cyan = cyan;
      _plotter->ai_fill_magenta = magenta;  
      _plotter->ai_fill_yellow = yellow;
      _plotter->ai_fill_black = black;
    }
  
  /* keep track of which colors AI uses */
  if (_plotter->ai_fill_cyan > 0.0)
    _plotter->ai_cyan_used = true;
  if (_plotter->ai_fill_magenta > 0.0)
    _plotter->ai_magenta_used = true;
  if (_plotter->ai_fill_yellow > 0.0)
    _plotter->ai_yellow_used = true;
  if (_plotter->ai_fill_black > 0.0)
    _plotter->ai_black_used = true;
}
