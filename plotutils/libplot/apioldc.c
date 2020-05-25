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

/* This file is specific to libplot, rather than libplotter.  It defines
   the `old' (i.e. non-thread-safe) C API.  The old C API consists of
   wrappers around the methods that may be applied to any Plotter object.

   The old API also contains pl_newpl/pl_selectpl/pl_deletepl, which
   construct and destroy Plotter instances, and maintain the global
   variable `_old_api_plotter'.  This is a pointer to a `currently
   selected' Plotter instance.  It is positioned by calling pl_selectpl.
   
   Because of this global variable, the old C API is not thread-safe.  In
   the new C API, which is thread-safe, each function in the API is passed
   a pointer to a Plotter as first argument.  Even in the absence of
   multithreading, this is a cleaner approach.

   By convention, _old_api_plotter is initialized to point to a Plotter
   instance that sends output in metafile format to standard output.
   Initialization takes place when the first function in the old API is
   invoked.  This convention is for compatibility with pre-GNU versions of
   libplot, which did not support pl_newpl/pl_select/pl_deletepl.

   The old C API also contains the pl_parampl function.  This function is
   held in common with the old (non-thread-safe) C++ API, in which it is
   simply called parampl.  It is defined in apioldcc.c.

   pl_parampl sets parameters in a global PlotterParams struct, a pointer
   to which is kept in the global variable
   `_old_api_global_plotter_params'.  (See its definition and
   initialization in g_defplot.c.)  This PlotterParams is used when any
   Plotter is created by pl_newpl.  The presence of this global state is
   another reason why the old C API is not thread-safe.  */

#include "sys-defines.h"
#include "extern.h"
#include "plot.h"		/* header file for C API's */

/* Sparse array of pointers to the old API's Plotter instances, and the
   array size; also a distinguished Plotter pointer, through which the old
   API will act. */
static Plotter **_old_api_plotters = NULL;
static int _old_api_plotters_len = 0;
static Plotter *_old_api_plotter = NULL;

/* initial size of _old_api_plotters[] */
#define INITIAL_PLOTTERS_LEN 4

/* default Plotter type (see list of supported types in the file devoted to
   the new C API) */
#ifndef DEFAULT_PLOTTER_TYPE
#define DEFAULT_PLOTTER_TYPE "meta"
#endif

/* forward references */
static void _api_warning (const char *msg);
static void _create_and_select_default_plotter (void);

/* Expand the local array of Plotters to include a single Plotter, of
   default type; also, select that Plotter.  When this is invoked, the
   array has zero size.  */
static void
_create_and_select_default_plotter (void)
{
  int i;
  Plotter *default_plotter;

  /* create the default Plotter by invoking function in new API (make sure
     global PlotterParams struct, used by the old API, is set up first) */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = pl_newplparams();
  default_plotter = pl_newpl_r (DEFAULT_PLOTTER_TYPE, stdin, stdout, stderr,
				_old_api_global_plotter_params);

  /* initialize local array of Plotters */
  _old_api_plotters = (Plotter **)_pl_xmalloc (INITIAL_PLOTTERS_LEN * sizeof(Plotter *));
  for (i = 0; i < INITIAL_PLOTTERS_LEN; i++)
    _old_api_plotters[i] = (Plotter *)NULL;
  _old_api_plotters_len = INITIAL_PLOTTERS_LEN;

  /* place default Plotter in local array, and select it */
  _old_api_plotters[0] = default_plotter;
  _old_api_plotter = default_plotter;
}

/* These are the 3 user-callable functions that are specific to the old C
   binding: newpl, selectpl, deletepl. */

