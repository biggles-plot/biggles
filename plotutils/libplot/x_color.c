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

/* This file contains device-specific color computation routines.  These
   routines are called by various XDrawablePlotter (and XPlotter)
   methods. */

#include "sys-defines.h"
#include "extern.h"

/* we call this routine to set the foreground color in the X GC used for
   drawing, only when needed (just before an object is written out) */

void
_pl_x_set_pen_color(S___(Plotter *_plotter))
{
  plColor old, new1;
  XColor rgb;

  new1 = _plotter->drawstate->fgcolor;
  old = _plotter->drawstate->x_current_fgcolor; /* i.e. as stored in gc */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && _plotter->drawstate->x_gc_fgcolor_status)
    /* can use current color cell */
    return;

  rgb.red = new1.red;
  rgb.green = new1.green;
  rgb.blue = new1.blue;

  /* retrieve matching color cell, if possible */
  if (_pl_x_retrieve_color (R___(_plotter) &rgb) == false)
    return;

  /* select pen color as foreground color in GC used for drawing */
  XSetForeground (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_gc_fgcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_gc_fgcolor_status = true;

  /* update non-opaque representation of stored foreground color */
  _plotter->drawstate->x_current_fgcolor = new1;
}

/* we call this routine to set the foreground color in the X GC used for
   filling, only when needed (just before an object is written out) */

void
_pl_x_set_fill_color(S___(Plotter *_plotter))
{
  plColor old, new1;
  XColor rgb;

  if (_plotter->drawstate->fill_type == 0) /* transparent */
    /* don't do anything, fill color will be ignored when writing objects*/
    return;

  new1 = _plotter->drawstate->fillcolor;
  old = _plotter->drawstate->x_current_fillcolor; /* as used in GC */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && _plotter->drawstate->x_gc_fillcolor_status)
    /* can use current color cell */
    return;

  rgb.red = (short)_plotter->drawstate->fillcolor.red;
  rgb.green = (short)_plotter->drawstate->fillcolor.green;
  rgb.blue = (short)_plotter->drawstate->fillcolor.blue;

  /* retrieve matching color cell, if possible */
  if (_pl_x_retrieve_color (R___(_plotter) &rgb) == false)
    return;

  /* select fill color as foreground color in GC used for filling */
  XSetForeground (_plotter->x_dpy, _plotter->drawstate->x_gc_fill, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_gc_fillcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_gc_fillcolor_status = true;

  /* update non-opaque representation of stored fill color */
  _plotter->drawstate->x_current_fillcolor = new1;
}

/* we call this routine to set the foreground color in the X GC used for
   erasing, only when needed (just before an erasure takes place) */

void
_pl_x_set_bg_color(S___(Plotter *_plotter))
{
  plColor old, new1;
  XColor rgb;

  new1 = _plotter->drawstate->bgcolor;
  old = _plotter->drawstate->x_current_bgcolor; /* i.e. as stored in gc */
  if (new1.red == old.red && new1.green == old.green && new1.blue == old.blue
      && _plotter->drawstate->x_gc_bgcolor_status)
    /* can use current color cell */
    return;

  rgb.red = new1.red;
  rgb.green = new1.green;
  rgb.blue = new1.blue;

  /* retrieve matching color cell, if possible */
  if (_pl_x_retrieve_color (R___(_plotter) &rgb) == false)
    return;

  /* select background color as foreground color in GC used for erasing */
  XSetForeground (_plotter->x_dpy, _plotter->drawstate->x_gc_bg, rgb.pixel);

  /* save the new pixel value */
  _plotter->drawstate->x_gc_bgcolor = rgb.pixel;

  /* flag this as a genuine pixel value */
  _plotter->drawstate->x_gc_bgcolor_status = true;

  /* update non-opaque representation of stored background color */
  _plotter->drawstate->x_current_bgcolor = new1;
}

