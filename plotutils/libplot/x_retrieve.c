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

/* This file contains the Plotter-specific _retrieve_font method, which is
   called by the _pl_g_set_font() function, which in turn is invoked by the
   API functions alabel() and flabelwidth().  It is called when the
   font_name, font_size, and textangle fields of the current drawing state
   have been filled in.  It retrieves the specified font, and fills in the
   font_type, typeface_index, font_index, font_is_iso8858, true_font_size,
   and font_ascent, and font_descent fields of the drawing state. */

/* This version is for XDrawablePlotters (and XPlotters).  It also fills in
   the x_font_struct and x_font_pixel_size fields of the drawing state,
   which are X-specific.

   This version operates by retrieving an 8-bit core X font from the X
   display, as follows.  First, it looks at the x_label field of the
   drawing state, which may contain a "hint": a string passed down from a
   higher level, containing the characters that will eventually be rendered
   in the font.  This is for efficiency.  If possible, only a subset of the
   font will be retrieved (most X displays support this).

   Nearly all core X fonts have XLFD (X Logical Font Description) names.
   So we run through various possibilities: first we try to interpret the
   font_name parameter as the name of a PS font in libplot's hardcoded font
   database (e.g. "Times-Roman"), in which case the database will supply a
   corresponding base XLFD name (e.g., "times-medium-r-normal"); then we
   try to interpret font_name as a base XLFD name which is not in the
   hardcoded database (e.g. "charter-medium-r-normal"); then finally we try
   to interpret it as a non-XLFD font name (e.g., "fixed" or "9x15").  In
   each of these cases, we try to retrieve only a subset of the font (as
   specified by the x_label field, just mentioned); if that fails, an
   attempt is made to retrieve the entire font.

   Text strings, including rotated and affinely transformed text strings,
   will eventually be rendered in x_text.c, using the XAffText module in
   x_afftext.c.  So here, all we do is retrieve a reasonable pixel size for
   the font, which will be scaled, etc., as needed.  (Note that the choice
   we make for the pixel size does not depend on the `textangle' drawing
   parameter, which is the rotation angle of the text in the user frame;
   though the XAffText module will pay attention to that parameter.)  The
   XAffText module will take care of scaling, so we set the true_font_size
   parameter to be the same as the user-specified font_size.

   Note: For speed, we maintain a linked-list cache of previously
   rasterized-and-retrieved fonts.  The linked list is accessible via the
   x_font_list member of the XDrawablePlotter (or XPlotter).  An
   ever-growing linked list is probably good enough if there aren't huge
   numbers of font or font size changes.  It's deallocated when the Plotter
   is destroyed; see x_defplot.c. */

#include "sys-defines.h"
#include "extern.h"
#include "g_her_metr.h"

/* max length user-level font name we'll accept; this may be either an XLFD
   base name (a string with exactly three hyphens in it), or something else
   (an alias, a pre-XLFD font name, etc.). */
#define MAX_USER_FONT_NAME_LENGTH 200

/* length of buffer for constructing an X font name; must be large enough
   to handle XFLD template, user-level font name */
#define MAX_FONT_NAME_LENGTH 255

/* XLFD templates, with holes into which we can punch the base XLFD name (a
   string with exactly three hyphens in it) and the integer size in terms
   of pixels.  We need a Latin-1 one and a generic one, since e.g. the
   Symbol font, which we support, has "adobe-fontspecific" as its last two
   fields. */
static const char * const xlfd_template_latin_1 = "-*-%s-*-%d-*-*-*-*-*-iso8859-1";
static const char * const xlfd_template_generic = "-*-%s-*-%d-*-*-*-*-*-*-*";

/* length of buffer for constructing an X11R6-style list-of-charset-ranges
   string, e.g. "[32 48_57 65_90]" would represent the non-contiguous set
   of characters [ 0-9A-Z]. */
#define MAX_CHARSET_SUBSET_LIST_LENGTH 767

#define PL_NUM_XLFD_FIELDS 14
#define XLFD_FIELD_FOUNDRY 0
#define XLFD_FIELD_FAMILY 1
#define XLFD_FIELD_WEIGHT 2
#define XLFD_FIELD_SLANT 3
#define XLFD_FIELD_SET_WIDTH 4
#define XLFD_FIELD_ADDITIONAL_STYLE 5
#define XLFD_FIELD_PIXELS 6
#define XLFD_FIELD_POINTS 7
#define XLFD_FIELD_HORIZONTAL_RESOLUTION 8
#define XLFD_FIELD_VERTICAL_RESOLUTION 9
#define XLFD_FIELD_SPACING 10
#define XLFD_FIELD_AVERAGE_WIDTH 11
#define XLFD_FIELD_CHARACTER_SET_MAJOR 12
#define XLFD_FIELD_CHARACTER_SET_MINOR 13 /* in X11R6 may include char subset */

/* forward references */
static bool is_a_subset (unsigned char set1[32], unsigned char set2[32]);
static char *xlfd_field (const char *name, int field);
static double min_sing_val (double m[4]);
static void print_bitvector (unsigned char v[32], char *s);
static void set_font_dimensions (Display *dpy, plXFontRecord *fptr);
static void string_to_bitvector (const unsigned char *s, unsigned char v[8]);
static plXFontRecord *select_x_font (Display *dpy, plXFontRecord **x_fontlist_ptr, const char *name, const unsigned char *s, bool subsetting);

