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

/* This file defines the initialization for any generic Plotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

/* additional include files for DJGPP */
#ifdef MSDOS
#include <io.h>			/* for setmode() */
#include <fcntl.h>		/* for O_BINARY */
#include <unistd.h>		/* for isatty() */
#endif

/* global library variables (user-settable error handlers) */
#ifndef LIBPLOTTER
int (*pl_libplot_warning_handler)(const char *) = NULL;
int (*pl_libplot_error_handler)(const char *) = NULL;
#else  /* LIBPLOTTER */
int (*pl_libplotter_warning_handler)(const char *) = NULL;
int (*pl_libplotter_error_handler)(const char *) = NULL;
#endif /* LIBPLOTTER */

/* The following variables (_plotters, _plotters_len) are global variables
   in libplot, but static data members of the Plotter class in libplotter.
   That's arranged by #ifdefs's in extern.h. */

/* Sparse array of pointers to Plotter instances, and its size.
   Initialized to a NULL pointer, and 0, respectively.  

   Accessed by _g_initialize() and _g_terminate() in this file, and by
   g_flush() in g_flushpl.c. */

Plotter **_plotters = NULL;
int _plotters_len = 0;

/* initial size for _plotters[], the sparse array of pointers to Plotter
   instances */
#define INITIAL_PLOTTERS_LEN 4

/* Mutex for locking _plotters[] and _plotters_len.  A global variable in
   both libplot and libplotter. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
pthread_mutex_t _plotters_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/* Pointer to distinguished (global) PlotterParams object, used by the old
   (non-thread-safe) bindings.  In libplotter, this is a static data member
   of the Plotter class.  That's arranged by an #ifdef in extern.h. */
