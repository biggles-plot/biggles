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

#define ONEBYTE (0xff)

#define USE_PEN_ZERO (_plotter->hpgl_version == 2 && (_plotter->hpgl_use_opaque_mode || _plotter->hpgl_can_assign_colors))

/* _h_set_pen_color() sets the pen color used by the HP-GL[/2] device to
   match the pen color in our current drawing state.  It's invoked just
   before any drawing operation.

   If the device's palette contains a matching color, the corresponding pen
   is selected.  Otherwise, we do one of several things.

   1. If we can add colors to the palette, we add the specified color, and
   then select the corresponding pen.

   2. If we can't add colors to the palette, but we can specify a shading
   level, i.e., a desaturation level, we find the closest point in the RGB
   cube to the specified color that's a shaded version of one of the colors
   in the palette.  Then we select it, by both selecting the appropriate
   pen, and selecting a shading level.

   There are two subcases to case #2: either the drawing operation to
   follow this invocation of set_pen_color will draw a path, or it will
   draw a text string using a font native to the HP-GL interpreter.  In the
   former case, we use the `SV' (screened vector) instruction to set the
   shading level; in the latter, we use the `CF' (character fill)
   instruction to set the shading level.  The two sub-cases are
   distinguished by a hint that's passed to us, by being placed in the
   HP-GL-specific part of the drawing state before this function is called.

   3. If we can't do either (1) or (2), then we search the palette for the
   closest match, and the corresponding `quantized' pen is selected.  We
   adopt a convention: nonwhite pen colors are never quantized to white.

   Pen #0 is the canonical white pen.  But on pen plotters, drawing with
   pen #0 isn't meaningful.  So we won't actually use pen #0 to draw with,
   unless HPGL_VERSION==2 and HPGL_OPAQUE_MODE=yes (or
   HPGL_ASSIGN_COLORS=yes, which presumably means the output is directed to
   a DesignJet).  If the closest match in case #3 is pen #0 and we won't be
   using pen #0 to draw with, we set the advisory `hpgl_bad_pen' flag in
   the Plotter to `true'; otherwise we set it to `false'. */

