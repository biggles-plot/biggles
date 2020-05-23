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

/* This implementation is for XPlotters.  When invoked, it pops up a
   plotting window on the default screen of the specified X display.  When
   the corresponding closepl method is invoked, the window is `spun off',
   i.e., is managed thenceforth by a forked-off child process. */

/* This file also contains the internal functions _pl_y_maybe_get_new_colormap
   and _pl_y_maybe_handle_x_events.  They override the corresponding functions
   in the XDrawablePlotter superclass, which are no-ops.

   The function _pl_y_maybe_handle_x_events is very important: it contains our
   hand-crafted loop for processing X events, which is called by an
   XPlotter after any libplot drawing operation is invoked on it. */

#include "sys-defines.h"
#include "extern.h"

/* song and dance to define struct timeval, and declare select() */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>		/* for struct timeval */
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		/* AIX needs this */
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for struct fdset, FD_ZERO, FD_SET */
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>		/* for select() */
#endif

/* Mutex for locking _xplotters[] and _xplotters_len.  Defined in
   y_defplot.c. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
extern pthread_mutex_t _xplotters_mutex;
#endif
#endif

/* fake app name, effectively our argv[0] */
#define XPLOT_APP_NAME "xplot"

/* app class, use for specifying resources */
#define XPLOT_APP_CLASS "Xplot"

/* Fallback resources for the preceding X11 class.  There are no
   user-specifiable X resources, except for the geometry.

   The default size of the plotting window is set here, as a default X
   resource, rather than in y_defplot.c.  Users may override the default by
   specifying a geometry in their .Xdefaults files (by specifying the
   Xplot.geometry or xplot.geometry resource).  This is equivalent to
   specifying the Plotter parameter BITMAPSIZE. */

static const String _xplot_fallback_resources[] = 
{
  (String)"Xplot*geometry:      570x570",
  (String)NULL 
};

/* Support for command-line mimicry.  Our fake argument vector,
   _fake_argv[], needs space for our fake application name,
   i.e. XPLOT_APP_NAME, the three options "-display", "-geometry", "-bg",
   each with an argument, and a final NULL. */
#define MAX_FAKE_ARGV_LENGTH 8

/* Translations for the canvas widget, before and after the Plotter is
   closed, i.e. before and after forking.  (After forking, translate any
   pressing of the `q' key, and any mouse click, to `Foldup'.) */
static const String _xplot_translations_before_forking =
#ifdef USE_MOTIF
(String)"<Btn2Down>:	ProcessDrag()";
#else
(String)"";
#endif

static const String _xplot_translations_after_forking =
#ifdef USE_MOTIF
(String)"<Btn1Down>:	Foldup()\n\
 <Btn2Down>:	ProcessDrag()\n\
 <Btn3Down>:	Foldup()\n\
 <Key>Q:	Foldup()\n\
 <Key>q:	Foldup()";
#else
(String)"<Btn1Down>:	Foldup()\n\
 <Btn3Down>:	Foldup()\n\
 <Key>Q:	Foldup()\n\
 <Key>q:	Foldup()";
#endif

/* forward references */
static bool _bitmap_size_ok (const char *bitmap_size_s);
static void Foldup (Widget widget, XEvent *event, String *params, Cardinal *num_params);

#ifndef HAVE_STRERROR
static char * _plot_strerror (int errnum);
#define strerror _plot_strerror
#endif

/* This is called by the child process produced in y_closepl.c, immediately
   after forking takes place.  It alters the translation table for the
   canvas widget so that Foldup() will be invoked when `q' is pressed or a
   mouse click is seen. */
void
_pl_y_set_data_for_quitting (S___(Plotter *_plotter))
{
  Arg wargs[1];		/* a lone werewolf */

#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNtranslations, 
	    XtParseTranslationTable(_xplot_translations_after_forking));
#else
  XtSetArg (wargs[0], XtNtranslations, 
	    XtParseTranslationTable(_xplot_translations_after_forking));
#endif
  XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);
}

