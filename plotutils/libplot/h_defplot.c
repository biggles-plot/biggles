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

/* This file defines the initializations for HPGLPlotter and PCLPlotter
   objects including both private data and public methods.  There is a
   one-to-one correspondence between public methods and user-callable
   functions in the C API. */

/* Originally, the only differences between the two types of Plotter were
   the PCL5 control codes that must be emitted to switch a PCL5 printer
   into HP-GL/2 mode, and back out of it.

   More recently, the two types of Plotter are distinguished by their
   viewport positioning.  A PCL Plotter positions its viewport on the page
   in the same position that a PS, AI, or Fig Plotter does, i.e. it centers
   it.  But a pure HPGL[/2] Plotter doesn't know where on the page the
   origin of the device coordinate system lies.  (Though it's probably
   close to a corner.)  Nor does can it set programmatically whether it's
   plotting in portrait or landscape mode.  (It can flip between them, but
   it doesn't know which is which.)

   So HPGL Plotters use a viewport of the same default size as PCL, PS, AI,
   and Fig Plotters.  But they don't position it: the lower left corner of
   the viewport is chosen to be the origin of the device coordinate system:
   what in HP-GL[/2] jargon is called "scaling point P1".

   For this to look reasonably good, the viewport needs to have a size
   appropriate for an HP-GL[/2] device.  And in fact, that's what
   determines our choice of default viewport size -- for all Plotters, not
   just HPGLPlotters.  See comments in g_pagetype.h.  */

#include "sys-defines.h"
#include "extern.h"

#define MAX_COLOR_NAME_LEN 32	/* long enough for all known colors */

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a HPGLPlotter struct. */
const Plotter _pl_h_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_h_initialize, _pl_h_terminate,
  /* page manipulation */
  _pl_h_begin_page, _pl_h_erase_page, _pl_h_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_h_paint_path, _pl_h_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_g_paint_marker, _pl_h_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_h_paint_text_string,
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

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a PCLPlotter struct.  It is the same as the above except for the
   different initialization and termination routines. */
const Plotter _pl_q_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_q_initialize, _pl_q_terminate,
  /* page manipulation */
  _pl_h_begin_page, _pl_h_erase_page, _pl_h_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_h_paint_path, _pl_h_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_g_paint_marker, _pl_h_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_h_paint_text_string,
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

/* The initializations for HPGL and PCL Plotters are similar.

   For HPGL Plotters, we determine the HP-GL version from the environment
   variable HPGL_VERSION ("1", "1.5", or "2", meaning generic HP-GL,
   HP7550A, and modern HP-GL/2 respectively), and determine the page size
   and the location on the page of the viewport, so that we'll be able to
   work out the map from user coordinates to device coordinates in
   g_space.c.

   We allow the user to shift the location of the viewport by specifying an
   offset vector, since the origin of the HP-GL coordinate system and the
   size of the `hard-clip region' within which graphics can be drawn are
   not known.  (There are so many HP-GL and HP-GL/2 devices.)

   We also work out which pens are available, and whether the device, if an
   HP-GL/2 device, supports the Palette Extension so that new logical pens
   can be defined as RGB triples.  The HPGL_PENS and HPGL_ASSIGN_COLORS
   environment variables are used for this.  (The default is for a generic
   HP-GL device to have exactly 1 pen, #1, and for an HP7550A or HP-GL/2
   device to have 7 pens, #1 through #7, with colors equal to the seven
   non-white vertices of the RGB color cube.  We allow the user to specify
   up to 31 pens, #1 through #31, via HPGL_PENS. */