void
_pl_h_set_pen_color(R___(Plotter *_plotter) int hpgl_object_type)
{
  bool found;
  int longred, longgreen, longblue;
  int red, green, blue;
  int i;
  plColor color;
  
  color = _plotter->drawstate->fgcolor;
  longred = color.red;
  longgreen = color.green;
  longblue = color.blue;

  /* truncate to 24-bit color */
  red = (longred >> 8) & ONEBYTE;
  green = (longgreen >> 8) & ONEBYTE;
  blue = (longblue >> 8) & ONEBYTE;
  
  /* Check whether color is in the palette, in which case all we need to do
     is select it. */
  found = false;
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->hpgl_pen_defined[i] != 0 /* i.e. defined (hard or soft) */
	  && _plotter->hpgl_pen_color[i].red == red
	  && _plotter->hpgl_pen_color[i].green == green
	  && _plotter->hpgl_pen_color[i].blue == blue)
	{
	  found = true;
	  break;
	}
    }
      
  if (found)
    /* Color is in palette: the simplest case.  Besides selecting the
       corresponding pen, must set pen type to solid, if there's support
       for altering the pen type via `screening of vectors'; since in that
       case the pen type could have been set to `shaded' previously.  If a
       label is to be drawn rather than a path, we must similarly update
       the character rendition type to `solid fill' rather than `shaded'.  */
    {
      if (i != 0 || (i == 0 && USE_PEN_ZERO))
	/* can be selected */
	{
	  /* select the pen */
	  _pl_h_set_hpgl_pen (R___(_plotter) i);
	  
	  /* in HP-GL/2 case, be sure that `solid' vector screening or
             character filling is used (one or the other, depending on a
             hint as to which type of object is to be drawn) */
	  switch (hpgl_object_type)
	    {
	    case HPGL_OBJECT_PATH:
	      if (_plotter->hpgl_version == 2 
		  && _plotter->hpgl_have_screened_vectors == true)
		/* set pen type to solid */
		_pl_h_set_hpgl_pen_type (R___(_plotter) HPGL_PEN_SOLID, 
					 /* options ignored */
					 0.0, 0.0);
	      break;
	    case HPGL_OBJECT_LABEL:
	      if (_plotter->hpgl_version == 2 
		  && _plotter->hpgl_have_char_fill == true)
		/* if necessary, emit `CF' instruction: specify that
		   characters are to be rendered by being filled solid with
		   the current pen color without edging, which is the
		   default */
		if (_plotter->hpgl_char_rendering_type != 
		    HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE)
		  {
		    sprintf (_plotter->data->page->point, "CF;");
		    _update_buffer (_plotter->data->page);
		    _plotter->hpgl_char_rendering_type =
		      HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE;
		  }
	      break;
	    default:
	      break;
	    }
	  
	  _plotter->hpgl_bad_pen = false;
	}
      else
	/* won't use pen #0, so set advisory flag */
	_plotter->hpgl_bad_pen = true;
    }

  else
    /* color not in palette, must do something */
    if (_plotter->hpgl_version == 2 && _plotter->hpgl_can_assign_colors)
      /* CASE #1: can soft-define pen colors (HP-GL/2, presumably a
	 DesignJet) */
      {
	/* assign current `free pen' to be the new color */
	sprintf (_plotter->data->page->point, "PC%d,%d,%d,%d;", 
		 _plotter->hpgl_free_pen, red, green, blue);
	_update_buffer (_plotter->data->page);
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].red = red;
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].green = green;
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].blue = blue;
	_plotter->hpgl_pen_defined[_plotter->hpgl_free_pen] = 1; /* soft-def */
	/* select pen */
	_pl_h_set_hpgl_pen (R___(_plotter) _plotter->hpgl_free_pen);
	/* update free pen, i.e. choose next non-hard-defined pen */
	do
	  _plotter->hpgl_free_pen = (_plotter->hpgl_free_pen + 1) % HPGL2_MAX_NUM_PENS;
	while (_plotter->hpgl_pen_defined[_plotter->hpgl_free_pen] == 2);
	
	/* in HP-GL/2 case, be sure that `solid' vector screening or
	   character filling is used (one or the other, depending on a hint
	   as to which type of object is to be drawn) */
	switch (hpgl_object_type)
	  {
	  case HPGL_OBJECT_PATH:
	    if (_plotter->hpgl_version == 2 
		&& _plotter->hpgl_have_screened_vectors == true)
	      /* set pen type to solid */
	      _pl_h_set_hpgl_pen_type (R___(_plotter) HPGL_PEN_SOLID, 
				       /* options ignored */
				       0.0, 0.0);
	    break;
	  case HPGL_OBJECT_LABEL:
	    if (_plotter->hpgl_version == 2 
		&& _plotter->hpgl_have_char_fill == true)
	      /* if necessary, emit `CF' instruction: specify that
		 characters are to be rendered by being filled solid with
		 the current pen color without edging, which is the default */
	      if (_plotter->hpgl_char_rendering_type != 
		  HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE)
		{
		  sprintf (_plotter->data->page->point, "CF;");
		  _update_buffer (_plotter->data->page);
		  _plotter->hpgl_char_rendering_type =
		    HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE;
		}
	    break;
	  default:
	    break;
	  }
	
	_plotter->hpgl_bad_pen = false;
      }
  
    else if (_plotter->hpgl_version == 2 
	     && _plotter->hpgl_have_screened_vectors == true
	     && hpgl_object_type == HPGL_OBJECT_PATH)
      /* CASE #2a: HP-GL/2, and we have a path to draw, according to the
	 passed hint; can't soft-define pen colors, but can set a pen
	 shading level via the `SV' instruction.  So locate closest point
	 in RGB cube that is a desaturated version of one of the defined
	 pen colors, and shade at the appropriate level.  */
      {
	double shading;
	
	_pl_h_hpgl_shaded_pseudocolor (R___(_plotter) 
				       red, green, blue, &i, &shading);
	
	if (i != 0 || (i == 0 && USE_PEN_ZERO))
	  /* can be selected */
	  {
	    /* select the pen */
	    _pl_h_set_hpgl_pen (R___(_plotter) i);
	    /* set shading level, as a percentage */
	    _pl_h_set_hpgl_pen_type (R___(_plotter) HPGL_PEN_SHADED, 
				     /* 2nd option ignored for HPGL_PEN_SHADED */
				     100.0 * shading, 0.0);
	    _plotter->hpgl_bad_pen = false;
	  }
	else
	  /* won't use pen #0, so set advisory flag */
	  _plotter->hpgl_bad_pen = true;
      }
  
    else if (_plotter->hpgl_version == 2 
	     && _plotter->hpgl_have_char_fill == true
	     && hpgl_object_type == HPGL_OBJECT_LABEL)
      /* CASE #2b: HP-GL/2, and we have a label to draw, according to the
	 passed hint; can't soft-define pen colors, but can set a character
	 shading level via the `CF' instruction.  So locate closest point
	 in RGB cube that is a desaturated version of one of the defined
	 pen colors, and shade at the appropriate level.  */
      {
	double shading;
	
	_pl_h_hpgl_shaded_pseudocolor (R___(_plotter) 
				       red, green, blue, &i, &shading);
	
	if (i != 0 || (i == 0 && USE_PEN_ZERO))
	  /* can be selected */
	  {
	    /* select the pen */
	    _pl_h_set_hpgl_pen (R___(_plotter) i);
	    /* if necessary, emit `CF' instruction: specify that characters
	       are to be rendered in a non-default way, by being filled
	       with the current fill type (without edging) */
	    if (_plotter->hpgl_char_rendering_type != HPGL_CHAR_FILL)
	      {
		sprintf (_plotter->data->page->point, "CF%d;", HPGL_CHAR_FILL);
		_update_buffer (_plotter->data->page);
		_plotter->hpgl_char_rendering_type = HPGL_CHAR_FILL;
	      }
	    /* set the fill type to be a shading level (expressed as a
	       percentage) */
	    _pl_h_set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SHADED, 
				      100.0 * shading, 0.0); /* 2nd option ignord */
	    _plotter->hpgl_bad_pen = false;
	  }
	else
	  /* won't use pen #0, so set advisory flag */
	  _plotter->hpgl_bad_pen = true;
      }
  
    else
      /* CASE #3: we're stuck with a fixed set of pen colors, from which we
	 need to choose. [HPGL_VERSION may be "1" (i.e. generic HP-GL) or
	 "1.5" (i.e. HP7550A), or "2" (i.e. modern HP-GL/2, but without the
	 ability to define a palette).]  So select closest defined pen in
	 RGB cube, using Euclidean distance as metric.  Final arg here is
	 `true' on account of our convention that a non-white pen color
	 [unlike a fill color] is never quantized to white (i.e. to pen
	 #0). */
      {
	i = _pl_h_hpgl_pseudocolor (R___(_plotter) red, green, blue, true);
	if (i != 0 || (i == 0 && USE_PEN_ZERO))
	  /* can be selected */
	  {
	    /* select the pen */
	    _pl_h_set_hpgl_pen (R___(_plotter) i);
	    
	    /* do some updating, based on the type of object to be drawn */
	    switch (hpgl_object_type)
	      {
	      case HPGL_OBJECT_PATH:
		if (_plotter->hpgl_version == 2 
		    && _plotter->hpgl_have_screened_vectors == true)
		  /* set pen type to solid */
		  _pl_h_set_hpgl_pen_type (R___(_plotter) HPGL_PEN_SOLID, 
					   /* options ignored */
					   0.0, 0.0);
		break;
	      case HPGL_OBJECT_LABEL:
		if (_plotter->hpgl_version == 2 
		    && _plotter->hpgl_have_char_fill == true)
		  /* if necessary, emit `CF' instruction: specify that
		     characters are to be rendered by being filled solid with
		     the current pen color without edging, which is the
		     default */
		  if (_plotter->hpgl_char_rendering_type != 
		      HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE)
		    {
		      sprintf (_plotter->data->page->point, "CF;");
		      _update_buffer (_plotter->data->page);
		      _plotter->hpgl_char_rendering_type =
			HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE;
		    }
		break;
	      default:
		break;
	      }
	    
	    _plotter->hpgl_bad_pen = false;
	  }
	else
	  /* won't use pen #0, so set advisory flag */
	  _plotter->hpgl_bad_pen = true;
      }
}