/* Foldup() is called by the Label widget when `q' is pressed or a mouse
   click is seen, provided that closepl() has previously been invoked.  In
   that case the spun-off window disappears (we destroy the parent widget,
   and being a forked-off child process managing it, we exit). */

static void			
Foldup (Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
  Display *dpy;
      
  dpy = XtDisplay (widget);
  XtDestroyWidget (XtParent (widget)); /* destroy toplevel widget */
  XFlush (dpy);		/* flush X output buffer */
  exit (EXIT_SUCCESS);
}

/* Application context-specific action table. */
static const XtActionsRec _xplot_actions[] = 
{
  {(String)"Foldup",	Foldup},
};

bool
_pl_y_begin_page (S___(Plotter *_plotter))
{
  Arg wargs[10];		/* werewolves */
  Dimension window_height, window_width;
  Screen *screen_struct;	/* screen structure */
  String fake_argv[MAX_FAKE_ARGV_LENGTH];
  const char *double_buffer_s;
  int fake_argc;
  int screen;			/* screen number */
  
  /* To permit openpl..closepl to be invoked repeatedly, we don't use the
     convenience routine XtAppInitialize(), since that function starts out
     by calling XtToolkitInitialize(), which shouldn't be called more than
     once.  (At least, in early versions of X11; in X11R6 calling it more
     than once is OK.)  Instead, we call XtToolkitInitialize() when the
     first XPlotter is created; see y_defplot.c.

     On every invocation of openpl() we call the other four functions that
     XtAppInitialize would call: XtCreateApplicationContext,
     XtAppSetFallbackResources, XtOpenDisplay, and XtAppCreateShell.  That
     sets up a new application context each time openpl() is called, which
     looks wasteful.  But since each openpl..closepl will yield a window
     managed by a forked-off process, it's appropriate. */

  /* create new application context for this Plotter page */
  _plotter->y_app_con = XtCreateApplicationContext();
  if (_plotter->y_app_con == (XtAppContext)NULL)
    {
      _plotter->error (R___(_plotter) "an X application context could not be created");
      return false;
    }
  /* set fallback resources to be used by canvas widget (currently, only
     the window size); specific to application context */
  XtAppSetFallbackResources (_plotter->y_app_con, 
			     (String *)_xplot_fallback_resources);

  /* register an action table [currently containing only
     "Foldup"->Foldup(), see above]; specific to application context */
  XtAppAddActions (_plotter->y_app_con, (XtActionsRec *)_xplot_actions,
		   XtNumber (_xplot_actions));
  
  /* punch options and parameters into our fake command line, beginning
     with the fake app name */
  fake_argc = 0;
  fake_argv[fake_argc++] = (String)XPLOT_APP_NAME;

  /* take argument of the "-display" option from the DISPLAY parameter */
  {
    const char *display_s;
	
    display_s = (char *)_get_plot_param (_plotter->data, "DISPLAY");
    if (display_s == NULL || *display_s == '\0')
      {
	_plotter->error (R___(_plotter)
			 "the Plotter could not be opened, as the DISPLAY parameter is null");
	return false;
      }
    fake_argv[fake_argc++] = (String)"-display";
    fake_argv[fake_argc++] = (String)display_s;
  }
  
  /* Take argument of "-geometry" option from BITMAPSIZE parameter, if set;
     otherwise size will be taken from Xplot.geometry.  Fallback size is
     specified at head of this file. */
  {
    char *bitmap_size_s;
	
    bitmap_size_s = (char *)_get_plot_param (_plotter->data, "BITMAPSIZE");
    if (bitmap_size_s && _bitmap_size_ok (bitmap_size_s))
      {
	fake_argv[fake_argc++] = (String)"-geometry";
	fake_argv[fake_argc++] = (String)bitmap_size_s;
      }
  }

  /* Take argument of "-bg" option from BG_COLOR parameter, if set;
     otherwise use default color (white). */
  {
    const char *bg_color_s;
	
    bg_color_s = (char *)_get_plot_param (_plotter->data, "BG_COLOR");
    if (bg_color_s)
      {
	plColor color;
	char rgb[8];		/* enough room for "#FFFFFF", incl. NUL */

	if (_string_to_color (bg_color_s, &color, _plotter->data->color_name_cache))
	  /* color is in our database */
	  {
	    if (_plotter->data->emulate_color)
	      /* replace by grayscale approximation */
	      {
		int gray;

		gray = _grayscale_approx (color.red, color.green, color.blue);
		sprintf (rgb, "#%02X%02X%02X", gray, gray, gray);
	      }
	    else
	      sprintf (rgb, "#%02X%02X%02X",
		       color.red, color.green, color.blue);
	    bg_color_s = rgb;
	  }
	else
	  /* color is not in our database */
	  {
	    if (_plotter->x_bg_color_warning_issued == false)
	      {
		char *buf;
		
		buf = (char *)_pl_xmalloc (strlen (bg_color_s) + 100);
		sprintf (buf, "substituting \"white\" for undefined background color \"%s\"", 
			 bg_color_s);
		_plotter->warning (R___(_plotter) buf);
		free (buf);
		_plotter->x_bg_color_warning_issued = true;
		
		bg_color_s = "white";
	      }
	  }

	fake_argv[fake_argc++] = (String)"-bg";
	fake_argv[fake_argc++] = (String)bg_color_s;
      }
  }

  /* append final NULL (some X implementations need this) */
  fake_argv[fake_argc] = (String)NULL;

  /* open new connection to the X display, using fake argv */
  _plotter->x_dpy = 
    XtOpenDisplay (_plotter->y_app_con,
		   /* display_string = NULL, so take from fake commandline */
		   (String)NULL, 
		   /* application name = NULL, so take from fake commandline */
		   (String)NULL,	
		   /* application class */
		   (String)XPLOT_APP_CLASS, 
		   /* application-specific commandline parsetable (for
		      XrmParseCommand), used in setting display resources */
		   NULL, (Cardinal)0, 
		   /* pass fake command-line (contains a fake argv[0] to
		      specify app name, and besides "-display", options
		      may include "-geometry", "-bg") */
		   &fake_argc, fake_argv);
  if (_plotter->x_dpy == (Display *)NULL)
    {
      char *display_s;

      display_s = (char *)_get_plot_param (_plotter->data, "DISPLAY");
      if (display_s == NULL)	/* shouldn't happen */
	_plotter->error (R___(_plotter)
			 "the X Window System display could not be opened, as it is null");
      else
	{
	  char *buf;

	  buf = (char *)_pl_xmalloc(strlen(display_s) + 1 + 50);
	  sprintf (buf, "the X Window System display \"%s\" could not be opened", 
		   display_s);
	  _plotter->error (R___(_plotter) buf);
	  free (buf);
	}
      return false;
    }
  
  /* display was opened, so determine its default screen, visual, colormap */
  screen = DefaultScreen (_plotter->x_dpy);
  screen_struct = ScreenOfDisplay (_plotter->x_dpy, screen);
  _plotter->x_visual = DefaultVisualOfScreen (screen_struct);
  _plotter->x_cmap = DefaultColormapOfScreen (screen_struct);
  _plotter->x_cmap_type = X_CMAP_ORIG; /* original cmap (not a private one) */
  
  /* find out how long polylines can get on this X display */
  _plotter->x_max_polyline_len = XMaxRequestSize(_plotter->x_dpy) / 2;
  
  /* For every invocation of openpl(), we create a toplevel Shell widget,
     associated with default screen of the opened display.  (N.B. could
     vary name of app instance; also select a non-default colormap by
     setting a value for XtNcolormap.) */
  XtSetArg(wargs[0], XtNscreen, screen_struct);
  XtSetArg(wargs[1], XtNargc, fake_argc);
  XtSetArg(wargs[2], XtNargv, fake_argv);
  _plotter->y_toplevel = XtAppCreateShell(NULL, /* name of app instance */
			     (String)XPLOT_APP_CLASS, /* app class */
			     applicationShellWidgetClass, 
			     _plotter->x_dpy, /* x_dpy to get resources from */
			     /* pass XtNscreen resource, and also fake
				command-line, to get resources from
				(options may include "-display"
				[redundant], and "-geometry", "-bg") */
			     wargs, (Cardinal)3); 

  /* Create drawing canvas (a Label widget) as child of toplevel Shell
     widget.  Set many obscure spacing parameters to zero, so that origin
     of bitmap will coincide with upper left corner of window. */
#ifdef USE_MOTIF
  XtSetArg(wargs[0], XmNmarginHeight, (Dimension)0);
  XtSetArg(wargs[1], XmNmarginWidth, (Dimension)0);
  XtSetArg(wargs[2], XmNmarginLeft, (Dimension)0);
  XtSetArg(wargs[3], XmNmarginRight, (Dimension)0);
  XtSetArg(wargs[4], XmNmarginTop, (Dimension)0);
  XtSetArg(wargs[5], XmNmarginBottom, (Dimension)0);
  XtSetArg(wargs[6], XmNshadowThickness, (Dimension)0);
  XtSetArg(wargs[7], XmNhighlightThickness, (Dimension)0);
  _plotter->y_canvas = XtCreateManagedWidget ((String)"", xmLabelWidgetClass,
					    _plotter->y_toplevel, 
					    wargs, (Cardinal)8);
#else  
  XtSetArg(wargs[0], XtNinternalHeight, (Dimension)0);
  XtSetArg(wargs[1], XtNinternalWidth, (Dimension)0);
  _plotter->y_canvas = XtCreateManagedWidget ((String)"", labelWidgetClass,
					    _plotter->y_toplevel, 
					    wargs, (Cardinal)2);
#endif
  
  /* realize both widgets */
  XtRealizeWidget (_plotter->y_toplevel);
  
  /* replace the Label widget's default translations by ours [see above;
     our default is no translations at all, with a nod to Motif] */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNtranslations, 
	    XtParseTranslationTable(_xplot_translations_before_forking));
