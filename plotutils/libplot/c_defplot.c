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

/* This file defines the initialization for any CGMPlotter object,
   including both private data and public methods.  There is a one-to-one
   correspondence between public methods and user-callable functions in the
   C API. */

#include "sys-defines.h"
#include "extern.h"

/* localtime_r() is currently not used, because there is apparently _no_
   universal way of ensuring that it is declared.  On some systems
   (e.g. Red Hat Linux), `#define _POSIX_SOURCE' will do it.  But on other
   systems, doing `#define _POSIX_SOURCE' **removes** the declaration! */
#ifdef HAVE_LOCALTIME_R
#undef HAVE_LOCALTIME_R
#endif

#ifdef MSDOS
#include <unistd.h>		/* for fsync() */
#endif

/* song and dance to define time_t, and declare both time() and localtime() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for time_t on some pre-ANSI Unix systems */
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() on some pre-ANSI Unix systems */
#include <time.h>		/* for localtime() */
#else  /* not TIME_WITH_SYS_TIME, include only one (prefer <sys/time.h>) */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else  /* not HAVE_SYS_TIME_H */
#include <time.h>
#endif /* not HAVE_SYS_TIME_H */
#endif /* not TIME_WITH_SYS_TIME */

/* forward references */
static void build_sdr_from_index (plOutbuf *sdr_buffer, int cgm_encoding, int x);
static void build_sdr_from_string (plOutbuf *sdr_buffer, int cgm_encoding, const char *s, int string_length, bool use_double_quotes);
static void build_sdr_from_ui8s (plOutbuf *sdr_buffer, int cgm_encoding, const int *x, int n);

#ifndef LIBPLOTTER
/* In libplot, this is the initialization for the function-pointer part of
   a CGMPlotter struct. */