/* _pl_h_set_fill_color() is similar to _pl_h_set_pen_color: it sets the
   HP-GL pen color (and fill type, if appropriate) to match the fill color
   in our current drawing state.  It's invoked before any filling
   operation.

   (Note that all filling operations will use the polygon buffer, except
   when we're emitting generic HP-GL [i.e., HPGL_VERSION="1"], which has no
   polygon buffer and no support for general filling operations.  In that
   case the only filling operations we perform are the filling of circles
   and rectangles aligned with the coordinate axes.)

   There are three cases.

   (1) An HP-GL/2 device supporting modification of the palette,
   i.e. `soft-definition' of pen colors. I.e., HPGL_VERSION="2" and
   HPGL_ASSIGN_COLORS="yes".  We use solid filling, after defining the fill
   color as a new pen color if necessary.

   (2) An HP-GL/2 device not supporting modification of the palette, but
   which do support shading at any specified intensity.  I.e.,
   HPGL_VERSION="2" and HPGL_ASSIGN_COLORS="no".  We determine which shade
   of which defined pen is closest to the fill color in the sense of
   Euclidean distance within the RGB cube.  `Shades' are really
   desaturations (interpolations between a pen color, and white).

   (3) An HP7550A-like device or generic HP-GL device, neither of which has
   firmware support for shading.  I.e., HPGL_VERSION="1.5" or "1".  Such
   devices do support cross-hatching, though.  So we (a) determine which
   shade of which defined pen is closest to the fill color in the sense of
   Euclidean distance within the RGB cube, and (b) select a cross-hatch
   distance that will emulate this shade.  For this, we use the algorithm
   that the HP-GL/2 counterpart of the HP7550A, the HP7550B, uses.

   (WARNING: our selection of cross-hatching includes the setting of the
   line type to `solid'.  As a consequence, if HPGL_VERSION="1.5" or "1",
   then `_pl_h_set_fill_color' does not commute with
   `_pl_h_set_attributes'.  This is taken into account in several places in
   h_path.c; grep for KLUDGE.)

   Pen #0 is the canonical white pen.  But on pen plotters, filling with
   pen #0 isn't meaningful.  So we won't actually use pen #0 to fill with
   unless HPGL_VERSION==2 and HPGL_OPAQUE_MODE=yes (or
   HPGL_ASSIGN_COLORS=yes, which presumably means the output is directed to
   a DesignJet).  Accordingly if the closest match here is pen #0, we set
   the advisory `hpgl_bad_pen' flag in the Plotter to `true'; otherwise we
   set it to `false'.  This is just as in set_pen_color() above. */