#else
  XtSetArg (wargs[0], XtNtranslations, 
	    XtParseTranslationTable(_xplot_translations_before_forking));
#endif
  XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);

  /* get Label widget's window; store it in Plotter struct as
     `drawable #2' */
  _plotter->x_drawable2 = (Drawable)XtWindow(_plotter->y_canvas);

  /* get the window size that was actually chosen, store it */
#ifdef USE_MOTIF
  XtSetArg (wargs[0], XmNwidth, &window_width);
  XtSetArg (wargs[1], XmNheight, &window_height);
#else
  XtSetArg (wargs[0], XtNwidth, &window_width);
  XtSetArg (wargs[1], XtNheight, &window_height);
#endif
  XtGetValues (_plotter->y_canvas, wargs, (Cardinal)2);
  _plotter->data->imin = 0;
  _plotter->data->imax = (int)window_width - 1;
  /* note flipped-y convention for this device: min > max */
  _plotter->data->jmin = (int)window_height - 1;
  _plotter->data->jmax = 0;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* request backing store for Label widget's window */
  if (DoesBackingStore(screen_struct))
    {
      XSetWindowAttributes attributes;
      unsigned long value_mask;

      attributes.backing_store = Always;
      value_mask = CWBackingStore;
      XChangeWindowAttributes (_plotter->x_dpy, (Window)_plotter->x_drawable2, 
			       value_mask, &attributes);
    }

  /* determine whether to use double buffering */
  _plotter->x_double_buffering = X_DBL_BUF_NONE;
  double_buffer_s = (const char *)_get_plot_param (_plotter->data, 
						   "USE_DOUBLE_BUFFERING");

  /* backward compatibility: "fast" now means the same as "yes" */
  if (strcmp (double_buffer_s, "fast") == 0)
    double_buffer_s = "yes";

