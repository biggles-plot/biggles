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

/* This file contains the internal method _pl_g_alabel_hershey(), which
   plots a label using Hershey fonts.  Each character in a Hershey font is
   a sequence of pen motions, so this function calls _API_fmoverel() and
   _API_fcontrel() to `stroke' each character in the argument string.

   The width of the string in user units is returned.  The internal method
   _pl_g_flabelwidth_hershey() is similar, but does not actually plot the
   label.  */

#include "sys-defines.h"
#include "extern.h"
#include "g_control.h"
#include "g_her_metr.h"

/* Shearing factor for oblique fonts, new_x = x + SHEAR * y  */

#define SHEAR (2.0/7.0)

/* Relative size of subscripts/superscripts (i.e. `indexical' size) */

#define SCRIPTSIZE (0.6)	

/* Positioning of subscripts/superscripts */

#define SUBSCRIPT_DX 0.0
#define SUBSCRIPT_DY (-0.25)
#define SUPERSCRIPT_DX 0.0
#define SUPERSCRIPT_DY 0.4

/* Positioning of accents (in Hershey units).  UP_SHIFT is amount by which
   accents are raised when placed over upper-case letters.  RIGHT_SHIFT is
   applied as well, if the upper-case letter is italic. */

#define ACCENT_UP_SHIFT 7.0
#define ACCENT_RIGHT_SHIFT 2.0

/* Relative size of small Japanese Kana */
#define SMALL_KANA_SIZE 0.725

/* Hershey glyph arrays */

#define OCCIDENTAL 0
#define ORIENTAL 1

/* Location of first Kana in the occidental glyph arrays.  (Kana, unlike
   Kanji, are placed in the occidental array, at the very end.) */
#define BEGINNING_OF_KANA 4195

/* forward references */
static bool composite_char (unsigned char *composite, unsigned char *character, unsigned char *accent);
static double label_width_hershey (const unsigned short *label);

/* An version of the alabel() method that is specific to the case when the
   current Plotter font is a Hershey font.  It handles escape sequences for
   subscripts, superscripts, shifts among Hershey fonts, etc.  */