void
_pl_h_set_fill_color(R___(Plotter *_plotter) bool force_pen_color)
{
  bool found;
  int longred, longgreen, longblue;
  int red, green, blue;
  int i;
  
  if (force_pen_color == false && _plotter->drawstate->fill_type == 0) 
    /* won't be doing filling, so punt */
    return;

  /* get 48-bit color; if force_pen_color is set, use pen color
     instead of fill color */
  if (force_pen_color)
    {
      longred = _plotter->drawstate->fgcolor.red;
      longgreen = _plotter->drawstate->fgcolor.green;
      longblue = _plotter->drawstate->fgcolor.blue;
    }
  else
    {
      longred = _plotter->drawstate->fillcolor.red;
      longgreen = _plotter->drawstate->fillcolor.green;
      longblue = _plotter->drawstate->fillcolor.blue;
    }

  /* truncate to 24-bit color */
  red = (longred >> 8) & ONEBYTE;
  green = (longgreen >> 8) & ONEBYTE;
  blue = (longblue >> 8) & ONEBYTE;
  
  /* check whether color is already in palette, in which case all we need
     to do is select it (and set fill type to solid) */
  found = false;
  for (i = 0; i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->hpgl_pen_defined[i] != 0 /* i.e. defined (hard or soft) */
	  && _plotter->hpgl_pen_color[i].red == red
	  && _plotter->hpgl_pen_color[i].green == green
	  && _plotter->hpgl_pen_color[i].blue == blue)
	{
	  found = true;
	  break;
	}
    }
      
  if (found)
    /* color is in palette */
    {
      if (i != 0 || (i == 0 && USE_PEN_ZERO))
	/* can be selected */
	{
	  /* select it */
	  _pl_h_set_hpgl_pen (R___(_plotter) i);
	  /* set fill type to solid, unidirectional */
	  _pl_h_set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SOLID_UNI, 
				    0.0, 0.0); /* options ignored */
	  _plotter->hpgl_bad_pen = false;
	}
      else
	/* aren't using pen #0, so set advisory flag */
	_plotter->hpgl_bad_pen = true;
    }

  else
  /* color not in palette, must do something */
    if (_plotter->hpgl_version == 2 && _plotter->hpgl_can_assign_colors)
      /* CASE #1: HP-GL/2 and can soft-define pen colors */
      {
	/* assign current `free pen' to be the new color */
	sprintf (_plotter->data->page->point, "PC%d,%d,%d,%d;", 
		 _plotter->hpgl_free_pen, red, green, blue);
	_update_buffer (_plotter->data->page);
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].red = red;
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].green = green;
	_plotter->hpgl_pen_color[_plotter->hpgl_free_pen].blue = blue;
	_plotter->hpgl_pen_defined[_plotter->hpgl_free_pen] = 1; /* soft-def */
	/* select pen */
	_pl_h_set_hpgl_pen (R___(_plotter) _plotter->hpgl_free_pen);
	/* update free pen, i.e. choose next non-hard-defined pen */
	do
	  _plotter->hpgl_free_pen = (_plotter->hpgl_free_pen + 1) % HPGL2_MAX_NUM_PENS;
	while (_plotter->hpgl_pen_defined[_plotter->hpgl_free_pen] == 2);
	/* set fill type to solid, unidirectional */
	_pl_h_set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SOLID_UNI, 
				  0.0, 0.0); /* options ignored */
	
	_plotter->hpgl_bad_pen = false;
      }

    else if (_plotter->hpgl_version == 2 
	     && _plotter->hpgl_can_assign_colors == false)
      /* CASE #2: HP-GL/2, but can't soft-define pen colors; locate closest
	 point in RGB cube that is a desaturated version of one of the
	 defined pen colors, and fill by shading at the appropriate level */
      {
	double shading;
	
	_pl_h_hpgl_shaded_pseudocolor (R___(_plotter) 
				       red, green, blue, &i, &shading);
	
	if (i != 0 || (i == 0 && USE_PEN_ZERO))
	  /* can be selected */
	  {
	    _pl_h_set_hpgl_pen (R___(_plotter) i);
	    /* shading level in HP-GL/2 is expressed as a percentage */
	    _pl_h_set_hpgl_fill_type (R___(_plotter) HPGL_FILL_SHADED, 
				      100.0 * shading, 0.0); /* 2nd option ignord */
	    _plotter->hpgl_bad_pen = false;
	  }
	else
	  /* aren't using pen #0, so set advisory flag */
	  _plotter->hpgl_bad_pen = true;
      }

    else
      /* CASE #3: HPGL_VERSION must be "1" (i.e. generic HP-GL) or "1.5"
	 (i.e. HP7550A), so (a) determine which shade of which defined pen
	 is closest to the fill color in the sense of Euclidean distance
	 within the RGB cube, and (b) select a cross-hatch distance that
	 will emulate this shade.  For this, we use the algorithm that the
	 HP-GL/2 counterpart of the HP7550A, the HP7550B, uses.  As with
	 the HP7550B, we use a cross-hatch angle of 45 degrees.  */
      {
	double shading;
	
	_pl_h_hpgl_shaded_pseudocolor (R___(_plotter) 
				       red, green, blue, &i, &shading);
	
	if (i != 0 && shading > 0.01)
	  /* pen can be selected; note that we insist that shading level be
	     at least 1%, to avoid silly huge inter-line spacings, and also
	     division by zero */
	  {
	    double interline_distance;
	    
	    _pl_h_set_hpgl_pen (R___(_plotter) i);
	    
	    /* convert shading fraction to cross-hatch distance */
	    
	    /* If w=width of pen, d=distance between lines, and f=fraction,
	       then f = (2wd - w^2)/(d^2).  I.e., fd^2 - 2wd +w^2 = 0.
	       Relevant solution is d = (w/f) [1 + sqrt(1-f)].
	       
	       HP7550B algorithm assume that w = 0.3mm = 12 plotter units,
	       which is a standard width for plotter pens.  So that's what
	       we use for w also; we call it HPGL_NOMINAL_PEN_WIDTH.
	       
	       We specify spacing in native plotter units because that's
	       what the HP7550B does.  Its interpretation of shading level
	       as crosshatching is entirely independent of the definition
	       of user units, the locations of the scaling points, etc. */

	    interline_distance 
	      = HPGL_NOMINAL_PEN_WIDTH * (1.0 + sqrt (1.0 - shading)) /shading;
	    
	    _pl_h_set_hpgl_fill_type (R___(_plotter) HPGL_FILL_CROSSHATCHED_LINES, 
				      interline_distance, 45.0); /* 45 degrees */
	    _plotter->hpgl_bad_pen = false;
	  }
	else
	  /* aren't doing any filling (which would be white or near-white),
	     so set advisory flag */
	  _plotter->hpgl_bad_pen = true;
      }
}

