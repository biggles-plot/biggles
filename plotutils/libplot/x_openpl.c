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

/* This implementation is for XDrawablePlotters.  It supports one or two
   drawables, which must be associated with the same display and have the
   same dimensions (width, height, depth).  A `drawable' is either a window
   or a pixmap. */

/* This file also contains the internal functions _pl_x_maybe_get_new_colormap
   and _pl_x_maybe_handle_x_events, which are no-ops.  However, they are
   virtual and are overridden in the derived XPlotter class, which both
   attempts to switch to a private colormap when color cells run out, and
   processes its own X events. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_x_begin_page (S___(Plotter *_plotter))
{
  Window root1, root2;
  int x, y;
  unsigned int border_width, depth1, depth2;
  unsigned int width1, height1, width2, height2;
  unsigned int width, height, depth;
  const char *double_buffer_s;

  if (_plotter->x_dpy == (Display *)NULL)
    /* pathological: user didn't set XDRAWABLE_DISPLAY parameter */
    {
      _plotter->error (R___(_plotter) "the Plotter cannot be opened, as the XDRAWABLE_DISPLAY parameter is null");
      return false;
    }

  /* find out how long polylines can get on this X display */
  _plotter->x_max_polyline_len = XMaxRequestSize(_plotter->x_dpy) / 2;

  /* determine dimensions of drawable(s) */
  if (_plotter->x_drawable1)
    XGetGeometry (_plotter->x_dpy, _plotter->x_drawable1,
		  &root1, &x, &y, &width1, &height1, &border_width, &depth1);
  if (_plotter->x_drawable2)
    XGetGeometry (_plotter->x_dpy, _plotter->x_drawable2,
		  &root2, &x, &y, &width2, &height2, &border_width, &depth2);
  
  if (_plotter->x_drawable1 && _plotter->x_drawable2)
    /* sanity check */
    {
      if (width1 != width2 || height1 != height2 
	  || depth1 != depth2 || root1 != root2)
	{
	  _plotter->error(R___(_plotter) "the Plotter cannot be opened, as the X drawables have unequal parameters");
	  return false;
	}
    }
  
  if (_plotter->x_drawable1)
    {
      width = width1;
      height = height1;
      depth = depth1;
    }
  else if (_plotter->x_drawable2)
    {
      width = width2;
      height = height2;
      depth = depth1;
    }
  else
  /* both are NULL, and we won't really be drawing, so these are irrelevant */
    {
      width = 1;
      height = 1;
      depth = 1;
    }

  _plotter->data->imin = 0;
  _plotter->data->imax = width - 1;
  /* note flipped-y convention for this device: for j, min > max */
  _plotter->data->jmin = height - 1;
  _plotter->data->jmax = 0;
  
  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* add X GC's to drawing state (which was constructed by openpl() before
     begin_page() was called), so we can at least fill with solid color */
  _pl_x_add_gcs_to_first_drawing_state (S___(_plotter));

  /* At this point, we don't clear the drawable(s) by filling them with the
     background color, which is what we would do here for an X Plotter (see
     y_openpl.c).  For an X DrawablePlotter, unlike an X Plotter, initial
     clearing is not appropriate.  However, if we're double buffering, we
     create an off-screen buffer and fill it with the color. */

  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      double_buffer_s = 
	(const char *)_get_plot_param (_plotter->data, "USE_DOUBLE_BUFFERING");
      if (strcmp (double_buffer_s, "yes") == 0
	  /* backward compatibility: "fast" now means the same as "yes" */
	  || strcmp (double_buffer_s, "fast") == 0)
	/* user requested double buffering, so do so `by hand': allocate
	   additional pixmap to serve as off-screen graphics buffer */
	{
	  _plotter->x_double_buffering = X_DBL_BUF_BY_HAND;
	  _plotter->x_drawable3
	    = XCreatePixmap(_plotter->x_dpy, 
			    /* this 2nd arg merely determines the screen*/
			    _plotter->x_drawable1 ? 
			    _plotter->x_drawable1 : _plotter->x_drawable2,
			    (unsigned int)width,
			    (unsigned int)height, 
			    (unsigned int)depth);

	  /* erase buffer by filling it with background color */
	  XFillRectangle (_plotter->x_dpy, _plotter->x_drawable3, 
			  _plotter->drawstate->x_gc_bg,
			  /* upper left corner */
			  0, 0,
			  (unsigned int)width, (unsigned int)height);
	}
    }

  /* Note: at this point the drawing state, which we added X GC's to, a few
     lines above, won't be ready for drawing graphics, since it won't
     contain an X font or meaningful line width.  To retrieve an X font and
     set the line width, user will need to invoke space() after openpl().  */

  return true;
}