#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#ifdef HAVE_DBE_SUPPORT
  if (strcmp (double_buffer_s, "yes") == 0)
    /* check whether X server supports DBE extension */
    {
      int major_version, minor_version;
      int one = 1;		/* number of screens to look at */
      XdbeScreenVisualInfo *sv_info;
      
      if (XdbeQueryExtension (_plotter->x_dpy, &major_version, &minor_version)
	  && (sv_info = XdbeGetVisualInfo (_plotter->x_dpy, 
					   /* 2nd arg specifies screen */
					   &_plotter->x_drawable2, 
					   &one)) != NULL)
	/* server supports DBE extension; for screen, a list of
	   visuals / depths / performance hints was returned */
	{
	  bool ok = false;
	  int i, num_visuals = sv_info->count;
	  XdbeVisualInfo *vis_info = sv_info->visinfo;
	  VisualID visual_id = XVisualIDFromVisual (_plotter->x_visual);

	  /* See whether default visual supports double buffering.  If not,
	     could invoke XGetVisualInfo() to check the depth and perflevel
	     of each visual that does, and select the `best' one.  (Would
	     also need to call XCreateColormap() to create a colormap of
	     that visual type.  When using the default visual we can use
	     the default colormap, but when not, we don't have that
	     luxury.)
	     
	     Maybe someday... That enhancement would be important for Xsgi,
	     which typically has a default 8-plane PseudoColor visual that
	     does _not_ support double buffering, and various other
	     visuals, including some 8-plane and 12-plane ones that do (and
	     some 15-plane and 24-plane ones that don't). */

	  for (i = 0; i < num_visuals; i++)
	    /* check visual ID for each visual in list */
	    if (vis_info[i].visual == visual_id) /* matches the default */
	      {
		ok = true;	/* default visual is OK */
		break;
	      }
	  XdbeFreeVisualInfo (sv_info);
	  if (ok)
	    /* allocate back buffer, to serve as our graphics buffer;
	       save it as `x_drawable3' */
	    {
	      _plotter->x_drawable3 = 
		XdbeAllocateBackBufferName (_plotter->x_dpy,
					    _plotter->x_drawable2, 
					    (XdbeSwapAction)XdbeUndefined);
	      /* set double buffering type in Plotter structure */
	      _plotter->x_double_buffering = X_DBL_BUF_DBE;
	    }
	}
    }
