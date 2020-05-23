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

/* This file defines the initialization for any PNM Plotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PNMPlotter struct.  It is the same as for a BitmapPlotter, except for
   the routines _pl_n_initialize and _pl_n_terminate. */
const Plotter _pl_n_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_n_initialize, _pl_n_terminate,
  /* page manipulation */
  _pl_b_begin_page, _pl_b_erase_page, _pl_b_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_b_paint_path, _pl_b_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_g_paint_marker, _pl_b_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_g_paint_text_string,
  _pl_g_get_text_width,
  /* private low-level `retrieve font' method */
  _pl_g_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _pl_g_flush_output,
  /* error handlers */
  _pl_g_warning,
  _pl_g_error,
};
#endif /* not LIBPLOTTER */

/* The private `initialize' method, which is invoked when a Plotter is
   created.  It is used for such things as initializing capability flags
   from the values of class variables, allocating storage, etc.  When this
   is invoked, _plotter points to the Plotter that has just been
   created. */

void
_pl_n_initialize (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_b_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_PNM;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES;

  /* initialize data members specific to this derived class */
  _plotter->n_portable_output = false;

  /* initialize certain data members from device driver parameters */
      
  /* determine version of PBM/PGM/PPM format (binary or ascii) */
  {
    const char *portable_s;
    
    portable_s = (const char *)_get_plot_param (_plotter->data, "PNM_PORTABLE");
    if (strcasecmp (portable_s, "yes") == 0)
      _plotter->n_portable_output = true;
    else
      _plotter->n_portable_output = false; /* default value */
  }

}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points (temporarily) to the Plotter that is about to be
   deleted. */

void
_pl_n_terminate (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_b_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
PNMPlotter::PNMPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	: BitmapPlotter (infile, outfile, errfile)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *outfile)
	: BitmapPlotter (outfile)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (istream& in, ostream& out, ostream& err)
	: BitmapPlotter (in, out, err)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (ostream& out)
	: BitmapPlotter (out)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter ()
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	: BitmapPlotter (infile, outfile, errfile, parameters)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (FILE *outfile, PlotterParams &parameters)
	: BitmapPlotter (outfile, parameters)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: BitmapPlotter (in, out, err, parameters)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (ostream& out, PlotterParams &parameters)
	: BitmapPlotter (out, parameters)
{
  _pl_n_initialize ();
}

PNMPlotter::PNMPlotter (PlotterParams &parameters)
	: BitmapPlotter (parameters)
{
  _pl_n_initialize ();
}

PNMPlotter::~PNMPlotter ()
{
  _pl_n_terminate ();
}
#endif
