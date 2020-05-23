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

/* This file contains device-specific color database access routines.  They
   are called by various FigPlotter methods, before drawing objects.  They
   set the appropriate FigPlotter-specific fields in the drawing state. */

#include "sys-defines.h"
#include "extern.h"

#define ONEBYTE 0xff

/* by setting this undocumented variable, user may request quantization of
   colors (no user-defined colors, only native xfig ones). */
#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
int _libplotfig_use_pseudocolor = 0;
#endif

/* FIG_COLOR returns the index of the Fig color corresponding to specified
   a 48-bit RGB value.  This has (or may have) a side effect.  If the Fig
   color is not standard, it will be added to the database of user-defined
   colors, and a Fig `color pseudo-object' will be output later.

   Note: according to Fig documentation, xfig should support as many as
   FIG_NUM_USER_COLORS (i.e., 512) user-defined colors.  However, I have
   observed that on at least one platform, it pops up a warning message
   unless the number is 511 or less.  That's why FIG_NUM_USER_COLORS - 1
   appears in the code below.

   We do not call this function whenever the user calls pencolor() or
   fillcolor(), since we don't want to fill up the database with colors
   that the user may not actually use.  Instead, we call it just before we
   write a colored object to the output buffer (lazy evaluation), by
   evaluating the f_set_pen_color() and f_set_fill_color() functions below.

   If the external variable _libplotfig_use_pseudocolor is nonzero, we
   don't actually maintain a database of user-defined colors.  Instead we
   just quantize to one of xfig's native 32 colors.  (They provide a
   [rather strange] partition of the color cube; see f_color2.c.) */

/* forward references */
static int _fig_pseudocolor (int red, int green, int blue, const long int *fig_usercolors, int fig_num_usercolors);

int
_pl_f_fig_color(R___(Plotter *_plotter) int red, int green, int blue)
{
  int fig_fgcolor_red, fig_fgcolor_green, fig_fgcolor_blue;
  long int fig_fgcolor_rgb;
  int i;

  /* xfig supports only 24-bit color, so extract 8 bits for each of R,G,B */
  fig_fgcolor_red = (red >> 8) & ONEBYTE;
  fig_fgcolor_green = (green >> 8) & ONEBYTE;
  fig_fgcolor_blue = (blue >> 8) & ONEBYTE;

#ifdef SUPPORT_FIG_COLOR_QUANTIZATION
  if (_libplotfig_use_pseudocolor)
    /* always quantize: approximate by closest standard color, and don't
       create user-defined colors at all */
    return _fig_pseudocolor (fig_fgcolor_red, fig_fgcolor_green,
			     fig_fgcolor_blue, 
			     (const long int *)NULL, 0);
#endif

  /* search list of standard colors */
  for (i = 0; i < FIG_NUM_STD_COLORS; i++)
    {
      if ((_pl_f_fig_stdcolors[i].red == fig_fgcolor_red)
	  && (_pl_f_fig_stdcolors[i].green == fig_fgcolor_green)
	  && (_pl_f_fig_stdcolors[i].blue == fig_fgcolor_blue))
	/* perfect match, return it */
	return i;
    }

  /* This is the 24-bit (i.e. 3-byte) integer used internally by xfig, and
     also by us when we stored user-defined colors.  We assume long ints
     are wide enough to handle 3 bytes. */
  fig_fgcolor_rgb = (fig_fgcolor_red << 16) + (fig_fgcolor_green << 8)
		    + (fig_fgcolor_blue);
    
  /* search list of user-defined colors */
  for (i = 0; i < _plotter->fig_num_usercolors; i++) 
    {
      if (_plotter->fig_usercolors[i] == fig_fgcolor_rgb)
	/* perfect match, return it */
	return FIG_USER_COLOR_MIN + i;
    }

  /* color wasn't found in either list */

  if (_plotter->fig_num_usercolors == FIG_MAX_NUM_USER_COLORS - 1)
    /* can't add new color to user-defined list, must approximate */
    {
      if (_plotter->fig_colormap_warning_issued == false)
	{
	  _plotter->warning (R___(_plotter) 
			     "supply of user-defined colors is exhausted");
	  _plotter->fig_colormap_warning_issued = true;
	}
      return _fig_pseudocolor (fig_fgcolor_red, fig_fgcolor_green,
			       fig_fgcolor_blue, 
			       _plotter->fig_usercolors,
			       FIG_MAX_NUM_USER_COLORS - 1);
    }
  else
    /* create new user-defined color, will emit it to the .fig file */
    {
      _plotter->fig_usercolors[_plotter->fig_num_usercolors] = fig_fgcolor_rgb;
      _plotter->fig_num_usercolors++;
      return FIG_USER_COLOR_MIN + _plotter->fig_num_usercolors - 1;
    }
}

/* Find closest known point to a specified 24-bit color within the RGB
   color cube, using Euclidean distance as our metric.  We search both
   Fig's standard colors and a specified number of user-defined colors,
   which are stored in an array, a pointer to which is passed.  Return
   value is Fig color index.  Standard Fig colors are located in
   0..FIG_NUM_STD_COLORS-1, and user-defined colors beginning at
   FIG_USER_COLOR_MIN, which is equal to FIG_NUM_STD_COLORS. */