/* _pl_x_retrieve_font() attempts to retrieve a core X font specified by a
   triple, namely {name, size, rotation}.  The rotation parameter is
   ignored, since the XAffText module will be used to rotate or transform
   any rendered string (see x_text.c, and x_afftext.c for the module).
   Four possible font retrievals are attempted, in order.

   1. `name' is taken to be an alias for an XLFD base name, as listed in
   libplot's hardcoded font database in g_fontdb.c.  (Aliases for the 35
   standard font names appear there.  E.g., name="times-roman" or
   "Times-Roman" is an alias for the three-hyphen XLFD base name
   "times-roman-r-normal".)

   2. `name' is taken to be an XLFD base name, of the form
   foundry-family-weight-slant-width, with exactly four hyphens.  E.g.,
   name="adobe-times-roman-r-normal" or
   name="bitstream-courier-medium-r-normal".  NOT YET IMPLEMENTED.

   3. `name' is taken to be an XLFD base name, of the form
   family-weight-slant-width, with exactly three hyphens.  E.g.,
   name="grotesk-bold-r-normal", or "times-medium-r-normal".

   4. `name' is taken to be a full font name, in which case `size' is
   ignored.  E.g., name="fixed" or name="9x15" or
   name="-dec-terminal-bold-r-normal--14-140-75-75-c-80-iso8859-1".  This
   option is mostly to support ancient core X fonts without proper XLFD
   names, such as "9x15".

   If a core X font is successfully retrieved (which will set the fields
   true_font_size, font_ascent, font_descent, and font_is_iso8859_1 of the
   drawing state, and the X-specific fields x_font_struct,
   x_font_pixel_size), then this function fills in the font_type field
   (PL_F_POSTSCRIPT [or PL_F_PCL] in case 1, PL_F_OTHER in cases 2, 3 and
   4), and returns true.  If a font is not successfully retrieved, this
   function returns false.

   The typeface_index and font_index fields are also filled in, in case 1.
   In the other cases (PL_F_OTHER) it's hardly worth it to fill them in, since
   switching to other fonts in the middle of a text string, except for a
   symbol font (`font #0') won't be supported.  See g_cntrlify.c. */