/* Low-level routine that emits the HP-GL `SP' instruction to set the pen
   color by selecting a pen in the palette, by number. */

void 
_pl_h_set_hpgl_pen (R___(Plotter *_plotter) int new_pen)
{
  if (new_pen != _plotter->hpgl_pen) /* need to select new pen */
    {
      if (_plotter->hpgl_pendown)
	{
	  sprintf (_plotter->data->page->point, "PU;");
	  _update_buffer (_plotter->data->page);
	  _plotter->hpgl_pendown = false;
	}
      sprintf (_plotter->data->page->point, "SP%d;", new_pen);
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_pen = new_pen;
    }
}

/* Low-level routine for HP-GL/2 only, which emits an `SV' instruction to
   select not a pen, but rather a `screening type', i.e. an area fill type
   such as a shading, that will be applied to all pen strokes.  (Nearly all
   HP-GL/2 devices that aren't pen plotters support `screened vectors'.)
   This permits accurate matching of user-specified pen colors; see
   above. */

void
_pl_h_set_hpgl_pen_type (R___(Plotter *_plotter) int new_hpgl_pen_type, double option1, double option2)
{
  if (new_hpgl_pen_type != _plotter->hpgl_pen_type
      /* in shading case, we store the current shading level in the option1
	 field */
      || (new_hpgl_pen_type == HPGL_PEN_SHADED 
	  && _plotter->hpgl_pen_option1 != option1)
      /* in predefined pattern case (there are six cross-hatch patterns
	 that are imported from PCL or RTL, each of which has line width 4
	 dots and cell size 32x32 dots on a 600dpi printer), we store the
	 current pattern type in the option1 field */
      || (new_hpgl_pen_type == HPGL_PEN_PREDEFINED_CROSSHATCH
	  && _plotter->hpgl_pen_option1 != option1))
    /* need to emit `SV' instruction to change vector screening */
    {
      switch (new_hpgl_pen_type)
	{
	case HPGL_PEN_SOLID:
	default:
	  /* options ignored */
	  sprintf (_plotter->data->page->point, "SV;");
	  break;
	case HPGL_PEN_SHADED:
	  /* option1 is shading level in percent */
	  sprintf (_plotter->data->page->point, "SV%d,%.1f;", 
		   new_hpgl_pen_type, option1);
	  /* stash shading level */
	  _plotter->hpgl_pen_option1 = option1;
	  break;
	case HPGL_PEN_PREDEFINED_CROSSHATCH: /* imported from PCL or RTL */
	  /* option1 is pattern type, in range 1..6 */
	  sprintf (_plotter->data->page->point, "SV%d,%d;",
		   new_hpgl_pen_type, IROUND(option1));
	  /* stash pattern type */
	  _plotter->hpgl_pen_option1 = option1;
	  break;
	}
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_pen_type = new_hpgl_pen_type;
    }
}