#endif /* HAVE_DBE_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_XDBE_H */

#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#ifdef HAVE_MBX_SUPPORT
  if (_plotter->x_double_buffering == X_DBL_BUF_NONE
      && strcmp (double_buffer_s, "yes") == 0)
    /* check whether X server supports the (obsolete) MBX extension, as a
       substitute for DBE */
    {
      int event_base, error_base;
      int major_version, minor_version;
      
      if (XmbufQueryExtension (_plotter->x_dpy, &event_base, &error_base)
	  && XmbufGetVersion (_plotter->x_dpy, &major_version, &minor_version))
	/* server supports MBX extension */
	{
	  Multibuffer multibuf[2];
	  int num;
	  
	  num = XmbufCreateBuffers (_plotter->x_dpy, 
				    (Window)_plotter->x_drawable2, 2, 
				    MultibufferUpdateActionUndefined,
				    MultibufferUpdateHintFrequent,
				    multibuf);
	  if (num == 2)
	    /* Yow, got a pair of multibuffers.  We'll write graphics to
	       the first (`x_drawable3'), and interchange them on each
	       erase().  See y_erase.c. */
	    {
	      _plotter->x_drawable3 = multibuf[0];
	      _plotter->y_drawable4 = multibuf[1];	      
	      /* set double buffering type in Plotter structure */
	      _plotter->x_double_buffering = X_DBL_BUF_MBX;
	    }
	  else
	    _plotter->warning (R___(_plotter) 
			       "X server refuses to support multibuffering");
	}
    }