bool
_pl_x_retrieve_font (S___(Plotter *_plotter))
{
  const char *name, *true_name = ""; /* keep compiler happy */
  bool matched_builtin = false;	/* font name matches name of a font in db? */
  bool success;			/* font retrieved from cache or server? */
  const char *name_p;
  const char *x_name = NULL, *x_name_alt = NULL; /* from db */
  const char *x_name_alt2 = NULL, *x_name_alt3 = NULL; /* from db */
  int typeface_index = 0, font_index = 0; /* from db */
  int font_type = PL_F_POSTSCRIPT; /* from db */
  int i, hyphen_count;

  name = _plotter->drawstate->font_name;

#ifdef DEBUG
  fprintf (stderr, "----------------------------------------------------------------------\n");
  fprintf (stderr, "_pl_x_retrieve_font(): name=\"%s\", size=%g, x_label=%s\n", 
		   name, _plotter->drawstate->font_size, _plotter->drawstate->x_label);
#endif

  if (strlen (name) > MAX_USER_FONT_NAME_LENGTH) /* avoid buffer overflow */
    return false;

  if (_plotter->drawstate->font_size == 0.0)
    /* don't try to retrieve zero-size fonts */
    return false;

  /* Search null-terminated table of recognized PS fonts, in g_fontdb.c,
     for a name matching the passed name.  We support either PS-style names
     (e.g. "Times-Roman") or shortened XLFD-style names
     (e.g. "times-medium-r-normal").  Alternative versions of latter are
     supported because some X servers use "zapfdingbats" instead of "itc
     zapf dingbats", etc. */
  i = -1;
  while (_pl_g_ps_font_info[++i].ps_name) /* array ends in NULL */
    {
      if ((strcasecmp (_pl_g_ps_font_info[i].ps_name, name) == 0)
	  /* check alternative ps font name if any */
	  || (_pl_g_ps_font_info[i].ps_name_alt
	      && strcasecmp (_pl_g_ps_font_info[i].ps_name_alt, name) == 0)
	  /* check 2nd alternative ps font name if any */
	  || (_pl_g_ps_font_info[i].ps_name_alt2
	      && strcasecmp (_pl_g_ps_font_info[i].ps_name_alt2, name) == 0)
	  /* check X font name */
	  || (strcasecmp (_pl_g_ps_font_info[i].x_name, name) == 0)
	  /* check alternative X font name if any */
	  || (_pl_g_ps_font_info[i].x_name_alt
	      && strcasecmp (_pl_g_ps_font_info[i].x_name_alt, name) == 0)
	  /* check 2nd alternative X font name if any */
	  || (_pl_g_ps_font_info[i].x_name_alt2
	      && strcasecmp (_pl_g_ps_font_info[i].x_name_alt2, name) == 0)
	  /* check 3rd alternative X font name if any */
	  || (_pl_g_ps_font_info[i].x_name_alt3
	      && strcasecmp (_pl_g_ps_font_info[i].x_name_alt3, name) == 0))
	break;
    }
  
  if (_pl_g_ps_font_info[i].ps_name) /* matched name of a PS font in database */
    {
      matched_builtin = true;
      true_name = _pl_g_ps_font_info[i].ps_name;
      x_name = _pl_g_ps_font_info[i].x_name;
      x_name_alt = _pl_g_ps_font_info[i].x_name_alt;
      x_name_alt2 = _pl_g_ps_font_info[i].x_name_alt2;
      x_name_alt3 = _pl_g_ps_font_info[i].x_name_alt3;
      font_type = PL_F_POSTSCRIPT;
      typeface_index = _pl_g_ps_font_info[i].typeface_index;
      font_index = _pl_g_ps_font_info[i].font_index;
    }

#ifdef USE_LJ_FONTS_IN_X
  if (matched_builtin == false)	/* PS match failed, so try PCL fonts too */
    {
      i = -1;
      while (_pl_g_pcl_font_info[++i].ps_name) /* array ends in NULL */
	{
	  if ((strcasecmp (_pl_g_pcl_font_info[i].ps_name, name) == 0)
	      /* check alternative ps font name if any */
	      || (_pl_g_pcl_font_info[i].ps_name_alt
		  && strcasecmp (_pl_g_pcl_font_info[i].ps_name_alt, name) == 0)
	      /* check X font name */
	      || (strcasecmp (_pl_g_pcl_font_info[i].x_name, name) == 0))
	    break;
	}
  
      if (_pl_g_pcl_font_info[i].ps_name) /* matched name of a PCL font in db */
	{
	  matched_builtin = true;
	  true_name = _pl_g_pcl_font_info[i].ps_name;
	  x_name = _pl_g_pcl_font_info[i].x_name;
	  x_name_alt = NULL;
	  font_type = PL_F_PCL;
	  typeface_index = _pl_g_pcl_font_info[i].typeface_index;
	  font_index = _pl_g_pcl_font_info[i].font_index;
	}
    }
#endif /* USE_LJ_FONTS_IN_X */

#ifdef DEBUG
  fprintf (stderr, "Matched database font %s = %s = %s = %s\n", 
	   x_name, x_name_alt, x_name_alt2, x_name_alt3);
#endif

  if (matched_builtin)
    {
      /* user passed the name of a PS or PCL font in libplot's database */
      success = _pl_x_select_xlfd_font_carefully (R___(_plotter) x_name, 
						  x_name_alt, x_name_alt2,
						  x_name_alt3);
      if (success)
	/* RETRIEVAL TYPE #1: we've retrieved a core X font, the XLFD name
           of which was listed in libplot's hardcoded database; and have
           filled in X-specific fields */
	{
	  free ((char *)_plotter->drawstate->true_font_name);
	  _plotter->drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (true_name) + 1);
	  strcpy ((char *)_plotter->drawstate->true_font_name, true_name);

	  _plotter->drawstate->font_type = font_type;
	  _plotter->drawstate->typeface_index = typeface_index;
	  _plotter->drawstate->font_index = font_index;

#ifdef DEBUG
	  fprintf (stderr, "_pl_x_retrieve_font(): retrieved \"%s\" as \"%s\", type=%d\n", name, _plotter->drawstate->true_font_name, _plotter->drawstate->font_type);
	  fprintf (stderr, "_pl_x_retrieve_font(): font_size=%g, true_font_size=%g, font_ascent=%g, font_descent=%g, font_is_iso8859_1=%d, x_font_pixel_size=%d\n", _plotter->drawstate->font_size, _plotter->drawstate->true_font_size, _plotter->drawstate->font_ascent, _plotter->drawstate->font_descent, _plotter->drawstate->font_is_iso8859_1, _plotter->drawstate->x_font_pixel_size);
#endif
	  return true;
	}
    }
  
  /* User-specified font name didn't match either of the names of any PS
     [or PCL] font in libplot's database, so first handle the possibility
     that it's an XLFD base name for some other core X font
     (e.g. "charter-medium-r-normal"), with exactly three hyphens. */
  name_p = name;
  hyphen_count = 0;
  while (*name_p)
    hyphen_count += (*name_p++ == '-' ? 1 : 0);

  if (hyphen_count == 3)
    /* treat as base of an XLFD name */
    {
      success = _pl_x_select_xlfd_font_carefully (R___(_plotter) name, 
						  NULL, NULL, NULL);

      if (success)
	/* RETRIEVAL TYPE #3: we've retrieved a core X font, the base XLFD
           name of which was passed by the user, that isn't one of the
           fonts in libplot's hardcoded database; and have filled in
           X-specific fields */
	{
	  free ((char *)_plotter->drawstate->true_font_name);
	  _plotter->drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (name) + 1);
	  strcpy ((char *)_plotter->drawstate->true_font_name, name);

	  _plotter->drawstate->font_type = PL_F_OTHER;
	  /* these two fields are irrelevant because we don't support
	     switching among fonts not in libplot's internal database */
	  _plotter->drawstate->typeface_index = 0;
	  _plotter->drawstate->font_index = 1;

#ifdef DEBUG
	  fprintf (stderr, "_pl_x_retrieve_font(): retrieved \"%s\" as \"%s\", type=%d\n", name, _plotter->drawstate->true_font_name, _plotter->drawstate->font_type);
	  fprintf (stderr, "_pl_x_retrieve_font(): font_size=%g, true_font_size=%g, font_ascent=%g, font_descent=%g, font_is_iso8859_1=%d, x_font_pixel_size=%d\n", _plotter->drawstate->font_size, _plotter->drawstate->true_font_size, _plotter->drawstate->font_ascent, _plotter->drawstate->font_descent, _plotter->drawstate->font_is_iso8859_1, _plotter->drawstate->x_font_pixel_size);
#endif
	  return true;
	}  
    }
  
  /* User-passed name didn't have exactly 3 hyphens, so try treating it as
     the full name of a core X font; ignore size.  This a kludge, included
     partly to support pre-XLFD fonts, e.g. "9x15", and aliases for XLFD
     fonts, e.g. "fixed".  Most of the latter are really pre-XLFD names. */

    {
      double det;
      
      det = _plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3]
	- _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2];
      if (det == 0.0)
	/* singular user-space -> device-space map; bail */
	return false;

      /* Try to retrieve font from server or cache list, given its full
	 name; and for the moment, ignore the preferred pixel size we just
	 computed.  3rd argument `false' requests entire font.  A GOOD IDEA? */
      success = _pl_x_select_font_carefully (R___(_plotter) name,
					 _plotter->drawstate->x_label,
					 false);

      if (success)
	/* RETRIEVAL TYPE #4: we've retrieved a core X font, the full name
           of which was passed by the user, that isn't one of the fonts in
           libplot's hardcoded database */
	{
	  free ((char *)_plotter->drawstate->true_font_name);
	  _plotter->drawstate->true_font_name = 
	    (const char *)_pl_xmalloc (strlen (name) + 1);
	  strcpy ((char *)_plotter->drawstate->true_font_name, name);

	  _plotter->drawstate->font_type = PL_F_OTHER;
	  /* these two fields are irrelevant because we don't support
	     switching among `other' fonts */
	  _plotter->drawstate->typeface_index = 0;
	  _plotter->drawstate->font_index = 1;

	  if (_plotter->drawstate->x_font_pixel_size == 0) /* paranoia */
	    return false;

#ifdef DEBUG
	  fprintf (stderr, "_pl_x_retrieve_font(): retrieved \"%s\" as \"%s\", type=%d\n", name, _plotter->drawstate->true_font_name, _plotter->drawstate->font_type);
#endif
	  return true;
	}
    }
  
  /* couldn't retrieve a matching X font, so declare failure; this will
     lead (at a higher level; see g_retrieve.c) either to the retrieval of
     a default substitute X font, or a builtin Hershey font */
#ifdef DEBUG
	  fprintf (stderr, "_pl_x_retrieve_font(): FAILURE, couldn't retrieve \"%s\"\n", name);
#endif
  return false;
}

