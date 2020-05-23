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

/* This file contains color methods that are GNU extensions to libplot:
   pencolor, fillcolor, bgcolor, and the convenience function, color.

   It also contains _grayscale_approx(), which computes grayscale
   approximations to RGB colors. */

/* It also includes the pencolorname, fillcolorname, and bgcolorname
   methods, which are GNU extensions to libplot; also the convenience
   function colorname.  They search a database of known names (stored in
   g_colorname.h) for a specified color name.  If the name is found, its
   interpretation as a 48-bit RGB color is determined, and pencolor,
   fillcolor, or bgcolor is called to set the color.  If the name is not
   found, a default color (black for pen and fill, white for bg) is
   substituted. */

#include "sys-defines.h"
#include "extern.h"
#include "g_colorname.h"

/* forward references */
static bool string_to_precise_color (const char *name, plColor *color_p);

/* The pencolor method, which is a GNU extension to libplot.  It sets a
   drawing attribute: the pen color (``foreground color'') of objects
   created in the drawing operations that follow.  The fill color may be
   set separately, by invoking fillcolor() and filltype().

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

int
_API_pencolor (R___(Plotter *_plotter) int red, int green, int blue)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "pencolor: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fgcolor.red;
      green = _default_drawstate.fgcolor.green;
      blue = _default_drawstate.fgcolor.blue;
    }

  if (_plotter->data->emulate_color)
    /* replace by grayscale approximation */
    red = green = blue = _grayscale_approx (red, green, blue);

  /* save our notion of foreground color */
  _plotter->drawstate->fgcolor.red = red;
  _plotter->drawstate->fgcolor.green = green;  
  _plotter->drawstate->fgcolor.blue = blue;

  return 0;
}

/* The fillcolor method, which is a GNU extension to libplot.  It sets a
   drawing attribute: the fill color of objects created in the following
   drawing operations.  Actually the true fill color (if filling is not
   disabled) will be a desaturated version of the user-specified fill
   color.  The desaturation level is set by invoking filltype().

   In principle we support 48-bit color (16 bits, i.e. 0x0000 through
   0xffff, for each of red, green, and blue). */

int
_API_fillcolor (R___(Plotter *_plotter) int red, int green, int blue)
{
  double red_d, green_d, blue_d;
  double desaturate;
  plColor new_rgb;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fillcolor: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.fillcolor.red;
      green = _default_drawstate.fillcolor.green;
      blue = _default_drawstate.fillcolor.blue;
    }

  if (_plotter->data->emulate_color)
    /* replace by grayscale approximation */
    red = green = blue = _grayscale_approx (red, green, blue);

  /* save the basic fillcolor (i.e. the base fillcolor, unaffected by the
     fill type) */
  _plotter->drawstate->fillcolor_base.red = red;
  _plotter->drawstate->fillcolor_base.green = green;  
  _plotter->drawstate->fillcolor_base.blue = blue;

  if (_plotter->drawstate->fill_type == 0)
    /* won't be doing filling, so stop right here */
    return 0;

  /* update fillcolor, taking fill type into account */

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

/* convenience function */

int
_API_color (R___(Plotter *_plotter) int red, int green, int blue)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "color: invalid operation");
      return -1;
    }

  _API_pencolor (R___(_plotter) red, green, blue);
  _API_fillcolor (R___(_plotter) red, green, blue);  

  return 0;
}

/* The bgcolor method, which is a GNU extension to libplot.  It sets a
   drawing attribute: the background color.

   We use the RGB color model.  In principle we support 48-bit color (16
   bits, i.e. 0x0000 through 0xffff, for each of red, green, and blue). */

int
_API_bgcolor (R___(Plotter *_plotter) int red, int green, int blue)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "bgcolor: invalid operation");
      return -1;
    }

  if ((red > 0xffff) || (green > 0xffff) || (blue > 0xffff))
    /* OOB switches to default */
    {
      red = _default_drawstate.bgcolor.red;
      green = _default_drawstate.bgcolor.green;
      blue = _default_drawstate.bgcolor.blue;
    }

  if (_plotter->data->emulate_color)
    /* replace by grayscale approximation */
    red = green = blue = _grayscale_approx (red, green, blue);

  /* save our notion of background color */
  _plotter->drawstate->bgcolor.red = red;
  _plotter->drawstate->bgcolor.green = green;  
  _plotter->drawstate->bgcolor.blue = blue;

  return 0;
}

/* compute a 16-bit grayscale approximation to a 48-bit RGB; optionally
   used by pencolorname, fillcolorname, bgcolorname methods */