#endif /* HAVE_MBX_SUPPORT */
#endif /* HAVE_X11_EXTENSIONS_MULTIBUF_H */

  if (_plotter->x_double_buffering == X_DBL_BUF_NONE)
    /* user didn't request double buffering, or did but special support for
       double buffering isn't contained in the X server */
    {
      Pixmap bg_pixmap;

      /* create background pixmap for Label widget; 2nd arg (window) is
         only used for determining the screen */
      bg_pixmap = XCreatePixmap(_plotter->x_dpy, 
				_plotter->x_drawable2,
				(unsigned int)window_width, 
				(unsigned int)window_height, 
				(unsigned int)PlanesOfScreen(screen_struct));
      /* If user requested double buffering but the server doesn't support
	 it, we'll double buffer `by hand', and this pixmap will be the one
	 (of two) into which we'll draw.  If user didn't request double
	 buffering, we'll use it as the 2nd of two drawables into which
	 we'll draw, the other being the window. */
      if (strcmp (double_buffer_s, "yes") == 0)
	{
	  _plotter->x_drawable3 = (Drawable)bg_pixmap;
	  _plotter->x_double_buffering = X_DBL_BUF_BY_HAND;
	}
      else
	{
	  _plotter->x_drawable1 = (Drawable)bg_pixmap;
	  _plotter->x_double_buffering = X_DBL_BUF_NONE;
	}
    }

  /* add X GC's to drawing state (which was constructed by openpl() before
     begin_page() was called), so we can at least fill with solid color */
  _pl_x_add_gcs_to_first_drawing_state (S___(_plotter));

  /* If not double-buffering, clear both pixmap and window by filling them
     with the drawing state's background color, via XFillRectangle.  If
     double buffering, do something similar (see y_erase.c). */
  _pl_y_erase_page (S___(_plotter));
  
  /* If double buffering, must invoke `erase' one more time to clear both
     graphics buffer and window, since what `erase' does in that case is
     (1) copy the graphics buffer to window, and (2) clear the graphics
     buffer. */
  if (_plotter->x_double_buffering != X_DBL_BUF_NONE) 
    _pl_y_erase_page (S___(_plotter));

  if (_plotter->x_double_buffering == X_DBL_BUF_NONE
      || _plotter->x_double_buffering == X_DBL_BUF_BY_HAND)
    /* have a pixmap, so install it as Label widget's background pixmap */
    {
      Pixmap bg_pixmap;
      
      bg_pixmap = ((_plotter->x_double_buffering == X_DBL_BUF_BY_HAND) ? 
		   _plotter->x_drawable3 : _plotter->x_drawable1);
#ifdef USE_MOTIF
      XtSetArg (wargs[0], XmNlabelPixmap, bg_pixmap);
      XtSetArg (wargs[1], XmNlabelType, XmPIXMAP);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)2);
#else
      XtSetArg (wargs[0], XtNbitmap, bg_pixmap);
      XtSetValues (_plotter->y_canvas, wargs, (Cardinal)1);
#endif
    }

  /* do an XSync on the display (this will cause the background color to
   show up if it hasn't already) */
  _pl_x_flush_output (S___(_plotter));

  /* Note: at this point the drawing state, which we added X GC's to, a few
     lines above, won't be ready for drawing graphics, since it won't
     contain an X font or meaningful line width.  To retrieve an X font and
     set the line width, user will need to invoke space() after openpl().  */

  return true;
}

static bool 
_bitmap_size_ok (const char *bitmap_size_s)
{
  int width, height;
  
  if (bitmap_size_s
      /* should parse this better */
      && (sscanf (bitmap_size_s, "%dx%d", &width, &height) == 2)
      && (width > 0) && (height > 0))
    return true;
  else
    return false;
}

/* This is the XPlotter-specific version of the _maybe_get_new_colormap()
   method, which is invoked when a Plotter's original colormap fills up.
   It overrides the XDrawable-specific version, which is a no-op. */
void
_pl_y_maybe_get_new_colormap (S___(Plotter *_plotter))
{
  Colormap new_pl_x_cmap;
  
  /* sanity check */
  if (_plotter->x_cmap_type != X_CMAP_ORIG)
    return;

  _plotter->warning (R___(_plotter) 
		     "color supply low, switching to private colormap");
  new_pl_x_cmap = XCopyColormapAndFree (_plotter->x_dpy, _plotter->x_cmap);

  if (new_pl_x_cmap == 0)
    /* couldn't create colormap */
    {
      _plotter->warning (R___(_plotter) 
			 "unable to create private colormap");
      _plotter->warning (R___(_plotter) 
			 "color supply exhausted, can't create new colors");
      _plotter->x_colormap_warning_issued = true;
    }
  else
    /* got a new colormap */
    {
      Arg wargs[1];		/* a lone werewolf */

      /* place in Plotter, flag as new */
      _plotter->x_cmap = new_pl_x_cmap;
      _plotter->x_cmap_type = X_CMAP_NEW;

      /* switch to it: install in y_toplevel shell widget */
      XtSetArg (wargs[0], XtNcolormap, _plotter->x_cmap);
      XtSetValues (_plotter->y_toplevel, wargs, (Cardinal)1);
    }
  
  return;
}