/* Low-level routine, which emits the HP-GL `FT' instruction to set a `fill
   type', e.g., a shading or a cross-hatching, that will be applied when
   doing filling operations.  WARNING: in the case of filling with
   cross-hatched or parallel lines, this monkeys with the line type (it
   sets it to `solid'). */

void
_pl_h_set_hpgl_fill_type (R___(Plotter *_plotter) int new_hpgl_fill_type, double option1, double option2)
{
  if (new_hpgl_fill_type != _plotter->hpgl_fill_type
      /* in shading case, we store the current shading level in the option1
	 field */
      || (new_hpgl_fill_type == HPGL_FILL_SHADED 
	  && _plotter->hpgl_fill_option1 != option1)
      /* in cross-hatched or parallel line case, we store the current
	 inter-line distance (in plotter units) in the option1 field, and
	 and the line angle in the option2 field */
      || ((new_hpgl_fill_type == HPGL_FILL_CROSSHATCHED_LINES
	   || new_hpgl_fill_type == HPGL_FILL_PARALLEL_LINES)
	  && (_plotter->hpgl_fill_option1 != option1
	      || _plotter->hpgl_fill_option2 != option2))
      /* in predefined fill pattern case (there are six cross-hatch
	 patterns that are imported from PCL or RTL, each of which has line
	 width 4 dots and cell size 32x32 dots on a 600dpi printer), we
	 store the current pattern type in the option1 field */
      || (new_hpgl_fill_type == HPGL_FILL_PREDEFINED_CROSSHATCH
	  && _plotter->hpgl_fill_option1 != option1))
    /* need to emit `FT' instruction to change fill type */
    {
      switch (new_hpgl_fill_type)
	{
	case HPGL_FILL_SOLID_BI: /* bidirectional solid fill */
	case HPGL_FILL_SOLID_UNI: /* unidirectional solid fill */
	default:
	  /* options ignored */
	  sprintf (_plotter->data->page->point, "FT%d;", new_hpgl_fill_type);
	  break;
	case HPGL_FILL_SHADED:
	  /* option1 is shading level in percent */
	  sprintf (_plotter->data->page->point, "FT%d,%.1f;", 
		   new_hpgl_fill_type, option1);
	  /* stash shading level */
	  _plotter->hpgl_fill_option1 = option1;
	  break;
	case HPGL_FILL_CROSSHATCHED_LINES:
	case HPGL_FILL_PARALLEL_LINES:
	  /* Our convention: option1 is inter-line distance in plotter
	     units (option2 is angle of lines).  By emitting `SC' commands,
	     we switch from using user units to plotter units, and back
	     (for the latter, cf. setup commands in h_openpl.c).  Also, we
	     always switch to the solid line type for drawing the lines
	     (see warning above). */
	  sprintf (_plotter->data->page->point, 
		   "LT;SC;FT%d,%d,%d;SC%d,%d,%d,%d;",
		   new_hpgl_fill_type, IROUND(option1), IROUND(option2),
		   IROUND (_plotter->data->xmin), IROUND (_plotter->data->xmax), 
		   IROUND (_plotter->data->ymin), IROUND (_plotter->data->ymax));
	  _plotter->hpgl_line_type = HPGL_L_SOLID;
	  /* stash inter-line distance and angle of lines */
	  _plotter->hpgl_fill_option1 = option1;
	  _plotter->hpgl_fill_option2 = option2;
	  break;
	case HPGL_FILL_PREDEFINED_CROSSHATCH: /* imported from PCL or RTL */
	  /* option1 is pattern type, in range 1..6 */
	  sprintf (_plotter->data->page->point, "FT%d,%d;",
		   new_hpgl_fill_type, IROUND(option1));
	  /* stash pattern type */
	  _plotter->hpgl_fill_option1 = option1;
	  break;
	}
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_fill_type = new_hpgl_fill_type;
    }
}