void
_pl_h_initialize (S___(Plotter *_plotter))
{
  int i;
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_g_initialize (S___(_plotter));
#endif

  /* override generic initializations (which are appropriate to the base
     Plotter class), as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_HPGL;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_ONE_PAGE_AT_A_TIME;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 0;
  _plotter->data->have_escaped_string_support = 0;
#ifdef USE_PS_FONTS_IN_PCL
  _plotter->data->have_ps_fonts = 1;
#else
  _plotter->data->have_ps_fonts = 0;
#endif
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 1;
  _plotter->data->have_extra_stick_fonts = 1;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->data->default_font_type = PL_F_HERSHEY;
  _plotter->data->pcl_before_ps = true;
  _plotter->data->have_horizontal_justification = false;
  _plotter->data->have_vertical_justification = false;
  _plotter->data->kern_stick_fonts = true;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = true;
  _plotter->data->allowed_arc_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;  
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;
  _plotter->data->allowed_box_scaling = AS_AXES_PRESERVED;
  _plotter->data->allowed_circle_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->data->xmax = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->data->ymin = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->data->ymax = HPGL_SCALED_DEVICE_TOP;
  _plotter->data->page_data = (plPageData *)NULL;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->hpgl_rotation = 0;
  _plotter->hpgl_p1.x = 0.0;
  _plotter->hpgl_p1.y = 8128.0;  
  _plotter->hpgl_p2.x = 0.0;
  _plotter->hpgl_p2.y = 8128.0;  
  _plotter->hpgl_plot_length = 10668.0;  
  _plotter->hpgl_have_screened_vectors = false;
  _plotter->hpgl_have_char_fill = false;
  _plotter->hpgl_can_assign_colors = false;
  _plotter->hpgl_use_opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->hpgl_pen = 1;  
  _plotter->hpgl_free_pen = 2;  
  _plotter->hpgl_bad_pen = false;  
  _plotter->hpgl_pendown = false;  
  _plotter->hpgl_pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
  _plotter->hpgl_miter_limit = 5.0; /* default HP-GL/2 value */
  _plotter->hpgl_pen_type = HPGL_PEN_SOLID;
  _plotter->hpgl_pen_option1 = 0.0;
  _plotter->hpgl_pen_option2 = 0.0;
  _plotter->hpgl_fill_type = HPGL_FILL_SOLID_BI;
  _plotter->hpgl_fill_option1 = 0.0;
  _plotter->hpgl_fill_option2 = 0.0;
  _plotter->hpgl_char_rendering_type = HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE;
  _plotter->hpgl_symbol_set = PCL_ROMAN_8;  
  _plotter->hpgl_spacing = 0;  
  _plotter->hpgl_posture = 0;  
  _plotter->hpgl_stroke_weight = 0;  
  _plotter->hpgl_pcl_typeface = PCL_STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HPGL_CHARSET_ASCII;
  _plotter->hpgl_charset_upper = HPGL_CHARSET_ASCII;
  _plotter->hpgl_rel_char_height = 0.0;
  _plotter->hpgl_rel_char_width = 0.0;  
  _plotter->hpgl_rel_label_rise = 0.0;    
  _plotter->hpgl_rel_label_run = 0.0;      
  _plotter->hpgl_tan_char_slant = 0.0;      
  _plotter->hpgl_position_is_unknown = true;
  _plotter->hpgl_pos.x = 0;
  _plotter->hpgl_pos.y = 0;

  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initializations below */

  /* initialize certain data members from device driver parameters */
      
  /* determine HP-GL version */
  {
    const char *version_s;
    
    version_s = (const char *)_get_plot_param (_plotter->data, "HPGL_VERSION");
    /* there are three subcases: "1", "1.5", and "2" (default, see above) */
    if (strcmp (version_s, "1") == 0) /* generic HP-GL, HP7220 or HP7475A */
      {
	_plotter->hpgl_version = 0;
	_plotter->data->have_wide_lines = 0;
	_plotter->data->have_dash_array = 0;
	_plotter->data->have_solid_fill = 0;
	_plotter->data->have_odd_winding_fill = 1;
	_plotter->data->have_nonzero_winding_fill = 0;
	_plotter->data->have_ps_fonts = 0;
	_plotter->data->have_pcl_fonts = 0;
	_plotter->data->have_stick_fonts = 1;
	_plotter->data->have_extra_stick_fonts = 0;
	_plotter->data->kern_stick_fonts = true;
	_plotter->data->have_other_fonts = 0;
      }
    else if (strcmp (version_s, "1.5") == 0) /* HP7550A */
      {
	_plotter->hpgl_version = 1;
	_plotter->data->have_wide_lines = 0;
	_plotter->data->have_dash_array = 0;
	_plotter->data->have_solid_fill = 1;
	_plotter->data->have_odd_winding_fill = 1;
	_plotter->data->have_nonzero_winding_fill = 0;
	_plotter->data->have_ps_fonts = 0;
	_plotter->data->have_pcl_fonts = 0;
	_plotter->data->have_stick_fonts = 1;
	_plotter->data->have_extra_stick_fonts = 1;
	_plotter->data->kern_stick_fonts = true;
	_plotter->data->have_other_fonts = 0;
      }
  }

  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */

  /* NOTE: HP-GL Plotters, unlike PCL Plotters, ignore the xorigin and
     yorigin fields of the PAGESIZE parameter.  That's because the device
     coordinate system isn't well specified.  However, the viewport can be
     shifted relative to its default location, as usual, by specifying the
     xoffset and yoffset fields. */

  /* We use the corners of the viewport, in device coordinates, as our
     `scaling points' P1 and P2 (see h_openpl.c).  The coordinates we use
     in our output file will be normalized device coordinates, not physical
     device coordinates (for the map from the former to the latter, which
     is accomplished by the HP-GL `SC' instruction, see h_openpl.c). */
  {
    /* determine page type, and viewport size and location */
    _set_page_type (_plotter->data);
  
    /* by default, viewport lower left corner is (0,0) in HP-GL
       coordinates; if a user wishes to change this, the xoffset and
       yoffset parameters should be added to PAGESIZE */
    _plotter->hpgl_p1.x = (HPGL_UNITS_PER_INCH 
			   * (0.0 
			      + _plotter->data->viewport_xoffset));
    _plotter->hpgl_p2.x = (HPGL_UNITS_PER_INCH 
			   * (0.0 
			      + _plotter->data->viewport_xoffset
			      + _plotter->data->viewport_xsize));

    _plotter->hpgl_p1.y = (HPGL_UNITS_PER_INCH 
			   * (0.0 
			      + _plotter->data->viewport_yoffset));
    _plotter->hpgl_p2.y = (HPGL_UNITS_PER_INCH 
			   * (0.0 
			      + _plotter->data->viewport_yoffset
			      + _plotter->data->viewport_ysize));
    
  _plotter->data->xmin = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->data->xmax = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->data->ymin = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->data->ymax = HPGL_SCALED_DEVICE_TOP;

  /* plot length (to be emitted in an HP-GL/2 `PS' instruction, important
     mostly for roll plotters; see h_openpl.c) */
  _plotter->hpgl_plot_length = 
    _plotter->data->page_data->hpgl2_plot_length * HPGL_UNITS_PER_INCH;
  }

  /* determine whether to rotate the figure (e.g. horizontal instead of
     vertical, see h_openpl.c) */
  {
    const char *rotate_s;

    rotate_s = (const char *)_get_plot_param (_plotter->data, "HPGL_ROTATE");
    /* four subcases: 0 (default), 90, 180, 270 (latter two only if "2") */
    if (strcasecmp (rotate_s, "yes") == 0
	|| strcmp (rotate_s, "90") == 0)
      _plotter->hpgl_rotation = 90;
    else if (strcmp (rotate_s, "180") == 0 && _plotter->hpgl_version == 2)
      _plotter->hpgl_rotation = 180;
    else if (strcmp (rotate_s, "270") == 0 && _plotter->hpgl_version == 2)
      _plotter->hpgl_rotation = 270;
    else
      _plotter->hpgl_rotation = 0;
  }

  /* Should we avoid emitting the `white is opaque' HP-GL/2 instruction?
     (HP-GL/2 pen plotters may not like it) */
  {
    const char *transparent_s;

    transparent_s = (const char *)_get_plot_param (_plotter->data, "HPGL_OPAQUE_MODE" );
    if (strcasecmp (transparent_s, "no") == 0)
      _plotter->hpgl_use_opaque_mode = false;
  }
  
  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     HPGL_ASSIGN_COLORS) */
  if (_plotter->hpgl_version == 2)
    {
      const char *palette_s;
	  
      palette_s = (const char *)_get_plot_param (_plotter->data, "HPGL_ASSIGN_COLORS");
      if (strcasecmp (palette_s, "yes") == 0)
	_plotter->hpgl_can_assign_colors = true;
    }
      
  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    _plotter->hpgl_pen_defined[i] = 0; /* pen absent, or at least undefined */
      
  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  _plotter->hpgl_pen_color[0].red = 255;
  _plotter->hpgl_pen_color[0].green = 255;
  _plotter->hpgl_pen_color[0].blue = 255;
  _plotter->hpgl_pen_defined[0] = 2; /* i.e. hard-defined */
      
  /* determine initial palette, i.e. available pens in 1..31 range */
  {
    const char *pen_s;

    pen_s = (const char *)_get_plot_param (_plotter->data, "HPGL_PENS");
    
    if (pen_s == NULL 
	|| _pl_h_parse_pen_string (R___(_plotter) pen_s) == false
	|| (_plotter->hpgl_can_assign_colors == false 
	    && _plotter->hpgl_pen_defined[1] == 0))
      /* Either user didn't assign a value, or it was bad; use default.
         Note that if no logical pens, we insist on pen #1 being present
         (for backward compatibility?). */
      {
	if (_plotter->hpgl_version == 0) /* i.e. generic HP-GL */
	  pen_s = HPGL_DEFAULT_PEN_STRING;
	else
	  pen_s = HPGL2_DEFAULT_PEN_STRING;
	_pl_h_parse_pen_string (R___(_plotter) pen_s); /* default is guaranteed to parse */
      }
  }
  
  /* Examine presence or absence of hard-defined pens in 2..31 range.
     0 = undefined, 1 = soft-defined (not yet), 2 = hard-defined. */
  {
    bool undefined_pen_seen = false;
	
    for (i = 2; i < HPGL2_MAX_NUM_PENS; i++)
      {
	if (_plotter->hpgl_pen_defined[i] == 0)
	  /* at least one pen with number > 1 is not yet defined */
	  {
	    /* record which such was encountered first */
	    _plotter->hpgl_free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->hpgl_can_assign_colors = false;
  }
}

/* Initialization for the PCLPlotter class, which is subclassed from the
   HPGLPlotter class. */

void
_pl_q_initialize (S___(Plotter *_plotter))
{
  int i;

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_h_initialize (S___(_plotter));
#endif

  /* Superclass initialization (i.e., of an HPGLPlotter) may well have
     screwed things up, since e.g. for a PCLPlotter, hpgl_version should
     always be equal to 2, irrespective of what HPGL_VERSION is; also the
     viewport positioning is different.  So we redo a large part of the
     initialization, most of which is redundant (FIXME). */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_PCL;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_ONE_PAGE_AT_A_TIME;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 1;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 1;
  _plotter->data->have_settable_bg = 0;
  _plotter->data->have_escaped_string_support = 0;
#ifdef USE_PS_FONTS_IN_PCL
  _plotter->data->have_ps_fonts = 1;
#else
  _plotter->data->have_ps_fonts = 0;
#endif
  _plotter->data->have_pcl_fonts = 1;
  _plotter->data->have_stick_fonts = 1;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user) */
  _plotter->data->default_font_type = PL_F_PCL;
  _plotter->data->pcl_before_ps = true;
  _plotter->data->have_horizontal_justification = false;
  _plotter->data->have_vertical_justification = false;
  _plotter->data->kern_stick_fonts = false; /* in PCL5 printers' HP-GL/2 emulation */
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = true;
  _plotter->data->allowed_arc_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;  
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_ANY;
  _plotter->data->allowed_box_scaling = AS_AXES_PRESERVED;
  _plotter->data->allowed_circle_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellipse_scaling = AS_NONE;

  /* dimensions, differ in derived classes */
  _plotter->data->display_model_type = (int)DISP_MODEL_PHYSICAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->data->flipped_y = false;
  _plotter->data->imin = 0;
  _plotter->data->imax = 0;  
  _plotter->data->jmin = 0;
  _plotter->data->jmax = 0;  
  _plotter->data->xmin = HPGL_SCALED_DEVICE_LEFT; 
  _plotter->data->xmax = HPGL_SCALED_DEVICE_RIGHT;
  _plotter->data->ymin = HPGL_SCALED_DEVICE_BOTTOM;
  _plotter->data->ymax = HPGL_SCALED_DEVICE_TOP;
  _plotter->data->page_data = (plPageData *)NULL;

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->hpgl_version = 2;
  _plotter->hpgl_rotation = 0;
  _plotter->hpgl_p1.x = 0.0;
  _plotter->hpgl_p1.y = 8128.0;  
  _plotter->hpgl_p2.x = 0.0;
  _plotter->hpgl_p2.y = 8128.0;  
  _plotter->hpgl_plot_length = 10668.0;  
  _plotter->hpgl_have_screened_vectors = true; /* different from HPGLPlotter */
  _plotter->hpgl_have_char_fill = true;	/* different from HPGLPlotter */
  _plotter->hpgl_can_assign_colors = false;
  _plotter->hpgl_use_opaque_mode = true;  
  /* dynamic variables */
  	/* pen_color[] and pen_defined[] arrays also used */
  _plotter->hpgl_pen = 1;  
  _plotter->hpgl_free_pen = 2;  
  _plotter->hpgl_bad_pen = false;  
  _plotter->hpgl_pendown = false;  
  _plotter->hpgl_pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
/* Maximum value the cosecant of the half-angle between any two line
   segments can have, if the join is to be mitered rather than beveled.
   Default HP-GL/2 value is 5.0. */
  _plotter->hpgl_miter_limit = 5.0;
  _plotter->hpgl_pen_type = HPGL_PEN_SOLID;
  _plotter->hpgl_pen_option1 = 0.0;
  _plotter->hpgl_pen_option2 = 0.0;
  _plotter->hpgl_fill_type = HPGL_FILL_SOLID_BI;
  _plotter->hpgl_fill_option1 = 0.0;
  _plotter->hpgl_fill_option2 = 0.0;
  _plotter->hpgl_char_rendering_type = HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE;
  _plotter->hpgl_symbol_set = PCL_ROMAN_8;  
  _plotter->hpgl_spacing = 0;  
  _plotter->hpgl_posture = 0;  
  _plotter->hpgl_stroke_weight = 0;  
  _plotter->hpgl_pcl_typeface = PCL_STICK_TYPEFACE;  
  _plotter->hpgl_charset_lower = HPGL_CHARSET_ASCII;
  _plotter->hpgl_charset_upper = HPGL_CHARSET_ASCII;
  _plotter->hpgl_rel_char_height = 0.0;
  _plotter->hpgl_rel_char_width = 0.0;  
  _plotter->hpgl_rel_label_rise = 0.0;    
  _plotter->hpgl_rel_label_run = 0.0;      
  _plotter->hpgl_tan_char_slant = 0.0;      

  /* note: this driver also uses pen_color[], pen_defined[] arrays;
     see initializations below */

  /* initialize certain data members from device driver parameters */
      
  /* Determine range of device coordinates over which the viewport will
     extend (and hence the transformation from user to device coordinates;
     see g_space.c). */

  /* We use the corners of the viewport, in device coordinates, as our
     `scaling points' P1 and P2 (see h_openpl.c).  The coordinates we use
     in our output file will be normalized device coordinates, not physical
     device coordinates (for the map from the former to the latter, which
     is accomplished by the HP-GL `SC' instruction, see h_openpl.c). */
  {
    /* determine page type, viewport size and location */
    _set_page_type (_plotter->data);
  
    /* convert viewport size-and-location data (in terms of inches) to
       device coordinates (i.e. HP-GL units) */

    /* NOTE: origin of HP-GL/2 coordinate system used by a PCL5 device is
       not at lower left corner of page; must compensate by subtracting the
       `pcl_hpgl2_?origin' quantities. */
    _plotter->hpgl_p1.x = (HPGL_UNITS_PER_INCH 
			   * (_plotter->data->viewport_xorigin
			      + _plotter->data->viewport_xoffset
			      - _plotter->data->page_data->pcl_hpgl2_xorigin));
    _plotter->hpgl_p2.x = (HPGL_UNITS_PER_INCH 
			   * (_plotter->data->viewport_xorigin
			      + _plotter->data->viewport_xoffset
			      + _plotter->data->viewport_xsize
			      - _plotter->data->page_data->pcl_hpgl2_xorigin));

    _plotter->hpgl_p1.y = (HPGL_UNITS_PER_INCH 
			   * (_plotter->data->viewport_yorigin
			      + _plotter->data->viewport_yoffset
			      - _plotter->data->page_data->pcl_hpgl2_yorigin));
    _plotter->hpgl_p2.y = (HPGL_UNITS_PER_INCH 
			   * (_plotter->data->viewport_yorigin
			      + _plotter->data->viewport_yoffset
			      + _plotter->data->viewport_ysize
			      - _plotter->data->page_data->pcl_hpgl2_yorigin));

  /* plot length (to be emitted in an HP-GL/2 `PS' instruction, important
     mostly for roll plotters; see h_openpl.c) */
  _plotter->hpgl_plot_length = 
    _plotter->data->page_data->hpgl2_plot_length * HPGL_UNITS_PER_INCH;
  }

  /* don't make use of HP-GL/2's plotting-area rotation facility; if we
     wish to switch between portrait and landscape modes we'll do so from
     within PCL5 */
  _plotter->hpgl_rotation = 0;

  /* do we support the HP-GL/2 palette extension, i.e. can we define new
     logical pens as RGB triples? (user must request this with
     PCL_ASSIGN_COLORS) */
  _plotter->hpgl_can_assign_colors = false;
  {
    const char *palette_s;
    
    palette_s = (const char *)_get_plot_param (_plotter->data, "PCL_ASSIGN_COLORS");
    if (strcasecmp (palette_s, "yes") == 0)
      _plotter->hpgl_can_assign_colors = true;
  }
      
  /* do we use the HP-GL/2 `BZ' instruction for drawing Beziers?  (the
     LaserJet III did not support it) */
  {
    const char *bezier_s;
    
    bezier_s = (const char *)_get_plot_param (_plotter->data, "PCL_BEZIERS");

    if (strcasecmp (bezier_s, "yes") != 0)
      _plotter->data->allowed_cubic_scaling = AS_NONE;
  }

  /* initialize pen color array, typically 0..31 */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    _plotter->hpgl_pen_defined[i] = 0; /* pen absent, or at least undefined */
      
  /* pen #0 (white pen, RGB=255,255,255) is always defined */
  _plotter->hpgl_pen_color[0].red = 255;
  _plotter->hpgl_pen_color[0].green = 255;
  _plotter->hpgl_pen_color[0].blue = 255;
  _plotter->hpgl_pen_defined[0] = 2; /* i.e. hard-defined */
      
  /* determine initial palette, i.e. available pens in 1..31 range; for a
     PCLPlotter we use the default HP-GL/2 pen string */
  {
    const char *pen_s;

    pen_s = HPGL2_DEFAULT_PEN_STRING;
    _pl_h_parse_pen_string (R___(_plotter) pen_s); /* default is guaranteed to parse */
  }
  
  /* Examine presence or absence of hard-defined pens in 2..31 range.
     0 = undefined, 1 = soft-defined (not yet), 2 = hard-defined. */
  {
    bool undefined_pen_seen = false;
	
    for (i = 2; i < HPGL2_MAX_NUM_PENS; i++)
      {
	if (_plotter->hpgl_pen_defined[i] == 0)
	  /* at least one pen with number > 1 is not yet defined */
	  {
	    /* record which such was encountered first */
	    _plotter->hpgl_free_pen = i;
	    undefined_pen_seen = true;
	    break;
	  }
      }
    if (!undefined_pen_seen)	
      /* too many pens specified, can't soft-define colors */
      _plotter->hpgl_can_assign_colors = false;
  }
}