double
_pl_g_alabel_hershey (R___(Plotter *_plotter) const unsigned char *s, int x_justify, int y_justify)
{
  unsigned short *codestring;
  char x_justify_c, y_justify_c;
  double label_width, label_height;
  double x_offset, y_offset;
  double x_displacement;
  double postdx, dx, dy;
  double theta;

  /* convert string to a codestring, including annotations */
  codestring = _pl_g_controlify (R___(_plotter) s);

  /* dimensions of the string in user units */
  label_width = HERSHEY_UNITS_TO_USER_UNITS(label_width_hershey (codestring));
  label_height = HERSHEY_UNITS_TO_USER_UNITS(HERSHEY_HEIGHT);
  
  x_justify_c = (char)x_justify;
  y_justify_c = (char)y_justify;  

  switch (x_justify_c)
    {
    case 'l': /* left justified */
    default:
      x_offset = 0.0;
      x_displacement = 1.0;
      break;

    case 'c': /* centered */
      x_offset = -0.5;
      x_displacement = 0.0;
      break;

    case 'r': /* right justified */
      x_offset = -1.0;
      x_displacement = -1.0;
      break;
    }

  switch (y_justify_c)
    {
    case 'b':			/* current point is at bottom */
      y_offset = (double)HERSHEY_DESCENT / (double)HERSHEY_HEIGHT;
      break;

    case 'x':			/* current point is on baseline */
    default:
      y_offset = 0.0;
      break;

    case 'c': 			/* current point midway between bottom, top */
      y_offset = 0.5 * ((double)HERSHEY_DESCENT - (double)HERSHEY_ASCENT) 
		   / (double)HERSHEY_HEIGHT;
      break;

    case 'C':			/* current point is on cap line */
      y_offset = - (double)HERSHEY_CAPHEIGHT / (double)HERSHEY_HEIGHT;
      break;

    case 't':			/* current point is at top */
      y_offset = - (double)HERSHEY_ASCENT / (double)HERSHEY_HEIGHT;
      break;
    }

  /* save relevant drawing attributes, and restore them later */
  {
    char *old_line_mode, *old_cap_mode, *old_join_mode;
    int old_fill_type;
    double oldposx, oldposy;
    bool old_dash_array_in_effect;

    old_line_mode = (char *)_pl_xmalloc (strlen (_plotter->drawstate->line_mode) + 1);
    old_cap_mode = (char *)_pl_xmalloc (strlen (_plotter->drawstate->cap_mode) + 1);
    old_join_mode = (char *)_pl_xmalloc (strlen (_plotter->drawstate->join_mode) + 1);
    oldposx = _plotter->drawstate->pos.x;
    oldposy = _plotter->drawstate->pos.y;    

    strcpy (old_line_mode, _plotter->drawstate->line_mode);
    strcpy (old_cap_mode, _plotter->drawstate->cap_mode);
    strcpy (old_join_mode, _plotter->drawstate->join_mode);
    old_fill_type = _plotter->drawstate->fill_type;
    old_dash_array_in_effect = _plotter->drawstate->dash_array_in_effect;
    
    /* Our choices for rendering: solid lines, rounded capitals and joins,
       a line width equal to slightly more than 1 Hershey unit, and no
       filling.  

       We don't set the pen type: we allow it to be 0, which will mean no
       stroking at all. */
    _API_linemod (R___(_plotter) "solid");
    _API_capmod (R___(_plotter) "round");
    _API_joinmod (R___(_plotter) "round");
    _API_filltype (R___(_plotter) 0);
    
    /* move to take horizontal and vertical justification into account */
    {
      double theta, deltax, deltay, dx_just, dy_just;

      theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  
      deltax = x_offset * label_width;
      deltay = y_offset * label_height;
      dx_just = cos(theta) * deltax - sin(theta) * deltay;
      dy_just = sin(theta) * deltax + cos(theta) * deltay;
      
      _API_fmoverel (R___(_plotter) dx_just, dy_just);
    }
    
    /* call stroker on the sequence of strokes obtained from each char (the
       stroker may manipulate the line width) */
    _pl_g_draw_hershey_string (R___(_plotter) codestring);
    
    /* Restore original values of relevant drawing attributes, free
       storage.  endpath() will be invoked in here automatically, flushing
       the created polyline object comprising the stroked text. */
    _API_linemod (R___(_plotter) old_line_mode);
    _API_capmod (R___(_plotter) old_cap_mode);
    _API_joinmod (R___(_plotter) old_join_mode);
    _API_filltype (R___(_plotter) old_fill_type);
    _plotter->drawstate->dash_array_in_effect = old_dash_array_in_effect;
    
    free (old_line_mode);
    free (old_cap_mode);
    free (old_join_mode);

    /* return to original position */
    _API_fmove (R___(_plotter) oldposx, oldposy);
  }

  /* amount by which to shift after printing label (user units) */
  postdx = x_displacement * label_width;
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  dx = cos (theta) * postdx;
  dy = sin (theta) * postdx;

  _API_fmoverel (R___(_plotter) dx, dy);

  free (codestring);

  return label_width;		/* user units */
}

/* A version of the flabelwidth() method that is specific to the case when
   the current Plotter font is a Hershey font. */
double
_pl_g_flabelwidth_hershey (R___(Plotter *_plotter) const unsigned char *s)
{
  double label_width;
  unsigned short *codestring;
  
  /* convert string to a codestring, including annotations */
  codestring = _pl_g_controlify (R___(_plotter) s);

  label_width = HERSHEY_UNITS_TO_USER_UNITS(label_width_hershey (codestring));
  free (codestring);
  
  return label_width;
}

/* _pl_g_draw_hershey_stroke() draws a stroke, taking into account the
   transformation from Hershey units to user units, and also the angle in
   user space at which the label should be plotted. */

void
_pl_g_draw_hershey_stroke (R___(Plotter *_plotter) bool pendown, double deltax, double deltay)
{
  double theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  double dx, dy;

  deltax = HERSHEY_UNITS_TO_USER_UNITS (deltax);
  deltay = HERSHEY_UNITS_TO_USER_UNITS (deltay);

  dx = cos(theta) * deltax - sin(theta) * deltay;
  dy = sin(theta) * deltax + cos(theta) * deltay;

  if (pendown)
    _API_fcontrel (R___(_plotter) dx, dy);
  else
    _API_fmoverel (R___(_plotter) dx, dy);
}

