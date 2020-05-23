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

#include "sys-defines.h"
#include "extern.h"

bool
_pl_h_begin_page (S___(Plotter *_plotter))
{
  int i;

  /* With each call to openpl(), we reset our knowledge of the HP-GL
     internal state, i.e. the dynamic derived-class-specific data members
     of the HPGL or PCL Plotter.  The values are the same as are used in
     initializing the Plotter (see h_defplot.c). */
     
  /* reset any soft-defined colors in the pen color array */
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    if (_plotter->hpgl_pen_defined[i] == 1) /* i.e. soft-defined */
      _plotter->hpgl_pen_defined[i] = 0; /* i.e. undefined */

  /* reset current pen */
  _plotter->hpgl_pen = 1;  

  /* if we can soft-define pen colors, reset free_pen data member by
     determining what the next free pen is */
  {
    bool undefined_pen_seen = false;
    
    if (_plotter->hpgl_can_assign_colors) /* can soft-define pen colors */
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
  
  /* reset additional data members of Plotter */
  _plotter->hpgl_bad_pen = false;  
  _plotter->hpgl_pendown = false;  
  _plotter->hpgl_pen_width = 0.001;  
  _plotter->hpgl_line_type = HPGL_L_SOLID;
  _plotter->hpgl_cap_style = HPGL_CAP_BUTT;
  _plotter->hpgl_join_style = HPGL_JOIN_MITER;
  _plotter->hpgl_miter_limit = 5.0; /* default HP-GL/2 value */
  _plotter->hpgl_fill_type = HPGL_FILL_SOLID_BI;
  _plotter->hpgl_fill_option1 = 0.0;
  _plotter->hpgl_fill_option2 = 0.0;
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

  /* if a PCL Plotter, switch from PCL mode to HP-GL/2 mode */
  _maybe_switch_to_hpgl (S___(_plotter));

  /* output HP-GL prologue */
  if (_plotter->hpgl_version == 2)
    {
      sprintf (_plotter->data->page->point, "BP;IN;");
      _update_buffer (_plotter->data->page);
      /* include HP-GL/2 `plot length' directive; important mostly for roll
	 plotters */
      sprintf (_plotter->data->page->point, "PS%d;",
	       IROUND(_plotter->hpgl_plot_length));
      _update_buffer (_plotter->data->page);
    }
  else
    {
      sprintf (_plotter->data->page->point, "IN;");
      _update_buffer (_plotter->data->page);
    }
  
  /* make use of HP-GL's plotting-area rotation capability, if requested by
     the HPGL_ROTATE parameter (this does not apply to PCL Plotters, for
     which rotation=0 always) */
  if (_plotter->hpgl_rotation != 0)
    {
      sprintf (_plotter->data->page->point, "RO%d;", _plotter->hpgl_rotation);
      _update_buffer (_plotter->data->page);
    }
  
  /* Set scaling points P1, P2 at lower left and upper right corners of our
     viewport; or more accurately, at the two points that (0,0) and (1,1),
     which are the lower right and upper right corners in NDC space, get
     mapped to. */
  sprintf (_plotter->data->page->point, "IP%d,%d,%d,%d;",
	   IROUND(_plotter->hpgl_p1.x), IROUND(_plotter->hpgl_p1.y),
	   IROUND(_plotter->hpgl_p2.x), IROUND(_plotter->hpgl_p2.y));
  _update_buffer (_plotter->data->page);
  
  /* Set up `scaled device coordinates' within the viewport.  All
     coordinates in the output file will be scaled device coordinates, not
     physical device coordinates.  The range of scaled coordinates will be
     independent of the viewport positioning, page size, etc.; see the
     definitions of xmin,xmax,ymin,ymax in h_defplot.c. */
  sprintf (_plotter->data->page->point, "SC%d,%d,%d,%d;",
	   IROUND (_plotter->data->xmin), IROUND (_plotter->data->xmax), 
	   IROUND (_plotter->data->ymin), IROUND (_plotter->data->ymax));
  _update_buffer (_plotter->data->page);
  
  if (_plotter->hpgl_version == 2)
    {
      /* Begin to define a palette, by specifying a number of logical pens.
	 (All HP-GL/2 devices should support the `NP' instruction, even
	 though many support only a default palette.) */
      if (_plotter->hpgl_can_assign_colors)
	{
	  sprintf (_plotter->data->page->point, "NP%d;", HPGL2_MAX_NUM_PENS);
	  _update_buffer (_plotter->data->page);
	}
      /* use relative units for pen width */
      sprintf (_plotter->data->page->point, "WU1;");
      _update_buffer (_plotter->data->page);
    }
  
  /* select pen #1 (standard plotting convention) */
  sprintf (_plotter->data->page->point, "SP1;");
  _update_buffer (_plotter->data->page);
  
  /* For HP-GL/2 devices, set transparency mode to `opaque', if the user
     allows it.  It should always be opaque to agree with libplot
     conventions, but on some HP-GL/2 devices (mostly pen plotters) the
     `TR' command allegedly does not NOP gracefully. */
  if (_plotter->hpgl_version == 2 && _plotter->hpgl_use_opaque_mode)
    {
      sprintf (_plotter->data->page->point, "TR0;");
      _update_buffer (_plotter->data->page);
    }

  /* freeze contents of output buffer, i.e. the initialization code we've
     just written to it, so that any later invocation of erase(), i.e.,
     erase_page(), won't remove it */
  _freeze_outbuf (_plotter->data->page);

  return true;
}

void
_pl_h_maybe_switch_to_hpgl (S___(Plotter *_plotter))
{
}

void
_pl_q_maybe_switch_to_hpgl (S___(Plotter *_plotter))
{
  if (_plotter->data->page_number > 1) /* not first page */
    /* eject previous page, by issuing PCL command */
    {	
      strcpy (_plotter->data->page->point, "\f"); /* i.e. form feed */
      _update_buffer (_plotter->data->page);
    }
  /* switch from PCL 5 to HP-GL/2 mode */
  strcpy (_plotter->data->page->point, "\033%0B\n");
  _update_buffer (_plotter->data->page);
}