/* _pl_x_select_xlfd_font_carefully() is a helper function that
   x_retrieve_font() above uses.  It constructs a full XLFD name of a core
   X11 font from each of several specified base XLFD names, such as
   "helvetica-medium-r-normal", and attempts to retrieve them in order,
   until a successful retrieval occurs.  The inclusion of several
   alternatives is useful, since each of the built-in fonts in libplot's
   database (see g_fontdb.c) is associated with several possible base XLFD
   names.  That's because there is no standardization of names across
   vendors, even for the commonly used `Adobe 35' fonts, i.e., libplot's
   supported `Postscript fonts'.

   For each alternative, a fontname ending in -iso8859-1 (indicating the
   ISO-Latin-1 encoding) is tried, and if that doesn't work, a fontname
   ending in -*-* (the encoding being left up to the server).  The
   lower-level function _pl_x_select_font_carefully() does the retrieval.
   It is passed the `x_label' field of the drawing state, which is a hint
   indicating which characters are needed.  A proper subset of the font
   will be retrieved, if possible, to save time.  The proper subset is
   indicated to the server, as usual, by a suffix on the font name.  E.g.,
   ....-iso8859-1[88 89] consists only of the letters `X' and `Y'
   (characters 88 and 89).

   This code, when requesting the retrieval, must place an integer pixel
   size in the XLFD name.  The pixel size it chooses is based on (1) the
   font_size in user coordinates and (2) the transformation matrix, which
   takes user-space to device-space, i.e., to X11 pixel space.
   
   When any label is drawn, the retrieved font will in general be scaled,
   by XAffDrawString(); see the code in x_text.c.  So this code cleverly
   chooses an integer pixel size which, if it weren't for rounding to the
   closest integer, wouldn't require any scaling of the glyph bitmaps at
   all, provided that the user-space -> device-space is uniform (not
   "anamorphic"), and doesn't involve a rotation. */

bool
_pl_x_select_xlfd_font_carefully (R___(Plotter *_plotter) const char *x_name, const char *x_name_alt, const char *x_name_alt2, const char *x_name_alt3)
{
  char *x_name_buf;		/* buffer for creating font name */
  bool success = false;
  int integer_font_size_in_pixels;
  double det, font_size_in_pixels;

  det = _plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3]
    - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2];
  
  if (det == 0.0)
    /* singular user-space -> device-space map; bail */
    return false;

  /* Compute preferred pixel size for the core X font: the user-space font
     size, multiplied by a measure of the size of the user-space to
     device-space transformation matrix.  The "measure" we choose is the
     minimum of the matrix's two singular values.  (There are other
     possible choices for this measure.) */

  font_size_in_pixels = 
    min_sing_val (_plotter->drawstate->transform.m) *_plotter->drawstate->font_size;
  if (font_size_in_pixels == 0.0)
    /* preferred device-space font size, in terms of pixels, is zero; bail */
    return false;

  /* quantize to an integer pixel size: round downward */
  integer_font_size_in_pixels = (int)font_size_in_pixels;
      
  if (font_size_in_pixels == 0)
    /* integer device-space size, in terms of pixels, is zero; bail */
    return false;
      
  /* prepare buffer for font name assemblage */
  x_name_buf = (char *)_pl_xmalloc ((MAX_FONT_NAME_LENGTH + 1) * sizeof (char));

  /* try to retrieve font from server or cache list, after punching the
     pixel size into the appropriate XLFD fontname template */

  /* try Latin-1 fontname, i.e. fontname ending in -iso8859-1 */

  sprintf (x_name_buf, xlfd_template_latin_1, x_name, integer_font_size_in_pixels);
  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
					 _plotter->drawstate->x_label,
					 true);
#ifdef DEBUG
  fprintf (stderr, "_pl_x_select_xlfd_font_carefully(): retrieval begins with %s\n",
	   x_name_buf);
#endif

  if (success == false)
    /* try fontname ending in -*-* */
    {
      sprintf (x_name_buf, xlfd_template_generic, 
	       x_name, integer_font_size_in_pixels);
      success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
					     _plotter->drawstate->x_label,
					     true);
    }
    
  if (x_name_alt)
    /* alternative base XLFD name was supplied, so try it too */
    {
      if (success == false)
	/* try Latin-1 fontname, i.e. fontname ending in -iso8859-1 */
	{
	  sprintf (x_name_buf, xlfd_template_latin_1, 
		   x_name_alt, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label,
						 true);
	}
      if (success == false)
	/* try fontname ending in -*-* */
	{
	  sprintf (x_name_buf, xlfd_template_generic, 
		   x_name_alt, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label,
						 true);
	}
    }

  if (x_name_alt2)
    /* 2nd alternative base XLFD name was supplied, so try it too */
    {
      if (success == false)
	/* try Latin-1 fontname, i.e. fontname ending in -iso8859-1 */
	{
	  sprintf (x_name_buf, xlfd_template_latin_1, 
		   x_name_alt2, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label, 
						 true);
	}
      if (success == false)
	/* try fontname ending in -*-* */
	{
	  sprintf (x_name_buf, xlfd_template_generic, 
		   x_name_alt2, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label, 
						 true);
	}
    }

  if (x_name_alt3)
    /* 3rd alternative base XLFD name was supplied, so try it too */
    {
      if (success == false)
	/* try Latin-1 fontname, i.e. fontname ending in -iso8859-1 */
	{
	  sprintf (x_name_buf, xlfd_template_latin_1, 
		   x_name_alt3, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label, 
						 true);
	}
      if (success == false)
	/* try fontname ending in -*-* */
	{
	  sprintf (x_name_buf, xlfd_template_generic, 
		   x_name_alt3, integer_font_size_in_pixels);
	  success = _pl_x_select_font_carefully (R___(_plotter) x_name_buf,
						 _plotter->drawstate->x_label, 
						 true);
	}
    }

  if (success)
    /* A clever hack.  Slightly alter the true_font_size field (and
       font_ascent/font_descent/font_cap_height for consistency), in a way
       that will permit any text string to be rendered in this font by
       XDrawString(), rather than by our bitmap-scaling code in
       x_afftext.c.  Provided, that is, the text string isn't rotated, and
       the user-space -> device-space transformation is a uniform scaling
       that respects coordinate axes.
       
       The reason this works is that the rendering code in x_text.c calls
       our bitmap-scaling function XAffDrawString() with a certain
       transformation matrix a[] as an argument.  Under the above
       circumstances, this matrix will turn out to be the identity matrix,
       due to the slight modification performed below.  That being the
       case, XAffDrawString() will simply call XDrawString rather than
       performing any bitmap scaling.  The result: in the above
       circumstances, which are very common, the glyphs in the text string
       won't undergo a slight scaling traceable to the integer quantization
       of font pixel size, and will look better. */
    {
      double factor = integer_font_size_in_pixels / font_size_in_pixels;
      
      /* scale by this factor, quite close to unity */
      _plotter->drawstate->true_font_size *= factor;
      _plotter->drawstate->font_ascent *= factor;
      _plotter->drawstate->font_descent *= factor;
      _plotter->drawstate->font_cap_height *= factor;
    }

  return success;
}

