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

/* This file contains a low-level method for adjusting the font of an HP-GL
   or PCL device to agree with an HPGL or PCL Plotter's notion of what it
   should be, prior to plotting a label.  Note: The `PCL 5' output by any
   PCL Plotter is simply a wrapped version of HP-GL/2.

   Before HP-GL/2 (introduced c. 1990), HP-GL devices supported only Stick
   fonts.  In modern PCL5 printers, the suite of 45 PCL fonts is accessible
   too.  Only a few modern high-end PCL5/PS printers (e.g. LaserJet 4000
   series laser printers) also support PS fonts in PCL mode.  PS fonts are
   supported by HP-GL/2 and PCL Plotters if the `--enable-ps-fonts-in-pcl'
   option is specified at configure time.

   After selecting the font, this method invokes the HP-GL DR/SR/SL
   instructions to size and slant the font, as needed.

   The font selection itself is accomplished in either of two ways.
   
   1. In versions of HP-GL prior to HP-GL/2, 7-bit font halves are selected
   with `CS' and `CA' instructions.  They must be switched between when the
   label is plotted via SO/SI; see h_text.c.  The HP-GL device will usually
   supply the upper font half in the Roman-8 encoding, and that too will
   need to be taken into account when the label is plotted.

   2. In HP-GL/2, a single 8-bit font is selected with the HP-GL/2 `SD'
   instruction.  In principle, no switching between 7-bit font halves is
   needed.

   In practice, it's more complicated than that.  For ISO-Latin-1 PCL
   fonts, the SD instruction allegedly allows the ISO-Latin-1 encoding to
   be requested.  But it doesn't work!  One or two characters in the lower
   half (!) don't come out right.  So instead, we use the `SD' instruction
   to retrieve an 8-bit version that uses the Roman-8 encoding, and the
   `AD' instruction to retrieve an alternative 8-bit version that uses the
   ISO-Latin-1 encoding.  We'll use the former for characters in the lower
   half, and the latter for characters in the upper half.  This is bizarre,
   but it works.  See additional comments in h_text.c. */

/* NOTE: This code assumes that P1 and P2 have different x coordinates, and
   different y coordinates.  If that isn't the case, it'll divide by zero.
   So we check for that possibility in _pl_h_paint_text_string() before
   calling this function.  See comment in h_text.c. */

#include "sys-defines.h"
#include "extern.h"

/* Shearing factor for oblique fonts, new_x = x + SHEAR * y  */

#define SHEAR (2.0/7.0)

