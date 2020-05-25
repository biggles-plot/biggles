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

#define NUM_SVG_COLORNAMES 16

static const plColorNameInfo _svg_colornames[NUM_SVG_COLORNAMES + 1] =
{
  {"aqua",		0x00, 0xff, 0xff},
  {"black",		0x00, 0x00, 0x00},
  {"blue",		0x00, 0x00, 0xff},
  {"fuchsia",		0xff, 0x00, 0xff},
  {"gray",		0x80, 0x80, 0x80},
  {"green",		0x00, 0x80, 0x00},
  {"lime",		0x00, 0xff, 0x00},
  {"maroon",		0x80, 0x00, 0x00},
  {"navy",		0x00, 0x00, 0x80},
  {"olive",		0x80, 0x80, 0x00},
  {"purple",		0x80, 0x00, 0x80},
  {"red",		0xff, 0x00, 0x00},
  {"silver",		0xc0, 0xc0, 0xc0},
  {"teal",		0x00, 0x80, 0x80},
  {"white",		0xff, 0xff, 0xff},
  {"yellow",		0xff, 0xff, 0x00},
  {NULL,		   0,    0,    0}
};

/* convert a 48-bit libplot color to a string resembling "#ffffff" for SVG;
   may also return a string in our database of SVG's builtin colors */

const char *
_libplot_color_to_svg_color (plColor color_48, char charbuf[8])
{
  plColor color_24;
  int i;
  bool found = false;
  const char *svg_color;
  
  color_24.red = ((unsigned int)color_48.red) >> 8;
  color_24.green = ((unsigned int)color_48.green) >> 8;
  color_24.blue = ((unsigned int)color_48.blue) >> 8;
  
  for (i = 0; i < NUM_SVG_COLORNAMES; i++)
    {
      if (color_24.red == _svg_colornames[i].red
	  && color_24.green == _svg_colornames[i].green
	  && color_24.blue == _svg_colornames[i].blue)
	{
	  found = true;
	  break;
	}
    }

  if (found)
    svg_color = _svg_colornames[i].name;
  else
    {
      sprintf (charbuf, "#%02x%02x%02x", 
	       color_24.red, color_24.green, color_24.blue);
      svg_color = charbuf;
    }
  
  return svg_color;
}