/* _pl_x_select_font_carefully() is a wrapper around select_x_font() below.
   It attempts to retrieve the font NAME from the X server or from the
   per-Plotter font cache of already-retrieved fonts.  The character subset
   desired (if any) is specified by the string S, and whether an attempt
   should actually be made to retrieve a subset rather than the entire is
   specified by the parameter SUBSETTING.  The return value indicates
   whether the retrieval succeeded.

   A NULL value for S means the font is being retrieved only to look at its
   metrics; in which case select_x_font() will attempt to retrieve the
   character 'X' only, to permit the cap-height metric to be determined;
   and the space character, for good measure.  This is arranged in the
   low-level function string_to_bitvector() below; we _always_ include the
   characters 'X' and ' ' among those we ask the server to rasterize.

   The inclusion of 'X' incidentally works around a bug reported by Georgy
   Salnikov <sge@nmr.nioch.nsc.ru>, in XFree86-3.2.  (We previously tried
   to retrieve not 'X', but rather the space character.  But if an XLFD
   font name ends in *-*[32], i.e. font contains only a single space,
   XFree86 reports an error retrieving the font, and any executable linked
   with this code client will terminate.)

   If the font is successfully retrieved (either from the X server or from
   the cache), this function fills in most font-related fields of the
   drawing state.  That includes the generic fields true_font_size,
   font_ascent, font_descent, font_cap_height, font_is_iso8859_1, and the
   X-specific fields x_font_struct and x_font_pixel_size.

   The true_font_size is simply set equal to font_size (which is whatever
   the user passed to libplot by invoking the fontname() function in the
   libplot API).  That is because in principle, libplot's X driver can
   match exactly any user-specified font size, by scaling bitmaps.  See
   x_text.c and x_afftext.c for the code that renders text strings. */

bool
_pl_x_select_font_carefully (R___(Plotter *_plotter) const char *name, const unsigned char *s, bool subsetting)
{
  plXFontRecord *fptr;

  if (s == (unsigned char *)NULL)
    s = (unsigned char *)"";	/* "" is effectively "X " */

  /* attempt to retrieve the specified (subset of the) font */
  fptr = select_x_font (_plotter->x_dpy, &(_plotter->x_fontlist),
			   name, s, subsetting);

#ifdef DEBUG
  fprintf (stderr, "_pl_x_select_font_carefully(): select_x_font() returns %p\n",
	   fptr);
#endif  
  
  if (subsetting && (fptr == (plXFontRecord *)NULL))
    /* failure; so try to retrieve entire font instead of a subset,
       ignoring the passed hint string S (perhaps server doesn't support
       subsetting?) */
    fptr = select_x_font (_plotter->x_dpy, &(_plotter->x_fontlist), 
			   name, s, false);
  
  if (fptr == (plXFontRecord *)NULL)
    /* couldn't retrieve font from cache or from server */
    return false;
  else
    /* Success, so fill in fields of the drawing state from the returned
       font record.  Most are generic rather than X-specific.  Ascent,
       descent and cap_height are user-space quantities; so are scaled by
       the font_size, which is expressed in user-space units. */
    {
      if (fptr->x_font_pixel_size <= 0) /* paranoia */
	return false;

#ifdef DEBUG
      fprintf (stderr, "fontname = %s, min_char_or_byte2 = %u, max_char_or_byte2 = %u, ascent = %d, descent = %d, default_char = %u\n", name, fptr->x_font_struct->min_char_or_byte2, fptr->x_font_struct->max_char_or_byte2, fptr->x_font_struct->ascent, fptr->x_font_struct->descent, fptr->x_font_struct->default_char);
#endif

      /* set generic fields */
      _plotter->drawstate->true_font_size = _plotter->drawstate->font_size;
      _plotter->drawstate->font_ascent =
	((fptr->x_font_struct->ascent * _plotter->drawstate->font_size)
	 / fptr->x_font_pixel_size);
      _plotter->drawstate->font_descent =
	((fptr->x_font_struct->descent * _plotter->drawstate->font_size)
	 / fptr->x_font_pixel_size);
      _plotter->drawstate->font_cap_height =
	((fptr->x_font_cap_height * _plotter->drawstate->font_size)
	 / fptr->x_font_pixel_size);
      
      _plotter->drawstate->font_is_iso8859_1 = fptr->x_font_is_iso8859_1;
      
      /* set X-specific fields */
      _plotter->drawstate->x_font_struct = fptr->x_font_struct;
      _plotter->drawstate->x_font_pixel_size = fptr->x_font_pixel_size;
      
      return true;
    }
}

/* Attempt to retrieve a core X font, as a plXFontRecord.  The font is
   specified by NAME, and a hint as to which characters will need to be
   rendered is passed as the non-null string S.  This permits the retrieval
   of a proper subset of the font, if desired.  The SUBSETTING parameter
   indicates whether the retrieval of an appropriate subset of the font
   should first be attempted, before retrieval of the entire font.

   The X_FONTLIST_PTR argument passes [by reference!] a pointer to a font
   cache, a linked list of plXFontRecords that will be searched.  If the
   font isn't found in the cache but can be successfully retrieved from the
   X display server instead, a new record is added to the head of this
   list; and if it can't be, a null (invalid) record is added to the head
   of the list; in both cases, to speed up later retrieval attempts.

   Return value: a pointer to the font record, if a font was found in the
   cache or newly added to it; otherwise NULL.  */