PlotterParams *_old_api_global_plotter_params = NULL;

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a Plotter struct. */
const Plotter _pl_g_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_g_initialize, _pl_g_terminate,
  /* page manipulation */
  _pl_g_begin_page, _pl_g_erase_page, _pl_g_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_g_paint_path, _pl_g_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_g_paint_marker, _pl_g_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_g_paint_text_string,
  _pl_g_get_text_width,
  /* private low-level `retrieve font' method */
  _pl_g_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _pl_g_flush_output,
  /* error handlers */
  _pl_g_warning,
  _pl_g_error
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, invoked when a Plotter is created.  It
   does such things as initializing capability flags from the values of
   class variables, allocating storage, etc.  In the C binding, when this
   is invoked _plotter points to the Plotter that has just been created.
   In the C++ binding, this is a function member of the Plotter class, and
   _plotter is an alias for `this'. */

void
_pl_g_initialize (S___(Plotter *_plotter))
{
  bool open_slot = false;
  int i, j;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _plotters[] and _plotters_len */
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif

  /* ensure plotter instance array is set up */
  if (_plotters_len == 0)
    {
      _plotters = (Plotter **)_pl_xmalloc (INITIAL_PLOTTERS_LEN * sizeof(Plotter *));
      for (i = 0; i < INITIAL_PLOTTERS_LEN; i++)
	_plotters[i] = (Plotter *)NULL;
      _plotters_len = INITIAL_PLOTTERS_LEN;
    }

  /* be sure there is an open slot (slot i) */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == NULL)
      {
	open_slot = true;
	break;
      }

  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _plotters_len;
      _plotters = 
	(Plotter **)_pl_xrealloc (_plotters, 
				    2 * _plotters_len * sizeof (Plotter *));
      for (j = _plotters_len; j < 2 * _plotters_len; j++)
	_plotters[j] = (Plotter *)NULL;
      _plotters_len *= 2;
    }
  
  /* place just-created Plotter in open slot */
  _plotters[i] = _plotter;

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _plotters[] and _plotters_len */
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif

  /* Initialize all data members (except in/out/err streams and device
     driver parameters). */

#ifndef LIBPLOTTER
  /* tag field, will differ in derived classes */
  _plotter->data->type = PL_GENERIC;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_NONE;

  /* I/O, will not differ in derived classes */
  _plotter->data->page = (plOutbuf *)NULL;
  _plotter->data->first_page = (plOutbuf *)NULL;  

  /* basic data members, will not differ in derived classes */
  _plotter->data->open = false;
  _plotter->data->opened = false;
  _plotter->data->page_number = 0;
  _plotter->data->fontsize_invoked = false;
  _plotter->data->linewidth_invoked = false;
  _plotter->data->frame_number = 0;

  /* drawing state stack (initially empty; same in derived classes) */
  _plotter->drawstate = (plDrawState *)NULL;  

  /* warnings, will not differ in derived classes */
  _plotter->data->font_warning_issued = false;
  _plotter->data->pen_color_warning_issued = false;
  _plotter->data->fill_color_warning_issued = false;
  _plotter->data->bg_color_warning_issued = false;
  
  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 1;
  _plotter->data->have_escaped_string_support = 1;
  _plotter->data->have_ps_fonts = 1;
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 1;
  _plotter->data->have_extra_stick_fonts = 0;	/* specific to HP-GL version "1.5" */
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->data->default_font_type = PL_F_HERSHEY;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->have_horizontal_justification = false;
  _plotter->data->have_vertical_justification = false;
  _plotter->data->kern_stick_fonts = false;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal) */
  _plotter->data->max_unfilled_path_length = PL_MAX_UNFILLED_PATH_LENGTH;
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;  
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;  
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_NONE;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* color-related parameters (also internal) */
  _plotter->data->emulate_color = false;
  
  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 1.0;
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 1.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* create, initialize cache of color name -> RGB correspondences */
  _plotter->data->color_name_cache = _create_color_name_cache ();

  /* initialize certain data members from values of relevant device
     driver parameters */

  /* emulate color by using grayscale? */
  {
    const char *emulate_s;

    emulate_s = (const char *)_get_plot_param (_plotter->data, 
					       "EMULATE_COLOR");
    if (strcmp (emulate_s, "yes") == 0)
      _plotter->data->emulate_color = true;
    else 
      _plotter->data->emulate_color = false;
  }

  /* set maximum polyline length (relevant to most Plotters, esp. those
     that do not do real time output) */
  {
    const char *length_s;
    int local_length;
	
    length_s = (const char *)_get_plot_param (_plotter->data, 
					      "MAX_LINE_LENGTH");

    if (sscanf (length_s, "%d", &local_length) <= 0 || local_length <= 0)
      {
	length_s = (const char *)_get_default_plot_param ("MAX_LINE_LENGTH");
	sscanf (length_s, "%d", &local_length);
      }
    _plotter->data->max_unfilled_path_length = local_length;
  }
      
  /* Ensure widths of labels rendered in the Stick fonts are correctly
     computed.  This is a kludge (in pre-HP-GL/2, Stick fonts were kerned;
     see g_alabel.c.)  */
  {
    const char *version_s;

    version_s = (const char *)_get_plot_param (_plotter->data, 
					       "HPGL_VERSION");
    if (strcmp (version_s, "2") == 0) /* modern HP-GL/2 (default) */
      _plotter->data->kern_stick_fonts = false;
    else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
      _plotter->data->kern_stick_fonts = true;
    else if (strcmp (version_s, "1") == 0) /* generic HP-GL */
      _plotter->data->kern_stick_fonts = true; /* meaningless (no stick fonts) */
  }

#ifdef MSDOS
  /* A DJGPP enhancement (thanks, Michel de Ruiter).  If the output fp is
     standard output, and standard output has been redirected to a file,
     then set the output mode to binary.  This turns off character mapping,
     which is a necessity when writing binary output formats such as GIF or
     Tektronix.  If the output fp is a (FILE *) other than standard output,
     then whoever opened it is responsible for having opened it with mode
     "wb" instead of "wt" or "w". */
  if (_plotter->data->outfp
      && (_plotter->data->outfp == stdout)
      && O_BINARY
      && !isatty (fileno (stdout)))
    {
      fflush (stdout);
      setmode (fileno (stdout), O_BINARY);
    }
#endif
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted, provided that it is non-NULL.  It may do such things as write
   to an output stream from internal storage, deallocate storage, etc. */

void
_pl_g_terminate (S___(Plotter *_plotter))
{
  int i;

  /* if specified plotter is open, close it */
  if (_plotter->data->open)
    _API_closepl (S___(_plotter));

  /* free instance-specific copies of class parameters */
  _pl_g_free_params_in_plotter (S___(_plotter));

  /* free color name cache */
  _delete_color_name_cache (_plotter->data->color_name_cache);

  /* remove Plotter from sparse Plotter array */

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the global variables _plotters[] and _plotters_len */
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i] == _plotter)
      {
	_plotters[i] = (Plotter *)NULL;
	break;
      }
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the global variables _plotters[] and _plotters_len */
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif
}

#ifdef LIBPLOTTER

/* OLD API; not thread-safe (these ctors take values for Plotter
   parameters from the global PlotterParams struct) */

Plotter::Plotter (FILE *infile, FILE *outfile, FILE *errfile)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = infile;
  _plotter->data->outfp = outfile;  
  _plotter->data->errfp = errfile;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _pl_g_copy_params_to_plotter (_old_api_global_plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (FILE *outfile)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = outfile;  
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _pl_g_copy_params_to_plotter (_old_api_global_plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (istream& in, ostream& out, ostream& err)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;  
  _plotter->data->errfp = NULL;
  if (in.rdbuf())
    _plotter->data->instream = &in;
  else
    _plotter->data->instream = NULL;
  if (out.rdbuf())
    _plotter->data->outstream = &out;  
  else
    _plotter->data->outstream = NULL;
  if (err.rdbuf())
    _plotter->data->errstream = &err;
  else
    _plotter->data->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _pl_g_copy_params_to_plotter (_old_api_global_plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (ostream& out)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  if (out.rdbuf())
    _plotter->data->outstream = &out;
  else
    _plotter->data->outstream = NULL;
  _plotter->data->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _pl_g_copy_params_to_plotter (_old_api_global_plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter ()
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the current values of device driver parameters */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = new PlotterParams;
  _pl_g_copy_params_to_plotter (_old_api_global_plotter_params);

  _pl_g_initialize ();
}

/* NEW API; thread-safe (since user can specify a local PlotterParams
   struct from which parameters should be taken) */

Plotter::Plotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &plotter_params)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = infile;
  _plotter->data->outfp = outfile;  
  _plotter->data->errfp = errfile;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _pl_g_copy_params_to_plotter (&plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (FILE *outfile, PlotterParams &plotter_params)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = outfile;  
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _pl_g_copy_params_to_plotter (&plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (istream& in, ostream& out, ostream& err, PlotterParams &plotter_params)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;  
  _plotter->data->errfp = NULL;
  if (in.rdbuf())
    _plotter->data->instream = &in;
  else
    _plotter->data->instream = NULL;
  if (out.rdbuf())
    _plotter->data->outstream = &out;  
  else
    _plotter->data->outstream = NULL;
  if (err.rdbuf())
    _plotter->data->errstream = &err;
  else
    _plotter->data->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _pl_g_copy_params_to_plotter (&plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (ostream& out, PlotterParams &plotter_params)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  if (out.rdbuf())
    _plotter->data->outstream = &out;
  else
    _plotter->data->outstream = NULL;
  _plotter->data->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _pl_g_copy_params_to_plotter (&plotter_params);

  _pl_g_initialize ();
}

Plotter::Plotter (PlotterParams &plotter_params)
{
  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  _plotter->data->infp = NULL;
  _plotter->data->outfp = NULL;
  _plotter->data->errfp = NULL;
  _plotter->data->instream = NULL;
  _plotter->data->outstream = NULL;  
  _plotter->data->errstream = NULL;
  /* copy in the specified values of device driver parameters */
  _pl_g_copy_params_to_plotter (&plotter_params);

  _pl_g_initialize ();
}

Plotter::~Plotter ()
{
  _pl_g_terminate ();

  /* destroy PlotterData structure in Plotter */
  free (_plotter->data);
}
#endif