static int
_fig_pseudocolor (int red, int green, int blue, const long int *fig_usercolors, int fig_num_usercolors)
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;
  
  for (i = 0; i < FIG_NUM_STD_COLORS; i++)
    {
      unsigned long int newdifference;
      
      if (_pl_f_fig_stdcolors[i].red == 0xff
	  && _pl_f_fig_stdcolors[i].green == 0xff
	  && _pl_f_fig_stdcolors[i].blue == 0xff)
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

      newdifference = (((_pl_f_fig_stdcolors[i].red - red) 
			* (_pl_f_fig_stdcolors[i].red - red))
		       + ((_pl_f_fig_stdcolors[i].green - green) 
			  * (_pl_f_fig_stdcolors[i].green - green))
		       + ((_pl_f_fig_stdcolors[i].blue - blue) 
			  * (_pl_f_fig_stdcolors[i].blue - blue)));
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;		/* save Fig color index */
	}
    }

  /* search through passed array of user-defined colors too */
  for (i = 0; i < fig_num_usercolors; i++)
    {
      unsigned long int newdifference;
      plColor usercolor;
      
      /* extract 3 RGB octets from 24-byte Fig-style color */
      usercolor.red = (fig_usercolors[i] >> 16) & ONEBYTE;
      usercolor.green = (fig_usercolors[i] >> 8) & ONEBYTE;
      usercolor.blue = (fig_usercolors[i] >> 0) & ONEBYTE;

      newdifference = ((usercolor.red - red) * (usercolor.red - red)
		       + (usercolor.green - green) * (usercolor.green - green)
		       + (usercolor.blue - blue) * (usercolor.blue - blue));
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i + FIG_USER_COLOR_MIN; /* save Fig color index */
	}
    }

  return best;
}

/* we call this routine to evaluate _plotter->drawstate->fig_fgcolor
   lazily, i.e. only when needed (just before an object is written to the
   output buffer) */
void
_pl_f_set_pen_color(S___(Plotter *_plotter))
{
  /* OOB switches to default color */
  if (((_plotter->drawstate->fgcolor).red > 0xffff) 
      || ((_plotter->drawstate->fgcolor).green > 0xffff) 
      || ((_plotter->drawstate->fgcolor).blue > 0xffff))
    _plotter->drawstate->fig_fgcolor = _default_drawstate.fig_fgcolor;
  else
    _plotter->drawstate->fig_fgcolor = 
      _pl_f_fig_color (R___(_plotter) 
		       (_plotter->drawstate->fgcolor).red,
		       (_plotter->drawstate->fgcolor).green, 
		       (_plotter->drawstate->fgcolor).blue);
  return;
}

/* we call this routine to evaluate _plotter->drawstate->fig_fillcolor and
   _plotter->drawstate->fig_fill_level lazily, i.e. only when needed (just
   before an object is written to the output buffer) */

void
_pl_f_set_fill_color(S___(Plotter *_plotter))
{
  double fill_level;

  /* OOB switches to default color */
  if (_plotter->drawstate->fillcolor_base.red > 0xffff
      || _plotter->drawstate->fillcolor_base.green > 0xffff
      || _plotter->drawstate->fillcolor_base.blue > 0xffff)
    _plotter->drawstate->fig_fillcolor = _default_drawstate.fig_fillcolor;

  else
    _plotter->drawstate->fig_fillcolor = 
      _pl_f_fig_color (R___(_plotter)
		       _plotter->drawstate->fillcolor_base.red,
		       _plotter->drawstate->fillcolor_base.green, 
		       _plotter->drawstate->fillcolor_base.blue);
  
  /* Now that we know drawstate->fig_fillcolor, we can compute the fig fill
     level that will match the user's requested fill level.  Fig fill level
     is interpreted in a color dependent way, as follows.  The value -1 is
     special; means no fill at all (objects will be transparent).  For
     other values, this is the interpretation:
     
     Color = black or default:
     		fill = 0  -> white
		fill = 1  -> very light grey
		     .
		     .
	        fill = 19 -> very dark grey
     		fill = 20 -> black

     Color = all colors other than black or default, including white
   		   fill = 0  -> black
   		   fill = 1  -> color, very faint intensity
			.
			.
		   fill = 19 -> color, very bright intensity
		   fill = 20 -> color, full intensity

     So 1->20 give increasingly intense "shades" of the color, with 20
     giving the color itself.  Values 20->40 are increasingly desaturated
     "tints" of the color, ranging from the color itself (20) to white
     (40).  A tint is defined as the color mixed with white.  (Values
     21->40 are not used when the color is black or default, or white
     itself.) */

  fill_level = ((double)_plotter->drawstate->fill_type - 1.)/0xFFFE;

  /* OOB sets fill level to a non-OOB default value */
  if (fill_level > 1.)
    fill_level = ((double)_default_drawstate.fill_type - 1.)/0xFFFE;

  /* level = 0 turns off filling (objects will be transparent) */
  else if (fill_level < 0.)
    fill_level = -1.0;

  if (fill_level == -1.0)
    _plotter->drawstate->fig_fill_level = -1;
  else
    {
      switch (_plotter->drawstate->fig_fillcolor)
	{
	case FIG_C_WHITE:	/* can't desaturate white */
	  _plotter->drawstate->fig_fill_level = 20;
	  break;
	case FIG_C_BLACK:
	  _plotter->drawstate->fig_fill_level = IROUND(20.0 - 20.0 * fill_level);
	  break;
	default:		/* interpret fill level as a saturation */
	  _plotter->drawstate->fig_fill_level = IROUND(20.0 + 20.0 * fill_level);
	  break;
	}
    }
  return;
}
