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

/* This file contains the openpl method, which is a standard part of
   libplot.  It opens a Plotter object. */

#include "sys-defines.h"
#include "extern.h"

int
_API_openpl (S___(Plotter *_plotter))
{
  bool retval;

  if (_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "openpl: invalid operation");
      return -1;
    }

  /* prepare buffer in which we'll cache graphics code for this page */
  switch ((int)_plotter->data->output_model)
    {
    case (int)PL_OUTPUT_NONE:
      /* a single page buffer is created here, and will be deleted by
	 closepl(); it's never actually written out */
      _plotter->data->page = _new_outbuf ();
      break;
      break;

    case (int)PL_OUTPUT_ONE_PAGE:
    case (int)PL_OUTPUT_ONE_PAGE_AT_A_TIME:
      /* a single page buffer is created here, and will be written out and
	 deleted by closepl(); in the former case, only page #1 is written
	 out */
      _plotter->data->page = _new_outbuf ();
      break;
      
    case (int)PL_OUTPUT_PAGES_ALL_AT_ONCE:
      {
	plOutbuf *new_page = _new_outbuf();

	if (_plotter->data->opened == false) /* first page */
	  {
	    _plotter->data->page = new_page;
	    /* Save a pointer to the first page, since we'll be caching all
	       pages until the Plotter is deleted. */
	    _plotter->data->first_page = new_page;
	  }
	else
	  /* add new page to tail of list, update pointer to current page */
	  {
	    _plotter->data->page->next = new_page;
	    _plotter->data->page = new_page;
	  }
      }
      break;

    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM:
      /* Plotter does its own output, and doesn't use libplot's
	 plOutbuf-based output system */
      _plotter->data->page = (plOutbuf *)NULL;
      break;

    default:			/* shouldn't happen */
      break;
    }

  /* flag device as open */
  _plotter->data->open = true;
  _plotter->data->opened = true;
  _plotter->data->page_number++;

  /* keep track of these; some lusers don't invoke ffontsize() or
     flinewidth(), so we need to choose a reasonable font size and/or line
     width for them */
  _plotter->data->fontsize_invoked = false;
  _plotter->data->linewidth_invoked = false;

  /* frames in page are numbered starting with zero */
  _plotter->data->frame_number = 0;

  /* create first drawing state, add it to the linked list of drawing
     states (this fills in only the device-independent part of the state) */
  _pl_g_create_first_drawing_state (S___(_plotter));

  /* copy background color, accessible to the user as a Plotter parameter,
     to the drawing state */
  {
    const char *bg_color_name_s;

    bg_color_name_s = 
      (const char *)_get_plot_param (_plotter->data, "BG_COLOR");
    if (bg_color_name_s)
      _API_bgcolorname (R___(_plotter) bg_color_name_s);
  }

  /* invoke Plotter-specific `begin page' method, to create
     device-dependent part of drawing state, if any; and possibly, do
     device-specific initializations of Plotter variables, such as the
     (NDC_frame)->(device_frame) map, which some Plotters initialize, once
     they've realized how large their device-space drawing region will be,
     by calling _compute_ndc_to_device_map() in g_space.c */
  retval = _plotter->begin_page (S___(_plotter));

  /* Set the composite (user frame)->(NDC_frame->(device frame) map in the
     drawing state, as if fsetmatrix() had been called.  At this point, the
     (user_frame)->(NDC_frame) map is the default, i.e., the identity, and
     we pass it to fsetmatrix as its argument.  The fsetmatrix method
     computes the (user_frame)->(NDC_frame)->(device_frame) map by
     composing the (user_frame)->(NDC frame) map with the
     (NDC_frame)->(device_frame) map (mentioned above).  Any subsequent
     user invocations of fsetmatrix() will not affect the latter. */
  _API_fsetmatrix (R___(_plotter) 
		   _plotter->drawstate->transform.m_user_to_ndc[0],
		   _plotter->drawstate->transform.m_user_to_ndc[1],
		   _plotter->drawstate->transform.m_user_to_ndc[2],
		   _plotter->drawstate->transform.m_user_to_ndc[3],
		   _plotter->drawstate->transform.m_user_to_ndc[4],
		   _plotter->drawstate->transform.m_user_to_ndc[5]);

  return (retval == true ? 0 : -1);
}

/* Plotter-specific initializations that take place when openpl() is
   invoked.  In a generic Plotter, this does nothing. */

bool
_pl_g_begin_page (S___(Plotter *_plotter))
{
  return true;
}

/* Create a new drawing state that will become the first drawing state in a
   Plotter's linked list of drawing states.  See above. */

