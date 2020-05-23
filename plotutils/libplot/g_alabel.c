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

/* This file contains the alabel method, which is a GNU extension to
   libplot.  It draws a label, i.e. a text string, at the current location.
   Horizontal and vertical justification must be specified.

   ALABEL takes three arguments X_JUSTIFY, Y_JUSTIFY, and S, and places the
   label S according to the x and y axis adjustments specified in X_JUSTIFY
   and Y_JUSTIFY.  X_JUSTIFY is equal to 'l', 'c', or 'r', signifying
   left-justified, centered, or right-justified, relative to the current
   position.  Y_JUSTIFY is equal to 'b', 'x', 'c', or 't', signifying that
   the bottom, baseline, center, or top of the label should pass through
   the current position. */

/* This file contains the label method, which is a standard part of libplot
   (supplied for backward compatibility).  It draws a label, i.e. a text
   string, at the current location of the graphics device cursor.  It is
   obsoleted by the alabel method, which allows justification. */

/* This file also contains the labelwidth method, which is a GNU extension
   to libplot.  It returns the width in user units of a label, i.e., a text
   string. */

#include "sys-defines.h"
#include "extern.h"
#include "g_control.h"

#define SCRIPTSIZE 0.6		/* rel. size of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.2)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.375

/* font we use for symbol escapes if the current font is a user-specified
   one [for X Windows] that doesn't belong to any of our builtin typefaces */
#define SYMBOL_FONT "Symbol"

/* Obsolete kludges to handle the zero-width marker symbols in our ArcMath
   and StickMath fonts; also zero-width overbar.  N.B. `8' AND `17' ARE
   HARDCODED IN THE TABLE IN g_fontd2.c. */
#define ARCMATH 8
#define STICKMATH 17
#define IS_MATH_FONT(fontnum) ((fontnum) == ARCMATH || (fontnum) == STICKMATH)
#define IS_CENTERED_SYMBOL(c) (((c) >= 'A' && (c) <= 'O') || (c) == 'e')

/* forward references */
static unsigned char *esc_esc_string (const unsigned char *s);
static bool simple_string (const unsigned short *codestring);
static bool clean_iso_string (unsigned char *s);

/* The flabelwidth() and falabel() methods.  After checking for control
   characters in the input string (not allowed), we invoke either a
   Hershey-specific or a non-Hershey-specific method. */

int
_API_alabel (R___(Plotter *_plotter) int x_justify, int y_justify, const char *s)
{
  char *t;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "alabel: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if (s == NULL)
    return 0;			/* avoid core dumps */

  /* copy because we may alter the string */
  t = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  /* allow only character set in ISO encoding */
  {
    bool was_clean;
    
    was_clean = clean_iso_string ((unsigned char *)t);
    if (!was_clean)
      _plotter->warning (R___(_plotter)
			 "ignoring control character (e.g. CR or LF) in label");
  }
  
  /* Be sure user-specified font has been retrieved.  Font is changed by
     fontname/fontsize/textangle, all of which invoke _pl_g_set_font(), and
     by space/space2/concat, which may not. */
  _pl_g_set_font (S___(_plotter));

  if (_plotter->data->have_escaped_string_support)
    /* Plotter supports the display of labels natively, including the
       escape sequences that we use in labels for subscripts, superscripts,
       shifts among fonts, etc.  Metafile Plotters are the only ones that
       are so powerful.  Actually they just write the label, escape
       sequences and all, to the output stream. :-) */
    _plotter->paint_text_string_with_escapes (R___(_plotter)
					      (unsigned char *)t, 
					      x_justify, y_justify);
  else
    /* must parse escape sequences (if any) in label */
    {
      if (_plotter->drawstate->font_type == PL_F_HERSHEY)
	/* call internal Hershey-specific routine to do the drawing, since
	   any label in a Hershey font supports additional escape sequences */
	_pl_g_alabel_hershey (R___(_plotter)
			      (unsigned char *)t, x_justify, y_justify);
      else
	/* non-Hershey: use parsing routine below, which ultimately calls
	   _plotter->paint_text_string to invoke Plotter-specific code */
	_pl_g_render_non_hershey_string (R___(_plotter)
					 t, true, x_justify, y_justify);
    }

  free (t);

  return 0;
}

int
_API_label (R___(Plotter *_plotter) const char *s)
{
  /* label should have baseline passing through current location, and
     should be left-justified */
  return _API_alabel (R___(_plotter) 'l', 'x', s);
}

double
_API_flabelwidth (R___(Plotter *_plotter) const char *s)
{
  double width = 0.0;
  char *t;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "flabelwidth: invalid operation");
      return -1;
    }

  if (s == NULL)
    return 0.0;			/* avoid core dumps */

  /* copy because we may alter the string */
  t = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (t, s);

  /* allow only character set in ISO encoding */
  {
    bool was_clean;
    
    was_clean = clean_iso_string ((unsigned char *)t);
    if (!was_clean)
      _plotter->warning (R___(_plotter) 
			 "ignoring control character (e.g. CR or LF) in label");
  }
  
  /* Be sure user-specified font has been retrieved.  Font is changed by
     fontname/fontsize/textangle, all of which invoke _pl_g_set_font(), and
     by space/space2/concat, which may not. */
  _pl_g_set_font (S___(_plotter));

  if (_plotter->drawstate->font_type == PL_F_HERSHEY)
    /* call Hershey-specific routine, since controlification acts slightly
       differently (a label in any Hershey font may contain more escape
       sequences than a label in a non-Hershey font) */
    width = _pl_g_flabelwidth_hershey (R___(_plotter)
				       (unsigned char *)t);
  else
    /* invoke routine below to compute width; final two args are ignored */
    width = _pl_g_render_non_hershey_string (R___(_plotter)
					     t, false, 'c', 'c');
  free (t);

  return width;
}

