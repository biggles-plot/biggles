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

/* This file contains device-specific color database access routines.
   These routines are called by various GIFPlotter methods, before drawing
   objects.  They set the appropriate GIFPlotter-specific fields in the
   drawing state. */

#include "sys-defines.h"
#include "extern.h"

/* forward references */
static int bit_depth (int colors);

/* we call this routine to evaluate _plotter->drawstate->i_pen_color_index
   lazily, i.e. only when needed (just before a drawing operation) */
void
_pl_i_set_pen_color(S___(Plotter *_plotter))
{
  int red_long, green_long, blue_long;
  int red, green, blue;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->fgcolor.red;
  green_long = _plotter->drawstate->fgcolor.green;
  blue_long = _plotter->drawstate->fgcolor.blue;

  /* 24-bit RGB (as used in GIFs) */
  red = (((unsigned int)red_long) >> 8) & 0xff;
  green = (((unsigned int)green_long) >> 8) & 0xff;
  blue = (((unsigned int)blue_long) >> 8) & 0xff;

  if (!(_plotter->drawstate->i_pen_color_status
	&& _plotter->drawstate->i_pen_color.red == red
	&& _plotter->drawstate->i_pen_color.green == green
	&& _plotter->drawstate->i_pen_color.blue == blue))
    /* need another color index: search table, expand if necessary */
    {
      unsigned char index;
      
      index = _pl_i_new_color_index (R___(_plotter) red, green, blue);

      /* set new 24-bit RGB and color index in the drawing state */
      _plotter->drawstate->i_pen_color.red = red;
      _plotter->drawstate->i_pen_color.green = green;
      _plotter->drawstate->i_pen_color.blue = blue;
      _plotter->drawstate->i_pen_color_index = index;

      /* flag this color index as genuine */
      _plotter->drawstate->i_pen_color_status = true;
    }
}

/* we call this routine to evaluate _plotter->drawstate->i_fill_color_index
   lazily, i.e. only when needed (just before a filling operation) */
void
_pl_i_set_fill_color(S___(Plotter *_plotter))
{
  int red_long, green_long, blue_long;
  int red, green, blue;

  if (_plotter->drawstate->fill_type == 0)
    /* don't do anything, fill color will be ignored when writing objects */
    return;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->fillcolor.red;
  green_long = _plotter->drawstate->fillcolor.green;
  blue_long = _plotter->drawstate->fillcolor.blue;

  /* 24-bit RGB (as used in GIFs) */
  red = (((unsigned int)red_long) >> 8) & 0xff;
  green = (((unsigned int)green_long) >> 8) & 0xff;
  blue = (((unsigned int)blue_long) >> 8) & 0xff;

  if (!(_plotter->drawstate->i_fill_color_status
	&& _plotter->drawstate->i_fill_color.red == red
	&& _plotter->drawstate->i_fill_color.green == green
	&& _plotter->drawstate->i_fill_color.blue == blue))
    /* need another color index: search table, expand if necessary */
    {
      unsigned char index;
      
      index = _pl_i_new_color_index (R___(_plotter) red, green, blue);

      /* set new 24-bit RGB and color index in the drawing state */
      _plotter->drawstate->i_fill_color.red = red;
      _plotter->drawstate->i_fill_color.green = green;
      _plotter->drawstate->i_fill_color.blue = blue;
      _plotter->drawstate->i_fill_color_index = index;
    }
}

/* we call this routine to compute and set
   _plotter->drawstate->i_bg_color_index, e.g. in _pl_i_new_image */
void
_pl_i_set_bg_color(S___(Plotter *_plotter))
{
  int red_long, green_long, blue_long;
  int red, green, blue;

  /* 48-bit RGB */
  red_long = _plotter->drawstate->bgcolor.red;
  green_long = _plotter->drawstate->bgcolor.green;
  blue_long = _plotter->drawstate->bgcolor.blue;

  /* 24-bit RGB (as used in GIFs) */
  red = (((unsigned int)red_long) >> 8) & 0xff;
  green = (((unsigned int)green_long) >> 8) & 0xff;
  blue = (((unsigned int)blue_long) >> 8) & 0xff;

  if (!(_plotter->drawstate->i_bg_color_status
	&& _plotter->drawstate->i_bg_color.red == red
	&& _plotter->drawstate->i_bg_color.green == green
	&& _plotter->drawstate->i_bg_color.blue == blue))
    /* need another color index: search table, expand if necessary */
    {
      unsigned char index;
      
      index = _pl_i_new_color_index (R___(_plotter) red, green, blue);

      /* set new 24-bit RGB and color index in the drawing state */
      _plotter->drawstate->i_bg_color.red = red;
      _plotter->drawstate->i_bg_color.green = green;
      _plotter->drawstate->i_bg_color.blue = blue;
      _plotter->drawstate->i_bg_color_index = index;

      /* flag this color index as genuine */
      _plotter->drawstate->i_bg_color_status = true;
    }
}

/* Internal function, called by each of the above.  It searches for a
   24-bit RGB in the color table.  If not found, it's added to table,
   unless table can't be expanded, in which case index with closest RGB is
   returned. */

unsigned char 
_pl_i_new_color_index (R___(Plotter *_plotter) int red, int green, int blue)
{
  int i, j;
  int sqdist;
  bool found = false;

  for (i = 0; i < _plotter->i_num_color_indices; i++)
    if (_plotter->i_colormap[i].red == red
	&& _plotter->i_colormap[i].green == green
	&& _plotter->i_colormap[i].blue == blue)
      {
	found = true;
	break;
      }
  if (found)
    return (unsigned char)i;

  /* not found, try to allocate new index */
  i = _plotter->i_num_color_indices;
  if (i < 256)
    {
      _plotter->i_colormap[i].red = red;
      _plotter->i_colormap[i].green = green;
      _plotter->i_colormap[i].blue = blue;
      _plotter->i_num_color_indices = i + 1;

      /* New bit depth of colormap, e.g. sizes 129..256 get mapped to 8.
	 In effect the colormap for any of these sizes will be of size 256. */
      _plotter->i_bit_depth = bit_depth (i + 1);

      return (unsigned char)i;
    }
  
  /* table full, do our best */
  sqdist = INT_MAX;
  i = 0;
  for (j = 0; j < 256; j++)
    {
      int new_sqdist;
      int a_red, a_green, a_blue;
      
      a_red = _plotter->i_colormap[j].red;
      a_green = _plotter->i_colormap[j].green;
      a_blue = _plotter->i_colormap[j].blue;
      new_sqdist = ((a_red - red) * (a_red - red)
		    + (a_green - green) * (a_green - green)
		    + (a_blue - blue) * (a_blue - blue));
      if (new_sqdist <= sqdist)
	{
	  sqdist = new_sqdist;
	  i = j;		/* best to date */
	}
    }
  return (unsigned char)i;
}

/* compute number of bits needed to represent all color indices
   (when this is called, colors >= 1) */
static int
bit_depth (int colors)
{
  int size;
  unsigned int ucolors;

  /* subtract 1, see how many bits needed to represent result */
  size = 0;
  for (ucolors = colors - 1; ucolors; ucolors = ucolors >> 1)
    size++;

  return size;
}
