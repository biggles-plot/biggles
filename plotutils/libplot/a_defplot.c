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

/* This file defines the initialization for any AIPlotter object, including
   both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   an AIPlotter struct. */
const Plotter _pl_a_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_a_initialize, _pl_a_terminate,
  /* page manipulation */
  _pl_a_begin_page, _pl_a_erase_page, _pl_a_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_a_paint_path, _pl_a_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_g_paint_marker, _pl_a_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_a_paint_text_string,
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

/* For AI Plotter objects, we determine the page size and the location of
   the viewport on the page, so that we'll be able to work out the map from
   user coordinates to device coordinates in space.c.  Also we determine
   which version of Illustrator file format we'll emit. */

void
_pl_a_initialize (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_AI;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_ONE_PAGE;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 0;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 1;
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = PL_F_POSTSCRIPT;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->issue_font_warning = true;
  _plotter->data->have_horizontal_justification = true;
  _plotter->data->have_vertical_justification = false;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = true;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;
  _plotter->data->allowed_quad_scaling = AS_NONE;
  _plotter->data->allowed_cubic_scaling = AS_ANY;
  _plotter->data->allowed_box_scaling = AS_NONE;
  _plotter->data->allowed_circle_scaling = AS_NONE;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_REAL;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  _plotter->ai_version = AI_VERSION_5;
  _plotter->ai_pen_cyan = 0.0;
  _plotter->ai_pen_magenta = 0.0;
  _plotter->ai_pen_yellow = 0.0;
  _plotter->ai_pen_black = 1.0;  
  _plotter->ai_fill_cyan = 0.0;
  _plotter->ai_fill_magenta = 0.0;
  _plotter->ai_fill_yellow = 0.0;
  _plotter->ai_fill_black = 1.0;  
  _plotter->ai_cyan_used = false;
  _plotter->ai_magenta_used = false;
  _plotter->ai_yellow_used = false;
  _plotter->ai_black_used = false;
  _plotter->ai_cap_style = AI_LINE_CAP_BUTT;
  _plotter->ai_join_style = AI_LINE_JOIN_MITER;  
/* Maximum value the cosecant of the half-angle between any two line
   segments can have, if the join is to be mitered rather than beveled.
   Default value for AI is 4.0. */
  _plotter->ai_miter_limit = 4.0;
  _plotter->ai_line_type = PL_L_SOLID;  
  _plotter->ai_line_width = 1.0;    
  _plotter->ai_fill_rule_type = 0; /* i.e. nonzero winding number rule */

  /* initialize certain data members from device driver parameters */

  /* determine which version of AI format we'll emit (obsolescent) */
  {
    const char *version_s;
    
    version_s = (const char *)_get_plot_param (_plotter->data, "AI_VERSION");
    if (strcmp (version_s, "3") == 0)
      _plotter->ai_version = AI_VERSION_3;
    else if (strcmp (version_s, "5") == 0)
      _plotter->ai_version = AI_VERSION_5;
    else      
      {
	version_s = (const char *)_get_default_plot_param ("AI_VERSION");
	if (strcmp (version_s, "3") == 0)
	  _plotter->ai_version = AI_VERSION_3;
	else if (strcmp (version_s, "5") == 0)
	  _plotter->ai_version = AI_VERSION_5;
      }
  }
  /* AI didn't support even-odd fill until version 5 */
  if (_plotter->ai_version == AI_VERSION_3)
    _plotter->data->have_odd_winding_fill = 0;

  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */
  {
    /* determine page type, viewport size and location */
    _set_page_type (_plotter->data);
  
    /* convert viewport size-and-location data (in terms of inches) to
       device coordinates (i.e. points) */
    _plotter->data->xmin = 72 * (_plotter->data->viewport_xorigin
				 + _plotter->data->viewport_xoffset);
    _plotter->data->xmax = 72 * (_plotter->data->viewport_xorigin
				 + _plotter->data->viewport_xoffset
				 + _plotter->data->viewport_xsize);

    _plotter->data->ymin = 72 * (_plotter->data->viewport_yorigin
				 + _plotter->data->viewport_yoffset);
    _plotter->data->ymax = 72 * (_plotter->data->viewport_yorigin
				 + _plotter->data->viewport_yoffset
				 + _plotter->data->viewport_ysize);
  }

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
_pl_a_terminate (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
AIPlotter::AIPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (ostream& out)
	: Plotter (out)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter ()
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _pl_a_initialize ();
}

AIPlotter::AIPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _pl_a_initialize ();
}

AIPlotter::~AIPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _pl_a_terminate ();
}
#endif