static plXFontRecord *
select_x_font (Display *dpy, plXFontRecord **x_fontlist_ptr, const char *name, const unsigned char *s, bool subsetting)
{
  bool found = false;
  unsigned char bitvec[32];
  plXFontRecord *x_fontlist, *fptr;

#ifdef DEBUG
  fprintf (stderr, "select_x_font (name=\"%s\", subset=\"%s\", subsetting=%d)\n", 
	   name, (const char *)s, subsetting);
#endif

  if (subsetting)
    /* construct 256-bit vector specifying charset subset */
    string_to_bitvector (s, bitvec);

  /* get head of linked-list cache */
  x_fontlist = *x_fontlist_ptr;

  /* attempt to find font in cache */
  for (fptr = x_fontlist; fptr; fptr = fptr->next)
    {
#ifdef DEBUG
      fprintf (stderr, "select_x_font(): cache entry: name=\"%s\", subset=%d\n",
	       fptr->x_font_name, fptr->subset);
#endif
      
      if (strcmp (name, fptr->x_font_name) == 0)
	{
	  if ((subsetting && fptr->subset
	       && is_a_subset (bitvec, fptr->subset_vector))
	      || (subsetting && (fptr->subset == false))
	      || (subsetting == false && fptr->subset == false))
	    {
	      found = true;
	      break;
	    }
	}
    }
  
  if (found)
    {
      if (fptr->x_font_struct)
	/* found record was a genuine one */
	{
#ifdef DEBUG
	  fprintf (stderr, "select_x_font(): font cache HIT on name=%s, s=\"%s\"\n", name, s);
#endif
	  return fptr;
	}
      else
	{
#ifdef DEBUG
	  fprintf (stderr, "select_x_font(): font cache HIT (fake) on name=\"%s\", s=\"%s\"\n", name, s);
#endif
	  /* invalid record: an earlier retrieval attempt must have failed */
	  return (plXFontRecord *)NULL;
	}
    }
  
#ifdef DEBUG
  fprintf (stderr, "select_x_font(): font cache miss on name=\"%s\", s=\"%s\"\n", name, s);
#endif

  /* no record in cache, so try to retrieve font from X server */
  {
    char *tmpname, *tmpname_perm, *_charset_subset_list = NULL;
    int extra = 0;

    /* allocate space for new record, update pointer to cache to include it */
    fptr = 
      (plXFontRecord *)_pl_xmalloc (sizeof (plXFontRecord));
    fptr->next = *x_fontlist_ptr;
    *x_fontlist_ptr = fptr;

    if (subsetting)
      {
	_charset_subset_list = 
	  (char *)_pl_xmalloc ((MAX_CHARSET_SUBSET_LIST_LENGTH + 1) * sizeof (char));
	print_bitvector (bitvec, _charset_subset_list);
	extra = strlen (_charset_subset_list);
      }
    tmpname_perm = (char *)_pl_xmalloc (1 + strlen (name));
    strcpy (tmpname_perm, name);
    tmpname = (char *)_pl_xmalloc (1 + strlen (name) + extra);
    strcpy (tmpname, name);
    if (subsetting)
      {
	/* append X11R6 list-of-ranges to name to be sent to server */
	strcat (tmpname, _charset_subset_list);
	free (_charset_subset_list);
      }
    
#ifdef DEBUG
    fprintf (stderr, "select_x_font(): trying to invoke XLoadQueryFont on \"%s\", subsetting=%d\n", tmpname, subsetting);
#endif
    
    /* attempt to retrieve font from server; return value from
       XLoadQueryFont() equalling NULL indicates failure */
    fptr->x_font_struct = 
      XLoadQueryFont (dpy, tmpname);
    free (tmpname);
      
    /* whether or not there was success, fill in some add'l fields of record */
    fptr->x_font_name = tmpname_perm; /* don't include subset in stored name */
    fptr->subset = subsetting;
    if (subsetting)
      memcpy (fptr->subset_vector, bitvec, 32 * sizeof (unsigned char));

    /* handle a special case: retrieval from server succeeded, but the
       retrieved font wasn't an 8-bit font, so we can't use it */

    if (fptr->x_font_struct 
	&& (fptr->x_font_struct->min_byte1 != 0
	    || fptr->x_font_struct->max_byte1 != 0))
      /* treat as if retrieval failed */
      {
	XFreeFont (dpy, fptr->x_font_struct);
	fptr->x_font_struct = (XFontStruct *)NULL;
      }

    if (fptr->x_font_struct)
      /* retrieval succeeded */
      {
#ifdef DEBUG
	fprintf (stderr, "select_x_font(): loaded font \"%s\"\n", name);
#endif	  
	/* fill in, as well, the x_font_pixel_size, x_font_cap_height,
	   x_font_iso_8859_1 fields of the font record */
	set_font_dimensions (dpy, fptr);
	
	return fptr;		/* X font selected */
      }
    else
      /* retrieval failed */
      {
#ifdef DEBUG
	fprintf (stderr, "select_x_font(): failed to load font \"%s\"\n", name);
#endif	  
	return (plXFontRecord *)NULL;
      }
  }
}

/* Complete the filling in of an plXFontRecord, by filling in the fields
   x_font_pixel_size, x_font_cap_height, and x_font_is_iso8859_1, on the
   basis of the x_font_struct field.  Called by preceding function.  */

