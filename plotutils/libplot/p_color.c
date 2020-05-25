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

/* This file contains device-specific color computation routines.  They are
   called by various PSPlotter methods, before drawing objects.  They set
   the appropriate PSPlotter-specific fields in the drawing state. */

#include "sys-defines.h"
#include "extern.h"

/* forward references */
static int _idraw_pseudocolor (int red, int green, int blue);

/* We call this routine to evaluate _plotter->drawstate->ps_fgcolor lazily,
   i.e. only when needed (just before an object is written to the output
   buffer).  It finds the best match from among idraw's "foreground
   colors", i.e., pen colors.  See p_color2.c for the list of colors. */

void
_pl_p_set_pen_color(S___(Plotter *_plotter))
{
  _plotter->drawstate->ps_fgcolor_red = 
    ((double)((_plotter->drawstate->fgcolor).red))/0xFFFF;
  _plotter->drawstate->ps_fgcolor_green = 
    ((double)((_plotter->drawstate->fgcolor).green))/0xFFFF;
  _plotter->drawstate->ps_fgcolor_blue = 
    ((double)((_plotter->drawstate->fgcolor).blue))/0xFFFF;

  /* quantize for idraw */
  _plotter->drawstate->ps_idraw_fgcolor = 
    _idraw_pseudocolor ((_plotter->drawstate->fgcolor).red,
			(_plotter->drawstate->fgcolor).green,
			(_plotter->drawstate->fgcolor).blue);

  return;
}

/* We call this routine to evaluate _plotter->drawstate->ps_fillcolor
   lazily, i.e. only when needed (just before an object is written to the
   output buffer).  It finds the best match from among the possible idraw
   colors.  In idraw, the fill color is always an interpolation between a
   "foreground color" and a "background color", both of which are selected
   from a fixed set.  See p_color2.c. */

void
_pl_p_set_fill_color(S___(Plotter *_plotter))
{
  double red, green, blue;

  if (_plotter->drawstate->fill_type == 0)
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  red = ((double)((_plotter->drawstate->fillcolor).red))/0xFFFF;
  green = ((double)((_plotter->drawstate->fillcolor).green))/0xFFFF;
  blue = ((double)((_plotter->drawstate->fillcolor).blue))/0xFFFF;

  _plotter->drawstate->ps_fillcolor_red = red;
  _plotter->drawstate->ps_fillcolor_green = green;
  _plotter->drawstate->ps_fillcolor_blue = blue;

  /* next subroutine needs fields that this will fill in... */
  _pl_p_set_pen_color (S___(_plotter));

  /* Quantize for idraw, in a complicated way; we can choose from among a
     finite discrete set of values for ps_idraw_bgcolor and
     ps_idraw_shading, to approximate the fill color.  We also adjust
     ps_fillcolor_* because the PS interpreter will use the
     ps_idraw_shading variable to interpolate between fgcolor and bgcolor,
     i.e. fgcolor and fillcolor. */
  _pl_p_compute_idraw_bgcolor (S___(_plotter));
  
  return;
}

/* Find, within the RGB color cube, the idraw pen color ("foreground
   color") that is closest to a specified color (our pen color).  Euclidean
   distance is our metric.  Our convention: no non-white color should be
   mapped to white. */
static int
_idraw_pseudocolor (int red, int green, int blue)
{
  double difference;
  int i;
  int best = 0;
  
  difference = DBL_MAX;
  for (i = 0; i < PS_NUM_IDRAW_STD_COLORS; i++)
    {
      double newdifference;
      
      if (_pl_p_idraw_stdcolors[i].red == 0xffff
	  && _pl_p_idraw_stdcolors[i].green == 0xffff
	  && _pl_p_idraw_stdcolors[i].blue == 0xffff)
	/* white is a possible quantization only for white itself (our
           convention) */
	{
	  if (red == 0xffff && green == 0xffff && blue == 0xffff)
	    {
	      difference = 0.0;
	      best = i;
	    }
	  continue;
	}

      newdifference = ((double)(_pl_p_idraw_stdcolors[i].red - red)
		       * (double)(_pl_p_idraw_stdcolors[i].red - red))
		    + ((double)(_pl_p_idraw_stdcolors[i].green - green) 
		       * (double)(_pl_p_idraw_stdcolors[i].green - green)) 
		    + ((double)(_pl_p_idraw_stdcolors[i].blue - blue)
		       * (double)(_pl_p_idraw_stdcolors[i].blue - blue));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	}
    }

  return best;
}