const Plotter _pl_c_default_plotter = 
{
  /* initialization (after creation) and termination (before deletion) */
  _pl_c_initialize, _pl_c_terminate,
  /* page manipulation */
  _pl_c_begin_page, _pl_c_erase_page, _pl_c_end_page,
  /* drawing state manipulation */
  _pl_g_push_state, _pl_g_pop_state,
  /* internal path-painting methods (endpath() is a wrapper for the first) */
  _pl_c_paint_path, _pl_c_paint_paths, _pl_g_path_is_flushable, _pl_g_maybe_prepaint_segments,
  /* internal methods for drawing of markers and points */
  _pl_c_paint_marker, _pl_c_paint_point,
  /* internal methods that plot strings in Hershey, non-Hershey fonts */
  _pl_g_paint_text_string_with_escapes, _pl_c_paint_text_string,
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
_pl_c_initialize (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass initialization method */
  _pl_g_initialize (S___(_plotter));
#endif

  /* override generic initializations (which are appropriate to the base
     Plotter class), as necessary */

#ifndef LIBPLOTTER
  /* tag field, differs in derived classes */
  _plotter->data->type = PL_CGM;
#endif

  /* output model */
  _plotter->data->output_model = PL_OUTPUT_PAGES_ALL_AT_ONCE;

  /* user-queryable capabilities: 0/1/2 = no/yes/maybe */
  _plotter->data->have_wide_lines = 1;
  _plotter->data->have_dash_array = 0;
  _plotter->data->have_solid_fill = 1;
  _plotter->data->have_odd_winding_fill = 1;
  _plotter->data->have_nonzero_winding_fill = 0;
  _plotter->data->have_settable_bg = 1;
  _plotter->data->have_escaped_string_support = 0;
  _plotter->data->have_ps_fonts = 1;
  _plotter->data->have_pcl_fonts = 0;
  _plotter->data->have_stick_fonts = 0;
  _plotter->data->have_extra_stick_fonts = 0;
  _plotter->data->have_other_fonts = 0;

  /* text and font-related parameters (internal, not queryable by user);
     note that we don't set kern_stick_fonts, because it was set by the
     superclass initialization (and it's irrelevant for this Plotter type,
     anyway) */
  _plotter->data->default_font_type = PL_F_POSTSCRIPT;
  _plotter->data->pcl_before_ps = false;
  _plotter->data->have_horizontal_justification = true;
  _plotter->data->have_vertical_justification = true;
  _plotter->data->issue_font_warning = true;

  /* path-related parameters (also internal); note that we
     don't set max_unfilled_path_length, because it was set by the
     superclass initialization */
  _plotter->data->have_mixed_paths = false;
  _plotter->data->allowed_arc_scaling = AS_NONE;
  _plotter->data->allowed_ellarc_scaling = AS_NONE;
  _plotter->data->allowed_quad_scaling = AS_NONE;  
  _plotter->data->allowed_cubic_scaling = AS_NONE;  
  _plotter->data->allowed_box_scaling = AS_AXES_PRESERVED;
  _plotter->data->allowed_circle_scaling = AS_UNIFORM;
  _plotter->data->allowed_ellipse_scaling = AS_ANY;

  /* dimensions */
  _plotter->data->display_model_type = (int)DISP_MODEL_VIRTUAL;
  _plotter->data->display_coors_type = (int)DISP_DEVICE_COORS_INTEGER_NON_LIBXMI;
  _plotter->data->flipped_y = false;
      /* we choose viewport coor range to be 1/4 of the integer range */
  _plotter->data->imin = - ((1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) - 1);
  _plotter->data->imax = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) - 1;
  _plotter->data->jmin = - ((1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) - 1);
  _plotter->data->jmax = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) - 1;
  _plotter->data->xmin = 0.0;
  _plotter->data->xmax = 0.0;  
  _plotter->data->ymin = 0.0;
  _plotter->data->ymax = 0.0;  
  _plotter->data->page_data = (plPageData *)NULL;

  /* initialize data members specific to this derived class */
  /* parameters */
  _plotter->cgm_encoding = CGM_ENCODING_BINARY;
  _plotter->cgm_max_version = 4;
  /* most important dynamic variables (global) */
  _plotter->cgm_version = 1;
  _plotter->cgm_profile = CGM_PROFILE_WEB;
  _plotter->cgm_need_color = false;
  /* corresponding dynamic variables (page-specific, i.e. picture-specific) */
  _plotter->cgm_page_version = 1;
  _plotter->cgm_page_profile = CGM_PROFILE_WEB;
  _plotter->cgm_page_need_color = false;
  /* colors (24-bit or 48-bit, initialized to nonphysical or dummy values) */
  _plotter->cgm_line_color.red = -1;
  _plotter->cgm_line_color.green = -1;
  _plotter->cgm_line_color.blue = -1;
  _plotter->cgm_edge_color.red = -1;
  _plotter->cgm_edge_color.green = -1;
  _plotter->cgm_edge_color.blue = -1;
  _plotter->cgm_fillcolor.red = -1;
  _plotter->cgm_fillcolor.green = -1;
  _plotter->cgm_fillcolor.blue = -1;
  _plotter->cgm_marker_color.red = -1;
  _plotter->cgm_marker_color.green = -1;
  _plotter->cgm_marker_color.blue = -1;
  _plotter->cgm_text_color.red = -1;
  _plotter->cgm_text_color.green = -1;
  _plotter->cgm_text_color.blue = -1;
  _plotter->cgm_bgcolor.red = -1; /* set in c_begin_page() */
  _plotter->cgm_bgcolor.green = -1;
  _plotter->cgm_bgcolor.blue = -1;
  /* other dynamic variables */
  _plotter->cgm_line_type = CGM_L_SOLID;
  _plotter->cgm_dash_offset = 0.0;
  _plotter->cgm_join_style = CGM_JOIN_UNSPEC;
  _plotter->cgm_cap_style = CGM_CAP_UNSPEC;  
  _plotter->cgm_dash_cap_style = CGM_CAP_UNSPEC;  
  	/* CGM's default line width: 1/1000 times the max VDC dimension */
  _plotter->cgm_line_width = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) / 500;
  _plotter->cgm_interior_style = CGM_INT_STYLE_HOLLOW;
  _plotter->cgm_edge_type = CGM_L_SOLID;
  _plotter->cgm_edge_dash_offset = 0.0;
  _plotter->cgm_edge_join_style = CGM_JOIN_UNSPEC;
  _plotter->cgm_edge_cap_style = CGM_CAP_UNSPEC;  
  _plotter->cgm_edge_dash_cap_style = CGM_CAP_UNSPEC;  
  	/* CGM's default edge width: 1/1000 times the max VDC dimension */
  _plotter->cgm_edge_width = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) / 500;
  _plotter->cgm_edge_is_visible = false;
  _plotter->cgm_miter_limit = 32767.0;
  _plotter->cgm_marker_type = CGM_M_ASTERISK;
  	/* CGM's default marker size: 1/1000 times the max VDC dimension */
  _plotter->cgm_marker_size = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) /500;
  	/* label-related variables */
  _plotter->cgm_char_height = -1; /* impossible (dummy) value */
  _plotter->cgm_char_base_vector_x = 1;
  _plotter->cgm_char_base_vector_y = 0;
  _plotter->cgm_char_up_vector_x = 0;
  _plotter->cgm_char_up_vector_y = 1;
  _plotter->cgm_horizontal_text_alignment = CGM_ALIGN_NORMAL_HORIZONTAL;
  _plotter->cgm_vertical_text_alignment = CGM_ALIGN_NORMAL_VERTICAL;
  _plotter->cgm_font_id = -1;	/* impossible (dummy) value */
  _plotter->cgm_charset_lower = 0; /* dummy value (we use values 1..4) */
  _plotter->cgm_charset_upper = 0; /* dummy value (we use values 1..4) */
  _plotter->cgm_restricted_text_type = CGM_RESTRICTED_TEXT_TYPE_BASIC;

  /* initialize certain data members from device driver parameters */

  /* determine page type, and viewport size and location */
  _set_page_type (_plotter->data);
  
  /* user may have specified a viewport aspect ratio other than 1:1, so
     carefully compute device-space coordinate ranges (i.e. don't use above
     default values for imin,imax,jmin,jmax, which are appropriate only for
     a square viewport) */
  {
    /* our choice: the larger side of the viewport will essentially be 1/4
       times the maximum range for integer device coordinates, i.e., half
       the larger side will be 1/8 times the maximum range */
    int half_side = (1 << (8*CGM_BINARY_BYTES_PER_INTEGER - 3)) - 1;
    int half_other_side;
    double xsize = _plotter->data->viewport_xsize;
    double ysize = _plotter->data->viewport_ysize;
    int xsign = xsize < 0.0 ? -1 : 1;
    int ysign = ysize < 0.0 ? -1 : 1;
    double fraction;
    
    /* There are two cases, plus a degenerate case.  For each,
       `scaling_factor' is the conversion factor from virtual to physical
       units. */

    if (xsize == 0.0 && ysize == 0.0)
      /* degenerate case, scaling_factor = 0 (or anything else :-)) */
      {
	_plotter->data->imin = 0;
	_plotter->data->imax = 0;
	_plotter->data->jmin = 0;
	_plotter->data->jmax = 0;
      }
    else if (FABS(ysize) > FABS(xsize))
      /* scaling_factor = FABS(ysize) / (2*half_side) */
      {
	fraction = FABS(xsize) / FABS(ysize);
	half_other_side = IROUND(half_side * fraction);
	_plotter->data->imin = - xsign * half_other_side;
	_plotter->data->imax = xsign * half_other_side;
	_plotter->data->jmin = - ysign * half_side;
	_plotter->data->jmax = ysign * half_side;
      }
    else		/* FABS(ysize) <= FABS(xsize), which is nonzero */
      /* scaling_factor = FABS(xsize) / (2*half_side) */
      {
	fraction = FABS(ysize) / FABS(xsize);
	half_other_side = IROUND(half_side * fraction);
	_plotter->data->imin = - xsign * half_side;
	_plotter->data->imax = xsign * half_side;
	_plotter->data->jmin = - ysign * half_other_side;
	_plotter->data->jmax = ysign * half_other_side;
      }
  }

  /* compute the NDC to device-frame affine map, set it in Plotter */
  _compute_ndc_to_device_map (_plotter->data);

  /* determine CGM encoding */
  {
    const char* cgm_encoding_type;
    
    cgm_encoding_type = 
      (const char *)_get_plot_param (_plotter->data, "CGM_ENCODING");
    if (cgm_encoding_type != NULL)
      {
	if (strcmp (cgm_encoding_type, "binary") == 0)
	  _plotter->cgm_encoding = CGM_ENCODING_BINARY;
	else if (strcmp (cgm_encoding_type, "clear text") == 0
		 || (strcmp (cgm_encoding_type, "cleartext") == 0)
		 || (strcmp (cgm_encoding_type, "clear_text") == 0))
	  _plotter->cgm_encoding = CGM_ENCODING_CLEAR_TEXT;
	else			/* we don't support the character encoding */
	  _plotter->cgm_encoding = CGM_ENCODING_BINARY;
      }
    else
      _plotter->cgm_encoding = CGM_ENCODING_BINARY; /* default value */
  }

  /* determine upper bound on CGM version number */
  {
    const char* cgm_max_version_type;
    
    cgm_max_version_type = 
      (const char *)_get_plot_param (_plotter->data, "CGM_MAX_VERSION");
    if (cgm_max_version_type != NULL)
      {
	if (strcmp (cgm_max_version_type, "1") == 0)
	  _plotter->cgm_max_version = 1;
	else if (strcmp (cgm_max_version_type, "2") == 0)
	  _plotter->cgm_max_version = 2;
	else if (strcmp (cgm_max_version_type, "3") == 0)
	  _plotter->cgm_max_version = 3;
	else if (strcmp (cgm_max_version_type, "4") == 0)
	  _plotter->cgm_max_version = 4;
	else			/* use default */
	  _plotter->cgm_max_version = 4;
      }
    else
      _plotter->cgm_max_version = 4; /* use default */
  }

  /* If the maximum CGM version number is greater than 1, relax the
     constraints on what path segments can be stored in libplot's path
     buffer.  By default, we allow only line segments. */
 
  /* Counterclockwise circular arcs have been in the CGM standard since
     version 1, but clockwise circular arcs were only added in version 2.
     To include a circular arc we insist on a uniform map from user to
     device coordinates, since otherwise it wouldn't be mapped to a
     circle.

     Similarly, we don't allow elliptic arcs into the arc buffer unless the
     version is 2 or higher.  Elliptic arcs have been in the standard since
     version 1, but the `closed figure' construction that we use to fill
     single elliptic (and circular!) arcs was only added in version 2. */
  if (_plotter->cgm_max_version >= 2)
    {
      _plotter->data->allowed_arc_scaling = AS_UNIFORM;
      _plotter->data->allowed_ellarc_scaling = AS_ANY;
    }

  /* Bezier cubics were added to the standard in version 3.  Closed mixed
     paths (`closed figures' in CGM jargon) have been in the standard since
     version 2, but open mixed paths (`compound lines' in CGM jargon) were
     only added in version 3. */
  if (_plotter->cgm_max_version >= 3)
    {
      _plotter->data->allowed_cubic_scaling = AS_ANY;
      _plotter->data->have_mixed_paths = true;
    }

  /* Beginning in version 3 CGM's, user can define line types, by
     specifying a precise dashing style. */
  if (_plotter->cgm_max_version >= 3)
    _plotter->data->have_dash_array = 1;
}