/* Find closest point within the RGB color cube that is a defined pen
   color, using Euclidean distance as our metric.  Final arg, if set,
   specifies that nonwhite colors should never be quantized to white. */
int
_pl_h_hpgl_pseudocolor (R___(Plotter *_plotter) int red, int green, int blue, bool restrict_white)
{
  unsigned long int difference = INT_MAX;
  int i;
  int best = 0;

  if (red == 0xff && green == 0xff && blue == 0xff)
    /* white pen */
    return 0;

  for (i = (restrict_white ? 1 : 0); i < HPGL2_MAX_NUM_PENS; i++)
    {
      if (_plotter->hpgl_pen_defined[i] != 0)
	{
	  unsigned long int newdifference;
	  int ored, ogreen, oblue;
	  
	  ored = _plotter->hpgl_pen_color[i].red;
	  ogreen = _plotter->hpgl_pen_color[i].green;
	  oblue = _plotter->hpgl_pen_color[i].blue;
	  newdifference = ((red - ored) * (red - ored)
			   + (green - ogreen) * (green - ogreen)
			   + (blue - oblue) * (blue - oblue));
	  
	  if (newdifference < difference)
	    {
	      difference = newdifference;
	      best = i;
	    }
	}
    }
  return best;
}

/* Locate closest point in RGB cube that is a desaturated ("shaded")
   version of one of the defined pen colors, using Euclidean distance as
   our metric. */
