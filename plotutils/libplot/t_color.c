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

/* For TekPlotter objects, setting the pen color or background color has no
   effect unless the plotter is using the Tektronix emulation of MS-DOS
   kermit.  If so, we compute a quantized color and output the appropriate
   ANSI escape sequence, if the color is different from the ANSI.SYS color
   the emulation is currently using.

   When the TekPlotter is created, the current ANSI.SYS pen and bg colors
   are set to -1 (see t_defplot.c).  That's a nonsensical value, equivalent
   to `unknown'. */

#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE (0xff)

/* forward references */
static int kermit_pseudocolor (int red, int green, int blue);

void
_pl_t_set_pen_color(S___(Plotter *_plotter))
{
  if (_plotter->tek_display_type == TEK_DPY_KERMIT)
    {
      int new_kermit_fgcolor;

      new_kermit_fgcolor = 
	kermit_pseudocolor (_plotter->drawstate->fgcolor.red, 
			    _plotter->drawstate->fgcolor.green, 
			    _plotter->drawstate->fgcolor.blue);
      if (new_kermit_fgcolor != _plotter->tek_kermit_fgcolor)
	{
	  _write_string (_plotter->data, 
				  _pl_t_kermit_fgcolor_escapes[new_kermit_fgcolor]);
	  _plotter->tek_kermit_fgcolor = new_kermit_fgcolor;
	}
    }
}  

void
_pl_t_set_bg_color(S___(Plotter *_plotter))
{
  if (_plotter->tek_display_type == TEK_DPY_KERMIT)
    {
      int new_kermit_bgcolor;

      new_kermit_bgcolor = 
	kermit_pseudocolor (_plotter->drawstate->bgcolor.red, 
			    _plotter->drawstate->bgcolor.green, 
			    _plotter->drawstate->bgcolor.blue);
      if (new_kermit_bgcolor != _plotter->tek_kermit_bgcolor)
	{
	  _write_string (_plotter->data,
				  _pl_t_kermit_bgcolor_escapes[new_kermit_bgcolor]);
	  _plotter->tek_kermit_bgcolor = new_kermit_bgcolor;
	}
    }
}  

/* kermit_pseudocolor quantizes to one of kermit's native 16 colors.  (They
   provide a [rather strange] partition of the color cube; see
   t_color2.c.) */

/* find closest known point within the RGB color cube, using Euclidean
   distance as our metric */
static int
kermit_pseudocolor (int red, int green, int blue)
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;
  
  /* reduce to 24 bits */
  red = (red >> 8) & ONEBYTE;
  green = (green >> 8) & ONEBYTE;
  blue = (blue >> 8) & ONEBYTE;

  for (i = 0; i < TEK_NUM_ANSI_SYS_COLORS; i++)
    {
      unsigned long int newdifference;
      
      if (_pl_t_kermit_stdcolors[i].red == 0xff
	  && _pl_t_kermit_stdcolors[i].green == 0xff
	  && _pl_t_kermit_stdcolors[i].blue == 0xff)
	/* white is a possible quantization only for white itself (our
           convention) */
	{
	  if (red == 0xff && green == 0xff && blue == 0xff)
	    {
	      difference = 0;
	      best = i;
	    }
	  continue;
	}

      newdifference = (((_pl_t_kermit_stdcolors[i].red - red) 
			* (_pl_t_kermit_stdcolors[i].red - red))
		       + ((_pl_t_kermit_stdcolors[i].green - green) 
			  * (_pl_t_kermit_stdcolors[i].green - green))
		       + ((_pl_t_kermit_stdcolors[i].blue - blue) 
			  * (_pl_t_kermit_stdcolors[i].blue - blue)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }
  return best;
}

