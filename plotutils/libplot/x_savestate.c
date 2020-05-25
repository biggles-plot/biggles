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

/* This file contains push_state() and pop_state() for XDrawablePlotters
   (and XPlotters).  They supplement the generic behavior of savestate()
   and restorestate(), which create and destroy drawing states on the
   stack.  push_state() constructs X11 GC's (i.e. graphics contexts) for a
   new drawing state.  We copy into them, from each of the current GC's,
   all attributes we're interested in.  pop_state() tears down the GC's. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_x_push_state (S___(Plotter *_plotter))
{
  Drawable drawable;
  XGCValues gcv;
      
  /* create the X-specified drawing state elements that are pointers (e.g.,
     GC's or lists) */

  /* determine which if either drawable we'll construct the GC's for */
  if (_plotter->x_drawable1)
    drawable = _plotter->x_drawable1;
  else if (_plotter->x_drawable2)
    drawable = _plotter->x_drawable2;
  else
    drawable = (Drawable)NULL;

  if (drawable != (Drawable)NULL)
    /* prepare GC's for new drawing state, by copying attributes we use */
    {
      unsigned long gcmask_fg, gcmask_fill, gcmask_bg;

      gcmask_fg = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction
	/* drawing attributes set by _pl_x_set_attributes() */
	/* NOTE: we also use GCDashOffset and GCDashList, but Xlib does not
	   support retrieving the dash list from a GC, so we'll copy the
	   dashing style in another (painful) way */
	| GCLineStyle | GCLineWidth | GCJoinStyle | GCCapStyle
	/* other GC elements set by the X Drawable driver */
	| GCForeground | GCFont;
      
      gcmask_fill = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction | GCArcMode 
	/* filling attributes set by _pl_x_set_attributes() */
	| GCFillRule
	/* other GC elements set by the X Drawable driver */
	| GCForeground;
      
      gcmask_bg = 
	/* constant attributes (never altered) */
	GCPlaneMask | GCFunction 
	/* other GC elements set by the X Drawable driver */
	| GCForeground;
      
      /* copy from previous drawing state */

      /* copy GC used for drawing */
      XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fg, 
		    gcmask_fg, &gcv);
      _plotter->drawstate->x_gc_fg = XCreateGC (_plotter->x_dpy, drawable, 
						gcmask_fg, &gcv);
      if (gcv.line_style != LineSolid)
	/* copy dash style info from previous state */
	{
	  int i, dash_list_len;
	  char *dash_list;
	  
	  /* add dash style elements to GC used for drawing */
	  XSetDashes (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		      _plotter->drawstate->previous->x_gc_dash_offset, 
		      _plotter->drawstate->previous->x_gc_dash_list,
		      _plotter->drawstate->previous->x_gc_dash_list_len);
	  
	  /* add non-opaque dash style elements */
	  dash_list_len = 
	    _plotter->drawstate->previous->x_gc_dash_list_len;
	  dash_list = (char *)_pl_xmalloc (dash_list_len * sizeof(char));
	  for (i = 0; i < dash_list_len; i++)
	    dash_list[i] =
	      _plotter->drawstate->previous->x_gc_dash_list[i];
	  _plotter->drawstate->x_gc_dash_list = dash_list;
	  
	  /* these two were already added by the copy operation that took
	     place in _g_savestate(), but we'll add them again */
	  _plotter->drawstate->x_gc_dash_list_len = dash_list_len;
	  _plotter->drawstate->x_gc_dash_offset = 
	    _plotter->drawstate->previous->x_gc_dash_offset;
	}
      else
	{
	  _plotter->drawstate->x_gc_dash_list = (char *)NULL;
	  _plotter->drawstate->x_gc_dash_list_len = 0;
	  _plotter->drawstate->x_gc_dash_offset = 0;
	}
      
      /* copy GC used for filling */
      XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_fill, 
		    gcmask_fill, &gcv);
      _plotter->drawstate->x_gc_fill = XCreateGC (_plotter->x_dpy, drawable, 
						  gcmask_fill, &gcv);
      /* copy GC used for erasing */
      XGetGCValues (_plotter->x_dpy, _plotter->drawstate->previous->x_gc_bg, 
		    gcmask_bg, &gcv);
      _plotter->drawstate->x_gc_bg = XCreateGC (_plotter->x_dpy, drawable, 
						gcmask_bg, &gcv);
    }
}

void
_pl_x_pop_state (S___(Plotter *_plotter))
{
  /* N.B. we do _not_ free _plotter->drawstate->x_font_struct anywhere,
     when restorestate() is invoked on an X Drawable or X Plotter */

  /* Free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see _pl_x_push_state()). */
  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      /* free the dash list in the X11-specific part of the drawing state */
      if (_plotter->drawstate->x_gc_dash_list_len > 0
	  && _plotter->drawstate->x_gc_dash_list != (char *)NULL)
	free ((char *)_plotter->drawstate->x_gc_dash_list);

      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
    }
}