static void
set_font_dimensions (Display *dpy, plXFontRecord *fptr)
{
  unsigned long retval;
  char *name, *pixel_field;
  char *charset_major_field, *charset_minor_field;

#ifdef DEBUG2
  {
    int i;

    for (i = 0; i < fptr->x_font_struct->n_properties; i++)
      fprintf (stderr, "\tproperty %s [atom %lu] is %ld\n", 
	       XGetAtomName(dpy, fptr->x_font_struct->properties[i].name),
	       fptr->x_font_struct->properties[i].name,
	       fptr->x_font_struct->properties[i].card32);
  }    
#endif

  if (XGetFontProperty (fptr->x_font_struct, XA_FONT, &retval))
    /* this font has a FONT property, as any well behaved font should */
    {
      /* Extract relevant fields from this property (i.e. from X server's
	 idea of the font name).  This will work if it's an XLFD name. */
      name = XGetAtomName (dpy, retval); 

#ifdef DEBUG
      fprintf (stderr, "set_font_dimensions(): FONT property is \"%s\"\n", name);
#endif
      pixel_field = xlfd_field (name, XLFD_FIELD_PIXELS);
      charset_major_field = xlfd_field (name, XLFD_FIELD_CHARACTER_SET_MAJOR);
      charset_minor_field = xlfd_field (name, XLFD_FIELD_CHARACTER_SET_MINOR);
      XFree (name);

      /* determine whether font encoding is ISO-Latin-1 */
      if ((charset_major_field != NULL) && (charset_minor_field != NULL)
	  && strcasecmp (charset_major_field, "iso8859") == 0
	  && (charset_minor_field[0] == (char)'1'
	      && (charset_minor_field[1] == (char)0 /* string terminator */
		  || charset_minor_field[1] == (char)'[')))
	  fptr->x_font_is_iso8859_1 = true;
      else
	  fptr->x_font_is_iso8859_1 = false;

      if (charset_major_field)
	free (charset_major_field);
      if (charset_minor_field)
	free (charset_minor_field);

      if (pixel_field != NULL)	
	/* font presumably has an XLFD name, since it has a pixel field */
	{
	  /* extract x_font_pixel_size from the pixel field */

	  unsigned int size;
	  
	  sscanf (pixel_field, "%u", &size);
	  fptr->x_font_pixel_size = size;
	  free (pixel_field);
	  
	  /* fill in the font_{cap_height} field; we get it from the ascent
	     of the `X' character, if it exists */
	  
	  if ('X' >= fptr->x_font_struct->min_char_or_byte2
	      && 'X' <= fptr->x_font_struct->max_char_or_byte2
	      && fptr->x_font_struct->per_char)
	    /* have `X' char in the font, and have per-char data */
	    {
	      int X = 'X' - fptr->x_font_struct->min_char_or_byte2;
	      
	      fptr->x_font_cap_height
		= fptr->x_font_struct->per_char[X].ascent;
	    }
	  else		/* do our best */
	      fptr->x_font_cap_height
		= fptr->x_font_struct->min_bounds.ascent;
	  
	  /* we've set all fields, so we can return */
	  return;
	}
#ifdef DEBUG2
      fprintf (stderr, "FONT property does not exist\n");
#endif
    }
  else
  /* font doesn't have an XLFD name (so no pixel size field), or there's no
     FONT property at all (a bad situation) */
    {
      Atom pixel_size_atom, resolution_y_atom;
      unsigned long point_size, resolution_y;
      
      fptr->x_font_is_iso8859_1 = false; /* assumed (worst case) */
      
      pixel_size_atom = XInternAtom (dpy, "PIXEL_SIZE", (Bool)false);

      if (XGetFontProperty (fptr->x_font_struct, pixel_size_atom, &retval))
	/* there's a PIXEL_SIZE property, so use it to compute font size */
	{
#ifdef DEBUG2
	  fprintf (stderr, "PIXEL_SIZE property is \"%lu\"\n", retval);
#endif	
	  fptr->x_font_pixel_size = retval;
	}
      else
	/* no PIXEL_SIZE, so try to compute pixel size from POINT_SIZE and
	   RESOLUTION_Y properties */
	{
#ifdef DEBUG2
	  fprintf (stderr, "PIXEL_SIZE property does not exist\n");
#endif
	  resolution_y_atom = XInternAtom (dpy, "RESOLUTION_Y", (Bool)false);
	  if (XGetFontProperty (fptr->x_font_struct, XA_POINT_SIZE, &point_size)
	      && (XGetFontProperty (fptr->x_font_struct, 
				    resolution_y_atom, &resolution_y)))
	    {
#ifdef DEBUG2
	      fprintf (stderr, "POINT_SIZE property is \"%lu\"\n", 
		       point_size);
	      fprintf (stderr, "RESOLUTION_Y property is \"%lu\"\n", 
		       resolution_y);
#endif	
	      fptr->x_font_pixel_size = 
		IROUND(((double)point_size * (double)resolution_y / 722.7));
	    }
	  else 
	    /* we can't compute the font size legitimately, so estimate it
	       from the XFontStruct (may not be reliable) */
	    {
#ifdef DEBUG2
	      fprintf (stderr, "POINT_SIZE and/or RESOLUTION_Y properties do not exist\n");
#endif
	      fptr->x_font_pixel_size = fptr->x_font_struct->ascent + fptr->x_font_struct->descent;
	    }
	}

      fptr->x_font_cap_height
	= fptr->x_font_struct->per_char['X' - fptr->x_font_struct->min_char_or_byte2].ascent;
    }
}

/* Extract a field from an XLFD name string, by number, and return it, via
   a call to malloc.  If `name' doesn't appear to be an XLFD name, NULL is
   returned. */

static char *
xlfd_field(const char *name, int field)
{
  const char *p;
  const char *fields[PL_NUM_XLFD_FIELDS];
  char *retstring;
  int len[PL_NUM_XLFD_FIELDS];
  int i, n, m;
  /* split into fields at hyphens */
  for (p = name, i = 0, n = 0, m = 0; 
       *p && (i < PL_NUM_XLFD_FIELDS); 
       p++, n++, m++)
    {
      if (*p == '-')
	{
	  if (i > 0)
	    len[i-1] = n;
	  n = 0;
	  fields[i++] = p;
	}
    }
  if (i < PL_NUM_XLFD_FIELDS)
    return NULL;

  len[PL_NUM_XLFD_FIELDS - 1] = strlen (name) - (m - 1); /* final field exhausts string */

  /* for len[] and fields[], each field includes initial hyphen */
  retstring = (char *)_pl_xmalloc (len[field] * sizeof(char));
  strncpy (retstring, fields[field] + 1, 
	   (unsigned int)(len[field] - 1)); /* skip initial - */
  retstring[len[field] - 1] = '\0';
  
  return retstring;
}     

/* Prepare a bit vector (length 256 bits, i.e. 32 bytes) indicating which
   characters in the range 1..255 are used in string S.  We always include
   the character 'X', even if it isn't present in the string.  `X' is
   special because we can subsequently use it to retrieve the cap height.
   For backward compatibility (not necessary?) we also include the space
   character. */

