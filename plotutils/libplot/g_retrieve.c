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

/* This file contains the internal _pl_g_set_font method, which is called
   when the font_name, font_size, and textangle fields of the current
   drawing state have been filled in.  It retrieves the specified font, and
   fills in the true_font_name, true_font_size, font_type, typeface_index,
   font_index, font_is_iso8858_1, font_ascent, font_descent, and
   font_cap_height fields of the drawing state.

   _pl_g_set_font is invoked by _API_alabel() and _API_flabelwidth(), and
   also by _API_fontname(), _API_fontsize(), and _API_textangle() (only
   because those three functions in the API must return a font size).

   _pl_g_set_font does the following, in order:

   1. If font_name is the name of a supported Hershey font, i.e., one in
   libplot's database, it fills in the fields itself.

   2. If the font name is the name of a supported font of a non-Hershey
   type (i.e. PS/PCL/Stick), it _tentatively_ fills in the fields.  If the
   font name doesn't appear in libplot's internal database, it simply sets
   the font_type to `PL_F_OTHER'.

   3a. If the font name isn't the name of a supported Hershey font, it
   invokes the Plotter-specific `retrieve_font' method.  In most Plotters
   this simply returns true.  (See the stub _pl_g_retrieve_font() below.)
   But in principle it may modify the fields arbitrarily, perhaps consult
   an external database, etc.

   3b. If retrieve_font returns false, a default Hershey font is
   substituted, and the fields are filled in.  This feature is used e.g. by
   Fig Plotters, which support any of the 35 PS fonts in libplot's
   database, except when the user frame -> device frame map is anisotropic
   (i.e. non-uniform).  It is also used by X Drawable Plotters, which may
   or may not have a PS font available, and in fact may or may not have any
   given font of type `PL_F_OTHER' (see above) available. */

#include "sys-defines.h"
#include "extern.h"
#include "g_her_metr.h"

#define MAP_HERSHEY_UNITS_TO_USER_UNITS(size, drawstate) \
	((size)*(drawstate->true_font_size)/(HERSHEY_EM))

/* forward references */
static bool _match_hershey_font (plDrawState *drawstate);
static bool _match_ps_font (plDrawState *drawstate);
static bool _match_pcl_font (plDrawState *drawstate);
static bool _match_stick_font (plDrawState *drawstate, bool have_extra_stick_fonts);