/* Lists of metafile elements that we use, indexed by the CGM version less
   unity, i.e., by 0,1,2,3 for CGM versions 1,2,3,4.  We use a standard
   shorthand: element class -1, and element id 0, 1, 2, etc., specifies
   certain sets of metafile elements.  E.g., class=-1 and id=1 specifies
   all the version-1 metafile elements; in the clear text encoding this is
   written as "DRAWINGPLUS". */

#define MAX_CGM_ELEMENT_LIST_LENGTH 1
typedef struct
{
  const char *text_string;
  int length;			/* number of genuine entries in list */
  int class_id[MAX_CGM_ELEMENT_LIST_LENGTH];
  int element_id[MAX_CGM_ELEMENT_LIST_LENGTH];
}
plCGMElementList;

static const plCGMElementList _metafile_element_list[4] =
{
  /* version 1 */
  { "DRAWINGPLUS", 1, {-1}, {1} },
  /* version 2 */
  { "VERSION2",    1, {-1}, {2} },
  /* version 3 */
  { "VERSION3",    1, {-1}, {5} },
  /* version 4 */
  { "VERSION4",    1, {-1}, {6} }
};

/* CGM character sets, for upper and lower halves of both ISO-Latin-1 and
   Symbol fonts.  We use standard 8-bit encoding when writing text strings
   in either sort of font, but the character set used in each font half
   needs to be specified explicitly.  Supported types of character set
   include "standard 94-character set" and "standard 96-character set".
   The character set is further specified by the "tail" string. */

typedef struct
{
  int type;			/* a CGM enumerative */
  const char *type_string;	/* its string representation, for cleartext */
  const char *tail;		/* the `designation sequence tail' */
}
plCGMCharset;

static const plCGMCharset _iso_latin_1_cgm_charset[2] =
{
  { 0, "std94", "4/2" },	/* ISO 8859-1 LH, tail is "A" */
  { 1, "std96", "4/1" }		/* ISO 8859-1 RH, tail is "B" */
};

static const plCGMCharset _symbol_cgm_charset[2] =
{
  { 0, "std94", "2/10 3/10" },	/* Symbol LH, tail is "*:" */
  { 0, "std94", "2/6 3/10" }	/* Symbol RH, tail is "&:" */
};

/* The private `terminate' method, which is invoked when a Plotter is
   deleted.  It may do such things as write to an output stream from
   internal storage, deallocate storage, etc.  When this is invoked,
   _plotter points to the Plotter that is about to be deleted. */

/* This version is for CGM Plotters...

   (CGM Plotters differ from most other plotters that do not plot in real
   time in that they emit output only after all pages have pages have been
   drawn, rather than at the end of each page.  This is necessary in order
   to produce the correct header lines.)

   When this is called, the CGM code for the body of each page is stored in
   a plOutbuf, and the page plOutbufs form a linked list. */