static void
string_to_bitvector (const unsigned char *s, unsigned char v[32])
{
  unsigned char c;
  unsigned int i, j;
  int k;

  for (k = 0; k < 32; k++)
    v[k] = 0;

  /* include the X character */
  c = 'X';
  i = c / 8;
  j = c % 8;
  v[i] |= (1 << j);

  /* include the space character too */
  c = ' ';
  i = c / 8;
  j = c % 8;
  v[i] |= (1 << j);

  /* include all characters in the passed string */
  while ((c = *s) != (unsigned char)'\0')
    {
      i = c / 8;
      j = c % 8;
#ifdef DEBUG2
      fprintf (stderr, "saw char %d (i.e. %c), stored as %d,%d\n", c, c, i, j);
#endif
      v[i] |= (1 << j);
      s++;
    }
}

/* This writes a bitvector as a string, in the form used in X11R6-style
   charset subsetting.  Each range of chars may require the writing of up
   to 8 bytes, e.g. " 160_180". The list of ranges is contained within
   brackets. */

static void
print_bitvector (unsigned char v[32], char *s)
{
  int i, num_ranges_output = 0, num_chars_output = 0;
  int start_of_range = 0;
  bool used;
  bool in_range = false;
  
  *s++ = '[';
  for (i = 0; i <= 256; i++)
    {
      if (i == 256)
	used = false;
      else
	used = (v[i / 8] & (1 << (i % 8))) ? true : false;

#ifdef DEBUG2
      if (used)
	fprintf (stderr, "stored char %d (i.e. %c), from %d,%d\n", i, i, i/8, i%8);
#endif

      if (used && in_range == false)
	/* begin new range */
	{
	  start_of_range = i;
	  in_range = true;
	}
      else if (used == false && in_range)
	/* end of range, so output the range */
	{
	  int hundreds, tens, ones;
	  bool hundreds_output;

	  if (num_chars_output > MAX_CHARSET_SUBSET_LIST_LENGTH - 8)
	    break;		/* abort to avoid buffer overrun */

	  if (num_ranges_output > 0)
	    /* use space as separator */
	    {
	      *s++ = ' ';
	      num_chars_output++;
	    }
	  
#ifdef DEBUG2
	  fprintf (stderr, "outputting character range %d..%d, i.e. %c..%c\n",
		   start_of_range, i-1, start_of_range, i-1);
#endif
	  if (start_of_range < (i - 1))
	    /* have a genuine range, start..(i-1), not a singleton */
	    {
	      /* output start of range, followed by underscore */
	      hundreds = start_of_range / 100;
	      tens = (start_of_range - hundreds * 100) / 10;
	      ones = start_of_range % 10;
	      hundreds_output = false;
	      if (hundreds > 0)
		{
		  *s++ = (char)'0' + hundreds;
		  hundreds_output = true;
		  num_chars_output++;
		}
	      if (hundreds_output || tens > 0)
		{
		  *s++ = (char)'0' + tens;
		  num_chars_output++;
		}
	      *s++ = (char)'0' + ones;
	      num_chars_output++;
	      *s++ = (char)'_';
	      num_chars_output++;	      
	    }

	  /* output end of range, which is i-1 */
	  hundreds = (i-1) / 100;
	  tens = ((i-1) - hundreds * 100) / 10;
	  ones = (i-1) % 10;
	  hundreds_output = false;
	  if (hundreds > 0)
	    {
	      *s++ = (char)'0' + hundreds;
	      hundreds_output = true;
	      num_chars_output++;
	    }
	  if (hundreds_output || tens > 0)
	    {
	      *s++ = (char)'0' + tens;
	      num_chars_output++;
	    }
	  *s++ = (char)'0' + ones;
	  num_chars_output++;

	  /* no longer in range */
	  in_range = false;
	  num_ranges_output++;
	}
    }
  *s++ = ']';
  /* add final null */
  *s = '\0';
}

static bool
is_a_subset (unsigned char set1[32], unsigned char set2[32])
{
  int i;
  bool retval = true;
  
  for (i = 0; i < 32; i++)
    if (set1[i] & ~(set2[i]))
      {
	retval = false;
	break;
      }

  return retval;
}

/* Compute the minimum of the two singular values of a 2x2 matrix.  Used
   for computing our favored pixel size, at which to retrieve a font; the
   matrix is the user-space -> device-space transformation matrix. */

static double
min_sing_val (double m[4])
{
  double mm[4], mprod[4];
  double mag, max_mag = 0.0;
  double trace, det, b2_4ac, min_sing_val_squared, min_sing_val;
  int i;

  /* scale the elements of m so that the largest has magnitude unity, to
     reduce the chance of floating point roundoff error; this scaling will
     be undone at the end */

  for (i = 0; i < 4; i++)
    {
      mag = fabs (m[i]);
      if (mag > max_mag)
	max_mag = mag;
    }
  if (max_mag <= 0.0)
    return 0.0;
  for (i = 0; i < 4; i++)
    mm[i] = m[i] / max_mag;

  /* Compute M times the transpose of M.  In the absence of floating-point
     rounding error, this product matrix, which is symmetric, will be
     "non-negative", i.e., its eigenvalues will be non-negative.  The
     singular values of M are (square roots of) its eigenvalues. */

  mprod[0] = mm[0]*mm[0] + mm[1]*mm[1];
  mprod[1] = mm[0]*mm[2] + mm[1]*mm[3];
  mprod[2] = mm[2]*mm[0] + mm[3]*mm[1];
  mprod[3] = mm[2]*mm[2] + mm[3]*mm[3];
  
  trace = mprod[0] + mprod[3];
  det = mprod[0] * mprod[3] - mprod[1] * mprod[2];

  if (det < 0.0)		/* rare rounding error problem */
    return 0.0;
  
  /* sing vals are (square roots of) solns of x^2 - trace * x + det = 0 */

  b2_4ac = trace * trace - 4 * det;

  if (b2_4ac < 0.0)
    /* a common, innocuous rounding error problem */
    b2_4ac = 0.0;

  min_sing_val_squared = 0.5 * (trace - sqrt (b2_4ac));
  if (min_sing_val_squared < 0.0) /* rare rounding error problem */
    return 0.0;
  
  min_sing_val = sqrt (min_sing_val_squared);

  /* return minimum singular value, not forgetting to undo the useful
     scaling with which we began */
  return min_sing_val * max_mag;
}