/* The non-Hershey version of the falabel() and flabelwidth() methods
   (merged).  They are distinguished by the do_render flag being
   true/false; the return values (the width of the string) are the same.
   The final two arguments, specifying justification, are relevant only if
   the do_render flag is `true'.  If do_render is true, the string is
   rendered in accordance with the justification instructions, and the
   graphics cursor position is updated accordingly. */

/* We `controlify' the string, translating escape sequences to annotations,
   before passing it to a lower-level rendering routine.  Note: for fonts
   of `OTHER' type [user-specified X Windows fonts], shifts between fonts
   within a single typeface are ignored, since we have no information on
   what the other fonts within the font's typeface are.  The annotations
   simply indicate whether or not a symbol font should be switched to, for
   the purpose of symbol escapes.  For fonts of `OTHER' type, font #1 means
   the user-specified font and font #0 means the symbol font. */

/* As noted, this version is invoked only if the current font is
   non-Hershey.  But due to failure to retrieve an X font, it is possible
   that the font could switch to a Hershey font during rendering.  So our
   lower-level rendering routine _pl_g_render_simple_string(), which this
   calls, may find itself invoked on a string to be rendered in a Hershey
   font.  That's OK; it can handle it. */

/* This routine checks whether a Plotter can handle horizontal and vertical
   justification requests.  If it can't, it does its own repositioning
   before invoking _pl_g_render_simple_string. */