/* user-callable */
int 
pl_newpl (const char *type, FILE *infile, FILE *outfile, FILE *errfile)
{
  Plotter *new_plotter;
  bool open_slot;
  int i, j;

  if (_old_api_plotters_len == 0)
    /* initialize local array of Plotters, and install default Plotter as
       Plotter #0 */
    _create_and_select_default_plotter ();

  /* create the default Plotter by invoking function in new API (make sure
     global PlotterParams struct, used by the old API, is set up first) */
  if (_old_api_global_plotter_params == NULL)
    _old_api_global_plotter_params = pl_newplparams();
  new_plotter = pl_newpl_r (type, infile, outfile, errfile,
			    _old_api_global_plotter_params);

  /* ensure local array has an open slot (slot i) */
  open_slot = false;
  for (i = 0; i < _old_api_plotters_len; i++)
    if (_old_api_plotters[i] == NULL)
      {
	open_slot = true;
	break;
      }

  if (!open_slot)
    /* expand array, clearing upper half */
    {
      i = _old_api_plotters_len;
      _old_api_plotters = 
	(Plotter **)_pl_xrealloc (_old_api_plotters, 
				    2 * _old_api_plotters_len * sizeof (Plotter *));
      for (j = _old_api_plotters_len; j < 2 * _old_api_plotters_len; j++)
	_old_api_plotters[j] = (Plotter *)NULL;
      _old_api_plotters_len *= 2;
    }
  
  /* place newly created Plotter in open slot */
  _old_api_plotters[i] = new_plotter;

  /* return index of newly created Plotter */
  return i;
}

/* user-callable, alters selected Plotter and returns index of the one that
   was previously selected */
int
pl_selectpl (int handle)
{
  int i;

  if (handle < 0 || handle >= _old_api_plotters_len 
      || _old_api_plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to select a nonexistent plotter");
      return -1;
    }

  /* determine index of currently selected Plotter in _old_api_plotters[] */
  for (i = 0; i < _old_api_plotters_len; i++)
    if (_old_api_plotters[i] == _old_api_plotter)
      break;

  /* select specified Plotter: alter value of the _old_api_plotter pointer */
  _old_api_plotter = _old_api_plotters[handle];

  /* return index of previously selected Plotter */
  return i;
}

/* user-callable */
int
pl_deletepl (int handle)
{
  if (handle < 0 || handle >= _old_api_plotters_len 
      || _old_api_plotters[handle] == NULL)
    {
      _api_warning ("ignoring request to delete a nonexistent plotter");
      return -1;
    }

  if (_old_api_plotters[handle] == _old_api_plotter)
    {
      _api_warning ("ignoring request to delete currently selected plotter");
      return -1;
    }

  /* delete Plotter by invoking function in new API */
  pl_deletepl_r (_old_api_plotters[handle]);

  /* remove now-invalid pointer from local array */
  _old_api_plotters[handle] = NULL;

  return 0;
}


/* function used in this file to print warning messages */
static void
_api_warning (const char *msg)
{
  if (pl_libplot_warning_handler != NULL)
    (*pl_libplot_warning_handler)(msg);
  else
    fprintf (stderr, "libplot: %s\n", msg);
}


/* The following are the C wrappers around the public functions in the
   Plotter class.  Together with the three functions above (pl_newpl,
   pl_selectpl, pl_deletepl), and pl_parampl, they make up the old
   (non-thread-safe) libplot C API.

   Each binding tests whether _old_api_plotter is non-NULL, which determines
   whether the array of Plotter instances has been initialized.  That is
   because it makes no sense to call these functions before the
   _old_api_plotter pointer points to a Plotter object.

   In fact, of the below functions, it really only makes sense to call
   openpl, havecap, or outfile [deprecated] before the Plotter array is
   initialized.  Calling any other of the below functions before the
   Plotter array is initialized will generate an error message because even
   though the call to _create_and_select_default_plotter will initialize
   the Plotter array and select a default Plotter instance, the Plotter
   will not be open.  No operation in the Plotter class, with the exception
   of the just-mentioned ones, may be invoked unless the Plotter that is
   being acted on is open. */

int 
pl_alabel (int x_justify, int y_justify, const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_alabel (_old_api_plotter, x_justify, y_justify, s);
}

