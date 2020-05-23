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

/* This file contains the pentype method, which is a GNU extension to
   libplot.  It sets a drawing attribute: whether or not a pen should be
   used.  (Objects drawn without a pen may still be filled.) */

/* This file also contains the filltype method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the desaturation level of the
   filling, for all objects created by the drawing operations that follow.
   (For those that can be filled, that is; text cannot be filled.)

   The argument to filltype ranges from 0 to 0xFFFF.  The value 0 is
   special; it signifies no filling at all (the object will be
   transparent).  The value 1 signifies that the fill color should be the
   user-specified fill color, and a value of 0xFFFF signifies complete
   desaturation of this color (i.e., white).  Values intermediate between 1
   and 0xFFFF yield intermediate saturations of the user-specified fill
   color.  An out-of-bounds argument resets the desaturation level to a
   default value.  */

#include "sys-defines.h"
#include "extern.h"

int
_API_pentype (R___(Plotter *_plotter) int level)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "pentype: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if ((level < 0) || (level > 0xffff))
    /* OOB switches to default */
    level = _default_drawstate.pen_type;

  _plotter->drawstate->pen_type = level;
  
  return 0;
}

int
_API_filltype (R___(Plotter *_plotter) int level)
{
  int red, green, blue;
  double red_d, green_d, blue_d;
  double desaturate;
  plColor new_rgb;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "filltype: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if ((level < 0) || (level > 0xffff))
    /* OOB switches to default */
    level = _default_drawstate.fill_type;

  _plotter->drawstate->fill_type = level;
  
  if (level == 0)
    /* won't be doing filling, so stop right here */
    return 0;

  /* update fillcolor, taking fill type into account */

  /* start with base fillcolor */
  red = _plotter->drawstate->fillcolor_base.red;
  green = _plotter->drawstate->fillcolor_base.green;
  blue = _plotter->drawstate->fillcolor_base.blue;

  /* scale each RGB from a 16-bit quantity to range [0.0,1.0] */
  red_d = ((double)red)/0xFFFF;
  green_d = ((double)green)/0xFFFF;
  blue_d = ((double)blue)/0xFFFF;

  /* fill_type, if nonzero, specifies the extent to which the nominal fill
     color should be desaturated.  1 means no desaturation, 0xffff means
     complete desaturation (white). */
  desaturate = ((double)_plotter->drawstate->fill_type - 1.)/0xFFFE;
  red_d = red_d + desaturate * (1.0 - red_d);
  green_d = green_d + desaturate * (1.0 - green_d);
  blue_d = blue_d + desaturate * (1.0 - blue_d);

  /* restore each RGB to a 16-bit quantity (48 bits in all) */
  new_rgb.red = IROUND(0xFFFF * red_d);
  new_rgb.green = IROUND(0xFFFF * green_d);
  new_rgb.blue = IROUND(0xFFFF * blue_d);

  /* store actual fill color in drawing state */
  _plotter->drawstate->fillcolor = new_rgb;

  return 0;
}