void
_pl_c_terminate (S___(Plotter *_plotter))
{
  int i;
  plOutbuf *current_page;
  bool ps_font_used_in_doc[PL_NUM_PS_FONTS];
  bool symbol_font_used_in_doc;
  bool cgm_font_id_used_in_doc[PL_NUM_PS_FONTS];
  bool doc_uses_fonts;
  int max_cgm_font_id;

  /* if no pages of graphics (i.e. Plotter was never opened), CGM file
     won't contain any pictures, and won't satisfy any standard profile */
  if (_plotter->data->first_page == (plOutbuf *)NULL)
    _plotter->cgm_profile = 
      IMAX(_plotter->cgm_profile, CGM_PROFILE_NONE);

  /* COMMENTED OUT BECAUSE USERS WOULD FIND THIS TOO CONFUSING! */
#if 0
  /* only the binary encoding satisfies the WebCGM profile */
  if (_plotter->cgm_encoding != CGM_ENCODING_BINARY)
    _plotter->cgm_profile = 
      IMAX(_plotter->cgm_profile, CGM_PROFILE_MODEL);
#endif

#ifdef LIBPLOTTER
  if (_plotter->data->outfp || _plotter->data->outstream)
#else
  if (_plotter->data->outfp)
#endif
    /* have an output stream, will emit CGM commands */
    {
      plOutbuf *doc_header, *doc_trailer;
      int byte_count, data_byte_count, data_len, string_length;
      
      doc_header = _new_outbuf ();

      /* emit "BEGIN METAFILE" command */
      {
	const char *string_param;

	string_param = "CGM plot";
	string_length = strlen (string_param);
	data_len = CGM_BINARY_BYTES_PER_STRING(string_length);
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_DELIMITER_ELEMENT, 1,
				  data_len, &byte_count,
				  "BEGMF");
	_cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
			  string_param, 
			  string_length, true,
			  data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* emit "METAFILE VERSION" command */
      {
	data_len = CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 1,
				  data_len, &byte_count,
				  "MFVERSION");
	_cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			   _plotter->cgm_version,
			   data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* emit "METAFILE ELEMENT LIST" command; this is encoding-dependent */

      {
	const plCGMElementList *element_list = 
	  &(_metafile_element_list[_plotter->cgm_version - 1]);
	int length = element_list->length;
	int k;

	/* 1 integer, plus `length' pairs of 2-byte indices */
	data_len =  CGM_BINARY_BYTES_PER_INTEGER + 2 * 2 * length;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 11,
				  data_len, &byte_count,
				  "MFELEMLIST");
	switch (_plotter->cgm_encoding)
	  {
	  case CGM_ENCODING_BINARY:
	  default:
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       length,
			       data_len, &data_byte_count, &byte_count);
	    for (k = 0; k < length; k++)
	      {
		_cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				 element_list->class_id[k],
				 data_len, &data_byte_count, &byte_count);
		_cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				 element_list->element_id[k],
				 data_len, &data_byte_count, &byte_count);
	      }
	    break;
	  case CGM_ENCODING_CHARACTER: /* not supported */
	    break;
	    
	  case CGM_ENCODING_CLEAR_TEXT:
	    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
			      element_list->text_string,
			      (int) strlen (element_list->text_string),
			      true,
			      data_len, &data_byte_count, &byte_count);
	    break;
	  }
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* emit "METAFILE DESCRIPTION" command, including profile string etc. */
      {
	time_t clock;
	const char *profile_string, *profile_edition_string;
	char string_param[254];
	struct tm *local_time_struct_ptr;
#ifdef HAVE_LOCALTIME_R
	struct tm local_time_struct;
#endif

	/* Work out ASCII specification of profile */
	switch (_plotter->cgm_profile)
	  {
	  case CGM_PROFILE_WEB:
	    profile_string = "WebCGM";
	    profile_edition_string = "1.0";
	    break;
	  case CGM_PROFILE_MODEL:
	    profile_string = "Model-Profile";
	    profile_edition_string = "1";
	    break;
	  case CGM_PROFILE_NONE:
	  default:
	    profile_string = "None";
	    profile_edition_string = "0.0"; /* waggish */
	    break;
	  }

	/* Compute an ASCII representation of the current time, in a
	   reentrant way if we're supporting pthreads (i.e. by using
	   localtime_r if it's available). */
	time (&clock);

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
#ifdef HAVE_LOCALTIME_R
	localtime_r (&clock, &local_time_struct);
	local_time_struct_ptr = &local_time_struct;
#else
	local_time_struct_ptr = localtime (&clock);
#endif
#else  /* not HAVE_PTHREAD_H */
	local_time_struct_ptr = localtime (&clock);
#endif /* not HAVE_PTHREAD_H */
#else  /* not PTHREAD_SUPPORT */
	local_time_struct_ptr = localtime (&clock);
#endif /* not PTHREAD_SUPPORT */

	sprintf (string_param,
		 "\"ProfileId:%s\" \"ProfileEd:%s\" \"ColourClass:%s\" \"Source:GNU libplot %s\" \"Date:%04d%02d%02d\"", 
		 profile_string, profile_edition_string,
		 _plotter->cgm_need_color ? "colour" : "monochrome",
		 PL_LIBPLOT_VER_STRING,
		 1900 + local_time_struct_ptr->tm_year,
		 1 + local_time_struct_ptr->tm_mon,
		 local_time_struct_ptr->tm_mday);
	
	string_length = strlen (string_param);
	data_len = CGM_BINARY_BYTES_PER_STRING(string_length);
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 2,
				  data_len, &byte_count,
				  "MFDESC");
	_cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
			  string_param, 
			  string_length, false,	/* delimit by single quotes */
			  data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* emit "VDC TYPE" command, selecting integer VDC's for the metafile */
      {
	data_len = 2;		/* 2 bytes per enum */
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 3,
				  data_len, &byte_count,
				  "VDCTYPE");
	_cgm_emit_enum (doc_header, false, _plotter->cgm_encoding,
			0,
			data_len, &data_byte_count, &byte_count,
			"integer");
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* Emit "INTEGER PRECISION" command.  Parameters are
	 encoding-dependent: in the binary encoding, the number of bits k,
	 and in clear text, a pair of integers: the minimum and maximum
	 integers representable in CGM format, i.e. -(2^(k-1) - 1) and
	 (2^(k-1) - 1), where k=8*CGM_BINARY_BYTES_PER_INTEGER. */
      {
	int j, max_int;
	
	data_len = 2;		/* in binary, 16 bits of data; see comment */
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 4,
				  data_len, &byte_count,
				  "INTEGERPREC");
	switch (_plotter->cgm_encoding)
	  {
	  case CGM_ENCODING_BINARY:
	  default:

	    /* The integer precision, in terms of bits, should be encoded
	       as an integer at the current precision (the default, which
	       is 16 bits), not the eventual precision.  So we don't call
	       _cgm_emit_integer; we call _cgm_emit_index instead.  We
	       always represent indices by 16 bits (the default). */

	    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
			     8 * CGM_BINARY_BYTES_PER_INTEGER,
			     data_len, &data_byte_count, &byte_count);
	    break;
	  case CGM_ENCODING_CHARACTER: /* not supported */
	    break;
	    
	  case CGM_ENCODING_CLEAR_TEXT:
	    max_int = 0;
	    for (j = 0; j < (8 * CGM_BINARY_BYTES_PER_INTEGER - 1); j++)
	      max_int += (1 << j);
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       -max_int,
			       data_len, &data_byte_count, &byte_count);
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       max_int,
			       data_len, &data_byte_count, &byte_count);
	    break;
	  }
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* Emit "REAL PRECISION" command, selecting default precision.

	 Parameters are encoding-dependent.  In the clear text encoding,
	 three numbers: the minimum real, the maximum real, and the number
	 of significant decimal digits (an integer).  Typical choices are
	 (-32767.0, 32767.0, 4) [the default].  In the binary encoding,
	 three objects: a 2-octet enumerative specifying the encoding of
	 reals (0=floating, 1=fixed), and two integers at current intege
	 precision specifying the size of each piece of the encoded real.
	 Typical choices are (1,16,16) [the default] or (0,9,23). */
      {
	data_len = 2 + 2 * CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 5,
				  data_len, &byte_count,
				  "REALPREC");
	switch (_plotter->cgm_encoding)
	  {
	  case CGM_ENCODING_BINARY:
	  default:
	    _cgm_emit_enum (doc_header, false, _plotter->cgm_encoding,
			    1,
			    data_len, &data_byte_count, &byte_count,
			    "DUMMY");
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       16,
			       data_len, &data_byte_count, &byte_count);
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       16,
			       data_len, &data_byte_count, &byte_count);
	    break;
	  case CGM_ENCODING_CHARACTER: /* not supported */
	    break;
	    
	  case CGM_ENCODING_CLEAR_TEXT:
	    _cgm_emit_real_fixed_point (doc_header, false, _plotter->cgm_encoding,
					-32767.0,
					data_len, &data_byte_count, &byte_count);
	    _cgm_emit_real_fixed_point (doc_header, false, _plotter->cgm_encoding,
					32767.0,
					data_len, &data_byte_count, &byte_count);
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       4,
			       data_len, &data_byte_count, &byte_count);
	    break;
	  }
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* Emit "COLOR PRECISION" command.  Parameters are
	 encoding-dependent: in the binary encoding, an integer specifying
	 the number of bits k, and in clear text, the maximum possible
	 color component value, i.e. (2^k - 1), where
	 k=8*CGM_BINARY_BYTES_PER_COLOR_COMPONENT. */
      {
	int j;
	unsigned int max_component;
	
	data_len = CGM_BINARY_BYTES_PER_INTEGER;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 7,
				  data_len, &byte_count,
				  "COLRPREC");
	switch (_plotter->cgm_encoding)
	  {
	  case CGM_ENCODING_BINARY:
	  default:
	    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
			       8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT,
			       data_len, &data_byte_count, &byte_count);
	    break;
	  case CGM_ENCODING_CHARACTER: /* not supported */
	    break;
	      
	  case CGM_ENCODING_CLEAR_TEXT:
	    max_component = 0;
	    for (j = 0; j < (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT); j++)
	      max_component += (1 << j);
	    _cgm_emit_unsigned_integer (doc_header, false, _plotter->cgm_encoding,
					max_component,
					data_len, &data_byte_count, &byte_count);
	    break;
	  }
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* emit "COLOR VALUE EXTENT" command, duplicating the information
	 we just supplied (this is necessary) */
      {
	int j;
	unsigned int max_component;

	data_len = 6 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 10,
				  data_len, &byte_count,
				  "COLRVALUEEXT");
	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   (unsigned int)0,
				   data_len, &data_byte_count, &byte_count);
	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   (unsigned int)0,
				   data_len, &data_byte_count, &byte_count);
	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   (unsigned int)0,
				   data_len, &data_byte_count, &byte_count);
	max_component = 0;
	for (j = 0; j < (8 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT); j++)
	  max_component += (1 << j);

	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   max_component,
				   data_len, &data_byte_count, &byte_count);
	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   max_component,
				   data_len, &data_byte_count, &byte_count);
	_cgm_emit_color_component (doc_header, false, _plotter->cgm_encoding,
				   max_component,
				   data_len, &data_byte_count, &byte_count);
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }

      /* determine fonts needed by document, by examining all pages */
      {
	current_page = _plotter->data->first_page;
	
	for (i = 0; i < PL_NUM_PS_FONTS; i++)
	  ps_font_used_in_doc[i] = false;
	while (current_page)
	  {
	    for (i = 0; i < PL_NUM_PS_FONTS; i++)
	      if (current_page->ps_font_used[i])
		ps_font_used_in_doc[i] = true;
	    current_page = current_page->next;
	  }
      }

      /* Map our internal indexing of PS fonts to the indexing we use in a
	 CGM file (which may be different, because we want the traditional
	 `Adobe 13' to come first, out of the `Adobe 35').  Also work out
	 whether Symbol font, which has its own character sets, is used. */
      symbol_font_used_in_doc = false;
      for (i = 0; i < PL_NUM_PS_FONTS; i++)
	{
	  cgm_font_id_used_in_doc[_pl_g_ps_font_to_cgm_font_id[i]] = ps_font_used_in_doc[i];
	  if (ps_font_used_in_doc[i] 
	      && strcmp (_pl_g_ps_font_info[i].ps_name, "Symbol") == 0)
	    symbol_font_used_in_doc = true;
	}
      
      /* compute maximum used font id, if any */
      max_cgm_font_id = 0;
      doc_uses_fonts = false;
      for (i = 0; i < PL_NUM_PS_FONTS; i++)
	{
	  if (cgm_font_id_used_in_doc[i] == true)
	    {
	      doc_uses_fonts = true;
	      max_cgm_font_id = i;
	    }
	}

      if (doc_uses_fonts)
	{
	  /* emit "FONT LIST" command */

	  /* command will include encoded strings, which are the names of
	     fonts in range 0..max_cgm_font_id; later in the CGM file,
	     they'll be referred to as 1..max_cgm_font_id+1 */
	  data_len = 0;
	  for (i = 0; i <= max_cgm_font_id; i++)
	    {
	      int ps_font_index;
	      int font_name_length, encoded_font_name_length;

	      ps_font_index = _pl_g_cgm_font_id_to_ps_font[i];
	      font_name_length = (int) strlen (_pl_g_ps_font_info[ps_font_index].ps_name);
	      encoded_font_name_length = 
		CGM_BINARY_BYTES_PER_STRING(font_name_length);
	      data_len += encoded_font_name_length;
	    }
	  byte_count = data_byte_count = 0;

	  _cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				    CGM_METAFILE_DESCRIPTOR_ELEMENT, 13,
				    data_len, &byte_count,
				    "FONTLIST");
	  for (i = 0; i <= max_cgm_font_id; i++)
	    {
	      int ps_font_index;

	      ps_font_index = _pl_g_cgm_font_id_to_ps_font[i];
	      _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				_pl_g_ps_font_info[ps_font_index].ps_name,
				(int) strlen (_pl_g_ps_font_info[ps_font_index].ps_name),
				true,
				data_len, &data_byte_count, &byte_count);
	    }
	  _cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
					&byte_count);
	  
	  if (_plotter->cgm_version >= 3)
	    /* emit version-3 "FONT PROPERTIES" commands; note that if
	       fonts are used and CGM_MAX_VERSION is >=3, then cgm_version
	       was previously bumped up to 3 in c_closepl.c */
	    {
	      /* For each font in the font list, we specify 7 properties.
		 Each "FONT PROPERTIES" command refers to a single font.
		 Its argument list is a sequence of 3-tuples, each being of
		 the form (property type [an index], priority [an integer],
		 value), where `value' is an SDR (structured data record).
		 One of the supported property types is
		 CGM_FONT_PROP_INDEX, the value of which is an index into
		 the font list.  For any invocation of the command, this
		 property type and its value must be supplied.

		 An SDR is a string-encoded structure, and each `run' of a
		 single CGM datatype within an SDR is encoded as (A) an
		 identifier for the datatype [an index], (B) a count of the
		 number of occurrences [an integer], and (C) the
		 occurrences of the datatype, themselves.  So in the binary
		 encoding, bytes per SDR equals
		 2 + bytes_per_integer + data_bytes, since we always encode
		 CGM indices as 2 bytes.

		 The only SDR's that occur in this context are
		 (1) single CGM indices [used for 5 of the 7 font properties],
		 (2) single CGM strings [used for the `family' property], and 
		 (3) three 8-bit unsigned integers [used for the font's
		 `design group'].

		 Because we always encode CGM indices as 2 bytes, bytes
		 per SDR, in the binary encoding, in these 3 cases are:
		 (1) 1+ CGM_BINARY_BYTES_PER_INTEGER + 4,
		 (2) 1+ CGM_BINARY_BYTES_PER_INTEGER + 2 + CGM_BYTES_PER_STRING
		 (3) 1+ CGM_BINARY_BYTES_PER_INTEGER + 5.
		 (This takes account of the initial byte used for the
		 string encoding of the SDR.)
		 
		 And bytes per 3-tuple in these three cases are:
		 (1) 2*CGM_BINARY_BYTES_PER_INTEGER + 7,
		 (2) 2*CGM_BINARY_BYTES_PER_INTEGER + 5 + CGM_BYTES_PER_STRING
		 (3) 2*CGM_BINARY_BYTES_PER_INTEGER + 8.
		 Since for every included font, we emit 5 3-tuples of type 1,
		 1 of type 2, and 1 of type 3, bytes per emitted font equals
		 14*CGM_BINARY_BYTES_PER_INTEGER + 48 + CGM_BYTES_PER_STRING().

		 In the binary encoding, this is the length, in bytes, of
		 the argument list of each "FONT PROPERTIES" command.  Here
		 CGM_BYTES_PER_STRING() stands for the string-encoded
		 length of the family name of the font. */

	      for (i = 0; i <= max_cgm_font_id; i++)
		{
		  int family_length;
		  plOutbuf *sdr_buffer;

		  family_length = strlen(_pl_g_cgm_font_properties[i].family);
		  data_len = (14 * CGM_BINARY_BYTES_PER_INTEGER 
			      + 48 /* hardcoded constants; see above */
			      + CGM_BINARY_BYTES_PER_STRING(family_length));
		  byte_count = data_byte_count = 0;

		  sdr_buffer = _new_outbuf ();
		  _cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
					    CGM_METAFILE_DESCRIPTOR_ELEMENT, 21,
					    data_len, &byte_count,
					    "FONTPROP");

		  /* now emit a sequence of 3-tuples: (index, integer, SDR);
		     for each 2nd element (a priority), we just specify `1' */

		  /* specify index of font in table (beginning with 1, not
                     with 0) */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_INDEX,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1, /* priority */
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_index (sdr_buffer, _plotter->cgm_encoding,
					   i + 1); /* add 1 to index */
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
	      
		  /* specify font family */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_FAMILY,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_string (sdr_buffer, _plotter->cgm_encoding,
					    _pl_g_cgm_font_properties[i].family,
					    (int)(strlen (_pl_g_cgm_font_properties[i].family)),
					    true); /* use double quotes */
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
	      
		  /* specify font posture */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_POSTURE,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_index (sdr_buffer, _plotter->cgm_encoding,
					   _pl_g_cgm_font_properties[i].posture);
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
	      
		  /* specify font weight */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_WEIGHT,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_index (sdr_buffer, _plotter->cgm_encoding,
					   _pl_g_cgm_font_properties[i].weight);
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
	      
		  /* specify font width */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_WIDTH,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_index (sdr_buffer, _plotter->cgm_encoding,
					   _pl_g_cgm_font_properties[i].proportionate_width);
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
		
		  /* specify font design group */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_DESIGN_GROUP,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_ui8s (sdr_buffer, _plotter->cgm_encoding,
					  _pl_g_cgm_font_properties[i].design_group,
					  3);
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }
	      
		  /* specify font structure */
		  {
		    _cgm_emit_index (doc_header, false, _plotter->cgm_encoding,
				     CGM_FONT_PROP_STRUCTURE,
				     data_len, &data_byte_count, &byte_count);
		    _cgm_emit_integer (doc_header, false, _plotter->cgm_encoding,
				       1,
				       data_len, &data_byte_count, &byte_count);
		    build_sdr_from_index (sdr_buffer, _plotter->cgm_encoding,
					   _pl_g_cgm_font_properties[i].structure);
		    _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				      sdr_buffer->base,
				      (int)(sdr_buffer->contents),
				      false,
				      data_len, &data_byte_count, &byte_count);
		    _reset_outbuf (sdr_buffer);
		  }

		  _cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
						&byte_count);
		  _delete_outbuf (sdr_buffer);
		}
	    }

	  /* Emit a "CHARACTER SET LIST" command.  Argument list is a
	     sequence of character sets, with each character set being
	     expressed both as a CGM enumerative and a CGM string (the
	     `designation sequence tail').

	     We include the 2 character sets used in the 8-bit ISO-Latin-1
	     encoding, and, if we're using the Symbol font, the two
	     character sets that Symbol uses, also.  So internally, we
	     index these character sets by 1,2,3,4 (the latter two may not
	     be present). */

	  data_len = 0;
	  for (i = 0; i < 2; i++)
	    {
	      int tail_length, encoded_tail_length;
	      
	      data_len += 2;	/* 2 bytes per enum */
	      tail_length = strlen (_iso_latin_1_cgm_charset[i].tail);
	      encoded_tail_length = 
		CGM_BINARY_BYTES_PER_STRING(tail_length);
	      data_len += encoded_tail_length;
	    }
	  if (symbol_font_used_in_doc)
	    for (i = 0; i < 2; i++)
	      {
		int tail_length, encoded_tail_length;
		
		data_len += 2;	/* 2 bytes per enum */
		tail_length = (int) strlen (_symbol_cgm_charset[i].tail);
		encoded_tail_length = 
		  CGM_BINARY_BYTES_PER_STRING(tail_length);
		data_len += encoded_tail_length;
	      }
	  byte_count = data_byte_count = 0;
	  
	  _cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				    CGM_METAFILE_DESCRIPTOR_ELEMENT, 14,
				    data_len, &byte_count,
				    "CHARSETLIST");
	  for (i = 0; i < 2; i++)
	    {
	      _cgm_emit_enum (doc_header, false, _plotter->cgm_encoding,
			      _iso_latin_1_cgm_charset[i].type,
			      data_len, &data_byte_count, &byte_count,
			      _iso_latin_1_cgm_charset[i].type_string);
	      _cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
				_iso_latin_1_cgm_charset[i].tail,
				(int) strlen (_iso_latin_1_cgm_charset[i].tail),
				true,
				data_len, &data_byte_count, &byte_count);
	    }
	  if (symbol_font_used_in_doc)
	    for (i = 0; i < 2; i++)
	      {
		_cgm_emit_enum (doc_header, false, _plotter->cgm_encoding,
				_symbol_cgm_charset[i].type,
				data_len, &data_byte_count, &byte_count,
				_symbol_cgm_charset[i].type_string);
		_cgm_emit_string (doc_header, false, _plotter->cgm_encoding,
  				  _symbol_cgm_charset[i].tail,
				  (int) strlen (_symbol_cgm_charset[i].tail),
				  true,
				  data_len, &data_byte_count, &byte_count);
	      }
	  _cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
					&byte_count);
	}

      /* emit "CHARACTER CODING ANNOUNCER" command, selecting 8-bit
	 character codes (no switching between font halves for us!) */
      {
	data_len = 2; /* 2 bytes per enum */
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_header, _plotter->cgm_encoding,
				  CGM_METAFILE_DESCRIPTOR_ELEMENT, 15,
				  data_len, &byte_count,
				  "CHARCODING");
	_cgm_emit_enum (doc_header, false, _plotter->cgm_encoding,
			1,
			data_len, &data_byte_count, &byte_count,
			"basic8bit");
	_cgm_emit_command_terminator (doc_header, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* WRITE DOCUMENT HEADER */
      _write_bytes (_plotter->data, 
			     (int)(doc_header->contents),
			     (unsigned char *)doc_header->base);
      _delete_outbuf (doc_header);

      /* loop over plOutbufs in which successive pages of graphics are
	 stored; emit each page as a CGM picture, and delete each plOutbuf
	 as we finish with it */
      current_page = _plotter->data->first_page;
      i = 1;

      while (current_page)
	{
	  plOutbuf *next_page;
	  plOutbuf *current_page_header, *current_page_trailer;
      
	  /* prepare a page header */

	  current_page_header = _new_outbuf ();

	  /* emit "BEGIN PICTURE" command */
	  {
	    char picture[32];
	    const char *string_param;
	    
	    sprintf (picture, "picture_%d", i);
	    string_param = picture;
	    string_length = strlen (string_param);
	    data_len = CGM_BINARY_BYTES_PER_STRING(string_length);
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_DELIMITER_ELEMENT, 3,
				      data_len, &byte_count,
				      "BEGPIC");
	    _cgm_emit_string (current_page_header, false, _plotter->cgm_encoding,
			      string_param,
			      string_length,
			      true,
			      data_len, &data_byte_count, &byte_count);
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
	  
	  /* emit "VDC EXTENT" command [specify virtual device coor ranges] */
	  {
	    int imin_true, imax_true, jmin_true, jmax_true;

	    data_len = 2 * 2 * 2;
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 6,
				      data_len, &byte_count,
				      "VDCEXT");

	    /* To see how we set extent values, see the initialize()
	       routine.  In that routine, we choose device-coordinates
	       ranges imin,imax,jmin,jmax so that the length of the longer
	       side of the viewport is one-fourth the maximum integer
	       range.  The other side will be reduced if the user specifies
	       (via PAGESIZE) an aspect ratio other than 1:1.  With this
	       scheme, if the user specifies xsize=ysize > 0, the same
	       user->device coordinate map will result, no matter what
	       xsize and ysize equal.

	       However, if the user specifies (via PAGESIZE) a negative
	       xsize or ysize, we flip the sign of imax-imin or jmax-jmin,
	       thereby greatly modifying the user->device coordinate map.
	       Which means we must flip it back, right here, before
	       emitting the VDC extents.  The reason for this sign-flipping
	       is that we don't trust CGM viewers to handle negative VDC
	       extents. */

	    if (_plotter->data->imax < _plotter->data->imin)
	      {
		imin_true = _plotter->data->imax;
		imax_true = _plotter->data->imin;
	      }
	    else
	      {
		imin_true = _plotter->data->imin;
		imax_true = _plotter->data->imax;
	      }

	    if (_plotter->data->jmax < _plotter->data->jmin)
	      {
		jmin_true = _plotter->data->jmax;
		jmax_true = _plotter->data->jmin;
	      }
	    else
	      {
		jmin_true = _plotter->data->jmin;
		jmax_true = _plotter->data->jmax;
	      }

	    /* In binary, we write each of these four coordinates as a
	       16-bit `index' i.e. integer, because we haven't yet changed
	       the VDC integer precision to our desired value (we can't do
	       that until the beginning of the picture). */
	    _cgm_emit_index (current_page_header, false, _plotter->cgm_encoding,
			     imin_true,
			     data_len, &data_byte_count, &byte_count);
	    _cgm_emit_index (current_page_header, false, _plotter->cgm_encoding,
			     jmin_true,
			     data_len, &data_byte_count, &byte_count);
	    _cgm_emit_index (current_page_header, false, _plotter->cgm_encoding,
			     imax_true,
			     data_len, &data_byte_count, &byte_count);
	    _cgm_emit_index (current_page_header, false, _plotter->cgm_encoding,
			     jmax_true,
			     data_len, &data_byte_count, &byte_count);
	    _cgm_emit_command_terminator (current_page_header, 
					  _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  /* emit "SCALING MODE" command.  Specify metric scaling (required
	   by WebCGM profile).  The argument is the number of millimeters
	   per VDC unit; it must be a floating-point real.  */
	  {
	    int irange, jrange;
	    double scaling_factor;

	    data_len = 6;	/* 2 bytes per enum, 4 per floating-pt. real */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 1,
				      data_len, &byte_count,
				      "SCALEMODE");
	    _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			     1,
			     data_len, &data_byte_count, &byte_count,
			     "metric");

	    /* Compute a metric scaling factor from the criterion that the
	       nominal physical width and height of VDC space be the
	       viewport xsize and ysize, as determined by the PAGESIZE
	       parameter.  

	       We can get this scaling factor easily, by computing a
	       quotient, because our scheme for setting imin,imax,jmin,jmax
	       in the initialize() method preserves aspect ratio, and
	       signs, as well (see comment immediately above).  But we must
	       be careful not to divide by zero, since zero-width and
	       zero-height viewports are allowed. */

	    irange = _plotter->data->imax - _plotter->data->imin;
	    jrange = _plotter->data->jmax - _plotter->data->jmin;
	    if (irange != 0)
	      scaling_factor = 
		(25.4 * _plotter->data->viewport_xsize) / irange;
	    else if (jrange != 0)
	      scaling_factor = 
		(25.4 * _plotter->data->viewport_ysize) / jrange;
	    else
	      /* degenerate case, viewport has zero size */
	      scaling_factor = 0.0;

	    /* yes, this needs to be a floating-point real, not fixed-point! */
	    _cgm_emit_real_floating_point (current_page_header, false, _plotter->cgm_encoding,
					   scaling_factor,
					   data_len, &data_byte_count, &byte_count);
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  /* emit "LINE WIDTH SPECIFICATION MODE" command [specify
	     absolute coordinates] */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 3,
				      data_len, &byte_count,
				      "LINEWIDTHMODE");
	    _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			     0,
			     data_len, &data_byte_count, &byte_count,
			     "abs");
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  /* emit "EDGE WIDTH SPECIFICATION MODE" command [specify absolute
             coordinates] */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 5,
				      data_len, &byte_count,
				      "EDGEWIDTHMODE");
	    _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			     0,
			     data_len, &data_byte_count, &byte_count,
			     "abs");
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  /* emit "MARKER SIZE SPECIFICATION MODE" command [specify
             absolute coordinates] */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 4,
				      data_len, &byte_count,
				      "MARKERSIZEMODE");
	    _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			     0,
			     data_len, &data_byte_count, &byte_count,
			     "abs");
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  /* emit "COLOR SELECTION MODE" command [specify direct color,
	     not indexed color] */
	  {
	    data_len = 2;	/* 2 bytes per enum */
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_PICTURE_DESCRIPTOR_ELEMENT, 2,
				      data_len, &byte_count,
				      "COLRMODE");
	    _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			     1,
			     data_len, &data_byte_count, &byte_count,
			     "direct");
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  if (current_page->bg_color_suppressed == false)
	    /* user didn't specify "none" as background color, so emit
	       "BACKGROUND COLOR" command.  (Note that in a CGM file,
	       background color is always a direct color specified by color
	       components, never an indexed color.)  The background color
	       for any page is stored in the `bg_color' element of its
	       plOutbuf at the time the page is closed; see g_closepl.c.  */
	    {
	      data_len = 3 * CGM_BINARY_BYTES_PER_COLOR_COMPONENT;
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
					CGM_PICTURE_DESCRIPTOR_ELEMENT, 7,
					data_len, &byte_count,
					"BACKCOLR");
	      _cgm_emit_color_component (current_page_header, false, _plotter->cgm_encoding,
					 (unsigned int)current_page->bg_color.red,
					 data_len, &data_byte_count, &byte_count);
	      _cgm_emit_color_component (current_page_header, false, _plotter->cgm_encoding,
					 (unsigned int)current_page->bg_color.green,
					 data_len, &data_byte_count, &byte_count);
	      _cgm_emit_color_component (current_page_header, false, _plotter->cgm_encoding,
					 (unsigned int)current_page->bg_color.blue,
					 data_len, &data_byte_count, &byte_count);
	      _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					    &byte_count);
	    }
	  
	  /* if user defined any line types, emit a sequence of "LINE AND
             EDGE TYPE DEFINITION" commands */
	  {
	    plCGMCustomLineType *linetype_ptr = (plCGMCustomLineType *)current_page->extra;
	    int linetype = 0;
	      
	    while (linetype_ptr)
	      {
		int k, cycle_length, dash_array_len, *dash_array;

		linetype--;	/* user-defined ones are -1,-2,-3,... */
		dash_array_len = linetype_ptr->dash_array_len;
		dash_array = linetype_ptr->dashes;
		cycle_length = 0;
		for (k = 0; k < dash_array_len; k++)
		  cycle_length += dash_array[k];
		
		/* data: a 2-byte index, the cycle length, and the array of
		   dash lengths (all integers) */
		data_len = 2 + (1 + dash_array_len) * CGM_BINARY_BYTES_PER_INTEGER;
		byte_count = data_byte_count = 0;
		_cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
					  CGM_PICTURE_DESCRIPTOR_ELEMENT, 17,
					  data_len, &byte_count,
					  "LINEEDGETYPEDEF");
		_cgm_emit_index (current_page_header, false, _plotter->cgm_encoding,
				 linetype,
				 data_len, &data_byte_count, &byte_count);
		_cgm_emit_integer (current_page_header, false, _plotter->cgm_encoding,
				   cycle_length,
				   data_len, &data_byte_count, &byte_count);
		for (k = 0; k < dash_array_len; k++)
		  _cgm_emit_integer (current_page_header, false, _plotter->cgm_encoding,
				     dash_array[k],
				     data_len, &data_byte_count, &byte_count);
		_cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					      &byte_count);

		/* on to next user-defined line type */
		linetype_ptr = linetype_ptr->next;
	      }
	  }
      
	  /* emit "BEGIN PICTURE BODY" command */
	  {
	    data_len = 0;
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_DELIMITER_ELEMENT, 4,
				      data_len, &byte_count,
				      "BEGPICBODY");
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
	  
	  /* Emit "VDC INTEGER PRECISION" command.  Very similar to the
	     "INTEGER PRECISION" command, except we emit this at the start
	     of each picture. */
	  {
	    int j, max_int;
	
	    data_len = CGM_BINARY_BYTES_PER_INTEGER;
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
				      CGM_CONTROL_ELEMENT, 1,
				      data_len, &byte_count,
				      "VDCINTEGERPREC");
	    switch (_plotter->cgm_encoding)
	      {
	      case CGM_ENCODING_BINARY:
	      default:
		_cgm_emit_integer (current_page_header, false, _plotter->cgm_encoding,
				   8 * CGM_BINARY_BYTES_PER_INTEGER,
				   data_len, &data_byte_count, &byte_count);
		break;
	      case CGM_ENCODING_CHARACTER: /* not supported */
		break;
	    
	      case CGM_ENCODING_CLEAR_TEXT:
		max_int = 0;
		for (j = 0; j < (8 * CGM_BINARY_BYTES_PER_INTEGER - 1); j++)
		  max_int += (1 << j);
		_cgm_emit_integer (current_page_header, false, _plotter->cgm_encoding,
				   -max_int,
				   data_len, &data_byte_count, &byte_count);
		_cgm_emit_integer (current_page_header, false, _plotter->cgm_encoding,
				   max_int,
				   data_len, &data_byte_count, &byte_count);
		break;
	      }
	    _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					  &byte_count);
	  }
      
	  if (doc_uses_fonts)
	    /* emit "TEXT PRECISION" command */
	    {
	      data_len = 2;	/* 2 bytes per enum */
	      byte_count = data_byte_count = 0;
	      _cgm_emit_command_header (current_page_header, _plotter->cgm_encoding,
					CGM_ATTRIBUTE_ELEMENT, 11,
					data_len, &byte_count,
					"TEXTPREC");
	      _cgm_emit_enum (current_page_header, false, _plotter->cgm_encoding,
			      2,
			      data_len, &data_byte_count, &byte_count,
			      "stroke");
	      _cgm_emit_command_terminator (current_page_header, _plotter->cgm_encoding,
					    &byte_count);
	    }
      
	  /* write the page header */
	  _write_bytes (_plotter->data, 
				 (int)(current_page_header->contents),
				 (unsigned char *)current_page_header->base);
	  _delete_outbuf (current_page_header);

	  /* WRITE THE PICTURE */
	  _write_bytes (_plotter->data, 
				 (int)(current_page->contents),
				 (unsigned char *)current_page->base);

	  /* prepare a page trailer */

	  current_page_trailer = _new_outbuf ();

	  /* emit "END PICTURE" command (no parameters) */
	  {
	    data_len = 0;
	    byte_count = data_byte_count = 0;
	    _cgm_emit_command_header (current_page_trailer, _plotter->cgm_encoding,
				      CGM_DELIMITER_ELEMENT, 5,
				      data_len, &byte_count,
				      "ENDPIC");
	    _cgm_emit_command_terminator (current_page_trailer, _plotter->cgm_encoding,
					  &byte_count);
	  }

	  /* write page trailer */
	  _write_bytes (_plotter->data, 
				 (int)(current_page_trailer->contents),
				 (unsigned char *)current_page_trailer->base);
	  _delete_outbuf (current_page_trailer);
	  
	  /* on to next page (if any) */
	  next_page = current_page->next;
	  current_page = next_page;
	  i++;
	}

      /* prepare a document trailer */

      doc_trailer = _new_outbuf ();

      /* emit "END METAFILE" command (no parameters) */
      {
	data_len = 0;
	byte_count = data_byte_count = 0;
	_cgm_emit_command_header (doc_trailer, _plotter->cgm_encoding,
				  CGM_DELIMITER_ELEMENT, 2,
				  data_len, &byte_count,
				  "ENDMF");
	_cgm_emit_command_terminator (doc_trailer, _plotter->cgm_encoding,
				      &byte_count);
      }
      
      /* WRITE DOCUMENT TRAILER */
      _write_bytes (_plotter->data, 
			     (int)(doc_trailer->contents),
			     (unsigned char *)doc_trailer->base);
      _delete_outbuf (doc_trailer);

    }
  
  /* delete all plOutbufs in which document pages are stored */
  current_page = _plotter->data->first_page;
  while (current_page)
    {
      plOutbuf *next_page;
	  
      next_page = current_page->next;

      /* deallocate page-specific table of user-specified line types, 
	 if any */
      if (current_page->extra)
	{
	  plCGMCustomLineType *linetype_ptr = (plCGMCustomLineType *)current_page->extra;
	  plCGMCustomLineType *old_linetype_ptr;
	  
	  while (linetype_ptr)
	    {
	      if (linetype_ptr->dash_array_len > 0 /* paranoia */
		  && linetype_ptr->dashes)
		free (linetype_ptr->dashes);
	      old_linetype_ptr = linetype_ptr;
	      linetype_ptr = linetype_ptr->next;
	      free (old_linetype_ptr);
	    }
	  _plotter->data->page->extra = (void *)NULL;
	}

      _delete_outbuf (current_page);
      current_page = next_page;
    }
  
  /* flush output stream if any */
  if (_plotter->data->outfp)
    {
      if (fflush(_plotter->data->outfp) < 0
#ifdef MSDOS
	  /* data can be caught in DOS buffers, so do an fsync() too */
	  || fsync (_plotter->data->outfp) < 0
#endif
	  )
	_plotter->error (R___(_plotter) "the output stream is jammed");
    }