int
pl_arc (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_arc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_arcrel (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_arcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_bezier2 (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bezier2 (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_bezier2rel (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bezier2rel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_bezier3 (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bezier3 (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
pl_bezier3rel (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bezier3rel (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
pl_bgcolor (int red, int green, int blue)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bgcolor (_old_api_plotter, red, green, blue);
}

int
pl_bgcolorname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_bgcolorname (_old_api_plotter, s);
}

int
pl_box (int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_box (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_boxrel (int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_boxrel (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_capmod (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_capmod (_old_api_plotter, s);
}

int
pl_circle (int x, int y, int r)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_circle (_old_api_plotter, x, y, r);
}

int
pl_circlerel (int x, int y, int r)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_circlerel (_old_api_plotter, x, y, r);
}

int
pl_closepath (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_closepath (_old_api_plotter);
}

int
pl_closepl (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_closepl (_old_api_plotter);
}

int
pl_color (int red, int green, int blue)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_color (_old_api_plotter, red, green, blue);
}

int
pl_colorname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_colorname (_old_api_plotter, s);
}

int
pl_cont (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_cont (_old_api_plotter, x, y);
}

int
pl_contrel (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_contrel (_old_api_plotter, x, y);
}

int
pl_ellarc (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ellarc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_ellarcrel (int xc, int yc, int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ellarcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_ellipse (int x, int y, int rx, int ry, int angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ellipse (_old_api_plotter, x, y, rx, ry, angle);
}

int
pl_ellipserel (int x, int y, int rx, int ry, int angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ellipserel (_old_api_plotter, x, y, rx, ry, angle);
}

int
pl_endpath (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_endpath (_old_api_plotter);
}

int
pl_endsubpath (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_endsubpath (_old_api_plotter);
}

int
pl_erase (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_erase (_old_api_plotter);
}

int
pl_farc (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_farc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_farcrel (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_farcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_fbezier2 (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fbezier2 (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_fbezier2rel (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fbezier2rel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_fbezier3 (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fbezier3 (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
pl_fbezier3rel (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fbezier3rel (_old_api_plotter, x0, y0, x1, y1, x2, y2, x3, y3);
}

int
pl_fbox (double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fbox (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_fboxrel (double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fboxrel (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_fcircle (double x, double y, double r)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fcircle (_old_api_plotter, x, y, r);
}

int
pl_fcirclerel (double x, double y, double r)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fcirclerel (_old_api_plotter, x, y, r);
}

int
pl_fconcat (double m0, double m1, double m2, double m3, double m4, double m5)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fconcat (_old_api_plotter, m0, m1, m2, m3, m4, m5);
}

int
pl_fcont (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fcont (_old_api_plotter, x, y);
}

int
pl_fcontrel (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fcontrel (_old_api_plotter, x, y);
}

int
pl_fellarc (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fellarc (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_fellarcrel (double xc, double yc, double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fellarcrel (_old_api_plotter, xc, yc, x0, y0, x1, y1);
}

int
pl_fellipse (double x, double y, double rx, double ry, double angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fellipse (_old_api_plotter, x, y, rx, ry, angle);
}

int
pl_fellipserel (double x, double y, double rx, double ry, double angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fellipserel (_old_api_plotter, x, y, rx, ry, angle);
}

double
pl_ffontname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ffontname (_old_api_plotter, s);
}

double
pl_ffontsize (double size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ffontsize (_old_api_plotter, size);
}

int
pl_fillcolor (int red, int green, int blue)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fillcolor (_old_api_plotter, red, green, blue);
}

int
pl_fillcolorname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fillcolorname (_old_api_plotter, s);
}

int
pl_fillmod (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fillmod (_old_api_plotter, s);
}

int
pl_filltype (int level)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_filltype (_old_api_plotter, level);
}

double
pl_flabelwidth (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_flabelwidth (_old_api_plotter, s);
}

int
pl_fline (double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fline (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_flinedash (int n, const double *dashes, double offset)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_flinedash (_old_api_plotter, n, dashes, offset);
}

int
pl_flinerel (double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_flinerel (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_flinewidth (double size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_flinewidth (_old_api_plotter, size);
}

int
pl_flushpl (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_flushpl (_old_api_plotter);
}

int
pl_fmarker (double x, double y, int type, double size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fmarker (_old_api_plotter, x, y, type, size);
}

int
pl_fmarkerrel (double x, double y, int type, double size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fmarkerrel (_old_api_plotter, x, y, type, size);
}

int
pl_fmiterlimit (double limit)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fmiterlimit (_old_api_plotter, limit);
}

int
pl_fmove (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fmove (_old_api_plotter, x, y);
}

int
pl_fmoverel (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fmoverel (_old_api_plotter, x, y);
}

int
pl_fontname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fontname (_old_api_plotter, s);
}

int
pl_fontsize (int size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fontsize (_old_api_plotter, size);
}

int
pl_fpoint (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fpoint (_old_api_plotter, x, y);
}

int
pl_fpointrel (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fpointrel (_old_api_plotter, x, y);
}

int
pl_frotate (double theta)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_frotate (_old_api_plotter, theta);
}

int
pl_fscale (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fscale (_old_api_plotter, x, y);
}

int
pl_fsetmatrix (double m0, double m1, double m2, double m3, double m4, double m5)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fsetmatrix (_old_api_plotter, m0, m1, m2, m3, m4, m5);
}

int
pl_fspace (double x0, double y0, double x1, double y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fspace (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_fspace2 (double x0, double y0, double x1, double y1, double x2, double y2)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_fspace2 (_old_api_plotter, x0, y0, x1, y1, x2, y2);
}

double
pl_ftextangle (double angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ftextangle (_old_api_plotter, angle);
}

int
pl_ftranslate (double x, double y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_ftranslate (_old_api_plotter, x, y);
}

int
pl_havecap (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_havecap (_old_api_plotter, s);
}

int
pl_joinmod (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_joinmod (_old_api_plotter, s);
}

int
pl_label (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_label (_old_api_plotter, s);
}

int
pl_labelwidth (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_labelwidth (_old_api_plotter, s);
}

int
pl_line (int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_line (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_linerel (int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_linerel (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_linewidth (int size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_linewidth (_old_api_plotter, size);
}

int
pl_linedash (int n, const int *dashes, int offset)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_linedash (_old_api_plotter, n, dashes, offset);
}

int
pl_linemod (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_linemod (_old_api_plotter, s);
}

int
pl_marker (int x, int y, int type, int size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_marker (_old_api_plotter, x, y, type, size);
}

int
pl_markerrel (int x, int y, int type, int size)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_markerrel (_old_api_plotter, x, y, type, size);
}

int
pl_move (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_move (_old_api_plotter, x, y);
}

int
pl_moverel (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_moverel (_old_api_plotter, x, y);
}

int
pl_openpl (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_openpl (_old_api_plotter);
}

int
pl_orientation (int direction)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_orientation (_old_api_plotter, direction);
}

FILE *
pl_outfile (FILE *outfile)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_outfile (_old_api_plotter, outfile);
}

int
pl_pencolor (int red, int green, int blue)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_pencolor (_old_api_plotter, red, green, blue);
}

int
pl_pencolorname (const char *s)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_pencolorname (_old_api_plotter, s);
}

int
pl_pentype (int level)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_pentype (_old_api_plotter, level);
}

int
pl_point (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_point (_old_api_plotter, x, y);
}

int
pl_pointrel (int x, int y)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_pointrel (_old_api_plotter, x, y);
}

int
pl_restorestate (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_restorestate (_old_api_plotter);
}

int
pl_savestate (void)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_savestate (_old_api_plotter);
}

int
pl_space (int x0, int y0, int x1, int y1)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_space (_old_api_plotter, x0, y0, x1, y1);
}

int
pl_space2 (int x0, int y0, int x1, int y1, int x2, int y2)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_space2 (_old_api_plotter, x0, y0, x1, y1, x2, y2);
}

int
pl_textangle (int angle)
{
  if (_old_api_plotters_len == 0)
    _create_and_select_default_plotter ();
  return _API_textangle (_old_api_plotter, angle);
}

/* END OF WRAPPERS */