/* This is the XPlotter-specific version of the _maybe_handle_x_events()
   method, which is invoked at the end of most XDrawablePlotter drawing
   operations.  It overrides the XDrawablePlotter-specific version, which
   is a no-op.  It does two things.

   1. Provided an XPlotter's X_AUTO_FLUSH parameter is "yes" (the default),
      it invokes XFlush() to flush the X output buffer.  This makes most
      drawing operations more or less unbuffered: as the libplot functions
      are invoked, the graphics are sent to the X display.

   2. It scans through the _xplotters[] sparse array, which contains
      pointers to all currently existing XPlotters, and processes pending
      X events associated with any of their application contexts.

   Why do we do #2?  Once closepl() has been invoked on an XPlotter, the
   window popped up by openpl() is managed by a forked-off process via
   XtAppMainLoop().  But until that time, we must process events manually.
   (To speed up drawing, we perform #2 only once per
   X_EVENT_HANDLING_PERIOD invocations of this function.)

   #2 is accomplished by an hand-crafted event loop, the heart of which is
   the line

       if (XtAppPending (_xplotters[i]->y_app_con))
         XtAppProcessEvent (_xplotters[i]->y_app_con, XtIMAll);

   which, for Plotter number i, flushes the X output buffer, checks for
   events and processes them.  This line is executed as many times as we
   think safe.  Thereby hangs a tale.

   Nathan Salwen <salwen@physics.harvard.edu> has discovered that before
   invoking XtAppPending(), we should really check, using Xlib calls and
   select(), whether there are events waiting (either in the libX11 input
   buffer, or on the network socket).  The reason is that if no events are
   available, XtAppPending() may block, at least on some systems.  This
   does not agree with Xt documentation, but happens nonetheless.  And it
   is not what we want.

   The reason for XtAppPending's unfortunate behavior is apparently the
   following.  

   XtAppPending() invokes the Xlib function XEventsQueued(), first with
   mode=QueuedAfterReading [which returns the number of events in the input
   queue if nonempty; if empty, tries to extract more events from the
   socket] and then with mode=QueuedAfterFlush [which flushes the output
   buffer with XFlush() and checks if there is anything in the input queue;
   if not, it tries to extract more events from the socket].  (N.B. If,
   alternatively, it used mode=QueuedAlready, it would look only at the
   number of events in the input queue.)  And sadly, when trying to extract
   events from the socket, XEventsQueued() calls select() in such a way
   that it can block.

   So before invoking XtAppPending() we call select() ourselves to check
   whether data is available on the network socket, and we don't allow it
   to block.  We invoke XtAppPending and XtAppProcessEvent only if we're
   absolutely sure they won't block.

   Thanks also to Massimo Santini <santini@dsi.unimi.it> for helping to
   clear up the problem. */

#define X_EVENT_HANDLING_PERIOD 4