#ifdef LIBPLOTTER
  else if (_plotter->data->outstream)
    {
      _plotter->data->outstream->flush ();
      if (!(*(_plotter->data->outstream)))
	_plotter->error (R___(_plotter) "the output stream is jammed");
    }
#endif

#ifndef LIBPLOTTER
  /* in libplot, manually invoke superclass termination method */
  _pl_g_terminate (S___(_plotter));
#endif
}

static void
build_sdr_from_index (plOutbuf *sdr_buffer, int cgm_encoding, int x)
{
  int dummy_data_len, dummy_data_byte_count, dummy_byte_count;

  dummy_data_len = dummy_data_byte_count = dummy_byte_count = 0;
  _cgm_emit_index (sdr_buffer, true, cgm_encoding,
		   CGM_SDR_DATATYPE_INDEX,
		   dummy_data_len, &dummy_data_byte_count, 
		   &dummy_byte_count);
  _cgm_emit_integer (sdr_buffer, true, cgm_encoding,
		     1,
		     dummy_data_len, &dummy_data_byte_count, 
		     &dummy_byte_count);
  _cgm_emit_index (sdr_buffer, true, cgm_encoding,
		   x,
		   dummy_data_len, &dummy_data_byte_count, 
		   &dummy_byte_count);
}

static void
build_sdr_from_string (plOutbuf *sdr_buffer, int cgm_encoding, const char *s, int string_length, bool use_double_quotes)
{
  int dummy_data_len, dummy_data_byte_count, dummy_byte_count;

  dummy_data_len = dummy_data_byte_count = dummy_byte_count = 0;
  _cgm_emit_index (sdr_buffer, true, cgm_encoding,
		   CGM_SDR_DATATYPE_STRING_FIXED,
		   dummy_data_len, &dummy_data_byte_count, 
		   &dummy_byte_count);
  _cgm_emit_integer (sdr_buffer, true, cgm_encoding,
		     1,
		     dummy_data_len, &dummy_data_byte_count, 
		     &dummy_byte_count);
  _cgm_emit_string (sdr_buffer, true, cgm_encoding,
		    s, string_length, use_double_quotes,
		    dummy_data_len, &dummy_data_byte_count, 
		    &dummy_byte_count);
}