int
_grayscale_approx (int red, int green, int blue)
{
  double gray;
  
  /* compute CIE luminance according to Rec. 709 */
  gray = 0.212671 * red + 0.715160 * green + 0.072169 * blue;
  return IROUND(gray);
}

/* Below are the pencolorname, fillcolorname, and bgcolorname methods,
   which are GNU extensions to libplot.  They search a database of known
   names (stored in g_colorname.h) for a specified color name.  If the name
   is found, its interpretation as a 48-bit RGB color is determined, and
   pencolor, fillcolor, or bgcolor is called to set the color.  If the name
   is not found, a default color (black for pen and fill, white for bg) is
   substituted.

   The lowest-level routine is _string_to_color(). */

int 
_API_pencolorname (R___(Plotter *_plotter) const char *name)
{
  plColor color;
  int intred, intgreen, intblue;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "pencolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  /* RGB values for default pen color */
  intred = _default_drawstate.fgcolor.red;
  intgreen = _default_drawstate.fgcolor.green;
  intblue = _default_drawstate.fgcolor.blue;

  if (_string_to_color (name, &color, _plotter->data->color_name_cache))
    {
      unsigned int red, green, blue;
      
      red = color.red;
      green = color.green;
      blue = color.blue;
      /* to convert from 24-bit to 48-bit color, double bytes */
      intred = (red << 8) | red;
      intgreen = (green << 8) | green;
      intblue = (blue << 8) | blue;
    }
  else if (_plotter->data->pen_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_pl_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"black\" for undefined pen color \"%s\"", 
	       name);
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->data->pen_color_warning_issued = true;
    }

  _API_pencolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

int 
_API_fillcolorname (R___(Plotter *_plotter) const char *name)
{
  plColor color;
  int intred, intgreen, intblue;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fillcolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  /* RGB values for default fill color */
  intred = _default_drawstate.fillcolor.red;
  intgreen = _default_drawstate.fillcolor.green;
  intblue = _default_drawstate.fillcolor.blue;

  if (_string_to_color (name, &color, _plotter->data->color_name_cache))
    {
      unsigned int red, green, blue;

      red = color.red;
      green = color.green;
      blue = color.blue;
      /* to convert from 24-bit to 48-bit color, double bytes */
      intred = (red << 8) | red;
      intgreen = (green << 8) | green;
      intblue = (blue << 8) | blue;
    }
  else if (_plotter->data->fill_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_pl_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"black\" for undefined fill color \"%s\"", 
	       name);
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->data->fill_color_warning_issued = true;
    }

  _API_fillcolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

/* convenience function */

int
_API_colorname (R___(Plotter *_plotter) const char *name)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "colorname: invalid operation");
      return -1;
    }

  _API_pencolorname (R___(_plotter) name);
  _API_fillcolorname (R___(_plotter) name);
  
  return 0;
}

int 
_API_bgcolorname (R___(Plotter *_plotter) const char *name)
{
  plColor color;
  int intred, intgreen, intblue;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "bgcolorname: invalid operation");
      return -1;
    }

  /* null pointer ignored */
  if (!name)
    return 0;

  if (strcmp (name, "none") == 0)
    /* turn off background (some Plotters can implement this) */
    {
      _plotter->drawstate->bgcolor_suppressed = true;
      /* treat as default, for benefit of Plotters that can't */
      name = "white";		
    }
  else
    _plotter->drawstate->bgcolor_suppressed = false;

  /* RGB values for default color [white, presumably] */
  intred = _default_drawstate.bgcolor.red;
  intgreen = _default_drawstate.bgcolor.green;
  intblue = _default_drawstate.bgcolor.blue;

  if (_string_to_color (name, &color, _plotter->data->color_name_cache))
    {
      unsigned int red, green, blue;

      red = color.red;
      green = color.green;
      blue = color.blue;
      /* to convert from 24-bit to 48-bit color, double bytes */
      intred = (red << 8) | red;
      intgreen = (green << 8) | green;
      intblue = (blue << 8) | blue;
    }
  else if (_plotter->data->bg_color_warning_issued == false)
    {
      char *buf;
		
      buf = (char *)_pl_xmalloc (strlen (name) + 100);
      sprintf (buf, "substituting \"white\" for undefined background color \"%s\"", 
	       name);
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->data->bg_color_warning_issued = true;
    }

  _API_bgcolor (R___(_plotter) intred, intgreen, intblue);

  return 0;
}