void
_pl_h_set_font (S___(Plotter *_plotter))
{
  bool font_changed = false;
  bool oblique;
  double cos_slant = 1.0, sin_slant = 0.0;
  double new_relative_label_run, new_relative_label_rise;
  double theta, sintheta, costheta;
  plVector base, up, base_native, up_native;
  double base_native_len, up_native_len, tan_slant;
  
  /* sanity check, should be unnecessary */
  if (_plotter->drawstate->font_type == PL_F_HERSHEY)
    return;

  if (_plotter->drawstate->font_type == PL_F_STICK)
    /* check whether obliquing of this font is called for */
    {
      int master_font_index;

      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      oblique = _pl_g_stick_font_info[master_font_index].obliquing;
    }
  else
    oblique = false;

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  costheta = cos (theta);
  sintheta = sin (theta);

  /* compute in the device frame a `base vector' which is (in the user
     frame) directed along the label, with length equal to the font size) */
  base.x = _plotter->drawstate->true_font_size * XDV(costheta,sintheta);
  base.y = _plotter->drawstate->true_font_size * YDV(costheta,sintheta);

  /* Compute rise and run, relative to x-distance and y-distance between
     scaling points P1,P2. (Either rise or run can be negative; overall
     normalization, e.g. the `100', is irrelevant.  We include the `100' to
     express them as percentages.) */
  new_relative_label_run =  100 * base.x / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  new_relative_label_rise = 100 * base.y / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
  if (new_relative_label_run != 0.0 || new_relative_label_rise != 0.0)
    /* (will always be true except when the font size is so small
       there's really no point in printing the label) */
    {
      /* update device-frame label rotation angle if needed */
      if (_plotter->hpgl_rel_label_run != new_relative_label_run
	  || _plotter->hpgl_rel_label_rise != new_relative_label_rise)
	{    
	  sprintf (_plotter->data->page->point, "DR%.3f,%.3f;",
		   new_relative_label_run, new_relative_label_rise);
	  _update_buffer (_plotter->data->page);
	  _plotter->hpgl_rel_label_run = new_relative_label_run;
	  _plotter->hpgl_rel_label_rise = new_relative_label_rise;
	}
    }

  /* emit command to select new font, if needed (see below) */
  if (_plotter->hpgl_version == 2)
    font_changed = _pl_h_hpgl2_maybe_update_font (S___(_plotter));
  else				/* 0 or 1, i.e. generic HP-GL or HP7550A */
    font_changed = _pl_h_hpgl_maybe_update_font (S___(_plotter));

  /* Compute image, in the device frame, of a so-called `up vector': a
     vector which in the user frame is perpendicular to the above `base'
     vector, and has the same length.  Some fonts are specially obliqued,
     so we take font obliquing (if any) into account here. */

  up.x = _plotter->drawstate->true_font_size * XDV(-sintheta,costheta);
  up.y = _plotter->drawstate->true_font_size * YDV(-sintheta,costheta);
  up.x += (oblique ? SHEAR : 0.0) * base.x;
  up.y += (oblique ? SHEAR : 0.0) * base.y;  

  /* Our `device frame' base and up vectors are really vectors in the
     normalized device frame, in which the viewport has a fixed size.  See
     h_defplot.c.  E.g., the viewport corners (0,0) and (1,1) in the NDC
     frame are respectively mapped to
     (HPGL_SCALED_DEVICE_LEFT,HPGL_SCALED_DEVICE_BOTTOM) and
     (HPGL_SCALED_DEVICE_RIGHT,HPGL_SCALED_DEVICE_TOP) in the normalized
     device frame.  The further mapping to native HP-GL coordinates is
     accomplished by an `SC' scaling instruction emitted at the head of the
     output file; see h_openpl.c.  This further mapping depends on the
     PAGESIZE parameter.

     Unfortunately, when dealing with anamorphically transformed fonts we
     need to manipulate not just vectors in the normalized device frame,
     but also vectors in the true device frame, i.e., in native HP-GL
     units. */

  /* These vectors use native HP-GL units. */
  base_native.x = base.x * (_plotter->hpgl_p2.x - _plotter->hpgl_p1.x) / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  base_native.y = base.y * (_plotter->hpgl_p2.y - _plotter->hpgl_p1.y) / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
  up_native.x = up.x * (_plotter->hpgl_p2.x - _plotter->hpgl_p1.x) / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  up_native.y = up.y * (_plotter->hpgl_p2.y - _plotter->hpgl_p1.y) / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);

  base_native_len = sqrt (base_native.x * base_native.x + base_native.y * base_native.y);
  up_native_len = sqrt (up_native.x * up_native.x + up_native.y * up_native.y);

  /* compute character slant angle (in the true device frame, NOT in the
     normalized device frame) */
  if (base_native_len == 0.0 || up_native_len == 0.0) /* a bad situation */
    tan_slant = 0.0;
  else
    {
      sin_slant = ((base_native.x * up_native.x + base_native.y * up_native.y) 
		   / (base_native_len * up_native_len));
      cos_slant = sqrt (1 - sin_slant * sin_slant);
      tan_slant = sin_slant / cos_slant;
    }

  /* Compute nominal horizontal and vertical character sizes as percentages
     of the horizontal and vertical distances between scaling points P1 and
     P2, and specify them with the SR instruction.  

     The two arguments of the SR instruction (the horizontal and vertical
     character sizes) should apparently be 0.5 times the font size, and 0.7
     times the font size.
     
     Why? The 0.5 and 0.7 = 1.4 * 0.5 factors are undocumented HP magic.
     This convention must have been introduced by HP to adapt the SR
     instruction, which dates back to fixed-width plotter fonts (i.e., the
     original Stick font), to modern outline fonts.  Fixed-width plotter
     fonts did not have a font size in the modern sense: they had a
     character width and a character height.  (The former being the width
     of the character proper, which occupied the left 2/3 of a character
     cell, and the latter being what we would nowadays call a cap height.)

     The convention probably arose because Stick fonts look best if the
     aspect ratio is 1.4 (= 0.7/0.5), i.e. if the `character height' is 1.4
     times the `character width'.  I am not sure where the 0.5 came from.
     Possibly back in stick font days, the nominal font size was defined to
     be 4/3 times the width of a character cell, or equivalently the width
     of a character cell was chosen to be 3/4 times the nominal font size.
     This would make the maximum character width (2/3)x(3/4) = (1/2) times
     the nominal font size. */

  {
    double fractional_char_width = 0.5;
    double fractional_char_height = 1.4 * 0.5;
    double new_relative_char_width, new_relative_char_height;

    /* If, in the physical device frame, the font is reflected, we must
       flip the sign of HP-GL's `character height', as used in the SR
       instruction.  To determine whether this sign-flipping is needed, we
       use the fact that the user_frame->physical_device_frame map is the
       product of the user_frame->normalized_device_frame map and the
       normalized_device_frame->physical_device_frame map.  Whether the
       first includes a reflection is precomputed and stored in the drawing
       state.  The second will include a reflection only if exactly one of
       the xsize,ysize fields of PAGESIZE is negative.  We can easily check
       for that by comparing the x,y coordinates of the HP-GL scaling
       points P1,P2. */

    int orientation = _plotter->drawstate->transform.nonreflection ? 1 : -1;

    if ((_plotter->hpgl_p2.x - _plotter->hpgl_p1.x) / (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT) < 0.0)
      orientation *= -1;
    if ((_plotter->hpgl_p2.y - _plotter->hpgl_p1.y) / (HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM) < 0.0)
      orientation *= -1;

    new_relative_char_width = fractional_char_width * 100 * base_native_len / (_plotter->hpgl_p2.x - _plotter->hpgl_p1.x);
    new_relative_char_height = 
      fractional_char_height * 100 * orientation * cos_slant * up_native_len / (_plotter->hpgl_p2.y - _plotter->hpgl_p1.y);
    
    /* emit SR instruction only if font was changed or if current
       size was wrong */
    if (font_changed || 
	(new_relative_char_width != _plotter->hpgl_rel_char_width
	 || new_relative_char_height != _plotter->hpgl_rel_char_height))
      {
	sprintf (_plotter->data->page->point, "SR%.3f,%.3f;", 
		 new_relative_char_width, new_relative_char_height);
	_update_buffer (_plotter->data->page);
	_plotter->hpgl_rel_char_width = new_relative_char_width;
	_plotter->hpgl_rel_char_height = new_relative_char_height;
      }
  }

  /* update slant angle if necessary */
  if (tan_slant != _plotter->hpgl_tan_char_slant)
    {
      sprintf (_plotter->data->page->point, "SL%.3f;", tan_slant);
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_tan_char_slant = tan_slant;
    }
}

