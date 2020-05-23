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

#define ONEBYTE 0xff
#define REGIS_NUM_STD_COLORS 8

/* standard ReGIS colors */
static const plColor regis_stdcolors[REGIS_NUM_STD_COLORS] = 
{
  {0xff, 0x00, 0x00},		/* Red */
  {0x00, 0xff, 0x00},		/* Green */
  {0x00, 0x00, 0xff},		/* Blue */
  {0x00, 0xff, 0xff},		/* Cyan */
  {0xff, 0x00, 0xff},		/* Magenta */
  {0xff, 0xff, 0x00},		/* Yellow */
  {0x00, 0x00, 0x00},		/* Black */
  {0xff, 0xff, 0xff}		/* White */
};

/* corresponding one-letter abbreviations (in same order as preceding) */
static const char regis_color_chars[REGIS_NUM_STD_COLORS] =
{ 'r', 'g', 'b', 'c', 'm', 'y', 'd', 'w' };

/* forward references */
static int rgb_to_best_stdcolor (plColor rgb);

void
_pl_r_set_pen_color(S___(Plotter *_plotter))
{
  int new_color;

  new_color = rgb_to_best_stdcolor (_plotter->drawstate->fgcolor);
  if (_plotter->regis_fgcolor_is_unknown
      || _plotter->regis_fgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(I(%c))\n", 
	       regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_fgcolor = new_color;
      _plotter->regis_fgcolor_is_unknown = false;
    }
}

void
_pl_r_set_fill_color(S___(Plotter *_plotter))
{
  int new_color;

  /* sanity check */
  if (_plotter->drawstate->fill_type == 0)
    return;

  new_color = rgb_to_best_stdcolor (_plotter->drawstate->fillcolor);
  if (_plotter->regis_fgcolor_is_unknown
      || _plotter->regis_fgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "W(I(%c))\n", 
	       regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_fgcolor = new_color;
      _plotter->regis_fgcolor_is_unknown = false;
    }
}

void
_pl_r_set_bg_color(S___(Plotter *_plotter))
{
  int new_color;

  new_color = rgb_to_best_stdcolor (_plotter->drawstate->bgcolor);
  if (_plotter->regis_bgcolor_is_unknown
      || _plotter->regis_bgcolor != new_color)
    {
      char tmpbuf[32];

      sprintf (tmpbuf, "S(I(%c))\n", 
	       regis_color_chars[new_color]);
      _write_string (_plotter->data, tmpbuf);
      _plotter->regis_bgcolor = new_color;
      _plotter->regis_bgcolor_is_unknown = false;

      /* note: must do an erase, for the just-set background color to show
	 up on the ReGIS display */
    }
}

/* compute best approximation, in color table, to a specified 48-bit color */
static int
rgb_to_best_stdcolor (plColor rgb)
{
  int red, green, blue;
  unsigned long int difference = INT_MAX;
  int i, best = 0;		/* keep compiler happy */
  
  /* convert from 48-bit color to 24-bit */
  red = rgb.red;
  green = rgb.green;
  blue = rgb.blue;
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < REGIS_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      newdifference = (((regis_stdcolors[i].red - red) 
			* (regis_stdcolors[i].red - red))
		       + ((regis_stdcolors[i].green - green) 
			  * (regis_stdcolors[i].green - green))
		       + ((regis_stdcolors[i].blue - blue) 
			  * (regis_stdcolors[i].blue - blue)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return best;
}
