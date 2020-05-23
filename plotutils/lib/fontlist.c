/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, 2009, Free Software
   Foundation, Inc.

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

/* This file contains the display_fonts and list_fonts routines, which are
   used in user-level executables that are linked with libplot.  (Their
   output is device-specific.)  These functions are declared in fontlist.h.

   Currently, they get information about font names by invoking
   undocumented members of the libplot API, which return pointers to
   internal library data structures.  These undocumented functions are
   _pl_get_hershey_font_info, _pl_get_ps_font_info, _pl_get_pcl_font_info,
   and _pl_get_stick_font_info. */

#include "sys-defines.h"
#include "libcommon.h"
#include "plot.h"

#include "fontlist.h"

/* for use in printing font names in two columns; assumption is that all
   font name strings have lengths in range 0..MAX_FONTNAME_LEN inclusive
   (not counting final null) */
#define MAX_FONTNAME_LEN 36
static char spaces[MAX_FONTNAME_LEN+1] = "                                   ";

/* The definitions of these structures are taken from ../libplot/extern.h.
   IF THOSE STRUCTURES CHANGE, THESE SHOULD TOO.

   Font information is stored in ../libplot/g_fontdb.c, and we'll retrieve
   pointers to it by using the undocumented libplot functions. */

struct plHersheyFontInfoStruct 
{
  const char *name;		/* font name */
  const char *othername;	/* an alias (for backward compatibility) */
  const char *orig_name;	/* Allen Hershey's original name for it */
  short chars[256];		/* array of vector glyphs */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
  bool visible;			/* whether font is visible, i.e. not internal */
};

struct plPSFontInfoStruct
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *ps_name_alt2;	/* 2nd alternative PS font name, if non-NULL */
  const char *x_name;		/* the X Windows font name */
  const char *x_name_alt;	/* alternative X Windows font name */
  const char *x_name_alt2;	/* 2nd alternative X Windows font name */
  const char *x_name_alt3;	/* 3rd alternative X Windows font name */
  const char *css_family;	/* CSS font family */
  const char *css_generic_family; /* CSS generic font family */
  const char *css_style;	/* CSS font style */
  const char *css_weight;	/* CSS font weight */
  const char *css_stretch;	/* CSS font stretch */
  const char *css_panose;	/* CSS font Panose */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int font_cap_height;		/* the font's cap height */
  int font_x_height;		/* the font's x height */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  int fig_id;			/* Fig's font id */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

struct plPCLFontInfoStruct
{
  const char *ps_name;		/* the postscript font name */
  const char *ps_name_alt;	/* alternative PS font name, if non-NULL */
  const char *substitute_ps_name; /* replacement name (for use in a PS file) */
  const char *x_name;		/* the X Windows font name */
  const char *css_family;	/* CSS font family */
  const char *css_generic_family; /* CSS generic font family */
  const char *css_style;	/* CSS font style */
  const char *css_weight;	/* CSS font weight */
  const char *css_stretch;	/* CSS font stretch */
  const char *css_panose;	/* CSS font Panose */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1, etc. */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int font_cap_height;		/* the font's cap height */
  int font_x_height;		/* the font's x height */
  short width[256];		/* per-character width information */
  short offset[256];		/* per-character left edge information */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool iso8859_1;		/* whether font encoding is iso8859-1 */
};

struct plStickFontInfoStruct 
{
  const char *ps_name;		/* the postscript font name */
  bool basic;			/* basic stick font (supp. on all devices)? */
  int pcl_typeface;		/* the PCL typeface number */
  int hpgl_spacing;		/* 0=fixed width, 1=variable */
  int hpgl_posture;		/* 0=upright, 1=italic, etc. */
  int hpgl_stroke_weight;	/* 0=normal, 3=bold, 4=extra bold, etc. */
  int hpgl_symbol_set;		/* 0=Roman-8, 14=ISO-8859-1 */
  int font_ascent;		/* the font's ascent (from bounding box) */
  int font_descent;		/* the font's descent (from bounding box) */
  int raster_width_lower;	/* width of abstract raster (lower half) */
  int raster_height_lower;	/* height of abstract raster (lower half) */
  int raster_width_upper;	/* width of abstract raster (upper half) */
  int raster_height_upper;	/* height of abstract raster (upper half) */
  int hpgl_charset_lower;	/* old HP character set number (lower half) */
  int hpgl_charset_upper;	/* old HP character set number (upper half) */
  int kerning_table_lower;	/* number of a kerning table (lower half) */
  int kerning_table_upper;	/* number of a kerning table (upper half) */
  char width[256];		/* per-character width information */
  int offset;			/* left edge (applies to all chars) */
  int typeface_index;		/* default typeface for the font */
  int font_index;		/* which font within typeface this is */
  bool obliquing;		/* whether to apply obliquing */
  bool iso8859_1;		/* encoding is iso8859-1? (after reencoding) */
};