/* Parse a pen string, e.g. a user-specified HPGL_PENS environment
   variable, specifying which pens are available.  Result is stored in the
   Plotter.  More pens (logical pens) may be added later to the array of
   available pens, if the plotter is an HP-GL/2 device and supports the
   palette extension.  User specifies this by setting the
   HPGL_ASSIGN_COLORS environment variable to "yes"; see above. */
bool
_pl_h_parse_pen_string (R___(Plotter *_plotter) const char *pen_s)
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;
      plColor color;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num >= HPGL2_MAX_NUM_PENS)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string and env var also */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* got color name string, parse it */
      if (_string_to_color (name, &color, _plotter->data->color_name_cache))
	{
	  _plotter->hpgl_pen_color[pen_num] = color;
	  _plotter->hpgl_pen_defined[pen_num] = 2; /* hard-defined */
	}
      else			/* couldn't match color name string */
	return false;
    }

  return true;
}  

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

void
_pl_h_terminate (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_g_terminate (S___(_plotter));
#endif
}

void
_pl_q_terminate (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_h_terminate (S___(_plotter));
#endif
}

#ifdef LIBPLOTTER
HPGLPlotter::HPGLPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (ostream& out)
	: Plotter (out)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter ()
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _pl_h_initialize ();
}

HPGLPlotter::HPGLPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _pl_h_initialize ();
}