void
_pl_h_hpgl_shaded_pseudocolor (R___(Plotter *_plotter) int red, int green, int blue, int *pen_ptr, double *shading_ptr)
{
  int best = 0;
  int i;
  double best_shading = 0.0;
  double difference = INT_MAX;
  double red_shifted, green_shifted, blue_shifted;
  
  /* shift color vector so that it emanates from `white' */
  red_shifted = (double)(red - 0xff);
  green_shifted = (double)(green - 0xff);
  blue_shifted = (double)(blue - 0xff);

  /* begin with pen #1 */
  for (i = 1; i < HPGL2_MAX_NUM_PENS; i++)
    {
      int ored, ogreen, oblue;
      double ored_shifted, ogreen_shifted, oblue_shifted;
      double red_proj_shifted, green_proj_shifted, blue_proj_shifted;
      double reciprocal_normsquared, dotproduct;
      double newdifference, shading;
      
      /* skip undefined pens */
      if (_plotter->hpgl_pen_defined[i] == 0)
	continue;
      
      /* shift each pen color vector so that it emanates from `white' */
      ored = _plotter->hpgl_pen_color[i].red;
      ogreen = _plotter->hpgl_pen_color[i].green;
      oblue = _plotter->hpgl_pen_color[i].blue;
      /* if luser specified a white pen, skip it to avoid division by 0 */
      if (ored == 0xff && ogreen == 0xff && oblue == 0xff)
	continue;
      ored_shifted = (double)(ored - 0xff);
      ogreen_shifted = (double)(ogreen - 0xff);
      oblue_shifted = (double)(oblue - 0xff);

      /* project shifted color vector onto shifted pen color vector */
      reciprocal_normsquared = 1.0 / (ored_shifted * ored_shifted
				      + ogreen_shifted * ogreen_shifted
				      + oblue_shifted * oblue_shifted);
      dotproduct = (red_shifted * ored_shifted
		    + green_shifted * ogreen_shifted
		    + blue_shifted * oblue_shifted);
      shading = reciprocal_normsquared * dotproduct;
      
      red_proj_shifted = shading * ored_shifted;
      green_proj_shifted = shading * ogreen_shifted;
      blue_proj_shifted = shading * oblue_shifted;
      
      newdifference = (((red_proj_shifted - red_shifted) 
			* (red_proj_shifted - red_shifted))
		       + ((green_proj_shifted - green_shifted) 
			  * (green_proj_shifted - green_shifted))
		       + ((blue_proj_shifted - blue_shifted) 
			  * (blue_proj_shifted - blue_shifted)));
      
      if (newdifference < difference)
	{
	  difference = newdifference;
	  best = i;
	  best_shading = shading;
	}
    }

  /* compensate for roundoff error */
  if (best_shading <= 0.0)
    best_shading = 0.0;

  *pen_ptr = best;
  *shading_ptr = best_shading;
}