void
_pl_g_set_font (S___(Plotter *_plotter))
{
  plDrawState *drawstate = _plotter->drawstate;
  plPlotterData *data = _plotter->data;
  const char *default_font_name;
  bool matched;

  /* try to match font name in our database */

  if (_match_hershey_font (drawstate))
    /* Matched a Hershey font name in our database, and all fields filled
       in definitively, so return without invoking Plotter-specific
       `retrieve_font', which knows nothing about Hershey fonts anyway */
    return;

  matched = false;

  /* Try to match the font name with the name of a PCL font or a PS font in
     the database, and tentatively fill in fields.  But there is a
     namespace collision: "Courier" etc. and "Symbol" occur on both lists.
     Which list we search first depends on what type of Plotter this is. */

  if (data->pcl_before_ps)
    {
      /* search PCL font list first */      
      if ((data->have_pcl_fonts 
	   && _match_pcl_font (drawstate))
	  ||
          (data->have_ps_fonts 
	   && _match_ps_font (drawstate)))
	matched = true;
    }
  else
    {
      /* search PS font list first */
      if ((data->have_ps_fonts 
	   && _match_ps_font (drawstate))
	  ||
	  (data->have_pcl_fonts 
	   && _match_pcl_font (drawstate)))
	matched = true;
    }

  /* if unmatched, search through Stick font list too */
  if (matched == false && data->have_stick_fonts
      && _match_stick_font (drawstate, 
			    data->have_extra_stick_fonts ? true : false))
    matched = true;

  if (matched == false)
    /* fill in the only fields we can */
    {
      free ((char *)drawstate->true_font_name);
      drawstate->true_font_name = 
	(const char *)_pl_xmalloc (strlen (drawstate->font_name) + 1);
      strcpy ((char *)drawstate->true_font_name, drawstate->font_name);
      drawstate->true_font_size = drawstate->font_size;

      drawstate->font_type = PL_F_OTHER;
      drawstate->typeface_index = 0;
      drawstate->font_index = 1;

      /* NOT filled in yet: font_is_iso8859_1, font_ascent, font_descent,
	 and font_cap_height; they're left to Plotter-specific retrieval
	 routine, if it can supply them (in the case of a Metafile Plotter,
	 it surely can't). */
    }

  /* If we got here, font name isn't that of a Hershey font, and `matched'
     indicates whether or not it's listed in our font name database, as one
     of the types of font this Plotter can handle.  So invoke low-level
     Plotter-specific retrieval routine.  Do it even if we haven't matched
     the font, if this Plotter claims to be able to handle `other' fonts,
     i.e., ones not listed in our database. */

  if (matched || (!matched && data->have_other_fonts))
    /* try to invoke low-level Plotter-specific routine to finish the job
       of filling in fields, including Plotter-specific ones */
    {
      if (_plotter->retrieve_font (S___(_plotter)))
	/* all finished... */
	return;
    }

  /* Plotter-specific retrieval failed: it doesn't like the font name or
     other drawstate parameters (size, textangle, transformation matrix?)  */

  /* Via a recursive call, try to retrieve default font for this Plotter
     type (if it's different from the one we just failed to retrieve;
     otherwise retrieve the default Hershey font).  */

  switch (data->default_font_type)
    {
    case PL_F_POSTSCRIPT:
      default_font_name = PL_DEFAULT_POSTSCRIPT_FONT;
      break;
    case PL_F_PCL:
      default_font_name = PL_DEFAULT_PCL_FONT;
      break;
    case PL_F_STICK:
      default_font_name = PL_DEFAULT_STICK_FONT;
      break;
    case PL_F_HERSHEY:		/* Hershey shouldn't happen */
    default:
      default_font_name = PL_DEFAULT_HERSHEY_FONT;
      break;

      /* N.B. No support yet for a Plotter-specific default font that is of
	 PL_F_OTHER type, i.e., isn't listed in the libplot font database. */
    }

  if (strcmp (drawstate->font_name, default_font_name) == 0
      || strcmp (drawstate->true_font_name, default_font_name) == 0)
    /* default font is the one we just failed to retrieve: so use Hershey:
       internal and guaranteed to be available */
    default_font_name = PL_DEFAULT_HERSHEY_FONT;
  
  /* stash fontname we failed to retrieve; then do recursive call, turning
     off font warnings for the duration; restore fontname field */
  {
    const char *saved_font_name;
    bool saved_font_warning_issued;

    saved_font_name = drawstate->font_name;
    drawstate->font_name = default_font_name;
    saved_font_warning_issued = _plotter->data->font_warning_issued;
    _plotter->data->font_warning_issued = true;	/* turn off warnings */
    _pl_g_set_font (S___(_plotter));
    _plotter->data->font_warning_issued = saved_font_warning_issued;
    drawstate->font_name = saved_font_name;
  }

  if (data->issue_font_warning && !_plotter->data->font_warning_issued)
    /* squawk */
    {
      char *buf;
      
      buf = (char *)_pl_xmalloc (strlen (drawstate->font_name) + strlen (drawstate->true_font_name) + 100);
      sprintf (buf, "cannot retrieve font \"%s\", using default \"%s\"", 
	       drawstate->font_name, drawstate->true_font_name);
      _plotter->warning (R___(_plotter) buf);
      free (buf);
      _plotter->data->font_warning_issued = true;
    }
}

