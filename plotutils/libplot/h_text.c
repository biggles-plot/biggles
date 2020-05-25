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

/* This is the low-level method which is used by any HP-GL or PCL Plotter
   for rendering a single-font label in a PCL, PS, or Stick font.  Note:
   The `PCL 5' output by any PCL Plotter is simply a wrapped version of
   HP-GL/2.

   This method internally invokes _pl_h_set_font to select the font.  See
   h_font.c for the font selection code.

   Before HP-GL/2 (introduced c. 1990), HP-GL devices supported only Stick
   fonts.  In modern PCL5 printers, the suite of 45 PCL fonts is accessible
   too.  Only a few modern high-end PCL5/PS printers (e.g. LaserJet 4000
   series laser printers) also support PS fonts in PCL mode.  PS fonts are
   supported by HP-GL/2 and PCL Plotters if the `--enable-ps-fonts-in-pcl'
   option is specified at configure time.

   Novel features of this driver include (1) the rightward shift that all
   fonts need, when accessed through HP-GL, (2) the re-encoding that Stick
   fonts need, (3) the compensation for the kerning used by full-featured
   HP-GL devices when rendering variable-width Stick fonts, and (4) the
   bizarre re-encoding that we apply to ISO-Latin-1 PCL fonts, due to HP's
   idiosyncratic definition of ISO-Latin-1 ("ECMA-96 Latin 1"):

   1. HP-GL rendering of a string is displaced leftward, relative to PS
   rendering, by an amount equal to the distance between the bounding box
   left edge and the left edge (`first ink') for the first character.  This
   is so that the first ink will be put on the page right where we start
   rendering the string.  This convention dates back to pen plotter days.

   The offset[] arrays in g_fontdb.c hold the information that we need to
   undo this leftward shift by a compensating initial rightward shift.
   After rendering the string, we undo the shift. 

   2. On most devices, stick fonts are available only in HP's Roman-8
   encoding.  So we need to remap them, if they are to be ISO-Latin-1
   fonts.  There are a few characters missing, but we do our best.

   3. Ideally, any HP-GL device kerns the variable-width Stick fonts that
   it supports, if any.  We compensate for this in g_alabel.c by using the
   spacing tables in g_fontd2.c when computing label widths.  The inclusion
   of kerning in the label width computation affects the horizontal
   positioning of the label, if it is centered or right-justifified rather
   than left-justified.

   4. PCL fonts (and the PS fonts available in PCL mode on a few high-end
   devices) in principle support ISO-Latin-1 encoding, natively.  However,
   HP interprets ISO-Latin-1 in an idiosyncratic way.  For example,
   left-quote and right-quote show up as accents, and tilde shows up as a
   tilde accent.  For this reason, for ISO-Latin-1 PCL fonts we use HP's
   Roman-8 encoding for the lower half, and HP's ISO-Latin-1 encoding for
   the upper half. */

#include "sys-defines.h"
#include "extern.h"
#include "h_roman8.h"

/* for switching to upper half of font charset, and switching back */
#define SHIFT_OUT 14		/* i.e. ASCII 0x0e, i.e. ^N */
#define SHIFT_IN 15		/* i.e. ASCII 0x0f, i.e. ^O */

/* for DFA that keeps track of which half we're in */
typedef enum { LOWER_HALF, UPPER_HALF } state_type;

/* kludge, see comment in code */
#define HP_ROMAN_8_MINUS_CHAR 0366

/* ARGS: h_just,v_just are PL_JUST_{LEFT|CENTER|RIGHT}, PL_JUST_{TOP etc.} */
double
_pl_h_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  bool created_temp_string = false;
  bool reencode_iso_as_roman8 = false;
  double hp_offset;
  double theta, costheta, sintheta;
  int master_font_index;
  unsigned char *t;
  unsigned char instruction_buf[4];
  
  /* if empty string, nothing to do */
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* sanity checks: this routine supports only baseline positioning and
     left-justification */
  if (v_just != PL_JUST_BASE || h_just != PL_JUST_LEFT)
    return 0.0;

  /* sanity check, should be unnecessary */
#ifndef USE_PS_FONTS_IN_PCL
  if (_plotter->drawstate->font_type != PL_F_PCL
      && _plotter->drawstate->font_type != PL_F_STICK)
    return 0.0;
#else  /* USE_PS_FONTS_IN_PCL */
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT
      && _plotter->drawstate->font_type != PL_F_PCL
      && _plotter->drawstate->font_type != PL_F_STICK)
    return 0.0;