/* List of Plotter types we support getting font information from,
   NULL-terminated.  This list also appears in the program text below. */
#ifdef INCLUDE_PNG_SUPPORT
#ifndef X_DISPLAY_MISSING
static const char *_known_devices[] =
{ "X", "png", "pnm", "gif", "svg", "ai", "ps", "cgm", "fig", "pcl", "hpgl", "regis", "tek", "meta", NULL };
#else
static const char *_known_devices[] =
{ "png", "pnm", "gif", "svg", "ai", "ps", "cgm", "fig", "pcl", "hpgl", "regis", "tek", "meta", NULL };
#endif
#else  /* not INCLUDE_PNG_SUPPORT */
#ifndef X_DISPLAY_MISSING
static const char *_known_devices[] =
{ "X", "pnm", "gif", "svg", "ai", "ps", "cgm", "fig", "pcl", "hpgl", "regis", "tek", "meta", NULL };
#else
static const char *_known_devices[] =
{ "pnm", "gif", "svg", "ai", "ps", "cgm", "fig", "pcl", "hpgl", "regis", "tek", "meta", NULL };
#endif
#endif /* not INCLUDE_PNG_SUPPORT */

int
display_fonts (const char *output_format, const char *progname)
{
  plPlotter *plotter;
  plPlotterParams *plotter_params;
  int numfonts, numpairs, i, j, k;
  bool found = false, odd;
  const char **device_ptr = _known_devices;

  while (*device_ptr)
    if (strcmp (output_format, *device_ptr++) == 0)
      {
	found = true;
	break;
      }

  if (found == false || strcmp (output_format, "meta") == 0)
    {
#ifdef INCLUDE_PNG_SUPPORT
#ifndef X_DISPLAY_MISSING
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format, and is one of:\n\
X, png, pnm, gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, tek (vector formats).\n",
	       progname);
#else  /* X_DISPLAY_MISSING */
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format, and is one of:\n\
png, pnm, gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, tek (vector formats).\n",
	       progname);
#endif /* X_DISPLAY_MISSING */
#else  /* not INCLUDE_PNG_SUPPORT */
#ifndef X_DISPLAY_MISSING
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format, and is one of:\n\
X, pnm, or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, tek (vector formats).\n",
	       progname);
#else  /* X_DISPLAY_MISSING */
      fprintf (stderr, "\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format, and is one of:\n\
pnm or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, tek (vector formats).\n",
	       progname);