static bool
_match_hershey_font (plDrawState *drawstate)
{
  int i;

  /* is font a Hershey font? */
  i = -1;
  while (_pl_g_hershey_font_info[++i].name)
    {
      if (_pl_g_hershey_font_info[i].visible) /* i.e. font not internal-only */
	if (strcasecmp (_pl_g_hershey_font_info[i].name, 
			drawstate->font_name) == 0
	    || (_pl_g_hershey_font_info[i].othername
		&& strcasecmp (_pl_g_hershey_font_info[i].othername, 
			       drawstate->font_name) == 0))
	  /* fill in fields */
	  {
	    free ((char *)drawstate->true_font_name);
	    drawstate->true_font_name = 
	      (const char *)_pl_xmalloc (strlen (_pl_g_hershey_font_info[i].name) + 1);
	    strcpy ((char *)drawstate->true_font_name, _pl_g_hershey_font_info[i].name);
	    drawstate->true_font_size = drawstate->font_size;

	    drawstate->font_type = PL_F_HERSHEY;
	    drawstate->typeface_index = _pl_g_hershey_font_info[i].typeface_index;
	    drawstate->font_index = _pl_g_hershey_font_info[i].font_index;
	    drawstate->font_is_iso8859_1 = _pl_g_hershey_font_info[i].iso8859_1;

	    /* N.B. this macro uses true_font_size */
	    drawstate->font_cap_height = 
	      MAP_HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_CAPHEIGHT, drawstate);
	    drawstate->font_ascent = 
	      MAP_HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_ASCENT, drawstate);
	    drawstate->font_descent = 
	      MAP_HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_DESCENT, drawstate);
	    
	    return true;
	  }
    }
  return false;
}

static bool
_match_pcl_font (plDrawState *drawstate)
{
  int i = -1;

  /* is font a PCL font in libplot's database? */
  while (_pl_g_pcl_font_info[++i].ps_name)
    {
      if (strcasecmp (_pl_g_pcl_font_info[i].ps_name, 
		      drawstate->font_name) == 0
	  /* try alternative PS font name if any */
	  || (_pl_g_pcl_font_info[i].ps_name_alt != NULL
	      && strcasecmp (_pl_g_pcl_font_info[i].ps_name_alt, 
			     drawstate->font_name) == 0)
	  /* try X font name too */
	  || strcasecmp (_pl_g_pcl_font_info[i].x_name, 
			 drawstate->font_name) == 0)
	{
	  free ((char *)drawstate->true_font_name);
	  drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (_pl_g_pcl_font_info[i].ps_name) + 1);
	  strcpy ((char *)drawstate->true_font_name, _pl_g_pcl_font_info[i].ps_name);
	  drawstate->true_font_size = drawstate->font_size;

	  drawstate->font_type = PL_F_PCL;
	  drawstate->typeface_index = _pl_g_pcl_font_info[i].typeface_index;
	  drawstate->font_index = _pl_g_pcl_font_info[i].font_index;
	  drawstate->font_is_iso8859_1 = _pl_g_pcl_font_info[i].iso8859_1;

	  drawstate->font_ascent 
	    = drawstate->true_font_size
	      * (double)(_pl_g_pcl_font_info[i].font_ascent)/1000.0;
	  drawstate->font_descent 
	    = drawstate->true_font_size
	      * (double)(_pl_g_pcl_font_info[i].font_descent)/1000.0;
	  drawstate->font_cap_height 
	    = drawstate->true_font_size
	      * (double)(_pl_g_pcl_font_info[i].font_cap_height)/1000.0;
	  return true;
	}
    }
  return false;
}