/* If needed, emit a new-style (HP-GL/2) `SD' font-selection command.
   Return value indicates whether font was changed. */

bool
_pl_h_hpgl2_maybe_update_font (S___(Plotter *_plotter))
{
  bool font_change = false;
  bool font_is_iso_latin_1;
  int master_font_index;
  int symbol_set, spacing, posture, stroke_weight, typeface;

  /* PCL, PS, and Stick fonts are handled separately here only because the
     font information for them is stored in different tables in g_fontdb.c.
     We compute parameters we'll need for the HP-GL/2 `SD' font-selection
     command. */

  switch (_plotter->drawstate->font_type)
    {
    case PL_F_PCL:
    default:
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      /* #1: symbol set */
      symbol_set = _pl_g_pcl_font_info[master_font_index].hpgl_symbol_set;
      /* #2: spacing */
      spacing = _pl_g_pcl_font_info[master_font_index].hpgl_spacing;
      /* #3, #4 are pitch and height (we use defaults) */
      /* #5: posture */
      posture = _pl_g_pcl_font_info[master_font_index].hpgl_posture;
      /* #6: stroke weight */
      stroke_weight = _pl_g_pcl_font_info[master_font_index].hpgl_stroke_weight;
      /* #7: typeface */
      typeface = _pl_g_pcl_font_info[master_font_index].pcl_typeface;  
      /* ISO-Latin-1 after reencoding (if any)? */
      font_is_iso_latin_1 = _pl_g_pcl_font_info[master_font_index].iso8859_1;
      break;
    case PL_F_POSTSCRIPT:
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      /* #1: symbol set */
      symbol_set = _pl_g_ps_font_info[master_font_index].hpgl_symbol_set;
      /* #2: spacing */
      spacing = _pl_g_ps_font_info[master_font_index].hpgl_spacing;
      /* #3, #4 are pitch and height (we use defaults) */
      /* #5: posture */
      posture = _pl_g_ps_font_info[master_font_index].hpgl_posture;
      /* #6: stroke weight */
      stroke_weight = _pl_g_ps_font_info[master_font_index].hpgl_stroke_weight;
      /* #7: typeface */
      typeface = _pl_g_ps_font_info[master_font_index].pcl_typeface;  
      /* ISO-Latin-1 after reencoding (if any)? */
      font_is_iso_latin_1 = _pl_g_ps_font_info[master_font_index].iso8859_1;
      break;
    case PL_F_STICK:
      /* compute index of font in master table of fonts, in g_fontdb.c */
      master_font_index =
	(_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
      
      /* #1: symbol set */
      symbol_set = _pl_g_stick_font_info[master_font_index].hpgl_symbol_set;
      /* #2: spacing */
      spacing = _pl_g_stick_font_info[master_font_index].hpgl_spacing;
      /* #3, #4 are pitch and height (we use defaults) */
      /* #5: posture */
      posture = _pl_g_stick_font_info[master_font_index].hpgl_posture;
      /* #6: stroke weight */
      stroke_weight = _pl_g_stick_font_info[master_font_index].hpgl_stroke_weight;
      /* #7: typeface */
      typeface = _pl_g_stick_font_info[master_font_index].pcl_typeface;  
      /* ISO-Latin-1 after reencoding (if any)? */
      font_is_iso_latin_1 = _pl_g_stick_font_info[master_font_index].iso8859_1;
      break;
    }
  
  if (symbol_set != _plotter->hpgl_symbol_set
      || spacing != _plotter->hpgl_spacing
      || posture != _plotter->hpgl_posture
      || stroke_weight != _plotter->hpgl_stroke_weight
      || typeface != _plotter->hpgl_pcl_typeface)
    font_change = true;
  
  if (font_change)
    {
      if (spacing == HPGL2_FIXED_SPACING)
	/* fixed-width font */
	sprintf (_plotter->data->page->point, 
		 /* #4 (nominal point size) not needed but included anyway */
		 "SD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, 
		 (double)HPGL2_NOMINAL_CHARS_PER_INCH, (double)HPGL2_NOMINAL_POINT_SIZE, 
		 posture, stroke_weight, typeface);
      else
	/* variable-width font */
	sprintf (_plotter->data->page->point, 
		 /* #3 (nominal chars per inch) not needed but incl'd anyway */
		 "SD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		 symbol_set, spacing, 
		 (double)HPGL2_NOMINAL_CHARS_PER_INCH, (double)HPGL2_NOMINAL_POINT_SIZE, 
		 posture, stroke_weight, typeface);
      _update_buffer (_plotter->data->page);

      /* A hack.  Due to HP's idiosyncratic definition of `ISO-Latin-1
	 encoding' for PCL fonts, when plotting a label in an ISO-Latin-1
	 PCL font we'll map characters in the lower half into HP's Roman-8
	 encoding, and characters in the upper half into HP's ISO-Latin-1
	 encoding.  We implement this by using two fonts: standard and
	 alternative.  See h_text.c for the DFA that switches back and
	 forth (if necessary) when the label is rendered. */
      if (_plotter->drawstate->font_type == PL_F_PCL
	  && font_is_iso_latin_1
	  && symbol_set == PCL_ROMAN_8)
	{
	  if (spacing == HPGL2_FIXED_SPACING)
	    /* fixed-width font */
	    sprintf (_plotter->data->page->point, 
		     /* #4 (nominal point size) not needed but included anyway */
		     "AD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		     PCL_ISO_8859_1, spacing, 
		     (double)HPGL2_NOMINAL_CHARS_PER_INCH, (double)HPGL2_NOMINAL_POINT_SIZE, 
		     posture, stroke_weight, typeface);
	  else
	    /* variable-width font */
	    sprintf (_plotter->data->page->point, 
		    /* #3 (nominal chars per inch) not needed but included anyway */
		     "AD1,%d,2,%d,3,%.3f,4,%.3f,5,%d,6,%d,7,%d;",
		     PCL_ISO_8859_1, spacing, 
		     (double)HPGL2_NOMINAL_CHARS_PER_INCH, (double)HPGL2_NOMINAL_POINT_SIZE, 
		     posture, stroke_weight, typeface);
	  _update_buffer (_plotter->data->page);
	}

      _plotter->hpgl_symbol_set = symbol_set;
      _plotter->hpgl_spacing = spacing;
      _plotter->hpgl_posture = posture;
      _plotter->hpgl_stroke_weight = stroke_weight;
      _plotter->hpgl_pcl_typeface = typeface;
    }

  return font_change;		/* was font changed? */
}

/* If needed, emit an old-style (pre-HP-GL/2) `CS' font-selection command,
   and also a `CA' font-change command to make the upper half of the
   selected font available via SO/SI.  Return value indicates whether font
   was changed.  (This is used only for Stick fonts, which is all that
   pre-HP/GL-2 HP-GL devices had.)  */

bool
_pl_h_hpgl_maybe_update_font (S___(Plotter *_plotter))
{
  bool font_change = false;
  int new_hpgl_charset_lower, new_hpgl_charset_upper, master_font_index;

  /* compute index of font in master table of fonts, in g_fontdb.c */
  master_font_index =
    (_pl_g_stick_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  
  /* determine HP character set numbers (old style, pre-HP-GL/2) */
  new_hpgl_charset_lower = _pl_g_stick_font_info[master_font_index].hpgl_charset_lower;
  new_hpgl_charset_upper = _pl_g_stick_font_info[master_font_index].hpgl_charset_upper;

  /* using `CS', select charset for lower half of font */
  if (new_hpgl_charset_lower != _plotter->hpgl_charset_lower)
    {
      sprintf (_plotter->data->page->point, "CS%d;", new_hpgl_charset_lower);
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_charset_lower = new_hpgl_charset_lower;
      font_change = true;
    }

  /* using `CA', select charset for upper half, if we have a genuine one (a
     negative value for the upper charset is our way of flagging that this
     is a 7-bit font; see comment in h_text.c) */
  if (new_hpgl_charset_upper >= 0 
      && new_hpgl_charset_upper != _plotter->hpgl_charset_upper)
    {
      sprintf (_plotter->data->page->point, "CA%d;", new_hpgl_charset_upper);
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_charset_upper = new_hpgl_charset_upper;
      font_change = true;
    }

  return font_change;
}