void
_pl_g_create_first_drawing_state (S___(Plotter *_plotter))
{
  plDrawState *drawstate;
  const plDrawState *copyfrom;
  char *fill_rule, *line_mode, *join_mode, *cap_mode;

  /* create a new state */
  drawstate = (plDrawState *)_pl_xmalloc (sizeof(plDrawState));
  
  /* copy from default drawing state (see g_defstate.c) */
  copyfrom = &_default_drawstate;
  memcpy (drawstate, copyfrom, sizeof(plDrawState));

  /* elements of state that are strings are treated specially */
  fill_rule = (char *)_pl_xmalloc (strlen (copyfrom->fill_rule) + 1);
  line_mode = (char *)_pl_xmalloc (strlen (copyfrom->line_mode) + 1);
  join_mode = (char *)_pl_xmalloc (strlen (copyfrom->join_mode) + 1);
  cap_mode = (char *)_pl_xmalloc (strlen (copyfrom->cap_mode) + 1);
  strcpy (fill_rule, copyfrom->fill_rule);
  strcpy (line_mode, copyfrom->line_mode);
  strcpy (join_mode, copyfrom->join_mode);
  strcpy (cap_mode, copyfrom->cap_mode);
  drawstate->fill_rule = fill_rule;
  drawstate->line_mode = line_mode;
  drawstate->join_mode = join_mode;
  drawstate->cap_mode = cap_mode;

  /* dash array, if non-empty, is treated specially too */
  if (copyfrom->dash_array_len > 0)
    {
      int i;
      double *dash_array;

      dash_array = (double *)_pl_xmalloc (copyfrom->dash_array_len * sizeof(double));
      for (i = 0; i < copyfrom->dash_array_len; i++)
	dash_array[i] = copyfrom->dash_array[i];
      drawstate->dash_array = dash_array;
    }

  /* The font_name, true_font_name, font_type, typeface_index, and
     font_index fields are special, since for the initial drawing state
     they're Plotter-dependent.

     The fill_rule_type field is also treated specially in the initial
     drawing state, because not all Plotters support both types of filling
     (odd vs. nonzero winding number). */
  {
    const char *font_name_init;
    char *font_name, *true_font_name;
    int typeface_index, font_index;
    
    switch (_plotter->data->default_font_type)
      {
      case PL_F_HERSHEY:
      default:
	font_name_init = PL_DEFAULT_HERSHEY_FONT;
	typeface_index = PL_DEFAULT_HERSHEY_TYPEFACE_INDEX;
	font_index = PL_DEFAULT_HERSHEY_FONT_INDEX;	  
	break;
      case PL_F_POSTSCRIPT:
	font_name_init = PL_DEFAULT_POSTSCRIPT_FONT;
	typeface_index = PL_DEFAULT_POSTSCRIPT_TYPEFACE_INDEX;
	font_index = PL_DEFAULT_POSTSCRIPT_FONT_INDEX;	  
	break;
      case PL_F_PCL:
	font_name_init = PL_DEFAULT_PCL_FONT;
	typeface_index = PL_DEFAULT_PCL_TYPEFACE_INDEX;
	font_index = PL_DEFAULT_PCL_FONT_INDEX;	  
	break;
      case PL_F_STICK:
	font_name_init = PL_DEFAULT_STICK_FONT;
	typeface_index = PL_DEFAULT_STICK_TYPEFACE_INDEX;
	font_index = PL_DEFAULT_STICK_FONT_INDEX;	  
	break;
      }
    
    font_name = (char *)_pl_xmalloc (strlen (font_name_init) + 1);
    strcpy (font_name, font_name_init);  
    drawstate->font_name = font_name;

    true_font_name = (char *)_pl_xmalloc (strlen (font_name_init) + 1);
    strcpy (true_font_name, font_name_init);  
    drawstate->true_font_name = true_font_name;

    drawstate->font_type = _plotter->data->default_font_type;      
    drawstate->typeface_index = typeface_index;
    drawstate->font_index = font_index;      
    
    /* Examine default fill mode.  If Plotter doesn't support it, use the
       other fill mode. */
    if (drawstate->fill_rule_type == PL_FILL_ODD_WINDING
	&& _plotter->data->have_odd_winding_fill == 0)
      drawstate->fill_rule_type = PL_FILL_NONZERO_WINDING;
    else if (drawstate->fill_rule_type == PL_FILL_NONZERO_WINDING
	     && _plotter->data->have_nonzero_winding_fill == 0)
      drawstate->fill_rule_type = PL_FILL_ODD_WINDING;
  }

  /* page begins with no compound path under construction */
  drawstate->path = (plPath *)NULL;
  drawstate->paths = (plPath **)NULL;
  drawstate->num_paths = 0;

  /* install new state at head of the state list */
  drawstate->previous = NULL;
  _plotter->drawstate = drawstate;
}