#endif

  /* Many HP-GL interpreters can't handle zero font size.  So bail if the
     font size we'll emit is zero. */
  if (_plotter->drawstate->true_font_size == 0.0)
    return 0.0;

  /* Our font selection code in h_font.c will divide by zero if the
     viewport in the device frame has zero area, i.e., if the HP-GL scaling
     points P1,P2 have the same x or y coordinates.  So bail now if that's
     the case. */
  if (_plotter->hpgl_p1.x == _plotter->hpgl_p2.x 
      || _plotter->hpgl_p1.y == _plotter->hpgl_p2.y)
    return _plotter->get_text_width (R___(_plotter) s);

  /* compute index of font in master table in g_fontdb.c */
  switch (_plotter->drawstate->font_type)
    {
    case PL_F_PCL:
    default:
      master_font_index =
	(_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_POSTSCRIPT:
      master_font_index =
	(_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    case PL_F_STICK:
      master_font_index =
	(_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      break;
    }

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  switch (_plotter->drawstate->font_type)
    {
    case PL_F_PCL:
    default:
      if (_pl_g_pcl_font_info[master_font_index].hpgl_symbol_set == PCL_ROMAN_8
	  && _pl_g_pcl_font_info[master_font_index].iso8859_1)
	/* An ISO-Latin-1 PCL font, for which we use HP's Roman-8 for lower
	   half and HP's Latin-1 for upper half.  Why?  Because it's what
	   works best; see comments in the font retrieval code in h_font.c.

	   There is one exception to this: right here, we map the ASCII
	   minus character `-', in the lower half, to
	   HP_ROMAN_8_MINUS_CHAR, i.e., to 0366.  This is a kludge, needed
	   to get a character whose width matches the width in the AFM
	   files that HP distributes. */
	{
	  state_type dfa_state = LOWER_HALF;
	  unsigned const char *sptr = s;
	  unsigned char *tptr;

	  /* temp string for rewritten label */
	  t = (unsigned char *)_pl_xmalloc (3 * strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;

	  /* SHIFT_OUT switches to alt. charset, SHIFT_IN back to standard */
	  while (*sptr)
	    {
	      unsigned char c;

	      c = *sptr++;
	      if (c < 0x80)
		/* lower half of font, use standard font (HP Roman-8) */
		{
		  if (c == '-')	/* kludge, map to a char in upper half */
		    c = HP_ROMAN_8_MINUS_CHAR;
		  if (dfa_state == UPPER_HALF)
		    {
		      *tptr++ = SHIFT_IN;
		      dfa_state = LOWER_HALF;
		    }
		  *tptr++ = c;
		}
	      else
		/* upper half of font, use alt. font (HP ECMA-96 Latin-1) */
		{
		  if (dfa_state == LOWER_HALF)
		    {
		      *tptr++ = SHIFT_OUT;
		      dfa_state = UPPER_HALF;
		    }
		  *tptr++ = c;
		}
	    }
	  
	  if (dfa_state == UPPER_HALF)
	    *tptr++ = SHIFT_IN;
	  *tptr = '\0';	/* end of rewritten label */
	}
      else
	/* a non-ISO-Latin-1 PCL font, no need for reencoding */
	t = (unsigned char *)s;
      break;
    case PL_F_POSTSCRIPT:
      /* no need for reencoding (HP's encoding of the font is good enough) */
      t = (unsigned char *)s;
      break;
    case PL_F_STICK:
      if (_pl_g_stick_font_info[master_font_index].hpgl_symbol_set == PCL_ROMAN_8
	  && _pl_g_stick_font_info[master_font_index].iso8859_1)
	/* stick font uses HP's Roman-8 encoding for its upper half, so
           must reencode ISO-Latin-1 as Roman-8 */
	reencode_iso_as_roman8 = true;

      if (_plotter->hpgl_version <= 1)
	/* HP-GL version is no greater than "1.5", i.e. HP7550A; so in
	   h_font.c, we'll have made both lower and upper font halves
	   available as 7-bit fonts that can be switched between via SO/SI */
	{
	  bool bogus_upper_half = false;
	  state_type dfa_state = LOWER_HALF;
	  unsigned const char *sptr = s;
	  unsigned char *tptr;

	  /* Check whether font is meant to be a pure 7-bit font with no
	     upper half; if so, we'll ignore all 8-bit characters.  This
	     case is recognized by the charset number for the upper half
	     being -1 (see table in g_fontdb.c). */
	  if (_pl_g_stick_font_info[master_font_index].hpgl_charset_upper < 0)
	    bogus_upper_half = true;

	  /* temp string for rewritten label */
	  t = (unsigned char *)_pl_xmalloc (3 * strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;

	  /* do 7-bit reencoding, using SO/SI */
	  /* SHIFT_OUT switches to alt. charset, SHIFT_IN back to standard */
	  while (*sptr)
	    {
	      unsigned char c;

	      c = *sptr++;
	      if (c >= 0x80 && reencode_iso_as_roman8)
		/* reencode upper half via lookup table in h_roman8.h */
		c = iso_to_roman8[c - 0x80];

	      if (c < 0x80)
		/* lower half of font, pass through */
		{
		  if (dfa_state == UPPER_HALF)
		    {
		      *tptr++ = SHIFT_IN;
		      dfa_state = LOWER_HALF;
		    }
		  *tptr++ = c;
		}
	      else
		/* upper half of font, move to lower half */
		if (bogus_upper_half == false)
		  {
		    if (dfa_state == LOWER_HALF)
		      {
			*tptr++ = SHIFT_OUT;
			dfa_state = UPPER_HALF;
		      }
		    *tptr++ = c - 0x80;
		  }
	    }
	  
	  /* ensure we switch back to standard font at end of label */
	  if (dfa_state == UPPER_HALF)
	    *tptr++ = SHIFT_IN;
	  *tptr = '\0';	/* end of rewritten label */
	}
      else
	/* HP-GL version is "2", i.e. HP-GL/2, so the only Stick fonts we
	   have are 8-bit ones; no need for 7-bit reencoding via a DFA.
	   Will still need to map ISO-Latin-1 to Roman-8, though. */
	{
	  unsigned const char *sptr = s;
	  unsigned char *tptr;
	
	  t = (unsigned char *)_pl_xmalloc (strlen ((const char *)s) + 1);
	  tptr = t;
	  created_temp_string = true;
	  while (*sptr)
	    {
	      if (*sptr < 0x80)
		*tptr++ = *sptr++;
	      else
		{
		  if (reencode_iso_as_roman8)
		    /* reencode upper half via lookup table in h_roman8.h */
		    *tptr++ = iso_to_roman8[(*sptr++) - 0x80];
		  else
		    *tptr++ = *sptr++;
		}
	    }
	  *tptr = '\0';		/* end of rewritten label */
	}
      break;
    }
  
  /* compute abovementioned HP-style rightward shift; depends on `offset'
     for first character in label, i.e. its `first ink' */
  switch (_plotter->drawstate->font_type)
    {
    case PL_F_PCL:
    default:
      /* per-character offset expressed in units where font size = 1000 */
      hp_offset = _pl_g_pcl_font_info[master_font_index].offset[*((unsigned char *)s)] / 1000.0;
      break;
    case PL_F_POSTSCRIPT:
      /* per-character offset expressed in units where font size = 1000 */
      hp_offset = _pl_g_ps_font_info[master_font_index].offset[*((unsigned char *)s)] / 1000.0;
      break;
    case PL_F_STICK:
      /* Offset expressed in HP's abstract raster units, need to divide by
	 what the font size equals in raster units.  
	 (Font size = 2 * raster width, by definition.) */

      /* For Stick fonts that we've defined in such a way that the raster
	 width differs between lower and upper halves, not sure what to do
	 here.  In particular ArcANK has JIS-ASCII encoding for lower half,
	 with raster width 42, and half-width Katakana encoding for upper
	 half, with raster width 45.  For now, just use the raster width
	 for the lower half. */

      hp_offset = (((double)(_pl_g_stick_font_info[master_font_index].offset)) /
		   (2.0 * _pl_g_stick_font_info[master_font_index].raster_width_lower));
      break;
    }

  /* do the rightward shift */
  _plotter->drawstate->pos.x += 
    costheta * _plotter->drawstate->true_font_size * hp_offset;
  _plotter->drawstate->pos.y += 
    sintheta * _plotter->drawstate->true_font_size * hp_offset;

  /* sync font and pen position */
  _pl_h_set_font (S___(_plotter));
  _pl_h_set_position (S___(_plotter));

  /* Sync pen color.  This may set the _plotter->hpgl_bad_pen flag (if optimal
     pen is #0 [white] and we're not allowed to use pen #0 to draw with).
     So we test _plotter->hpgl_bad_pen before drawing the label (see below). */
  _pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_LABEL);

  if (t[0] != '\0' /* i.e. label nonempty */
      && _plotter->hpgl_bad_pen == false)
    /* output the label via an `LB' instruction, including label
       terminator; don't use sprintf to avoid having to escape % and \ */
    {
      strcpy (_plotter->data->page->point, "LB");
      _update_buffer (_plotter->data->page);
      strcpy (_plotter->data->page->point, (const char *)t);
      _update_buffer (_plotter->data->page);      
      instruction_buf[0] = (unsigned char)3; /* ^C = default label terminator*/
      instruction_buf[1] = ';';
      instruction_buf[2] = '\0';
      strcpy (_plotter->data->page->point, (const char *)instruction_buf);
      _update_buffer (_plotter->data->page);

      /* where is the plotter pen now located?? we don't know, exactly */
      _plotter->hpgl_position_is_unknown = true;
    }

   if (created_temp_string)
     /* created a temp string, so free it */
     free (t);

  /* Undo HP's rightward shift */

  _plotter->drawstate->pos.x -=
    costheta * _plotter->drawstate->true_font_size * hp_offset;
  _plotter->drawstate->pos.y -= 
    sintheta * _plotter->drawstate->true_font_size * hp_offset;

  return _plotter->get_text_width (R___(_plotter) s);
}
