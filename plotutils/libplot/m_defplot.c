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

/* This file defines the initialization for any MetaPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a MetaPlotter struct. */
const Plotter _pl_m_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_m_initialize, _pl_m_terminate,
  /* page manipulation */
  _pl_m_begin_page, _pl_m_erase_page, _pl_m_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_m_paint_path, _pl_m_paint_paths, _pl_m_path_is_flushable, _pl_m_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_m_paint_marker, _pl_m_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_m_paint_text_string_with_escapes, _pl_g_paint_text_string,
  _pl_g_get_text_width,
  /* internal `retrieve font' method */
  _pl_g_retrieve_font,
  /* `flush output' method, called only if Plotter handles its own output */
  _pl_g_flush_output,
  /* internal `error handler' methods */
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
_pl_m_initialize (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_g_initialize (S___(_plotter));
#endif

  /* override superclass initializations, as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_META;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 2;
  _plotter->data->have_dash_array = 2;
  _plotter->data->have_solid_fill = 2;
  _plotter->data->have_odd_winding_fill = 2;
  _plotter->data->have_nonzero_winding_fill = 2;
  _plotter->data->have_settable_bg = 2;
  _plotter->data->have_escaped_string_support = 1;
  _plotter->data->have_ps_fonts = 1;
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 1;
  _plotter->data->have_extra_stick_fonts = 1;
  _plotter->data->have_other_fonts = 1;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = PL_F_HERSHEY;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->have_horizontal_justification = true;
  _plotter->data->have_vertical_justification = true;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal) */
  _plotter->data->max_unfilled_path_length = PL_MAX_UNFILLED_PATH_LENGTH;
  _plotter->data->have_mixed_paths = true;
  _plotter->data->allowed_arc_scaling = AS_ANY;
  _plotter->data->allowed_ellarc_scaling = AS_ANY;
  _plotter->data->allowed_quad_scaling = AS_ANY;  
  _plotter->data->allowed_cubic_scaling = AS_ANY;  
  _plotter->data->allowed_box_scaling = AS_ANY;
  _plotter->data->allowed_circle_scaling = AS_ANY;
  _plotter->data->allowed_ellipse_scaling = AS_ANY;

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

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->meta_portable_output = false;
  /* dynamic variables */
  _plotter->meta_pos.x = 0.0;
  _plotter->meta_pos.y = 0.0;
  _plotter->meta_position_is_unknown = false;
  _plotter->meta_m_user_to_ndc[0] = 1.0;
  _plotter->meta_m_user_to_ndc[1] = 0.0;
  _plotter->meta_m_user_to_ndc[2] = 0.0;
  _plotter->meta_m_user_to_ndc[3] = 1.0;
  _plotter->meta_m_user_to_ndc[4] = 0.0;
  _plotter->meta_m_user_to_ndc[5] = 0.0;
  _plotter->meta_fill_rule_type = PL_FILL_ODD_WINDING;
  _plotter->meta_line_type = PL_L_SOLID;
  _plotter->meta_points_are_connected = true;  
  _plotter->meta_cap_type = PL_CAP_BUTT;  
  _plotter->meta_join_type = PL_JOIN_MITER;  
  _plotter->meta_miter_limit = PL_DEFAULT_MITER_LIMIT;  
  _plotter->meta_line_width = 0.0;
  _plotter->meta_line_width_is_default = true;
  _plotter->meta_dash_array = (const double *)NULL;
  _plotter->meta_dash_array_len = 0;
  _plotter->meta_dash_offset = 0.0;  
  _plotter->meta_dash_array_in_effect = false;  
  _plotter->meta_pen_type = 1;  
  _plotter->meta_fill_type = 0;
  _plotter->meta_orientation = 1;  
  _plotter->meta_font_name = (const char *)NULL;
  _plotter->meta_font_size = 0.0;
  _plotter->meta_font_size_is_default = true;
  _plotter->meta_text_rotation = 0.0;  
  _plotter->meta_fgcolor.red = 0;
  _plotter->meta_fgcolor.green = 0;
  _plotter->meta_fgcolor.blue = 0;
  _plotter->meta_fillcolor_base.red = 0;
  _plotter->meta_fillcolor_base.green = 0;
  _plotter->meta_fillcolor_base.blue = 0;
  _plotter->meta_bgcolor.red = 65535;
  _plotter->meta_bgcolor.green = 65535;
  _plotter->meta_bgcolor.blue = 65535;
  
  /* initialize certain data members from device driver parameters */
      
  /* determine version of metafile format */
  {
    const char *portable_s;
    
    portable_s = (const char *)_get_plot_param (_plotter->data, 
						"META_PORTABLE");
    if (strcasecmp (portable_s, "yes") == 0)
      _plotter->meta_portable_output = true;
    else
      _plotter->meta_portable_output = false; /* default value */
  }
}

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
_pl_m_terminate (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_g_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
MetaPlotter::MetaPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	: Plotter (infile, outfile, errfile)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *outfile)
	: Plotter (outfile)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (ostream& out)
	: Plotter (out)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter ()
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _pl_m_initialize ();
}

MetaPlotter::MetaPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _pl_m_initialize ();
}

MetaPlotter::~MetaPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _pl_m_terminate ();
}
#endif