/* ARGS: do_render = draw the string? */
double
_pl_g_render_non_hershey_string (R___(Plotter *_plotter) const char *s, bool do_render, int x_justify, int y_justify)
{
  int h_just = PL_JUST_LEFT;	/* all devices can handle left justification */
  int v_just = PL_JUST_BASE;
  unsigned short *codestring;
  unsigned short *cptr;
  double width = 0.0, added_width;
  double pushed_width = 0.0;	/* pushed by user */
  int current_font_index;
  /* initial values of these attributes (will be restored at end) */
  double initial_font_size;
  const char *initial_font_name;
  int initial_font_type;
  /* initial and saved locations */
  double initial_position_x = _plotter->drawstate->pos.x;
  double initial_position_y = _plotter->drawstate->pos.y;
  double pushed_position_x = _plotter->drawstate->pos.x;
  double pushed_position_y = _plotter->drawstate->pos.y;
  /* misc. */
  char x_justify_c, y_justify_c;
  double x_offset, y_offset;
  double x_displacement = 1.0, x_displacement_internal = 1.0;
  double overall_width = 0.0;
  double cap_height, ascent, descent;
  double userdx, userdy, theta, sintheta = 0.0, costheta = 1.0;
  
  /* convert string to a codestring, including annotations */
  codestring = _pl_g_controlify (R___(_plotter) (const unsigned char *)s);

  if (do_render)		/* perform needed computations; reposition */
    {
      /* compute label width in user units via a recursive call; final two
	 args are ignored */
      overall_width = _pl_g_render_non_hershey_string (R___(_plotter)
						       s, false, 'c', 'c');
      
      /* compute initial offsets that must be performed due to
       justification; also displacements that must be performed after
       rendering (see above)*/
      x_justify_c = (char)x_justify;
      y_justify_c = (char)y_justify;  

      switch (x_justify_c)
	{
	case 'l':		/* left justified */
	default:
	  h_just = PL_JUST_LEFT;
	  x_offset = 0.0;
	  x_displacement = 1.0;
	  x_displacement_internal = 1.0;
	  /* range [0,1] */
	  break;
	  
	case 'c':		/* centered */
	  h_just = PL_JUST_CENTER;
	  x_offset = -0.5;
	  x_displacement = 0.0;
	  x_displacement_internal = 0.0;
	  /* range [-0.5,0.5] */
	  break;
	  
	case 'r':		/* right justified */
	  h_just = PL_JUST_RIGHT;
	  x_offset = -1.0;
	  x_displacement = -1.0;
	  x_displacement_internal = -1.0;
	  /* range [-1,0] */
	  break;
	}

      /* need these to compute offset for vertical justification */
      cap_height = _plotter->drawstate->font_cap_height;
      ascent = _plotter->drawstate->font_ascent;
      descent = _plotter->drawstate->font_descent;
      
      switch (y_justify_c)	/* placement of label with respect
				   to y coordinate */
	{
	case 'b':		/* current point is at bottom */
	  v_just = PL_JUST_BOTTOM;
	  y_offset = descent;
	  break;
	  
	case 'x':		/* current point is on baseline */
	default:
	  v_just = PL_JUST_BASE;
	  y_offset = 0.0;
	  break;
	  
	case 'c':		/* current point midway between bottom, top */
	  v_just = PL_JUST_HALF;
	  y_offset = 0.5 * (descent - ascent);
	  break;
	  
	case 'C':		/* current point is on cap line */
	  v_just = PL_JUST_CAP;
	  y_offset = - cap_height;
	  break;
	  
	case 't':		/* current point is at top */
	  v_just = PL_JUST_TOP;
	  y_offset = - ascent;
	  break;
	}

      /* If codestring is a string in a single font, with no control codes,
	 we'll render it using native device justification, rather than
	 positioning a left-justified string by hand.  So e.g., if right or
	 centered justification was specified when alabel() was called by
	 the user, the string as drawn on the device will have the same
	 justification.  This is particularly important for the Fig and AI
	 drivers.  Anything else would exasperate the user, even if the
	 positioning is correct. */

      if ((_plotter->drawstate->font_type == PL_F_HERSHEY
	   || _plotter->data->have_horizontal_justification)
	  && simple_string (codestring))
	/* will use native justification, so don't perform initial offset */
	x_offset = 0.0;
      else
	/* will use x_offset to position by hand */
	{
	  h_just = PL_JUST_LEFT;
	  x_displacement_internal = 1.0;
	}
	  
      /* Similarly, in simple cases use native vertical justification if
         it's available (very few types of Plotter support it). */

      if ((_plotter->drawstate->font_type == PL_F_HERSHEY
	   || _plotter->data->have_vertical_justification)
	  && simple_string (codestring))
	/* will use native justification, so don't perform initial offset */
	y_offset = 0.0;
      else
	/* will use y_offset to position by hand */
	v_just = PL_JUST_BASE;
	  
      /* justification-related offsets we'll carry out */
      userdx = x_offset * overall_width;
      userdy = y_offset;
      
      /* label rotation angle in radians */
      theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
      sintheta = sin (theta);
      costheta = cos (theta);

      /* perform both horizontal and vertical offsets; after this, current
	 point will be on intended baseline of label */
      _plotter->drawstate->pos.x += costheta * userdx - sintheta * userdy;
      _plotter->drawstate->pos.y += sintheta * userdx + costheta * userdy;
    }

  /* save font name (will be restored at end) */
  {
    char *font_name;
    
    initial_font_name = _plotter->drawstate->font_name;
    font_name = (char *)_pl_xmalloc (1 + strlen (initial_font_name));
    strcpy (font_name, initial_font_name);
    _plotter->drawstate->font_name = font_name;
  }

  /* save font size too */
  initial_font_size = _plotter->drawstate->font_size;

  /* also save the font type, since for fonts of type PL_F_OTHER (e.g.,
     user-specified X Windows fonts not in our tables), switching fonts
     between substrings, e.g. to use the X Windows symbol font, may
     inconveniently switch _plotter->drawstate->font_type on us */
  initial_font_type = _plotter->drawstate->font_type;

  /* initialize current font index (font type presumably is not Hershey) */
  switch (_plotter->drawstate->font_type)
    {
    case PL_F_HERSHEY:
      current_font_index =
	(_pl_g_hershey_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_POSTSCRIPT:
      current_font_index =
	(_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_PCL:
      current_font_index =
	(_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_STICK:
      current_font_index =
	(_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_OTHER:
      current_font_index = 1;	/* `1' just means the font we start out with */
      break;
    default:			/* shouldn't happen */
      return 0.0;
    }

  /* now loop through codestring, parsing each code in succession; when
     shifting to subscripts/superscripts we change the nominal font size,
     and retrieve a new font */

  cptr = codestring;
  while (*cptr)			/* end when (unsigned short)0 is seen */
    {
      unsigned short c;
      
      c = *cptr;
      if (c & CONTROL_CODE)	
	/* parse control code; many possibilities */
	{	
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	      width += SUBSCRIPT_DX * _plotter->drawstate->true_font_size;
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += 
		    (costheta * SUBSCRIPT_DX - sintheta * SUBSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += 
		    (sintheta * SUBSCRIPT_DX + costheta * SUBSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		}
	      _plotter->drawstate->font_size *= SCRIPTSIZE;
	      _pl_g_set_font (S___(_plotter));
	      break;

	    case C_BEGIN_SUPERSCRIPT :
	      width += SUPERSCRIPT_DX * _plotter->drawstate->true_font_size;
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += 
		    (costheta * SUPERSCRIPT_DX - sintheta * SUPERSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += 
		    (sintheta * SUPERSCRIPT_DX + costheta * SUPERSCRIPT_DY) 
		      * _plotter->drawstate->true_font_size;
		}
	      _plotter->drawstate->font_size *= SCRIPTSIZE;
	      _pl_g_set_font (S___(_plotter));
	      break;

	    case C_END_SUBSCRIPT:
	      width -= SUBSCRIPT_DX * _plotter->drawstate->true_font_size;
	      _plotter->drawstate->font_size /= SCRIPTSIZE;
	      _pl_g_set_font (S___(_plotter));
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= (costheta * SUBSCRIPT_DX 
						   - sintheta * SUBSCRIPT_DY) * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= (sintheta * SUBSCRIPT_DX
						   + costheta * SUBSCRIPT_DY) * _plotter->drawstate->true_font_size;
		}
	      break;
	      
	    case C_END_SUPERSCRIPT:
	      width -= SUPERSCRIPT_DX * _plotter->drawstate->true_font_size;
	      _plotter->drawstate->font_size /= SCRIPTSIZE;
	      _pl_g_set_font (S___(_plotter));
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= (costheta * SUPERSCRIPT_DX 
						   - sintheta * SUPERSCRIPT_DY) * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= (sintheta * SUPERSCRIPT_DX
						   + costheta * SUPERSCRIPT_DY) * _plotter->drawstate->true_font_size;
		}
	      break;
	      
	    case C_PUSH_LOCATION:
	      pushed_position_x = _plotter->drawstate->pos.x;
	      pushed_position_y = _plotter->drawstate->pos.y;
	      pushed_width = width;
	      break;
	      
	    case C_POP_LOCATION:
	      if (do_render)
		{
		  _plotter->drawstate->pos.x = pushed_position_x;
		  _plotter->drawstate->pos.y = pushed_position_y;
		}
	      width = pushed_width;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      if (do_render)
		{
		  _plotter->drawstate->pos.x += costheta * _plotter->drawstate->true_font_size;
		  _plotter->drawstate->pos.y += sintheta * _plotter->drawstate->true_font_size;
		}
	      width += _plotter->drawstate->true_font_size;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 2.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 2.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 2.0;
	      break;

	    case C_RIGHT_QUARTER_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 4.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 4.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 4.0;
	      break;

	    case C_RIGHT_SIXTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 6.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 6.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 6.0;
	      break;

	    case C_RIGHT_EIGHTH_EM:
	      if (do_render)
		{

		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 8.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 8.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 8.0;
	      break;

	    case C_RIGHT_TWELFTH_EM:
	      if (do_render)
		{

		  (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size / 12.0;
		  (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size / 12.0;
		}
	      
	      width += _plotter->drawstate->true_font_size / 8.0;
	      break;

	      /* Kludge: used only for \rn macro, i.e. in square roots, if
		 the current font is a PS or PCL font.  See g_cntrlify.c.
		 If the font is a Hershey font, \rn is implemented
		 differently, and for Stick fonts it isn't implemented at all.

		 Painfully, the amount of shift differs depending whether
		 this is a PS or a PCL typeface, since the `radicalex'
		 characters are quite different.  See comment in
		 g_cntrlify.c. */
	    case C_RIGHT_RADICAL_SHIFT:
	      if (do_render)
		{
		  if (_plotter->drawstate->font_type == PL_F_PCL)
		    {
		      (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		    }
		  else
		    {
		      (_plotter->drawstate->pos).x += costheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y += sintheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		    }
		  /* I'm going to let this serve for the PCL case; it seems
		     to work (i.e. yield more or less the correct width).
		     We definitely don't want PCL_RADICAL_WIDTH here. */
		}
	      width += _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
	      break;

	    case C_LEFT_ONE_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size;
		}
	      
	      width -= _plotter->drawstate->true_font_size;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 2.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 2.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 2.0;
	      break;

	    case C_LEFT_QUARTER_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 4.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 4.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 4.0;
	      break;

	    case C_LEFT_SIXTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 6.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 6.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 6.0;
	      break;

	    case C_LEFT_EIGHTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 8.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 8.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 8.0;
	      break;

	    case C_LEFT_TWELFTH_EM:
	      if (do_render)
		{
		  (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size / 12.0;
		  (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size / 12.0;
		}
	      
	      width -= _plotter->drawstate->true_font_size / 8.0;
	      break;

	      /* Kludge: used only for \rn macro, i.e. in square roots.
		 Painfully, the amount of shift differs depending whether
		 this is a PS or a PCL typeface, since the `radicalex'
		 characters are quite different.  See comment above, and
		 comment in g_cntrlify.c. */
	    case C_LEFT_RADICAL_SHIFT:
	      if (do_render)
		{
		  if (_plotter->drawstate->font_type == PL_F_PCL)
		    {
		      (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size * PCL_RADICAL_WIDTH;
		    }
		  else
		    {
		      (_plotter->drawstate->pos).x -= costheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		      (_plotter->drawstate->pos).y -= sintheta * _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
		    }
		}
	      /* see comment in C_RIGHT_RADICAL_SHIFT case, above */
	      width -= _plotter->drawstate->true_font_size * PS_RADICAL_WIDTH;
	      break;
	      
	      /* unrecognized control code */
	    default:
	      break;
	    }

	  cptr++;		/* on to next element of codestring */
	}
      
      else		/* an ordinary character, with font annotation */
	{
	  unsigned char *s, *sptr;
	  int new_font_index = (c >> FONT_SHIFT) & ONE_BYTE;

	  /* perform font switching if necessary */
	  if (new_font_index != current_font_index)
	    {
	      /* We check initial_font_type, not _drawstate->font_type,
		 because the latter gets trashed if e.g. (1) we start out
		 with a font of type PL_F_OTHER, e.g. a user-specified X
		 Windows font not in our tables, and (2) we switch to the X
		 Windows Symbol font in mid-string, since that font is of
		 type PL_F_POSTSCRIPT. */
	      switch (initial_font_type)
		{
		case PL_F_HERSHEY:
		  free ((char *)_plotter->drawstate->font_name);
		  {
		    char *font_name;
		    
		    font_name =
		      (char *)_pl_xmalloc(1 + strlen (_pl_g_hershey_font_info[new_font_index].name));
		    strcpy (font_name, _pl_g_hershey_font_info[new_font_index].name);
		    _plotter->drawstate->font_name = font_name;
		  }
		  break;

		case PL_F_POSTSCRIPT:
		  free ((char *)_plotter->drawstate->font_name);
		  {
		    char *font_name;
		    
		    font_name =
		      (char *)_pl_xmalloc(1 + strlen (_pl_g_ps_font_info[new_font_index].ps_name));
		    strcpy (font_name, _pl_g_ps_font_info[new_font_index].ps_name);
		    _plotter->drawstate->font_name = font_name;
		  }
		  break;

		case PL_F_PCL:
		  free ((char *)_plotter->drawstate->font_name);
		  {
		    char *font_name;

		    font_name =
		      (char *)_pl_xmalloc(1 + strlen (_pl_g_pcl_font_info[new_font_index].ps_name));
		    strcpy (font_name, _pl_g_pcl_font_info[new_font_index].ps_name);
		    _plotter->drawstate->font_name = font_name;
		  }
		  break;

		case PL_F_STICK:
		  free ((char *)_plotter->drawstate->font_name);
		  {
		    char *font_name;

		    font_name =
		      (char *)_pl_xmalloc(1 + strlen (_pl_g_stick_font_info[new_font_index].ps_name));
		    strcpy (font_name, _pl_g_stick_font_info[new_font_index].ps_name);
		    _plotter->drawstate->font_name = font_name;
		  }
		  break;

		case PL_F_OTHER:
		  free ((char *)_plotter->drawstate->font_name);
		  {
		    char *font_name;

		    if (new_font_index == 0) /* symbol font */
		      {
			font_name =
			  (char *)_pl_xmalloc(1 + strlen (SYMBOL_FONT));
			strcpy (font_name, SYMBOL_FONT);
		      }
		    else
		      /* Currently, only alternative to zero (symbol font) is
			 1, i.e. restore font we started out with. */
		      {
			font_name =
			  (char *)_pl_xmalloc(1 + strlen (initial_font_name));
			strcpy (font_name, initial_font_name);
		      }

		    _plotter->drawstate->font_name = font_name;
		  }
		  break;

		default:	/* shouldn't happen */
		  break;
		}

	      _pl_g_set_font (S___(_plotter));
	      current_font_index = new_font_index;
	    }
	  
	  /* extract substring consisting of characters in the same font */
	  sptr = s 
	    = (unsigned char *)_pl_xmalloc ((4 * _codestring_len (cptr) + 1) * sizeof(char));
	  while (*cptr 
		 && (*cptr & CONTROL_CODE) == 0 
		 && ((*cptr >> FONT_SHIFT) & ONE_BYTE) == current_font_index)
	    *sptr++ = (*cptr++) & ONE_BYTE;
	  *sptr = (unsigned char)'\0';

	  /* Compute width of single-font substring in user units, add it.
	     Either render or not, as requested. */
	  added_width = _pl_g_render_simple_string (R___(_plotter)
						    s, do_render, h_just, v_just);
	  width += added_width;
	  if (do_render)
	    {
	      /* resposition due to rendering of label */
	      _plotter->drawstate->pos.x += 
		costheta * x_displacement_internal * added_width;
	      _plotter->drawstate->pos.y += 
		sintheta * x_displacement_internal * added_width;
	    }

	  free (s);
	}
    }

  /* free the codestring (no memory leaks please) */
  free (codestring);

  /* restore initial font */
  free ((char *)_plotter->drawstate->font_name);
  _plotter->drawstate->font_name = initial_font_name;
  _plotter->drawstate->font_size = initial_font_size;
  _pl_g_set_font (S___(_plotter));
  
  if (do_render)
    {
      /* restore position to what it was before printing label */
      _plotter->drawstate->pos.x = initial_position_x;
      _plotter->drawstate->pos.y = initial_position_y;
      /* shift due to printing of label */
      _plotter->drawstate->pos.x += costheta * x_displacement * overall_width;
      _plotter->drawstate->pos.y += sintheta * x_displacement * overall_width;
    }

  return width;
}

/* Compute the width of an ordinary single-font string (no escape sequences
   to switch fonts or position subscripts and superscripts, etc.), and also
   render it, if requested.

   The rendering only takes place if the do_render flag is set.  If it is
   not, the width is returned only (the h_just and v_just arguments being
   ignored).

   The font type here is arbitrary (either non-Hershey or non-Hershey).
   That makes this callable by _pl_g_render_non_hershey_string(), inside
   which the font can switch from non-Hershey to Hershey.  See comments
   above.

   This is never called to do rendering unless the Plotter can handle the
   specified types of justification. */

/* ARGS: h_just,v_just are PL_JUST_{LEFT|CENTER|RIGHT}, PL_JUST_{TOP etc.} */
double 
_pl_g_render_simple_string (R___(Plotter *_plotter) const unsigned char *s, bool do_render, int h_just, int v_just)
{
  double width;

  if (_plotter->drawstate->font_type == PL_F_HERSHEY)
      /* Use our internal Hershey width-computation or rendering routine.
	 But they do more than is needed: they handle escape sequences too,
	 via their own controlification.  So we escape all backslashes.
	 More importantly, we work around the fact that unlike the
	 Plotter-specific `paint_text_string' rendering routines,
	 _pl_g_alabel_hershey() shifts the current graphics cursor
	 position, since it draws Hershey characters as polygonal paths. */
    {
      unsigned char *t;
      
      t = esc_esc_string (s);
      width = _pl_g_flabelwidth_hershey (R___(_plotter) t);
      if (do_render)
	{
	  plPoint initial_pos;

	  initial_pos = _plotter->drawstate->pos; /* save */
	  _pl_g_alabel_hershey (R___(_plotter) t, h_just, v_just);
	  _plotter->drawstate->pos = initial_pos; /* restore */
	}
      free (t);
    }
  else
    /* not a Hershey font */
    {
      if (do_render)
	width = _plotter->paint_text_string (R___(_plotter) s, h_just, v_just);
      else
	width = _plotter->get_text_width (R___(_plotter) s);
    }

  return width;
}

/* A generic internal method that computes the width (total delta x) of a
   character string to be rendered in the currently selected font, so long
   as it is non-Hershey.  It accesses the font database in g_fontdb.c.

   The string is just a string (no control codes, font switchings, font
   annotations, etc.).

   This supports the 35 standard PS fonts, the 45 standard PCL fonts, and
   our Stick fonts (i.e. device-resident HP fonts).  It does not support
   `other' fonts, which some Plotters support; for such fonts, it returns
   0.0.  So Plotters that support `other' fonts, such as XDrawable and X
   Plotters, will need to override this, in toto. */

double
_pl_g_get_text_width (R___(Plotter *_plotter) const unsigned char *s)
{
  int index;
  int width = 0;
  double swidth = 0.0;
  unsigned char current_char;
  int master_font_index;	/* index into master table */
  double retval;

  switch (_plotter->drawstate->font_type)
    {
    case PL_F_POSTSCRIPT:
      /* compute font index in master PS font table */
      master_font_index =
	(_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      for (index=0; s[index]!='\0'; index++)
	{
	  current_char = (unsigned int)s[index];
	  width 
	    += ((_pl_g_ps_font_info[master_font_index]).width)[current_char];
	}
      
      retval = _plotter->drawstate->true_font_size * (double)width / 1000.0;
      break;
      
    case PL_F_PCL:
      /* compute font index in master PCL font table */
      master_font_index =
    (_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      for (index=0; s[index]!='\0'; index++)
	{
	  current_char = (unsigned int)s[index];
	  width 
	    += ((_pl_g_pcl_font_info[master_font_index]).width)[current_char];
	}
      
      retval = _plotter->drawstate->true_font_size * (double)width / 1000.0;
      break;
      
    case PL_F_STICK:
      /* compute font index in master table of device-resident HP fonts */
      master_font_index =
	(_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

      /* The width tables for Stick fonts (in g_fontdb.c) give character
	 widths in terms of abstract raster units (the grid on which the
	 character was defined).  Font size is twice the width of the
	 abstract raster (by definition).  So in principle, to compute the
	 width of a character string, we just need to add the character
	 widths together, and normalize using the font size.

	 It's more complicated than that, in part because our width tables
	 for Stick fonts, unlike those for PCL fonts, contain the bounding
	 box widths rather than the character cell widths.  Also, for
	 agreement with the PS rendering convention, we need to add a bit
	 of leading whitespace, and a bit of trailing whitespace.  There is
	 also the key issue of kerning: full-featured HP-GL and HP-GL/2
	 plotters normally kern text strings in any Stick font via
	 in-device kerning tables, although PCL devices such as LaserJets,
	 when doing HP-GL/2 emulation, apparently don't.

	 So there are two cases.

	 1. The case of no device-resident kerning, which is the case for
	 PCL devices such as LaserJets that do (lamebrained) HP-GL/2
	 emulation.  Much as for a string rendered in a PCL font, the true
	 width (character cell width) of a character equals

	 offset + bounding box width + offset

	 In fact, offset is independent of character; it depends only on
	 the font.  So the string width we compute for a string consisting
	 of n characters is:

	 offset + bb width #1 + offset 
	 + offset + bb width #2 + offset 
	 + ...
	 + offset + bb width #n + offset
	 
	 The first and last offsets in this formula provide the leading and
	 trailing bits of whitespace.

	 2. The case of device-resident kerning, according to HP's spacing
	 tables (our copies of the device spacing tables are in
	 g_fontd2.c).  The string width we return is:

	 offset + bb width #1 + spacing(1,2) + bb width #2 + spacing(2,3) 
	 + ... + spacing(n-1,n) + bb width #n + offset

	 where spacing(1,2) is the spacing between characters 1 and 2, etc.
	 
	 The basic reference for HP's kerning scheme for Stick fonts is
	 "Firmware Determines Plotter Personality", by L. W. Hennessee,
	 A. K. Frankel, M. A. Overton, and R. B. Smith, Hewlett-Packard
	 Journal, Nov. 1981, pp. 16-25.  Every character belongs to a `row
	 class' and a `column class', i.e., `right edge class' and `left
	 edge class'.  Any spacing table is indexed by row class and column
	 class.

	 [What HP later did in their LaserJets, which don't do kerning of
	 Stick fonts, is apparently a degenerate case of this setup, with
	 all the inter-character spacings changed to 2*offset.]

	 A couple of additional comments on kerning:
	 
	 Comment A.  The width of the space character (ASCII SP) is 3/2
	 times as large if kerning is used, as it is in in the absence of
	 kerning (e.g., in a LaserJet).  Without kerning, it's 0.5 times
	 the font size, like any other character, but in devices with
	 kerning, it's 0.75 times the font size.  That sounds like a major
	 difference, but the use of kerning more or less compensates for
	 it.  See comment in code below.

	 Comment B.  Our homebrewed ANK fonts consist of a lower half
	 encoded according to JIS ASCII, and an upper half encoded
	 according to the half-width Katakana encoding.  These two halves
	 are different HP 7-bit character sets and use different spacing
	 tables, since their abstract raster widths differ (42 and 45,
	 respectively).  HP's convention is apparently that if, between
	 character k and character k+1, there's a switch between spacing
	 tables and spacing(k,k+1) can't be computed via lookup, then

	 bb width #k + spacing(k,k+1) + bb width #(k+1)
	
	 should be replaced by

	 width_of_space_character + bb width #(k+1)

	 That's the way we do it.  */

      if (_plotter->data->kern_stick_fonts)
	/* have device-resident kerning, so we compute inter-character
	   spacing from spacing tables in g_fontd2.c, which we hope match
	   the device-resident tables */
	{
	  const struct plStickFontSpacingTableStruct *ktable_lower, *ktable_upper;
	  const struct plStickCharSpacingTableStruct *stable_lower, *stable_upper;
	  const short *lower_spacing, *upper_spacing;	/* spacing tables */
	  int lower_cols, upper_cols; 			/* table sizes */
	  const char *lower_char_to_row, *lower_char_to_col; /* char to pos */
	  const char *upper_char_to_row, *upper_char_to_col; /* char to pos */
	  bool halves_use_different_tables; /* upper/lower spacing tables differ?*/
	  
	  /* kerning table and spacing table structs, for each font half */
	  ktable_lower = &(_pl_g_stick_kerning_tables[_pl_g_stick_font_info[master_font_index].kerning_table_lower]);
	  ktable_upper = &(_pl_g_stick_kerning_tables[_pl_g_stick_font_info[master_font_index].kerning_table_upper]);
	  stable_lower = &(_pl_g_stick_spacing_tables[ktable_lower->spacing_table]);
	  stable_upper = &(_pl_g_stick_spacing_tables[ktable_upper->spacing_table]);
	  
	  /* do font halves use different spacing tables (e.g. ANK fonts)? */
	  halves_use_different_tables 
	    = (stable_lower != stable_upper ? true : false);
	  
	  /* numbers of columns in each of the two spacing tables (number of
	     rows isn't used) */
	  lower_cols = stable_lower->cols;  
	  upper_cols = stable_upper->cols;  
	  
	  /* arrays (size 128), mapping character to row/column of spacing
             table */
	  lower_char_to_row = ktable_lower->row;
	  lower_char_to_col = ktable_lower->col;  
	  upper_char_to_row = ktable_upper->row;
	  upper_char_to_col = ktable_upper->col;  
	  
	  /* spacing tables for each half of the font */
	  lower_spacing = stable_lower->kerns;
	  upper_spacing = stable_upper->kerns;  
	  
	  /* add an initial bit of whitespace (an `offset'), to make the
	     Stick font rendering agree with the PS font rendering
	     convention */
	  swidth		
	    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
		/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
	  
	  /* loop through chars in label */
	  for (index=0; s[index]!='\0'; index++)
	    {
	      unsigned char c, d;
	      
	      c = (unsigned int)s[index];
	      
	      if (c < 0x80)
		/* lower half */
		{
		  double spacefactor, char_width;
		  
		  /* Our width tables in g_fontd2.c are most appropriate
		     for LaserJets doing HP-GL/2 emulation, rather than for
		     true HP-GL/2.  Major difference is that in true
		     HP-GL/2, width of space character is 3/2 times larger,
		     e.g. in the Arc font it is 42 abstract raster units
		     rather than 28.  (This difference is partly
		     compensated for by true HP-GL/2 having kerning, unlike
		     LaserJets' HP-GL/2 emulation.)  */
		  if (c == ' ')
		    spacefactor = 1.5;
		  else
		    spacefactor = 1.0;
		  
		  /* add width of char */
		  char_width
		    = (((double)(_pl_g_stick_font_info[master_font_index].width[c]))
		       * spacefactor
		       /(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
		  swidth += char_width;
		  
		  if ((d = (unsigned int)s[index+1]) != '\0')
		    /* current char is not final char in string, so add spacing
		       between it and the next char */
		    {
		      int row, col;
		      int spacing;
		      
		      /* compute row class for current character, i.e., its
			 `right edge class' */
		      row = lower_char_to_row[c];
		      
		      /* compute and add spacing; if we switch from lower
			 to upper half here, and upper half uses a
			 different spacing table, just replace width of c
			 by width of ` ' (see explanation above) */
		      if (d < 0x80)
			{
			  col = lower_char_to_col[d];
			  spacing = lower_spacing[row * lower_cols + col];
			}
		      else if (!halves_use_different_tables)
			{
			  col = upper_char_to_col[d - 0x80];
			  spacing = lower_spacing[row * lower_cols + col];
			}
		      else if (c == ' ' || (d == ' ' + 0x80))
			/* space characters have no kerning */
			spacing = 0;
		      else	
			/* c -> ` ', see above. */
			spacing = 
			  - IROUND(spacefactor * _pl_g_stick_font_info[master_font_index].width[c])
			  + IROUND(1.5 * _pl_g_stick_font_info[master_font_index].width[' ']);
		      
		      swidth		
			+= ((double)spacing)
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower);
		    }
		}
	      else
		/* upper half */
		{
		  double spacefactor, char_width;
		  
		  if (c == ' ' + 0x80) /* i.e. `unbreakable SP' */
		    spacefactor = 1.5;
		  else
		    spacefactor = 1.0;
		  
		  /* add width of char */
		  char_width		
		    = (((double)(_pl_g_stick_font_info[master_font_index].width[c]))
		       /(2 * _pl_g_stick_font_info[master_font_index].raster_width_upper));
		  swidth += char_width;
		  
		  if ((d = (unsigned int)s[index+1]) != '\0')
		    /* current char is not final char in string, so add spacing
		       between it and the next char */
		    {
		      int row, col;
		      int spacing;
		      
		      /* compute row class for current character, i.e., its
			 `right edge class' */
		      row = upper_char_to_row[c - 0x80];
		      
		      /* compute and add spacing; if we switch from upper
			 to lower half here, and lower half uses a
			 different spacing table, just replace width of c
			 by width of ` ' (see explanation above) */
		      if (d >= 0x80)
			{
			  col = upper_char_to_col[d - 0x80];
			  spacing = upper_spacing[row * upper_cols + col];
			}
		      else if (!halves_use_different_tables)
			{
			  col = lower_char_to_col[d];
			  spacing = upper_spacing[row * upper_cols + col];
			}
		      else if ((c == ' ' + 0x80) || d == ' ')
			/* space characters have no kerning */
			spacing = 0;
		      else
			/* c -> ` ', see above. */
			spacing = 
			  - IROUND(spacefactor * _pl_g_stick_font_info[master_font_index].width[c])
			  + IROUND(1.5 * _pl_g_stick_font_info[master_font_index].width[' ']);
		      
		      swidth		
			+= ((double)spacing)
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_upper);
		    }
		}
	    }

	  /* add a trailing bit of whitespace (an `offset'), to make the
	     Stick font rendering agree with the PS rendering convention */
	  swidth		
	    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
		/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
	}
      else
	/* No device-resident kerning; this is the case, e.g., for PCL5
	   devices doing their (lamebrained) HP-GL/2 emulation.  We use a
	   fixed offset between each pair of characters, which is the way
	   HP LaserJets.  We also use this offset as the width of the `bit
	   of whitespace' that we add at beginning and end of label. */
	{
	  /* loop through chars in label */
	  for (index=0; s[index]!='\0'; index++)
	    {
	      unsigned char c;
	      
	      c = (unsigned int)s[index];
	      
#if 0
	      /* COMMENTED OUT BECAUSE IT WAS IDIOTIC */
	      /* kludge around HP's convention for centered marker symbols
		 (poor fellows ain't got no width a-tall) */
	      if (IS_MATH_FONT(master_font_index) && IS_CENTERED_SYMBOL(c))
		continue;
#endif
	      if (c < 0x80)
		/* lower half */
		{
		  swidth		
		    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
		  swidth		
		    += (((double)(_pl_g_stick_font_info[master_font_index].width[c]))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
		  swidth		
		    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
		}
	      else
		/* upper half */
		{
		  swidth		
		    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_upper));
		  swidth	
		    += (((double)(_pl_g_stick_font_info[master_font_index].width[c]))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_upper));
		  swidth		
		    += (((double)(_pl_g_stick_font_info[master_font_index].offset))
			/(2 * _pl_g_stick_font_info[master_font_index].raster_width_upper));
		}
	    }
	}
      
      /* normalize: use font size to convert width to user units */
      retval = _plotter->drawstate->true_font_size * (double)swidth;
      break;
      
    case PL_F_OTHER:
      retval = 0.0;
      break;
      
    default:			/* shouldn't happen */
      retval = 0.0;
      break;
    }

  return retval;
}

/* test whether a controlified string is simple in the sense that it
   consists of characters in a single font, and no control codes */
static bool
simple_string (const unsigned short *codestring)
{
  const unsigned short *cptr = codestring;
  unsigned short c, d;
  int font_index;

  if (*codestring == 0)
    return true;
  c = *codestring;
   if (c & CONTROL_CODE)
    return false;
  font_index = (c >> FONT_SHIFT) & ONE_BYTE;
  while ((d = *cptr++) != 0)
    {
      int local_font_index;

      if (d & CONTROL_CODE)
	return false;
      local_font_index = (d >> FONT_SHIFT) & ONE_BYTE;      
      if (local_font_index != font_index)
	return false;
    }
  return true;
}

/* Removes all characters not in the ISO-8859-?  character sets from a
   string.  I.e.  remove control characters (characters in the range 0x01
   to 0x1F, including LF and CR, and also 0x7f, i.e. DEL).  We take
   characters in the range 0x80 to 0x9F to be control characters too, since
   they are undefined in the ISO character sets.

   Actually, in PS fonts (with ISO encoding vector) they encode accents;
   and in the encoding used in Fig files, they encode a few special
   characters not found elsewhere.  But the interpretation of the
   0x80--0x9F range is device dependent, and our goal is device
   independence, so away the range goes. */

#define GOOD_ISO(c) (((c >= 0x20) && (c <= 0x7E)) || ((c >= 0xA0) && (c <= 0xFF)))

static bool
clean_iso_string (unsigned char *s)
{
  bool was_clean = true;
  unsigned char *t;
  
  for (t = s; *s; s++)
    {
      if (GOOD_ISO(*s))
	{
	  *t = *s;
	  t++;
	}
      else
	was_clean = false;
      
    }
  *t = (unsigned char)'\0';
  
  return was_clean;
}


/* escape all backslashes in a string; the returned string is allocated on
   the heap and can be freed. */
static unsigned char *
esc_esc_string (const unsigned char *s)
{
  const unsigned char *sptr;
  unsigned char *t, *tptr;

  t = (unsigned char *)_pl_xmalloc (2 * strlen ((char *)s) + 1);
  sptr = s;
  tptr = t;
  while (*sptr)
    {
      *tptr++ = *sptr;
      if (*sptr == '\\')
	*tptr++ = *sptr;
      sptr++;
    }
  *tptr = '\0';

  return t;
}

/* Versions of the falabel() method that do nothing; derived (non-generic)
   Plotters must override them if they wish to use them. */

void
_pl_g_paint_text_string_with_escapes (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  return;
}

double
_pl_g_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  return 0.0;
}