HPGLPlotter::~HPGLPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _pl_h_terminate ();
}
#endif

#ifdef LIBPLOTTER
PCLPlotter::PCLPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:HPGLPlotter (infile, outfile, errfile)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *outfile)
	:HPGLPlotter (outfile)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (istream& in, ostream& out, ostream& err)
	: HPGLPlotter (in, out, err)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (ostream& out)
	: HPGLPlotter (out)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter ()
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:HPGLPlotter (infile, outfile, errfile, parameters)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (FILE *outfile, PlotterParams &parameters)
	:HPGLPlotter (outfile, parameters)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: HPGLPlotter (in, out, err, parameters)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (ostream& out, PlotterParams &parameters)
	: HPGLPlotter (out, parameters)
{
  _pl_q_initialize ();
}

PCLPlotter::PCLPlotter (PlotterParams &parameters)
	: HPGLPlotter (parameters)
{
  _pl_q_initialize ();
}

PCLPlotter::~PCLPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _pl_q_terminate ();
}
#endif

#ifndef LIBPLOTTER
/* The following forwarding functions provide special support in libplot
   for deriving the PCLPlotter class from the HPGLPlotter class.  In
   libplotter, forwarding is implemented by a virtual function; see
   plotter.h. */

/* Two forwarding functions called by any HPGLPlotter/PCLPlotter in
   begin_page() and end_page(), respectively.  See h_openpl.c and
   h_closepl.c for the forwarded-to functions _pl_h_maybe_switch_to_hpgl(),
   _pl_q_maybe_switch_to_hpgl(), _pl_h_maybe_switch_from_hpgl(),
   _pl_q_maybe_switch_from_hpgl().  The HPGLPlotter versions are no-ops, but
   the PCLPlotter versions switch the printer to HP-GL/2 mode from PCL 5
   mode, and back to PCL 5 mode from HP-GL/2 mode. */
   
/* Eject page (if page number > 1) and switch from PCL 5 mode to HP-GL/2
   mode, if a PCL 5 printer (otherwise it's a no-op).  Invoked by
   begin_page(). */
void
_maybe_switch_to_hpgl (Plotter *_plotter)
{
  switch ((int)(_plotter->data->type))
    {
    case (int)PL_HPGL:
    default:
      _pl_h_maybe_switch_to_hpgl (_plotter); /* no-op */
      break;
    case (int)PL_PCL:
      _pl_q_maybe_switch_to_hpgl (_plotter);
      break;
    }
}

/* Switch back to PCL 5 mode from HP-GL/2 mode, if a PCL 5 printer
   (otherwise it's a no-op).  Invoked by end_page(). */
void
_maybe_switch_from_hpgl (Plotter *_plotter)
{
  switch ((int)(_plotter->data->type))
    {
    case (int)PL_HPGL:
    default:
      _pl_h_maybe_switch_from_hpgl (_plotter); /* no-op */
      break;
    case (int)PL_PCL:
      _pl_q_maybe_switch_from_hpgl (_plotter);
      break;
    }
}
#endif /* not LIBPLOTTER */