void
_pl_y_maybe_handle_x_events(S___(Plotter *_plotter))
{
  if (_plotter->y_auto_flush)
  /* Flush output buffer if we're *not* in the middle of constructing a
     path, or if we are, but the path will be drawn with a solid,
     zero-width pen.  Latter is for consistency with our convention that
     solid, zero-width paths should appear on the display as they're drawn
     (see x_cont.c). */
    {
      if (_plotter->drawstate->path == (plPath *)NULL
	  || (_plotter->drawstate->line_type == PL_L_SOLID
	      && !_plotter->drawstate->dash_array_in_effect
	      && _plotter->drawstate->points_are_connected
	      && _plotter->drawstate->quantized_device_line_width == 0))
	XFlush (_plotter->x_dpy);
    }
      
  if (_plotter->y_event_handler_count % X_EVENT_HANDLING_PERIOD == 0)
    /* process all XPlotters' events, if any are available */
    {
      int i;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_lock (&_xplotters_mutex);
#endif
#endif

  /* loop over XPlotters */
  for (i = 0; i < _xplotters_len; i++)
    {
      if (_xplotters[i] != NULL
	  && _xplotters[i]->data->opened /* paranoia */
	  && _xplotters[i]->data->open
	  && _xplotters[i]->y_app_con != NULL) /* paranoia */
	/* XPlotter is open */
	{
	  /* Our handcrafted event handling loop.  Check for pending X
	     events, either in the libX11 input queue or on the network
	     socket itself, and pull them off and process them one by one,
	     trying very hard not to generate a call to select() that would
	     block.  We loop until no more events are available. */
	  for ( ; ; )
	    {
	      bool have_data;
	  
	      have_data = false; /* default */

	      if (QLength(_xplotters[i]->x_dpy) > 0)
		/* one or more events has already been pulled off the
		   socket and are in the libX11 input queue; so we can
		   safely invoke XtAppPending(), and it will return `true' */
		have_data = true;
	      
	      else
		/* libX11 input queue is empty, so check whether data is
		   available on the socket by doing a non-blocking select() */
		{
		  int connection_number;
		  int maxfds, select_return;
		  fd_set readfds;
		  struct timeval timeout;
		  
		  timeout.tv_sec = 0; /* make select() non-blocking! */
		  timeout.tv_usec = 0;
		  
		  connection_number = 
		    ConnectionNumber(_xplotters[i]->x_dpy);
		  maxfds = 1 + connection_number;
		  FD_ZERO (&readfds);
		  FD_SET (connection_number, &readfds);
		  select_return = 
		    select (maxfds, &readfds, NULL, NULL, &timeout);
		  
		  if (select_return < 0 && errno != EINTR)
		    {
		      _plotter->error (R___(_plotter) strerror (errno));
		      break;	/* on to next Plotter */
		    }
		  if (select_return > 0)
		    /* have data waiting on the socket, waiting to be
		       pulled off, so we'll invoke XtAppPending() to move
		       it into the libX11 input queue */
		    have_data = true;
		}
	      
	      if (have_data == false)
		/* no data, so on to next XPlotter */
		break;
	      
	      /* Since we got here, we have waiting input data: at least
		 one event is either already in the libX11 queue or still
		 on the socket.  So we can safely call XtAppPending() to
		 read event(s) from the queue, if nonempty, or from the
		 socket.  In the latter case (the case of an empty queue),
		 XtAppPending() will call XEventsQueued(), which will, in
		 turn, do a [potentially blocking!] select().  But the way
		 we've done things, we should get an event without
		 blocking.
		 
		 After invoking XtAppPending, we invoke XtAppProcessEvent,
		 which could also potentially block, except that if an
		 event is pending, it won't.  So all should be well.

		 (Possibly irrelevant side comment.  XtAppPending will
		 flush the output buffer if no events are pending.) */

	      if (XtAppPending (_xplotters[i]->y_app_con))
		/* XtAppPending should always return true, but we invoke it
		   anyway to be on the safe side.  Note: it also checks for
		   timer and other types of event, besides X events. */
		XtAppProcessEvent (_xplotters[i]->y_app_con, XtIMAll);
	    }
	  /* end of for() loop, i.e. of our hand-crafted event loop */
	}
      /* end of if() test for a open XPlotter */
    }
  /* end of loop over XPlotters */

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _xplotters[] and _xplotters_len */
  pthread_mutex_unlock (&_xplotters_mutex);
#endif
#endif

    }
  _plotter->y_event_handler_count++;
}

#ifndef HAVE_STRERROR
/* A libplot-specific version of strerror(), for very old systems that
   don't have one. */

extern char *sys_errlist[];
extern int sys_nerr;

static char *
_plot_strerror (int errnum)
{
  if (errnum < 0 || errnum >= sys_nerr)
    return "unknown error";

  return sys_errlist[errnum];
}
#endif