/* _string_to_color() searches a database of known color names, in
   g_colorname.h, for a specified string.  Matches are case-insensitive and
   ignore spaces.  The retrieved RGB components are returned via a pointer.

   We don't wish to search through the entire (long) color database.  (It
   contains 600+ color name strings.)  So any Plotter maintains a cache
   of previously found colors. */

bool
_string_to_color (const char *name, plColor *color_p, plColorNameCache *color_name_cache)
{
  plColor color;
  plCachedColorNameInfo **cached_colors_p;
  bool found = false;
  char *squeezed_name, *nptr;
  const plColorNameInfo *info, *found_info = NULL;
  const char *optr;
  plCachedColorNameInfo *cached_info;

  if (name == NULL)		/* avoid core dumps */
    return false;
  
  if (color_name_cache == NULL)	/* avoid core dumps */
    return false;

  /* first check whether string is of the form "#ffffff" */
  if (string_to_precise_color (name, &color))
    {
      *color_p = color;
      return true;
    }

  /* copy string, removing spaces */
  squeezed_name = (char *)_pl_xmalloc (strlen (name) + 1);
  optr = name, nptr = squeezed_name;
  while (*optr)
    {
      if (*optr == '\0')
	break;
      if (*optr != ' ')
	*nptr++ = *optr;
      optr++;
    }
  *nptr = '\0';

  /* Search our list of cached, previously used color names, doing string
     comparison.  If this were only for use by the X11 driver, we'd use
     XrmPermStringToQuark to get a faster-compared representation. */

  cached_colors_p = &color_name_cache->cached_colors;
  cached_info = *cached_colors_p;
  while (cached_info)
    {
      if (strcasecmp (cached_info->info->name, squeezed_name) == 0)
	{
	  found = true;
	  found_info = cached_info->info;
	  break;
	}
      cached_info = cached_info->next;
    }

  if (!found)
   /* not previously used, so search master colorname table (this is slower) */
    {
      info = _pl_g_colornames; /* start at head of list in g_colorname.h */
      while (info->name)
	{
	  if (strcasecmp (info->name, squeezed_name) == 0)
	    {
	      found = true;
	      found_info = info;
	      break;
	    }
	  info++;
	}

      if (found)
	/* copy to head of cached color list */
	{
	  plCachedColorNameInfo *old_cached_colors, *cached_colors;

	  old_cached_colors = *cached_colors_p;
	  cached_colors = 
	    (plCachedColorNameInfo *)_pl_xmalloc (sizeof (plCachedColorNameInfo));
	  cached_colors->next = old_cached_colors;
	  cached_colors->info = found_info;
	  *cached_colors_p = cached_colors;
	}
    }
  
  free (squeezed_name);
  if (found)
    {
      color_p->red = found_info->red;
      color_p->green = found_info->green;
      color_p->blue = found_info->blue;
    }

  return found;
}

/* Attempt to map a string to a 24-bit RGB; this will work if the string is
   of the form "#ffffff". */

static bool
string_to_precise_color (const char *name, plColor *color_p)
{
  const char *good_hex_digits = "0123456789abcdefABCDEF";
  int i, num_assigned;
  
  if (name == (const char *)NULL || *name != '#')
    return false;
  
  for (i = 1; i <= 8 ; i++)
    {
      bool found;
      const char *cp;
      
      if (name[i] == '\0')
	break;
      cp = good_hex_digits;
      found = false;
      while (*cp)
	{
	  if (name[i] == *cp)
	    {
	      found = true;
	      break;
	    }
	  cp++;
	}
      if (found == false)
	return false;
    }
  if (i != 7)
    return false;
  
  /* okay, have something like "#ffffff"; can now safely use scanf() */

  num_assigned = sscanf (name, "#%2x%2x%2x", 
			 &(color_p->red), &(color_p->green), &(color_p->blue));

  return (num_assigned == 3 ? true : false);
}


/* The cache of color names is currently implemented as a linked list. */

plColorNameCache *
_create_color_name_cache (void)
{
  plColorNameCache *new_cache;
  
  new_cache = (plColorNameCache *)_pl_xmalloc(sizeof(plColorNameCache));
  new_cache->cached_colors = NULL;
  return new_cache;
}

void 
_delete_color_name_cache (plColorNameCache *color_name_cache)
{
  plCachedColorNameInfo *colorptr;

  if (color_name_cache == (plColorNameCache *)NULL)
    return;

  colorptr = color_name_cache->cached_colors;
  while (colorptr != NULL)	/* free linked list */
    {
      plCachedColorNameInfo *next_colorptr;
      
      next_colorptr = colorptr->next;
      free (colorptr);
      colorptr = next_colorptr;
    }

  free (color_name_cache);	/* free structure itself */
}