#endif /* X_DISPLAY_MISSING */
#endif /* not INCLUDE_PNG_SUPPORT */
      return 0;
    }

  plotter_params = pl_newplparams ();
  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr,
			     plotter_params)) == NULL)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, output_format);
      return 0;
    }

  if (pl_havecap_r (plotter, "HERSHEY_FONTS"))
    {
      const struct plHersheyFontInfoStruct *hershey_font_info = 
	(const struct plHersheyFontInfoStruct *)_pl_get_hershey_font_info (plotter);
      int visible_num;

      numfonts = 0;
      for (i=0; hershey_font_info[i].name; i++)
	if (hershey_font_info[i].visible)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      /* compute j and k: j=0, k=numpairs + (odd ? 1 : 0) in terms of
         visibles */
      j = 0;
      k = 0;
      visible_num = -1;
      for (i=0; hershey_font_info[i].name; i++)
	if (hershey_font_info[i].visible)
	  {
	    visible_num++;  /* visible_num is index into array of visibles */
	    if (visible_num == 0)
	      j = i;
	    else if (visible_num == numpairs + (odd ? 1 : 0))
	      k = i;
	  }

      fprintf (stdout, 
	       "Names of supported Hershey vector fonts (case-insensitive):\n");
      for (i=0; i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (hershey_font_info[j].name);
	  fprintf (stdout, "\t%s", hershey_font_info[j].name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", hershey_font_info[k].name);
	  /* bump both j and k */
	  do
	    j++;
	  while (hershey_font_info[j].visible == false);
	  if (i < numpairs - 1)
	    {
	      do
		k++;
	      while (hershey_font_info[k].visible == false);
	    }
	}
      if (odd)
	fprintf (stdout, "\t%s\n", hershey_font_info[j].name);
    }

  if (pl_havecap_r (plotter, "STICK_FONTS"))
    {
      const struct plStickFontInfoStruct *stick_font_info = 
	(const struct plStickFontInfoStruct *)_pl_get_stick_font_info (plotter);
      int extra_fonts, *goodfonts;

      numfonts = 0;
      for (i=0; stick_font_info[i].ps_name; i++)
	numfonts++;

      /* if this Plotter doesn't support extras, skip them */
      extra_fonts = pl_havecap_r (plotter, "EXTRA_STICK_FONTS");
      goodfonts = (int *)xmalloc (numfonts * sizeof(int));
      for (i=0, j=0; stick_font_info[i].ps_name; i++)
	{
	  if (!extra_fonts && stick_font_info[i].basic == false)
	    continue;
	  goodfonts[j++] = i;
	}
      numfonts = j;

      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported HP vector fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (stick_font_info[goodfonts[j]].ps_name);
	  fprintf (stdout, "\t%s", stick_font_info[goodfonts[j++]].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", stick_font_info[goodfonts[k++]].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", stick_font_info[goodfonts[j]].ps_name);

      free (goodfonts);
    }

  if (pl_havecap_r (plotter, "PCL_FONTS"))
    {
      const struct plPCLFontInfoStruct *pcl_font_info = 
	(const struct plPCLFontInfoStruct *)_pl_get_pcl_font_info (plotter);

      numfonts = 0;
      for (i=0; pcl_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported PCL fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (pcl_font_info[j].ps_name);
	  fprintf (stdout, "\t%s", pcl_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", pcl_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", pcl_font_info[j].ps_name);
    }

  if (pl_havecap_r (plotter, "PS_FONTS"))
    {
      const struct plPSFontInfoStruct *ps_font_info = 
	(const struct plPSFontInfoStruct *)_pl_get_ps_font_info (plotter);
      numfonts = 0;
      for (i=0; ps_font_info[i].ps_name; i++)
	numfonts++;
      odd = (numfonts % 2 == 1 ? true : false);
      numpairs = numfonts / 2;

      fprintf (stdout, 
	       "Names of supported Postscript fonts (case-insensitive):\n");
      for (i=0, j=0, k=numpairs + (odd ? 1 : 0); i < numpairs; i++)
	{
	  int len;
	  
	  len = strlen (ps_font_info[j].ps_name);
	  fprintf (stdout, "\t%s", ps_font_info[j++].ps_name);
	  spaces[MAX_FONTNAME_LEN - len] = '\0';
	  fputs (spaces, stdout);
	  spaces[MAX_FONTNAME_LEN - len] = ' ';
	  fprintf (stdout, "%s\n", ps_font_info[k++].ps_name);
	}
      if (odd)
	fprintf (stdout, "\t%s\n", ps_font_info[j].ps_name);
    }

  if (strcmp (output_format, "X") == 0)
    {
      fprintf (stdout, 
	       "Most core X Window System fonts, such as charter-medium-r-normal,\n");
      fprintf (stdout,
	       "can also be used.\n");
    }

  return 1;
}

/* Write font names to standard output, in a line-by-line rather than a
   tabular form. */

int
list_fonts (const char *output_format, const char *progname)
{
  plPlotter *plotter;
  plPlotterParams *plotter_params;
  bool found = false;
  int i;
  const char **device_ptr = _known_devices;

  while (*device_ptr)
    if (strcmp (output_format, *device_ptr++) == 0)
      {
	found = true;
	break;
      }

  if (found == false)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, output_format);
      return 0;
    }

  plotter_params = pl_newplparams ();
  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr,
			     plotter_params)) == NULL)
    {
      fprintf (stderr, 
	       "%s: no font information on display device \"%s\" is available\n",
	       progname, output_format);
      return 0;
    }

  if (pl_havecap_r (plotter, "HERSHEY_FONTS"))
    {
      const struct plHersheyFontInfoStruct *hershey_font_info = 
	(const struct plHersheyFontInfoStruct *)_pl_get_hershey_font_info (plotter);
      for (i=0; hershey_font_info[i].name; i++)
	if (hershey_font_info[i].visible)
	  fprintf (stdout, "%s\n", hershey_font_info[i].name);
    }

  if (pl_havecap_r (plotter, "STICK_FONTS"))
    {
      const struct plStickFontInfoStruct *stick_font_info = 
	(const struct plStickFontInfoStruct *)_pl_get_stick_font_info (plotter);
      int extra_fonts = pl_havecap_r (plotter, "EXTRA_STICK_FONTS");

      for (i=0; stick_font_info[i].ps_name; i++)
	{
	  if (!extra_fonts && stick_font_info[i].basic == false)
	    continue;
	  fprintf (stdout, "%s\n", stick_font_info[i].ps_name);
	}
    }

  if (pl_havecap_r (plotter, "PCL_FONTS"))
    {
      const struct plPCLFontInfoStruct *pcl_font_info = 
	(const struct plPCLFontInfoStruct *)_pl_get_pcl_font_info (plotter);

      for (i=0; pcl_font_info[i].ps_name; i++)
	fprintf (stdout, "%s\n", pcl_font_info[i].ps_name);
    }
      
  if (pl_havecap_r (plotter, "PS_FONTS"))
    {
      const struct plPSFontInfoStruct *ps_font_info = 
	(const struct plPSFontInfoStruct *)_pl_get_ps_font_info (plotter);

      for (i=0; ps_font_info[i].ps_name; i++)
	fprintf (stdout, "%s\n", ps_font_info[i].ps_name);
    }

  return 1;
}