static void
build_sdr_from_ui8s (plOutbuf *sdr_buffer, int cgm_encoding, const int *x, int n)
{
  int i, dummy_data_len, dummy_data_byte_count, dummy_byte_count;

  dummy_data_len = dummy_data_byte_count = dummy_byte_count = 0;
  _cgm_emit_index (sdr_buffer, true, cgm_encoding,
		   CGM_SDR_DATATYPE_UNSIGNED_INTEGER_8BIT,
		   dummy_data_len, &dummy_data_byte_count, 
		   &dummy_byte_count);
  _cgm_emit_integer (sdr_buffer, true, cgm_encoding,
		     n,
		     dummy_data_len, &dummy_data_byte_count, 
		     &dummy_byte_count);
  for (i = 0; i < n; i++)
    _cgm_emit_unsigned_integer_8bit (sdr_buffer, true, cgm_encoding,
				     (unsigned int)(x[i]),
				     dummy_data_len, &dummy_data_byte_count, 
				     &dummy_byte_count);
}

#ifdef LIBPLOTTER
CGMPlotter::CGMPlotter (FILE *infile, FILE *outfile, FILE *errfile)
	:Plotter (infile, outfile, errfile)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (FILE *outfile)
	:Plotter (outfile)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (istream& in, ostream& out, ostream& err)
	: Plotter (in, out, err)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (ostream& out)
	: Plotter (out)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter ()
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (FILE *infile, FILE *outfile, FILE *errfile, PlotterParams &parameters)
	:Plotter (infile, outfile, errfile, parameters)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (FILE *outfile, PlotterParams &parameters)
	:Plotter (outfile, parameters)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (istream& in, ostream& out, ostream& err, PlotterParams &parameters)
	: Plotter (in, out, err, parameters)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (ostream& out, PlotterParams &parameters)
	: Plotter (out, parameters)
{
  _pl_c_initialize ();
}

CGMPlotter::CGMPlotter (PlotterParams &parameters)
	: Plotter (parameters)
{
  _pl_c_initialize ();
}

CGMPlotter::~CGMPlotter ()
{
  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl ();

  _pl_c_terminate ();
}
#endif