static bool
_match_ps_font (plDrawState *drawstate)
{
  int i = -1;

  /* is font a PS font in libplot's database ? */
  while (_pl_g_ps_font_info[++i].ps_name)
    {
      if (strcasecmp (_pl_g_ps_font_info[i].ps_name, 
		      drawstate->font_name) == 0
	  /* try alternative PS font name if any */
	  || (_pl_g_ps_font_info[i].ps_name_alt != NULL
	      && strcasecmp (_pl_g_ps_font_info[i].ps_name_alt, 
			     drawstate->font_name) == 0)
	  /* try 2nd alternative PS font name if any */
	  || (_pl_g_ps_font_info[i].ps_name_alt2 != NULL
	      && strcasecmp (_pl_g_ps_font_info[i].ps_name_alt2, 
			     drawstate->font_name) == 0)
	  /* try X font name too */
	  || strcasecmp (_pl_g_ps_font_info[i].x_name, 
			 drawstate->font_name) == 0
	  /* try alternative X font name if any */
	  || (_pl_g_ps_font_info[i].x_name_alt != NULL
	      && strcasecmp (_pl_g_ps_font_info[i].x_name_alt,
			     drawstate->font_name) == 0))
	{
	  free ((char *)drawstate->true_font_name);
	  drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (_pl_g_ps_font_info[i].ps_name) + 1);
	  strcpy ((char *)drawstate->true_font_name, _pl_g_ps_font_info[i].ps_name);

	  drawstate->true_font_size = drawstate->font_size;

	  drawstate->font_type = PL_F_POSTSCRIPT;
	  drawstate->typeface_index = _pl_g_ps_font_info[i].typeface_index;
	  drawstate->font_index = _pl_g_ps_font_info[i].font_index;
	  drawstate->font_is_iso8859_1 = _pl_g_ps_font_info[i].iso8859_1;

	  drawstate->font_ascent 
	    = drawstate->true_font_size
	      * (double)(_pl_g_ps_font_info[i].font_ascent)/1000.0;
	  drawstate->font_descent 
	    = drawstate->true_font_size
	      * (double)(_pl_g_ps_font_info[i].font_descent)/1000.0;
	  drawstate->font_cap_height
	    = drawstate->true_font_size
	      * (double)(_pl_g_ps_font_info[i].font_cap_height)/1000.0;
	  return true;
	}
    }
  return false;
}

static bool
_match_stick_font (plDrawState *drawstate, bool have_extra_stick_fonts)
{
  int i = -1;

  /* is font a PCL font in libplot's database? */
  while (_pl_g_stick_font_info[++i].ps_name)
    {
      if (_pl_g_stick_font_info[i].basic == false
	  && !have_extra_stick_fonts)
	/* not a basic Stick font, and this Plotter supports only the basic
	   ones */
	continue;
      
      if (strcasecmp (_pl_g_stick_font_info[i].ps_name, 
		      drawstate->font_name) == 0)
	/* fill in fields */
	{
	  free ((char *)drawstate->true_font_name);
	  drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (_pl_g_stick_font_info[i].ps_name) + 1);
	  strcpy ((char *)drawstate->true_font_name, _pl_g_stick_font_info[i].ps_name);
	  drawstate->true_font_size = drawstate->font_size;
	  drawstate->true_font_size = drawstate->font_size;
	  drawstate->true_font_size = drawstate->font_size;

	  drawstate->font_type = PL_F_STICK;
	  drawstate->typeface_index = _pl_g_stick_font_info[i].typeface_index;
	  drawstate->font_index = _pl_g_stick_font_info[i].font_index;
	  drawstate->font_is_iso8859_1 = _pl_g_stick_font_info[i].iso8859_1;

	  drawstate->font_ascent 
	    = drawstate->true_font_size
	    * (double)(_pl_g_stick_font_info[i].font_ascent)/1000.0;
	  drawstate->font_descent 
	    = drawstate->true_font_size
	    * (double)(_pl_g_stick_font_info[i].font_descent)/1000.0;
	  drawstate->font_cap_height
	    /* The `0.7' is undocumented HP magic; see comments in
	       h_font.c and g_fontd2.c */
	    = 0.7 * drawstate->true_font_size;
	  
	  return true;
	}
    }
  return false;
}

/* This is the generic version of the _retrieve_font method, which does
   nothing.  Many types of Plotter use this, but some, e.g., FigPlotters
   and XDrawablePlotters (and X Plotters) override it.  See
   _pl_f_retrieve_font() and _pl_x_retrieve_font(). */

bool
_pl_g_retrieve_font (S___(Plotter *_plotter))
{
  return true;
}