/* label_width_hershey() computes the width (total delta x) of a
   controlified character string to be rendered in a vector font, in
   Hershey units.  Parsing must take into account the many control
   sequences which perform shifts, and initiate/terminate
   subscripts/superscripts. */

/* In addition to scaling the character sizes and the `width', we perform
   the following (dx, dy):
   
   enter subscript	(dx, dy) = (-1/9, -1/2) * width
   exit subscript	(dx, dy) = (+1/6, +1/2) * width
   
   enter superscript	(dx, dy) = (-1/9, +1/2) * width
   exit superscript	(dx, dy) = (+1/6, -1/2) * width
   
   For clarity here, `width' refers to the width _before_ it is
   multiplied by a factor 2/3.
   
   [N.B. In Bob Beach's original UGS character stroke generator,
   the +1/6's here were +2/9 instead.  Better?] */

static double
label_width_hershey (const unsigned short *label) 
{ 
  const unsigned short *ptr = label;
  unsigned short c;
  double charsize = 1.0;	/* relative char size, 1.0 means full size */
  double saved_charsize = 1.0;
  double width = 0.0;		/* label width */
  double saved_width = 0.0;
  
  /* loop through unsigned shorts in label */
  while ((c = (*ptr)) != (unsigned short)'\0') 
    {
      int glyphnum;		/* glyph in Hershey array */
      const unsigned char *glyph;
      
      if (c & RAW_HERSHEY_GLYPH) 
	/* glyph was spec'd via an escape, not as a char in a font */
	{
	  glyphnum = c & GLYPH_SPEC;
	  glyph = (const unsigned char *)(_pl_g_occidental_hershey_glyphs[glyphnum]);
	  
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      else if (c & RAW_ORIENTAL_HERSHEY_GLYPH) 
	/* glyph was spec'd via an escape, not as a char in a font */
	{
	  glyphnum = c & GLYPH_SPEC;
	  glyph = (const unsigned char *)_pl_g_oriental_hershey_glyphs[glyphnum];
	  
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      else if (c & CONTROL_CODE)	/* parse control code */
	{
	  switch (c & ~CONTROL_CODE)
	    {
	    case C_BEGIN_SUBSCRIPT:
	    case C_BEGIN_SUPERSCRIPT :
	      charsize *= SCRIPTSIZE;
	      break;
	      
	    case C_END_SUBSCRIPT:
	    case C_END_SUPERSCRIPT:
	      charsize /= SCRIPTSIZE;
	      break;
	      
	    case C_PUSH_LOCATION:
	      saved_width = width;
	      saved_charsize = charsize;
	      break;
	      
	    case C_POP_LOCATION:
	      width = saved_width;
	      charsize = saved_charsize;
	      break;
	      
	    case C_RIGHT_ONE_EM:
	      width += charsize * HERSHEY_EM;
	      break;
	      
	    case C_RIGHT_HALF_EM:
	      width += charsize * HERSHEY_EM / 2.0;
	      break;
	      
	    case C_RIGHT_QUARTER_EM:
	      width += charsize * HERSHEY_EM / 4.0;
	      break;
	      
	    case C_RIGHT_SIXTH_EM:
	      width += charsize * HERSHEY_EM / 6.0;
	      break;
	      
	    case C_RIGHT_EIGHTH_EM:
	      width += charsize * HERSHEY_EM / 8.0;
	      break;
	      
	    case C_RIGHT_TWELFTH_EM:
	      width += charsize * HERSHEY_EM / 12.0;
	      break;
	      
	    case C_LEFT_ONE_EM:
	      width -= charsize * HERSHEY_EM;
	      break;
	      
	    case C_LEFT_HALF_EM:
	      width -= charsize * HERSHEY_EM / 2.0;
	      break;
	      
	    case C_LEFT_QUARTER_EM:
	      width -= charsize * HERSHEY_EM / 4.0;
	      break;
	      
	    case C_LEFT_SIXTH_EM:
	      width -= charsize * HERSHEY_EM / 6.0;
	      break;
	      
	    case C_LEFT_EIGHTH_EM:
	      width -= charsize * HERSHEY_EM / 8.0;
	      break;
	      
	    case C_LEFT_TWELFTH_EM:
	      width -= charsize * HERSHEY_EM / 12.0;
	      break;
	      
	      /* unrecognized control code */
	    default:
	      break;
	    }
	}
      else			/* yow, an actual character */
	{
	  int raw_fontnum;
	  
	  /* compute index of font, in table in g_fontdb.c */
	  raw_fontnum = (c >> FONT_SHIFT) & ONE_BYTE;
	  
	  c &= ~FONT_SPEC;	/* extract character proper */
	  glyphnum = (_pl_g_hershey_font_info[raw_fontnum].chars)[c];

	  /* could be a pseudo glyph number, e.g. an indication that
	     character is composite */
	  if (glyphnum == ACC0 || glyphnum == ACC1 || glyphnum == ACC2)
	    {
	      unsigned char composite, character, accent;

	      /* if so, use 1st element of composite character */
	      composite = (unsigned char)c;
	      if (composite_char (&composite, &character, &accent))
		glyphnum = (_pl_g_hershey_font_info[raw_fontnum].chars)[character];
	      else
		glyphnum = UNDE; /* hope this won't happen */
	    }

	  /* could also be a glyph number displaced by KS, to indicate
	     that this is a small kana */
	  if (glyphnum & KS)
	    glyphnum -= KS;

	  glyph = (const unsigned char *)(_pl_g_occidental_hershey_glyphs[glyphnum]);
	  if (*glyph != '\0')	/* nonempty glyph */
	    /* 1st two chars are bounds */
	    width += charsize * ((int)glyph[1] - (int)glyph[0]);
	}
      
      ptr++;			/* bump pointer in string */
    }

  return width;
}  

/* _pl_g_draw_hershey_penup_stroke() draws a penup stroke, along a vector
   specified in Hershey units.  Size scaling and obliquing (true/false) are
   specified.  This is used for repositioning during rendering of composite
   (accented) characters. */
void
_pl_g_draw_hershey_penup_stroke(R___(Plotter *_plotter) double dx, double dy, double charsize, bool oblique)
{
  double shear;

  shear = oblique ? (SHEAR) : 0.0;
  _pl_g_draw_hershey_stroke (R___(_plotter)
			     false, /* pen up */
			     charsize * (dx + shear * dy), 
			     charsize * dy);
}

/* _pl_g_draw_hershey_glyph() invokes move() and cont() to draw a raw
   Hershey glyph, specified by index in the occidental or oriental glyph
   arrays.  Size scaling and obliquing (true/false) are specified. */
void
_pl_g_draw_hershey_glyph (R___(Plotter *_plotter) int glyphnum, double charsize, int type, bool oblique)
{
  double xcurr, ycurr;
  double xfinal, yfinal;
  bool pendown = false;
  const unsigned char *glyph;
  double dx, dy;
  double shear;
  
  shear = oblique ? (SHEAR) : 0.0;
  switch (type)
    {
    case OCCIDENTAL:
    default:
      glyph = (const unsigned char *)(_pl_g_occidental_hershey_glyphs[glyphnum]);
      break;
    case ORIENTAL:
      glyph = (const unsigned char *)(_pl_g_oriental_hershey_glyphs[glyphnum]); 
      break;
    }

  if (*glyph != '\0')	/* nonempty glyph */
    {
      xcurr = charsize * (double)glyph[0];
      xfinal = charsize * (double)glyph[1];
      ycurr = yfinal = 0.0;
      glyph += 2;
      while (*glyph)
	{
	  int xnewint;
	  
	  xnewint = (int)glyph[0];
	  
	  if (xnewint == (int)' ')
	    pendown = false;
	  else
	    {
	      double xnew, ynew;

	      xnew = (double)charsize * xnewint;
	      ynew = (double)charsize 
		* ((int)'R' 
		   - ((int)glyph[1] + (double)HERSHEY_BASELINE));
	      dx = xnew - xcurr;
	      dy = ynew - ycurr;
	      _pl_g_draw_hershey_stroke (R___(_plotter) 
					 pendown, dx + shear * dy, dy);
	      xcurr = xnew, ycurr = ynew;
	      pendown = true;
	    }
	  
	  glyph +=2;	/* on to next pair */
	}
      
      /* final penup stroke, to end where we should */
      dx = xfinal - xcurr;
      dy = yfinal - ycurr;
      _pl_g_draw_hershey_stroke (R___(_plotter) false, dx + shear * dy, dy);
    }
}

/* _pl_g_draw_hershey_string() strokes a string beginning at present
   location, which is taken to be on the string's baseline.  Besides
   invoking move() and cont(), it invokes linewidth(). */
void
_pl_g_draw_hershey_string (R___(Plotter *_plotter) const unsigned short *string)
{
  unsigned short c;
  const unsigned short *ptr = string;
  double charsize = 1.0;
  double saved_charsize = 1.0;
  double saved_position_x = _plotter->drawstate->pos.x;
  double saved_position_y = _plotter->drawstate->pos.y;
  double old_line_width;
  int line_width_type = 0;	/* 0,1,2 = unset,occidental,oriental */

  /* save line width (will restore at end) */
  old_line_width = _plotter->drawstate->line_width;

  while ((c = (*ptr++)) != '\0')
    {
      /* Check for the four possibilities: (1) a Hershey glyph specified by
	 glyph number, (2) an oriental Hershey glyph specified by glyph
	 number, (3) a control code, and (4) an ordinary font character,
	 which will be mapped to a Hershey glyph by one of the tables in
	 g_fontdb.c. */

      if (c & RAW_HERSHEY_GLYPH)
	{
	  if (line_width_type != 1)
	    {
	      _API_flinewidth (R___(_plotter)
				    HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_STROKE_WIDTH));
	      line_width_type = 1;
	    }
	  _pl_g_draw_hershey_glyph (R___(_plotter)
				    c & GLYPH_SPEC, charsize, OCCIDENTAL, false);
	}

      else if (c & RAW_ORIENTAL_HERSHEY_GLYPH)
	{
	  if (line_width_type != 2)
	    {
	      _API_flinewidth (R___(_plotter)
				    HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_ORIENTAL_STROKE_WIDTH));
	      line_width_type = 2;
	    }
	  _pl_g_draw_hershey_glyph (R___(_plotter)
				    c & GLYPH_SPEC, charsize, ORIENTAL, false);
	}

      else if (c & CONTROL_CODE)	
	switch (c & ~CONTROL_CODE) /* parse control codes */
	  {
	  case C_BEGIN_SUPERSCRIPT :
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, 
				       SUPERSCRIPT_DX * charsize * HERSHEY_EM,
				       SUPERSCRIPT_DY * charsize * HERSHEY_EM);
	    charsize *= SCRIPTSIZE;
	    break;
		
	  case C_END_SUPERSCRIPT:
	    charsize /= SCRIPTSIZE;
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, 
				       - SUPERSCRIPT_DX * charsize * HERSHEY_EM,
				       - SUPERSCRIPT_DY * charsize * HERSHEY_EM);
	    break;
		
	  case C_BEGIN_SUBSCRIPT:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, 
				       SUBSCRIPT_DX * charsize * HERSHEY_EM,
				       SUBSCRIPT_DY * charsize * HERSHEY_EM);
	    charsize *= SCRIPTSIZE;
	    break;
		
	  case C_END_SUBSCRIPT:
	    charsize /= SCRIPTSIZE;
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, 
				       - SUBSCRIPT_DX * charsize * HERSHEY_EM,
				       - SUBSCRIPT_DY * charsize * HERSHEY_EM);
	    break;
		
	  case C_PUSH_LOCATION:
	    saved_charsize = charsize;
	    saved_position_x = _plotter->drawstate->pos.x;
	    saved_position_y = _plotter->drawstate->pos.y;
	    break;
		
	  case C_POP_LOCATION:
	    charsize = saved_charsize;
	    _API_fmove (R___(_plotter)
			     saved_position_x, saved_position_y);
	    break;
		
	  case C_RIGHT_ONE_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM, 0.0);
	    break;
		
	  case C_RIGHT_HALF_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM / 2.0, 0.0);
	    break;
		
	  case C_RIGHT_QUARTER_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM / 4.0, 0.0);
	    break;
		
	  case C_RIGHT_SIXTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM / 6.0, 0.0);
	    break;
		
	  case C_RIGHT_EIGHTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM / 8.0, 0.0);
	    break;
		
	  case C_RIGHT_TWELFTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, charsize * HERSHEY_EM / 12.0, 0.0);
	    break;
		
	  case C_LEFT_ONE_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM, 0.0);
	    break;
		
	  case C_LEFT_HALF_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM / 2.0, 0.0);
	    break;
		
	  case C_LEFT_QUARTER_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM / 4.0, 0.0);
	    break;
		
	  case C_LEFT_SIXTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM / 6.0, 0.0);
	    break;
		
	  case C_LEFT_EIGHTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM / 8.0, 0.0);
	    break;

	  case C_LEFT_TWELFTH_EM:
	    _pl_g_draw_hershey_stroke (R___(_plotter)
				       false, - charsize * HERSHEY_EM / 12.0, 0.0);
	    break;
		
	    /* unrecognized control code, punt */
	  default:
	    break;
	  }
      
      else
	/* yow, an actual font character!  Several possibilities: could be
	   a composite (accented) character, could be a small Kana, or
	   could be a garden-variety character. */
	{
	  int raw_fontnum;
	  int glyphnum;		/* glyph in Hershey array */
	  int char_glyphnum, accent_glyphnum; /* for composite chars */
	  int char_width, accent_width; /* for composite chars */
	  const unsigned char *char_glyph, *accent_glyph;
	  unsigned char composite, character, accent;
	  bool oblique, small_kana = false;
	  
	  /* compute index of font, in font table in g_fontdb.c */
	  raw_fontnum = (c >> FONT_SHIFT) & ONE_BYTE;
	  /* shear font?  (for HersheySans-Oblique, etc.) */
	  oblique = _pl_g_hershey_font_info[raw_fontnum].obliquing;
	  
	  c &= ~FONT_SPEC;	/* extract character proper */
	  glyphnum = (_pl_g_hershey_font_info[raw_fontnum].chars)[c];
	  
	  if (glyphnum & KS) /* a small kana? */
	    {
	      glyphnum -= KS;
	      small_kana = true;
	    }
	  
	  switch (glyphnum)
	    {
	      /* special case: this is a composite (accented) character;
		 search font table in g_fontdb.c for it */
	    case ACC0:
	    case ACC1:
	    case ACC2:
	      composite = (unsigned char)c;
	      if (composite_char (&composite, &character, &accent))
		{
		  char_glyphnum = 
		    (_pl_g_hershey_font_info[raw_fontnum].chars)[character];
		  accent_glyphnum = 
		    (_pl_g_hershey_font_info[raw_fontnum].chars)[accent];
		}
	      else
		{		/* hope this won't happen */
		  char_glyphnum = UNDE; 
		  accent_glyphnum = 0;
		}
	      char_glyph = 
		(const unsigned char *)_pl_g_occidental_hershey_glyphs[char_glyphnum];
	      accent_glyph = 
		(const unsigned char *)_pl_g_occidental_hershey_glyphs[accent_glyphnum];
	  
	      if (*char_glyph != '\0') /* nonempty glyph */
		/* 1st two chars are bounds, in Hershey units */
		char_width = (int)char_glyph[1] - (int)char_glyph[0];
	      else
		char_width = 0;

	      if (*accent_glyph != '\0') /* nonempty glyph */
		/* 1st two chars are bounds, in Hershey units */
		accent_width = (int)accent_glyph[1] - (int)accent_glyph[0];
	      else
		accent_width = 0;

	      /* draw the character */
	      if (line_width_type != 1)
		{
		  _API_flinewidth (R___(_plotter)
					HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_STROKE_WIDTH));
		  line_width_type = 1;
		}
	      _pl_g_draw_hershey_glyph (R___(_plotter)
					char_glyphnum, charsize, 
					OCCIDENTAL, oblique);
	      /* back up to draw accent */
	      _pl_g_draw_hershey_penup_stroke (R___(_plotter)
					       -0.5 * (double)char_width
					       -0.5 * (double)accent_width,
					       0.0, charsize, oblique);

	      /* repositioning for uppercase and uppercase italic */
	      if (glyphnum == ACC1)
		_pl_g_draw_hershey_penup_stroke (R___(_plotter)
						 0.0, 
						 (double)(ACCENT_UP_SHIFT),
						 charsize, oblique);
	      else if (glyphnum == ACC2)
		_pl_g_draw_hershey_penup_stroke (R___(_plotter)
						 (double)(ACCENT_RIGHT_SHIFT),
						 (double)(ACCENT_UP_SHIFT),
						 charsize, oblique);

	      /* draw the accent */
	      _pl_g_draw_hershey_glyph (R___(_plotter)
					accent_glyphnum, charsize, 
					OCCIDENTAL, oblique);

	      /* undo special repositioning if any */
	      if (glyphnum == ACC1)
		_pl_g_draw_hershey_penup_stroke (R___(_plotter)
						 0.0, 
						 -(double)(ACCENT_UP_SHIFT),
						 charsize, oblique);
	      else if (glyphnum == ACC2)
		_pl_g_draw_hershey_penup_stroke (R___(_plotter)
						 -(double)(ACCENT_RIGHT_SHIFT),
						 -(double)(ACCENT_UP_SHIFT),
						 charsize, oblique);

	      /* move forward, to end composite char where we should */
	      _pl_g_draw_hershey_penup_stroke (R___(_plotter)
					       0.5 * (double)char_width
					       -0.5 * (double)accent_width,
					       0.0, charsize, oblique);
	      break;

	      /* not a composite (accented) character; just an ordinary
		 glyph from occidental+Kana array (could be a Kana, in
		 particular, could be a small Kana) */
	    default:
	      if (small_kana)
		{
		  int kana_width;
		  const unsigned char *kana_glyph;
		  double shift = 0.5 * (1.0 - (SMALL_KANA_SIZE));

		  kana_glyph = 
		    (const unsigned char *)_pl_g_occidental_hershey_glyphs[glyphnum];
		  kana_width = (int)kana_glyph[1] - (int)kana_glyph[0];

		  /* draw small Kana, preceded and followed by a penup
		     stroke in order to traverse the full width of an
		     ordinary Kana */
		  _pl_g_draw_hershey_penup_stroke (R___(_plotter)
						   shift * (double)kana_width,
						   0.0, charsize, oblique);
		  if (line_width_type != 2)
		    {
		      _API_flinewidth (R___(_plotter)
					    HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_ORIENTAL_STROKE_WIDTH));
		      line_width_type = 2;
		    }
		  _pl_g_draw_hershey_glyph (R___(_plotter)
					    glyphnum, 
					    (SMALL_KANA_SIZE) * charsize,
					    OCCIDENTAL, oblique);
		  _pl_g_draw_hershey_penup_stroke (R___(_plotter)
						   shift * (double)kana_width,
						   0.0, charsize, oblique);
		}
	      else
		/* whew! just an ordinary glyph from the occidental array
		   (could be a Kana however, since they're confusingly
		   placed in that array, at the end) */
		{
		  if (glyphnum >= BEGINNING_OF_KANA)
		    {
		      if (line_width_type != 2)
			{
			  _API_flinewidth (R___(_plotter)  
						HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_ORIENTAL_STROKE_WIDTH));
			  line_width_type = 2;
			}
		    }
		  else
		      if (line_width_type != 1)
			{
			  _API_flinewidth (R___(_plotter)
						HERSHEY_UNITS_TO_USER_UNITS (HERSHEY_STROKE_WIDTH));
			  line_width_type = 1;
			}
		_pl_g_draw_hershey_glyph (R___(_plotter)
					  glyphnum, charsize, 
					  OCCIDENTAL, oblique);
		}
	      break;
	    } /* end of case statement that switches based on glyphnum */

	} /* end of font character case */

    } /* end of loop through unsigned shorts in the codestring */
  
  if (line_width_type != 0)
    /* must restore old line width */
    _API_flinewidth (R___(_plotter) old_line_width);
  
  return;
}

/* retrieve the two elements of a composite character from the table in
   g_fontdb.c */
static bool
composite_char (unsigned char *composite, unsigned char *character, unsigned char *accent)
{
  const struct plHersheyAccentedCharInfoStruct *compchar = _pl_g_hershey_accented_char_info;
  bool found = false;
  unsigned char given = *composite;
  
  while (compchar->composite)
    {
      if (compchar->composite == given)
	{
	  found = true;
	  /* return char and accent via pointers */
	  *character = compchar->character;
	  *accent = compchar->accent;
	}
      compchar++;
    }

  return found;
}