/* Once the idraw foreground color (i.e. quantized pen color) has been
   determined, this routine computes the idraw background color and idraw
   shading (0.0, 0.25, 0.5, 0.75, or 1.0) that will most closely match the
   user-specified fill color.  It is called only when the elements
   ps_fillcolor_*, ps_idraw_fgcolor_* of the drawing state have been filled in.

   At the end of this function we adjust ps_fillcolor_* so that the output
   file will produce similar colors when parsed both by idraw and the PS
   interpreter.  In fact we can persuade the PS interpreter to produce
   exactly the fill color specified by the user, except when the idraw
   shading is 0.0.  In that case the fill color must be the same as the pen
   color.  That situation will occur only if the user-specified fill color
   is very close to the user-specified pen color. */

void
_pl_p_compute_idraw_bgcolor(S___(Plotter *_plotter))
{
  double truered, truegreen, trueblue;
  double fgred, fggreen, fgblue;
  double difference = DBL_MAX;
  int i, j;
  int best_bgcolor = 0, best_shading = 0;
  double best_shade = 0.0;

  truered = 0xFFFF * _plotter->drawstate->ps_fillcolor_red;
  truegreen = 0xFFFF * _plotter->drawstate->ps_fillcolor_green;
  trueblue = 0xFFFF * _plotter->drawstate->ps_fillcolor_blue;

  fgred = (double)(_pl_p_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].red);
  fggreen = (double)(_pl_p_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].green);
  fgblue = (double)(_pl_p_idraw_stdcolors[_plotter->drawstate->ps_idraw_fgcolor].blue);

  for (i = 0; i < PS_NUM_IDRAW_STD_COLORS; i++)
    {
      double bgred, bggreen, bgblue;

      bgred = (double)(_pl_p_idraw_stdcolors[i].red);
      bggreen = (double)(_pl_p_idraw_stdcolors[i].green);
      bgblue = (double)(_pl_p_idraw_stdcolors[i].blue);

      for (j = 0; j < PS_NUM_IDRAW_STD_SHADINGS; j++)
	{
	  double approxred, approxgreen, approxblue;
	  double shade, newdifference;
	  
	  shade = _pl_p_idraw_stdshadings[j];
	  
	  approxred = shade * bgred + (1.0 - shade) * fgred;
	  approxgreen = shade * bggreen + (1.0 - shade) * fggreen;
	  approxblue = shade * bgblue + (1.0 - shade) * fgblue;	  

	  newdifference = (truered - approxred) * (truered - approxred)
	    + (truegreen - approxgreen) * (truegreen - approxgreen)
	      + (trueblue - approxblue) * (trueblue - approxblue);
	  
	  if (newdifference < difference)
	    {
	      difference = newdifference;
	      best_bgcolor = i;
	      best_shading = j;
	      best_shade = shade;
	    }
	}
    }

  _plotter->drawstate->ps_idraw_bgcolor = best_bgcolor;
  _plotter->drawstate->ps_idraw_shading = best_shading;

  /* now adjust ps_fillcolor_* fields so that interpolation between
     ps_fgcolor_* and ps_fillcolor_*, as specified by the shade, will yield
     the user-specified fill color.  According to the PS prologue, the PS
     interpreter will compute a fill color thus:

     true_FILLCOLOR = shade * PS_FILLCOLOR + (1-shade) * PS_FGCOLOR

     we can compute an adjusted fillcolor thus:

     PS_FILLCOLOR = (true_FILLCOLOR - (1-shade) * PS_FGCOLOR) / shade.

     This is possible unless shade=0.0, in which case both idraw and the PS
     interpreter will use the pen color as the fill color. */

  if (best_shade != 0.0)
    {
      _plotter->drawstate->ps_fillcolor_red 
	= (_plotter->drawstate->ps_fillcolor_red 
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_red) / best_shade;
      _plotter->drawstate->ps_fillcolor_green
	= (_plotter->drawstate->ps_fillcolor_green
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_green) / best_shade;
      _plotter->drawstate->ps_fillcolor_blue
	= (_plotter->drawstate->ps_fillcolor_blue
	   - (1.0 - best_shade) * _plotter->drawstate->ps_fgcolor_blue) / best_shade;
    }
}