/* This is the internal X color retrieval routine.  If the visual class
   is known and is TrueColor, it computes the X pixel value from a 48-bit
   RGB without invoking XAllocColor(), which would require a round trip
   to the server.

   Otherwise, it first searches for a specified RGB in a cache of
   previously retrieved color cells, and if that fails, tries to allocate a
   new color cell by calling XAllocColor().  If that fails, and a new
   colormap can be switched to, it switches to a new colormap and tries
   again.  If that attempt also fails, it searches the cache for the
   colorcell with an RGB that's closest to the specified RGB.  Only if that
   fails as well (i.e. the cache is empty), does it return false.

   Cache is maintained as a linked list (not optimal, but it facilitates
   color cell management; see comment in x_erase.c). */

bool 
_pl_x_retrieve_color (R___(Plotter *_plotter) XColor *rgb_ptr)
{
  plColorRecord *cptr;
  int rgb_red = rgb_ptr->red;
  int rgb_green = rgb_ptr->green;  
  int rgb_blue = rgb_ptr->blue;
  int xretval;

#ifdef LIBPLOTTER
  if (_plotter->x_visual && _plotter->x_visual->c_class == TrueColor)
#else
#ifdef __cplusplus
  if (_plotter->x_visual && _plotter->x_visual->c_class == TrueColor)
#else
  if (_plotter->x_visual && _plotter->x_visual->class == TrueColor)
#endif
#endif
    /* can compute pixel value from RGB without calling XAllocColor(), by
       bit-twiddling */
    {
      unsigned long red_mask, green_mask, blue_mask;
      int red_shift, green_shift, blue_shift;
      int red_bits, green_bits, blue_bits;

      /* first, compute {R,G,B}_bits and {R,G,B}_shift (should be precomputed) */

      red_mask = _plotter->x_visual->red_mask; red_shift = red_bits = 0;
      while (!(red_mask & 1))                                        
        {
	  red_mask >>= 1; red_shift++;
        }                                                               
      while (red_mask & 1)
        {
	  red_mask >>= 1; red_bits++;                                     
        }
      green_mask = _plotter->x_visual->green_mask; green_shift = green_bits = 0;
      while (!(green_mask & 1))                                        
        {
	  green_mask >>= 1; green_shift++;
        }                                                               
      while (green_mask & 1)
        {
	  green_mask >>= 1; green_bits++;                                     
        }
      blue_mask = _plotter->x_visual->blue_mask; blue_shift = blue_bits = 0;
      while (!(blue_mask & 1))                                        
        {
	  blue_mask >>= 1; blue_shift++;
        }                                                               
      while (blue_mask & 1)
        {
	  blue_mask >>= 1; blue_bits++;                                     
        }

      /* compute and pass back pixel, as a 32-bit unsigned long */
      rgb_red = rgb_red >> (16 - red_bits);
      rgb_green = rgb_green >> (16 - green_bits);
      rgb_blue = rgb_blue >> (16 - blue_bits);
      rgb_ptr->pixel = ((rgb_red << red_shift) & _plotter->x_visual->red_mask)
			 | ((rgb_green << green_shift) & _plotter->x_visual->green_mask)
			 | ((rgb_blue << blue_shift) & _plotter->x_visual->blue_mask);
#if 0
      fprintf (stderr, "pixel=0x%lx, R=0x%hx, G=0x%hx, B=0x%hx\n",
	       rgb_ptr->pixel, rgb_ptr->red, rgb_ptr->green, rgb_ptr->blue);
#endif

      return true;
    }

  /* If we got here, we weren't able to compute the pixel value from the
     RGB without calling XAllocColor().  So may have to do that, but first
     we consult a list of previously allocated color cells. */

  /* search cache list */
  for (cptr = _plotter->x_colorlist; cptr; cptr = cptr->next)
    {
      XColor cached_rgb;

      cached_rgb = cptr->rgb;
      if (cached_rgb.red == rgb_red
	  && cached_rgb.green == rgb_green
	  && cached_rgb.blue == rgb_blue)
	/* found in cache */
	{
	  /* keep track of page, frame number in which cell was most
	     recently accessed */
	  cptr->page_number = _plotter->data->page_number;
	  cptr->frame_number = _plotter->data->frame_number;
	  /* return stored pixel value */
	  *rgb_ptr = cached_rgb;
	  return true;
	}
    }

  /* not in cache, so try to allocate a new color cell, if colormap hasn't
     been flagged as bad (i.e. full) */
  if (_plotter->x_cmap_type != X_CMAP_BAD)
    {
      xretval = XAllocColor (_plotter->x_dpy, _plotter->x_cmap, rgb_ptr);

      if (xretval == 0)
	/* failure */
	{
	  if (_plotter->x_cmap_type == X_CMAP_ORIG)
	    /* colormap is the one we started with, so try switching and
	       reallocating */
	    {
	      /* Which method is invoked here depends on the type of
		 Plotter.  If this is an X Plotter, replace its colormap by
		 a copied, private colormap if we can; otherwise we flag
		 the colormap as bad (i.e. filled up).  If this is an
		 XDrawable Plotter, this method doesn't do anything, so
		 colormap just gets flagged as bad. */
	      _maybe_get_new_colormap (S___(_plotter));
	      if (_plotter->x_cmap_type != X_CMAP_NEW)
		_plotter->x_cmap_type = X_CMAP_BAD;
	  
	      if (_plotter->x_cmap_type != X_CMAP_BAD)
		/* got a new colormap; try again to allocate color cell */
		xretval = XAllocColor (_plotter->x_dpy, _plotter->x_cmap, rgb_ptr);
	    }
	}
    }
  else
    /* colormap is bad, i.e. full; no hope of allocating a new colorcell */
    xretval = 0;

  if (xretval == 0)
    /* allocation failed, and no switching or further switching of
       colormaps is possible; so simply search cache list for closest
       color, among previously allocated cells */
    {
      XColor cached_rgb;
      plColorRecord *best_cptr = NULL;
      double distance = DBL_MAX;

      /* flag colormap as bad, i.e. full; no further color cell allocations
         will be attempted */
      _plotter->x_cmap_type = X_CMAP_BAD;

      if (_plotter->x_colormap_warning_issued == false)
	{
	  _plotter->warning(R___(_plotter) 
			    "color supply exhausted, can't create new colors");
	  _plotter->x_colormap_warning_issued = true;
	}

      for (cptr = _plotter->x_colorlist; cptr; cptr = cptr->next)
	{
	  double newdistance;
	  
	  cached_rgb = cptr->rgb;
	  newdistance = (((rgb_red - cached_rgb.red) 
			  * (rgb_red - cached_rgb.red))
			 + ((rgb_green - cached_rgb.green) 
			    * (rgb_green - cached_rgb.green))
			 + ((rgb_blue - cached_rgb.blue) 
			    * (rgb_blue - cached_rgb.blue)));
	  if (newdistance < distance)
	    {
	      distance = newdistance;
	      best_cptr = cptr;
	    }
	}
	
      if (best_cptr != (plColorRecord *)NULL)
	{
	  /* keep track of page, frame number in which cell was most
	     recently accessed */
	  best_cptr->page_number = _plotter->data->page_number;
	  best_cptr->frame_number = _plotter->data->frame_number;
	  /* return pixel value via pointer */
	  *rgb_ptr = best_cptr->rgb;
	  return true;
	}
      else
	/* cache must be empty; bad news */
	return false;
    }

  else
    /* allocation succeeded, add new color cell to head of cache list */
    {
      cptr = (plColorRecord *)_pl_xmalloc (sizeof (plColorRecord));
      cptr->rgb = *rgb_ptr;
      /* include unquantized RGB values */
      cptr->rgb.red = rgb_red;
      cptr->rgb.green = rgb_green;
      cptr->rgb.blue = rgb_blue;
      cptr->allocated = true;	/* vestigial field */
      /* keep track of page, frame number in which cell was allocated */
      cptr->page_number = _plotter->data->page_number;
      cptr->frame_number = _plotter->data->frame_number;
      cptr->next = _plotter->x_colorlist;
      _plotter->x_colorlist = cptr;
#if 0
      fprintf (stderr, "pixel=0x%lx, R=0x%hx, G=0x%hx, B=0x%hx\n",
	       cptr->rgb.pixel, cptr->rgb.red, cptr->rgb.green, cptr->rgb.blue);
#endif
      return true;
    }
}