/* Flesh out an XDrawable or X Plotter's first drawing state, by adding
   X11-specific elements: GC's or lists.  This is invoked by the
   corresponding begin_page() routines, and hence by openpl().
   
   As supplemented, the drawing state won't be fully ready for drawing
   graphics, since it won't contain a X font.  However, the the API
   function alabel(), before drawing a text string, invokes _set_font(),
   which in turns invokes the Plotter-specific function retrieve_font().
   And x_retrieve_font() does the job of retrieving an X font from the
   server and placing it in the drawing state. */

void
_pl_x_add_gcs_to_first_drawing_state (S___(Plotter *_plotter))
{
  Drawable drawable;
  
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
      
      /* build new GC's from scratch */
      {
	XGCValues gcv_fg, gcv_fill, gcv_bg;
	
	/* Initialize GC used for drawing.  (Always initialize the line
	   style to LineSolid, irrespective of what the default drawing
	   state contains; it would be silly for the default drawing state
	   to include a non-solid value for the line style.) */
	gcv_fg.plane_mask = AllPlanes;
	gcv_fg.function = GXcopy;
	gcv_fg.line_width = _default_drawstate.x_gc_line_width;
	gcv_fg.line_style = LineSolid;
	gcv_fg.join_style = _default_drawstate.x_gc_join_style;
	gcv_fg.cap_style = _default_drawstate.x_gc_cap_style;
	gcmask_fg &= ~(GCFont); /* initialized much later; see below */
	gcmask_fg &= ~(GCForeground);	/* color is initialized separately */
	
	/* initialize GC used for filling */
	gcv_fill.plane_mask = AllPlanes;
	gcv_fill.function = GXcopy;
	gcv_fill.arc_mode = ArcChord; /* libplot convention */
	gcv_fill.fill_rule = _default_drawstate.x_gc_fill_rule;
	gcmask_fill &= ~(GCForeground); /* color is initialized separately */
	  
	/* initialize GC used for erasing */
	gcv_bg.plane_mask = AllPlanes;
	gcv_bg.function = GXcopy;
	gcmask_bg &= ~(GCForeground); /* color is initialized separately */
	
	/* create the 3 GC's */
	_plotter->drawstate->x_gc_fg = 
	  XCreateGC (_plotter->x_dpy, drawable, gcmask_fg, &gcv_fg);
	_plotter->drawstate->x_gc_fill = 
	  XCreateGC (_plotter->x_dpy, drawable, gcmask_fill, &gcv_fill);
	_plotter->drawstate->x_gc_bg = 
	  XCreateGC (_plotter->x_dpy, drawable, gcmask_bg, &gcv_bg);
	
	/* set X-specific elements in the drawing state, specifying
	   (non-opaquely) what the 3 GC's contain */
	_plotter->drawstate->x_gc_line_width = gcv_fg.line_width;
	_plotter->drawstate->x_gc_line_style = gcv_fg.line_style;
	_plotter->drawstate->x_gc_join_style = gcv_fg.join_style;
	_plotter->drawstate->x_gc_cap_style = gcv_fg.cap_style;
	_plotter->drawstate->x_gc_dash_list = (char *)NULL;
	_plotter->drawstate->x_gc_dash_list_len = 0;
	_plotter->drawstate->x_gc_dash_offset = 0;
	_plotter->drawstate->x_gc_fill_rule = gcv_fill.fill_rule;
	
	/* do the separate initialization of color (i.e. GCForeground
	   element) in each GC */
	_pl_x_set_pen_color (S___(_plotter));
	_pl_x_set_fill_color (S___(_plotter));
	_pl_x_set_bg_color (S___(_plotter));
	
	/* At this point, all 3 GC's are functional, except the GC used
	   for drawing lacks a GCFont element.
	   
	   We do not retrieve a font from the X server here; not even a
	   default font.  fsetmatrix() or space(), when invoked (which we
	   require after each invocation of openpl()), will select a
	   default size for the font.  A font will be retrieved from the X
	   server only when fontname/fontsize/textangle is invoked to
	   select a different font, or when alabel/labelwidth is invoked
	   (see g_alabel.c).

	   The invocation of fsetmatrix() or space() will also set the line
	   width in the drawing state.  Any changed attributes, such as
	   line width, will be written to the GC's just before drawing; see
	   g_attribs.c. */
      }
    }
}

/* This is the XDrawablePlotter-specific version of the
   _maybe_get_new_colormap() method, which is invoked when a Plotter's
   original colormap fills up.  It's a no-op; in XPlotters, it's overridden
   by a version that actually does something. */
void
_pl_x_maybe_get_new_colormap (S___(Plotter *_plotter))
{
  return;
}  

/* This is the XDrawablePlotter-specific version of the
   _maybe_handle_x_events() method, which is invoked after most drawing
   operations.  It's a no-op; in XPlotters, it's overridden by a version
   that actually does something. */
void
_pl_x_maybe_handle_x_events(S___(Plotter *_plotter))
{
  return;
}
