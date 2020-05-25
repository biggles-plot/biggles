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

/* This is the chief include file for GNU libplot/libplotter.  It
   supplements the include files ../include/sys-defines.h,
   ../include/plot.h and ../include/plotter.h.  plot.h is libplot-specific,
   but plotter.h is included both in libplot and libplotter.  plotter.h
   defines what a Plotter object is (a struct for libplot, but a class for
   libplotter). */

/* This file contains many #defines and declarations of data structures.
   More importantly, it contains declarations of all the Plotter methods.
   They are declared differently, depending on whether we are compiling
   libplot (signalled if NOT_LIBPLOTTER is #defined) or libplotter.

   In libplot, the plotter operations are implemented as global functions
   that are members of the Plotter struct.  They are set up differently for
   the different types of Plotter; for example, the `openpl' slot in the
   struct contains the method _pl_g_openpl for generic [i.e. base]
   Plotters, the method _pl_m_openpl for MetaPlotters, etc.  The files
   ?_defplot.c contain the initializations that are used for the different
   types of Plotter.  In this file, if NOT_LIBPLOTTER is defined then each
   of these many methods is declared as a global function.

   In libplotter, the different types of Plotter are implemented as
   distinct classes, which are derived from the generic [i.e. base] Plotter
   class.  This file contains a great many #defines that are appropriate to
   that situation.  For example, _pl_m_openpl is #defined to be
   MetaPlotter::openpl if NOT_LIBPLOTTER is not defined.  The MetaPlotter
   class, like all other Plotter classes, is defined in plotter.h. */


/*************************************************************************/
/* INCLUDE FILES                                         */
/*************************************************************************/

/* 1. OUR OWN INCLUDE FILE */

/* Determine which of libplot/libplotter this is. */
#ifndef LIBPLOTTER
#define NOT_LIBPLOTTER
#endif

/* Always include plotter.h.  (If NOT_LIBPLOTTER is defined, it's a C-style
   header file, declaring the Plotter struct, rather than a declaration
   file for the Plotter class.) */
#include "plotter.h"

/* 2. INCLUDE FILES FOR THE X WINDOW SYSTEM */

#ifndef X_DISPLAY_MISSING
#include <X11/Xatom.h>
#include <X11/Xlib.h>		/* included also in plotter.h */
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#ifdef HAVE_X11_EXTENSIONS_MULTIBUF_H
#include <X11/extensions/multibuf.h>
#endif
#ifdef HAVE_X11_EXTENSIONS_XDBE_H
#include <X11/extensions/Xdbe.h>
#endif
#ifdef USE_MOTIF
#include <Xm/Label.h>
#else
#include <X11/Xaw/Label.h>
#endif
#endif /* not X_DISPLAY_MISSING */


/*************************************************************************/
/* DEFINITIONS RELATED TO OUR FONT DATABASE (g_fontdb.c and g_fontd2.c)  */
/*************************************************************************/

/* The types of font we support.  The final type (`other') is a catchall,
   currently used for any user-specified font, with a name not contained in
   our font database, that can be retrieved from an X server. */
#define PL_F_HERSHEY 0
#define PL_F_POSTSCRIPT 1
#define PL_F_PCL 2
#define PL_F_STICK 3
#define PL_F_OTHER 4

/* PL_NUM_PS_FONTS and PL_NUM_PCL_FONTS should agree with the number of
   fonts of those two types found in g_fontdb.c/g_fontd2.c.  These are also
   defined in plotter.h. */
#define PL_NUM_PS_FONTS 35
#define PL_NUM_PCL_FONTS 45

/* Default fonts, of each type.  Any Plotter has a `default_font_type'
   field, and the appropriate values are copied into the Plotter drawing
   state when the Plotter is first opened (see g_savestate.c).  The
   typeface and font indices index into the tables in
   g_fontdb.c/g_fontd2.c.  PL_DEFAULT_HERSHEY_FONT is also used as a backup
   by some Plotters if no scalable (or anamorphically transformed, etc.)
   font can be retrieved; see e.g. f_retrieve.c and x_retrieve.c. */

#define PL_DEFAULT_HERSHEY_FONT "HersheySerif"
#define PL_DEFAULT_HERSHEY_TYPEFACE_INDEX 0
#define PL_DEFAULT_HERSHEY_FONT_INDEX 1

#define PL_DEFAULT_POSTSCRIPT_FONT "Helvetica"
#define PL_DEFAULT_POSTSCRIPT_TYPEFACE_INDEX 0
#define PL_DEFAULT_POSTSCRIPT_FONT_INDEX 1

#define PL_DEFAULT_PCL_FONT "Univers"
#define PL_DEFAULT_PCL_TYPEFACE_INDEX 0
#define PL_DEFAULT_PCL_FONT_INDEX 1

#define PL_DEFAULT_STICK_FONT "Stick"
#define PL_DEFAULT_STICK_TYPEFACE_INDEX 3
#define PL_DEFAULT_STICK_FONT_INDEX 1

/* HERSHEY FONTS */

/* our information about each of the 22 Hershey vector fonts in g_fontdb.c,
   and the typefaces they belong to */
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
  bool visible;			/* whether font is visible, i.e. not internal*/
};

extern const struct plHersheyFontInfoStruct _pl_g_hershey_font_info[];

/* This numbering should agree with the numbering of Hershey fonts in the
   _pl_g_hershey_font_info[] array in g_fontdb.c. */
#define PL_HERSHEY_SERIF 0
#define PL_HERSHEY_SERIF_ITALIC 1
#define PL_HERSHEY_SERIF_BOLD 2
#define PL_HERSHEY_CYRILLIC 4
#define PL_HERSHEY_HIRAGANA 6	/* hidden font */
#define PL_HERSHEY_KATAKANA 7	/* hidden font */
#define PL_HERSHEY_EUC 8
#define PL_HERSHEY_GOTHIC_GERMAN 16
#define PL_HERSHEY_SERIF_SYMBOL 18

/* accented character information (used in constructing Hershey ISO-Latin-1
   accented characters, see table in g_fontdb.c) */
struct plHersheyAccentedCharInfoStruct
{
  unsigned char composite, character, accent;
};

extern const struct plHersheyAccentedCharInfoStruct _pl_g_hershey_accented_char_info[];

/* types of accent, for a composite character in a Hershey font */
#define ACC0 (16384 + 0)	/* superimpose on character */
#define ACC1 (16384 + 1)	/* elevate by 7 Hershey units */
#define ACC2 (16384 + 2)	/* same, also shift right by 2 units */

/* a flag in a Hershey glyph number indicating a `small Kana' */
#define KS 8192			/* i.e. 0x200 */

/* HERSHEY VECTOR GLYPHS */

/* arrays of Hershey vector glyphs in g_her_glyph.c */
extern const char * const _pl_g_occidental_hershey_glyphs[];
extern const char * const _pl_g_oriental_hershey_glyphs[];

/* position of `undefined character' symbol (a bundle of horizontal lines)
   in the Hershey _pl_g_occidental_hershey_glyphs[] array */
#define UNDE 4023

/* POSTSCRIPT FONTS */

/* our information about each of the 35 standard PS fonts in g_fontdb.c,
   and the typefaces they belong to */
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

extern const struct plPSFontInfoStruct _pl_g_ps_font_info[];

/* PCL FONTS */

/* our information about each of the 45 PCL fonts in g_fontdb.c, and the
   typefaces they belong to.  (The `substitute_ps_name' field is present
   only to support the Tidbits-is-Wingdings botch.) */
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

extern const struct plPCLFontInfoStruct _pl_g_pcl_font_info[];

/* STICK FONTS */

/* our information about each of the Stick fonts (i.e., vector fonts
   resident in HP's devices) listed in g_fontdb.c, and the typefaces they
   belong to */
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

extern const struct plStickFontInfoStruct _pl_g_stick_font_info[];

/* Device-resident kerning data (`spacing table' in HP documentation),
   indexed by `right edge character class' and `left edge character class',
   i.e., `row class' and `column class'.  There are three such spacing
   tables, shared among old-style HP character sets of size 128, and hence
   among our Stick fonts.  See the article by L. W. Hennessee et al. in the
   Nov. 1981 issue of the Hewlett-Packard Journal. */
struct plStickCharSpacingTableStruct
{
  int rows, cols;
  const short *kerns;
};

extern const struct plStickCharSpacingTableStruct _pl_g_stick_spacing_tables[];

/* Kerning tables for 128-character halves of our Stick fonts.  A kerning
   table for the lower or upper half of one of our 256-character fonts
   specifies a spacing table (see above), and maps each character in the
   half-font to the appropriate row and column class. */
struct plStickFontSpacingTableStruct
{
  int spacing_table;
  char row[128], col[128];	/* we use char's as very short int's */
};

extern const struct plStickFontSpacingTableStruct _pl_g_stick_kerning_tables[];

/* TYPEFACES */

/* typeface information, applicable to all four sorts of font in our font
   database (Hershey, PS, PCL, Stick) */

#define PL_MAX_FONTS_PER_TYPEFACE 10

struct plTypefaceInfoStruct
{
  int numfonts;
  int fonts[PL_MAX_FONTS_PER_TYPEFACE];
};

extern const struct plTypefaceInfoStruct _pl_g_hershey_typeface_info[];
extern const struct plTypefaceInfoStruct _pl_g_ps_typeface_info[];
extern const struct plTypefaceInfoStruct _pl_g_pcl_typeface_info[];
extern const struct plTypefaceInfoStruct _pl_g_stick_typeface_info[];


/***********************************************************************/
/* GENERAL DEFINITIONS, TYPEDEFS, & EXTERNAL VARIABLES                 */
/***********************************************************************/

/* miscellaneous data types */

typedef plPoint plVector;
typedef plIntPoint plIntVector;

/* Initializations for default values of Plotter data members, performed
   when space() is first called.  Latter doesn't apply to Plotters whose
   device models have type DISP_DEVICE_COORS_INTEGER_LIBXMI; the default
   for such Plotters is to use zero-width (i.e. Bresenham) lines.  See
   g_space.c. */
#define PL_DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE (1.0/50.0)
#define PL_DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE (1.0/850.0)

/* horizontal justification types for labels (our numbering) */
#define PL_NUM_HORIZ_JUST_TYPES 3
#define PL_JUST_LEFT 0
#define PL_JUST_CENTER 1
#define PL_JUST_RIGHT 2

/* vertical justification types for labels (our numbering) */
#define PL_NUM_VERT_JUST_TYPES 5
#define PL_JUST_TOP 0
#define PL_JUST_HALF 1
#define PL_JUST_BASE 2
#define PL_JUST_BOTTOM 3
#define PL_JUST_CAP 4

/* fill rules (our numbering) */
#define PL_NUM_FILL_RULES 2
#define PL_FILL_ODD_WINDING 0	/* i.e. `even-odd' fill */
#define PL_FILL_NONZERO_WINDING 1

/* canonical line types, or styles (our numbering, used to index the dash
   patterns in g_dash2.c) */
#define PL_NUM_LINE_TYPES 7
#define PL_L_SOLID 0
#define PL_L_DOTTED 1
#define PL_L_DOTDASHED 2
#define PL_L_SHORTDASHED 3
#define PL_L_LONGDASHED 4
#define PL_L_DOTDOTDASHED 5
#define PL_L_DOTDOTDOTDASHED 6

/* maximum length of dash array for our canonical line styles (see line
   style database in g_dash2.c; for example "dotted" corresponds to
   length-2 dash array [ 1 3 ] ) */
#define PL_MAX_DASH_ARRAY_LEN 8

typedef struct
{
  const char *name;		/* user-level name (e.g. "dotted") */
  int type;			/* internal number (e.g. PL_L_DOTTED) */
  int dash_array_len;		/* length of dash array for this style */
  int dash_array[PL_MAX_DASH_ARRAY_LEN]; /* dash array for this style */
} plLineStyle;

extern const plLineStyle _pl_g_line_styles[PL_NUM_LINE_TYPES];

/* when using a canonical line style, numbers appearing in the dash array,
   specifying dash/gap distances, mean multiples of the line width, except
   the following floor is put on the line width */
#define PL_MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE (1.0/576.0)

/* cap and join types (our internal numbering) */

#define PL_NUM_JOIN_TYPES 4
#define PL_JOIN_MITER 0
#define PL_JOIN_ROUND 1
#define PL_JOIN_BEVEL 2
#define PL_JOIN_TRIANGULAR 3

#define PL_NUM_CAP_TYPES 4
#define PL_CAP_BUTT 0
#define PL_CAP_ROUND 1
#define PL_CAP_PROJECT 2
#define PL_CAP_TRIANGULAR 3

/* A Plotter type is first classified according to its `display device
   model', i.e., according to whether the display device to which the user
   frame is mapped is physical or virtual.

   A `physical' display device is one for which the viewport is located on
   a page of known type and size (e.g. "letter", "a4").  I.e. the Plotter
   with a physical display device is one for which the PAGESIZE parameter
   is meaningful.  A Plotter with a `virtual' display device is one for
   which it normally is not: the viewport size that the Plotter uses may be
   fixed (as is the case for a CGM Plotter), or set in a Plotter-dependent
   way (e.g. via the BITMAPSIZE parameter).  */

enum { DISP_MODEL_PHYSICAL, DISP_MODEL_VIRTUAL };

/* Any Plotter is also classified according to the coordinate type it uses
   when writing output (i.e. when writing to its display device, if it has
   one).  A Plotter may use real coordinates (e.g., a generic, Metafile, PS
   or AI Plotter).  A Plotter may also use integer coordinates.  There are
   two subtypes of the latter: one in which a bitmap is produced using
   libxmi or compatible scan-conversion routines (e.g., Bitmap, PNM, PNG,
   GIF, X, X Drawable Plotters), and one in which graphics with integer
   coordinates are drawn by other means (e.g., Fig, HPGL, PCL, ReGIS, and
   Tektronix Plotters).  The only significant distinction is that in vector
   graphics drawn with libxmi, zero-width lines are visible: by convention,
   zero-width lines are interpreted as Bresenham lines. */

enum { DISP_DEVICE_COORS_REAL, DISP_DEVICE_COORS_INTEGER_LIBXMI, DISP_DEVICE_COORS_INTEGER_NON_LIBXMI };

/* The user->device coordinate transformation */

/* X, Y Device: transform user coordinates to device coordinates */
#define XD(x,y) XD_INTERNAL((x),(y),_plotter->drawstate->transform.m)
#define YD(x,y) YD_INTERNAL((x),(y),_plotter->drawstate->transform.m)

#ifdef __GNUC__
#define XD_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[4] + _x * _m[0] + _y * _m[2]; _retval; })
#define YD_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[5] + _x * _m[1] + _y * _m[3]; _retval; })
#else
#define XD_INTERNAL(x,y,m) ((m)[4] + (x) * (m)[0] + (y) * (m)[2])
#define YD_INTERNAL(x,y,m) ((m)[5] + (x) * (m)[1] + (y) * (m)[3])
#endif

/* X,Y Device Vector: transform user vector to device vector */
#define XDV(x,y) XDV_INTERNAL((x),(y),_plotter->drawstate->transform.m)
#define YDV(x,y) YDV_INTERNAL((x),(y),_plotter->drawstate->transform.m)

#ifdef __GNUC__
#define XDV_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[0] * _x + _m[2] * _y; _retval; })
#define YDV_INTERNAL(x,y,m) ({double _x = (x), _y = (y), *_m = (m); double _retval = _m[1] * _x + _m[3] * _y; _retval; })
#else
#define XDV_INTERNAL(x,y,m) ((m)[0] * (x) + (m)[2] * (y))
#define YDV_INTERNAL(x,y,m) ((m)[1] * (x) + (m)[3] * (y))
#endif

/* X, Y User Vector: transform device vector back to user vector 
   (used by X11 driver only) */
#ifdef __GNUC__
#define XUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; double _retval = (m[3] * _x - m[2] * _y) / (m[0] * m[3] - m[1] * m[2]); _retval; })
#define YUV(x,y) ({double _x = (x), _y = (y), *m = _plotter->drawstate->transform.m; double _retval = (- m[1] * _x + m[0] * _y) / (m[0] * m[3] - m[1] * m[2]); _retval; })
#else
#define XUV(x,y) ((_plotter->drawstate->transform.m[3] * (x) - _plotter->drawstate->transform.m[2] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#define YUV(x,y) ((- _plotter->drawstate->transform.m[1] * (x) + _plotter->drawstate->transform.m[0] * (y)) / (_plotter->drawstate->transform.m[0] * _plotter->drawstate->transform.m[3] - _plotter->drawstate->transform.m[1] * _plotter->drawstate->transform.m[2]))
#endif


/*************************************************************************/
/* MISC. DEFS on POLYLINES and PATHS(relevant to all or most display devices)*/
/*************************************************************************/

/* Default value for libplot's miter limit (see comments in g_miter.c).
   This is the same as the value used by X11: it chops off all mitered line
   joins if the join angle is less than 11 degrees. */
#define PL_DEFAULT_MITER_LIMIT 10.4334305246

/* Default length an unfilled path (stored in the path buffer's segment
   list) is allowed to grow to, before it is flushed out by an automatic
   invocation of endpath().  (We don't flush filled paths, since they need
   to be preserved as discrete objects if filling is to be performed
   properly). */
#define PL_MAX_UNFILLED_PATH_LENGTH 500
#define PL_MAX_UNFILLED_PATH_LENGTH_STRING "500"


/************************************************************************/
/* DEFINITIONS & EXTERNALS SPECIFIC TO INDIVIDUAL DEVICE DRIVERS */
/************************************************************************/

/************************************************************************/
/* Metafile device driver */
/************************************************************************/

/* string with which to begin each metafile, must begin with '#' to permit
   parsing by our plot filters */
#define PL_PLOT_MAGIC "#PLOT"

/* bit fields for specifying, via a mask, which libplot attributes should
   be updated (see m_attribs.c) */
#define PL_ATTR_POSITION (1<<0)
#define PL_ATTR_TRANSFORMATION_MATRIX (1<<1)
#define PL_ATTR_PEN_COLOR (1<<2)
#define PL_ATTR_FILL_COLOR (1<<3)
#define PL_ATTR_BG_COLOR (1<<4)
#define PL_ATTR_PEN_TYPE (1<<5)
#define PL_ATTR_FILL_TYPE (1<<6)
#define PL_ATTR_LINE_STYLE (1<<7) /* line mode and/or dash array */
#define PL_ATTR_LINE_WIDTH (1<<8)
#define PL_ATTR_FILL_RULE (1<<9)
#define PL_ATTR_JOIN_STYLE (1<<10)
#define PL_ATTR_CAP_STYLE (1<<11)
#define PL_ATTR_MITER_LIMIT (1<<12)
#define PL_ATTR_ORIENTATION (1<<13)
#define PL_ATTR_FONT_NAME (1<<14)
#define PL_ATTR_FONT_SIZE (1<<15)
#define PL_ATTR_TEXT_ANGLE (1<<16)

/************************************************************************/
/* ReGIS (remote graphics instruction set) device driver */
/************************************************************************/

/* For a ReGIS device we clip to the rectangular physical display
   [0..767]x[0..479], not to the square libplot graphics display
   [144..623]x[0..479], which is specified in r_defplot.c.  Note: ReGIS
   uses a flipped-y convention. */

#define REGIS_DEVICE_X_MIN 0
#define REGIS_DEVICE_X_MAX 767
#define REGIS_DEVICE_Y_MIN 0
#define REGIS_DEVICE_Y_MAX 479

#define REGIS_CLIP_FUZZ 0.0000001
#define REGIS_DEVICE_X_MIN_CLIP (REGIS_DEVICE_X_MIN - 0.5 + REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_X_MAX_CLIP (REGIS_DEVICE_X_MAX + 0.5 - REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_Y_MIN_CLIP (REGIS_DEVICE_Y_MIN - 0.5 + REGIS_CLIP_FUZZ)
#define REGIS_DEVICE_Y_MAX_CLIP (REGIS_DEVICE_Y_MAX + 0.5 - REGIS_CLIP_FUZZ)


/************************************************************************/
/* Tektronix device driver */
/************************************************************************/

/* For a Tektronix device we clip to the rectangular physical display
   [0..4095]x[0..3119], not to the square libplot graphics display
   [488..3607]x[0..3119], which is specified in t_defplot.c.  Note:
   Tektronix displays do not use a flipped-y convention. */

#define TEK_DEVICE_X_MIN 0
#define TEK_DEVICE_X_MAX 4095
#define TEK_DEVICE_Y_MIN 0
#define TEK_DEVICE_Y_MAX 3119

#define TEK_CLIP_FUZZ 0.0000001
#define TEK_DEVICE_X_MIN_CLIP (TEK_DEVICE_X_MIN - 0.5 + TEK_CLIP_FUZZ)
#define TEK_DEVICE_X_MAX_CLIP (TEK_DEVICE_X_MAX + 0.5 - TEK_CLIP_FUZZ)
#define TEK_DEVICE_Y_MIN_CLIP (TEK_DEVICE_Y_MIN - 0.5 + TEK_CLIP_FUZZ)
#define TEK_DEVICE_Y_MAX_CLIP (TEK_DEVICE_Y_MAX + 0.5 - TEK_CLIP_FUZZ)

/* Tektronix modes (our private numbering, values are not important but
   order is, see t_tek_md.c) */
#define TEK_MODE_ALPHA 0
#define TEK_MODE_PLOT 1
#define TEK_MODE_POINT 2
#define TEK_MODE_INCREMENTAL 3	/* currently not used */

/* Tektronix display types (generic / Tek emulation in MS-DOS kermit / Tek
   emulation in `xterm -t') */
#define TEK_DPY_GENERIC 0
#define TEK_DPY_KERMIT 1
#define TEK_DPY_XTERM 2

/* colors supported by MS-DOS kermit Tek emulation, see t_color2.c */

#define TEK_NUM_ANSI_SYS_COLORS 16
extern const plColor _pl_t_kermit_stdcolors[TEK_NUM_ANSI_SYS_COLORS];
extern const char * const _pl_t_kermit_fgcolor_escapes[TEK_NUM_ANSI_SYS_COLORS];
extern const char * const _pl_t_kermit_bgcolor_escapes[TEK_NUM_ANSI_SYS_COLORS];
/* must agree with the ordering in t_color2.c */
#define TEK_ANSI_SYS_BLACK   0
#define TEK_ANSI_SYS_GRAY30  8
#define TEK_ANSI_SYS_GRAY55  7
#define TEK_ANSI_SYS_WHITE  15


/************************************************************************/
/* HP-GL device driver */
/************************************************************************/

/* An HPGLPlotter plots using virtual device coordinates: not the native
   device coordinates, but rather scaled coordinates in which the graphics
   display is [0,10000]x[0,10000].  To arrange this, in the initialization
   code in h_defplot.c we move the HP-GL `scaling points' to the lower left
   and upper right corners of our graphics display, and use the HP-GL `SC'
   instruction to set up a scaled set of coordinates. */
#define HPGL_SCALED_DEVICE_LEFT 0
#define HPGL_SCALED_DEVICE_RIGHT 10000
#define HPGL_SCALED_DEVICE_BOTTOM 0
#define HPGL_SCALED_DEVICE_TOP 10000

#define HPGL_UNITS_PER_INCH 1016 /* 1 HP-GL unit = 1/40 mm */

/* HP-GL line attribute types (HP-GL numbering; see h_attribs.c) */
#define HPGL_L_SOLID (-100)	/* no numeric parameter at all */
#define HPGL_L_DOTTED 1
#define HPGL_L_DOTDASHED 5
#define HPGL_L_SHORTDASHED 2
#define HPGL_L_LONGDASHED 3
#define HPGL_L_DOTDOTDASHED 6
#define HPGL_L_DOTDOTDOTDASHED (-10) /* pseudo */

#define HPGL_JOIN_MITER 1	/* miter length is clamped by miter limit */
#define HPGL_JOIN_MITER_BEVEL 2	/* miter or bevel, based on miter limit */
#define HPGL_JOIN_TRIANGULAR 3
#define HPGL_JOIN_ROUND 4
#define HPGL_JOIN_BEVEL 5

#define HPGL_CAP_BUTT 1
#define HPGL_CAP_PROJECT 2
#define HPGL_CAP_TRIANGULAR 3
#define HPGL_CAP_ROUND 4

/* HP-GL/2 pen types, i.e. screening types: the type of area fill to be
   applied to wide pen strokes.  (HP-GL/2 numbering, as used in the `SV'
   [screened vectors] instruction.  Screened vectors are supported only on
   HP-GL/2 devices that are not pen plotters.) */
#define HPGL_PEN_SOLID 0
#define HPGL_PEN_SHADED 1
#define HPGL_PEN_PREDEFINED_CROSSHATCH 21 /* imported from PCL or RTL */

/* HP-GL and HP-GL/2 fill types.  (Their numbering, as used in the `FT'
   instruction.) */
#define HPGL_FILL_SOLID_BI 1
#define HPGL_FILL_SOLID_UNI 2
#define HPGL_FILL_PARALLEL_LINES 3
#define HPGL_FILL_CROSSHATCHED_LINES 4
#define HPGL_FILL_SHADED 10
#define HPGL_FILL_PREDEFINED_CROSSHATCH 21 /* imported from PCL or RTL */

/* HP-GL/2 character rendering types, as used in the `CF' [character fill
   mode] instruction.  By default the current pen is used for edging, and
   for filling too, if filling is requested.  Some fill types [set with the
   `FT' command'] include color information, in which case the current pen
   is not used for filling.  Types 0,1,2 allow specification of an edge pen
   which may be different from the present pen.  (At least for type 0,
   specifying edge pen 0 seems to turn off edging.  For types 1 and 3, edge
   pen 0 may request white edging [on color devices].)  Note that there are
   three kinds of font: bitmap, stick, and outline, which are treated
   slightly differently: bitmap and stick chars are filled, not edged, so
   edging doesn't apply to them. */

/* Default rendering is type 0, with edge pen 0, which as just mentioned
   turns off edging. */

#define HPGL_CHAR_FILL_SOLID_AND_MAYBE_EDGE 0
#define HPGL_CHAR_EDGE 1	/* bitmap, stick chars are filled instead */
#define HPGL_CHAR_FILL 2	/* i.e. with current fill type */
#define HPGL_CHAR_FILL_AND_EDGE 3 /* i.e. with current fill type */

/* HP-GL object types (our numbering), which we use when passing an
   argument to an HPGLPlotter's internal _h_set_pen_color() method, letting
   it know the type of object that will be drawn.  Passing the libplot pen
   color down to the HP-GL/2 level, prior to drawing a label rather than a
   path, may involve changing the character rendition type. */
#define HPGL_OBJECT_PATH 0
#define HPGL_OBJECT_LABEL 1

/* Nominal pen width in native HP-GL units (so this is 0.3mm).  Used by our
   HP7550B-style cross-hatching algorithm, which we employ when emulating
   shading (if HPGL_VERSION is 1 or 1.5, i.e. if there's no true shading).  */
#define HPGL_NOMINAL_PEN_WIDTH 12 

/* default values for HPGL_PENS environment variable, for HP-GL[/2]; this
   lists available pens and their positions in carousel */
#define HPGL_DEFAULT_PEN_STRING "1=black"
#define HPGL2_DEFAULT_PEN_STRING "1=black:2=red:3=green:4=yellow:5=blue:6=magenta:7=cyan"

/* PCL 5 font information: symbol set, i.e. encoding */
#define PCL_ISO_8859_1 14
#define PCL_ROMAN_8 277

/* PCL typeface number for default HP-GL/2 typeface */
#define PCL_STICK_TYPEFACE 48

/* Old (pre-HP-GL/2) 7-bit HP-GL character sets */
#define HPGL_CHARSET_ASCII 0
#define HPGL_CHARSET_ROMAN_EXTENSIONS 7

/* The nominal HP-GL/2 fontsize we use for drawing a label (for fixed-width
   and proportional fonts, respectively).  We retrieve fonts in the size
   specified by whichever of the two following parameters is relevant, and
   then rescale it as needed before drawing the label. */
#define HPGL2_NOMINAL_CHARS_PER_INCH 8.0
#define HPGL2_NOMINAL_POINT_SIZE 18

/* Spacing characteristic of the PCL and Stick fonts, in HP-GL/2 */
#define HPGL2_FIXED_SPACING 0
#define HPGL2_PROPORTIONAL_SPACING 1


/************************************************************************/
/* xfig device driver */
/************************************************************************/

/* Standard Fig unit size in v. 3.0+ */
#define FIG_UNITS_PER_INCH 1200

/* device units <-> printer's points; number of points per inch == 72 */
#define FIG_UNITS_TO_POINTS(size) ((size)*72.0/FIG_UNITS_PER_INCH)
#define POINTS_TO_FIG_UNITS(size) ((size)*((double)FIG_UNITS_PER_INCH)/72.0)

/* xfig specifies line widths in `Fig display units' rather than `Fig units'
   (there are 80 of the former per inch). */
#define FIG_UNITS_TO_FIG_DISPLAY_UNITS(width) ((width)*80.0/FIG_UNITS_PER_INCH)

/* For historical reasons, xfig scales the fonts down by a factor
   FONT_SCALING, i.e., (80.0)/(72.0).  So we have to premultiply font sizes
   by the same factor.  The current release of xfig unfortunately can't
   handle font sizes that aren't integers, so it rounds them.  Ah well. */
#define FIG_FONT_SCALING ((80.0)/(72.0))

/* Fig supported line styles.  DOTTED and DASHED line styles are specified
   by (respectively) the length of the gap between successive dots, and the
   length of each dash (equal to the length of the gap between successive
   dashes, except in the DASHDOTTED case).  */
#define FIG_L_DEFAULT (-1)
#define FIG_L_SOLID 0
#define FIG_L_DASHED 1
#define FIG_L_DOTTED 2
#define FIG_L_DASHDOTTED 3
#define FIG_L_DASHDOUBLEDOTTED 4
#define FIG_L_DASHTRIPLEDOTTED 5

/* Fig's line styles, indexed into by internal line type number
   (PL_L_SOLID/PL_L_DOTTED/
   PL_L_DOTDASHED/PL_L_SHORTDASHED/PL_L_LONGDASHED. */
extern const int _pl_f_fig_line_style[PL_NUM_LINE_TYPES];

#define FIG_JOIN_MITER 0
#define FIG_JOIN_ROUND 1
#define FIG_JOIN_BEVEL 2

#define FIG_CAP_BUTT 0
#define FIG_CAP_ROUND 1
#define FIG_CAP_PROJECT 2

/* Fig join and cap styles, see f_path.c, indexed by our internal join and
   cap type numbers (miter/rd./bevel/triangular and
   butt/rd./project/triangular) */
extern const int _pl_f_fig_join_style[PL_NUM_JOIN_TYPES];
extern const int _pl_f_fig_cap_style[PL_NUM_CAP_TYPES];

/* these constants for Fig colors are hardcoded in xfig */

#define FIG_STD_COLOR_MIN 0	/* see f_color2.c for colors 0,...,31 */
#define FIG_C_BLACK 0		/* i.e. #0 in table in f_color2.c */
#define FIG_C_WHITE 7		/* i.e. #7 in table */
#define FIG_NUM_STD_COLORS 32
#define FIG_USER_COLOR_MIN 32
extern const plColor _pl_f_fig_stdcolors[FIG_NUM_STD_COLORS];

/* xfig's depth attribute ranges from 0 to FIG_MAXDEPTH. */
#define FIG_MAXDEPTH 999

/* depth of the first object we'll draw (we make it a bit less than
   FIG_MAXDEPTH, since the user may wish to edit the drawing with xfig to
   include deeper, i.e. obscured objects) */
#define FIG_INITIAL_DEPTH 989


/************************************************************************/
/* CGM device driver */
/************************************************************************/

/* CGM output file `profiles', which are increasingly general (our
   numbering).  The most restrictive is the WebCGM profile.  We increment
   the profile number appropriately whenever anything noncompliant is
   seen. */

#define CGM_PROFILE_WEB 0
#define CGM_PROFILE_MODEL 1
#define CGM_PROFILE_NONE 2

/* Possible encodings of the CGM output file (our numbering).  Only the
   first (binary) is allowed by the WebCGM profile. */

#define CGM_ENCODING_BINARY 0	/* default */
#define CGM_ENCODING_CHARACTER 1 /* not supported by libplot */
#define CGM_ENCODING_CLEAR_TEXT 2

/* In the binary encoding, how many bytes we use to represent an integer
   parameter of a CGM command.  This determines the range over which
   integers (e.g., point coordinates) can vary, and hence the granularity
   of our quantization to integer coordinates in the output file.  This
   value should not be greater than the number of bytes used in the system
   representation for an integer (see comments in c_emit.c).

   Don't change this value unless you know what you're doing.  Some old
   [buggy] CGM interpreters can't handle any value except 2, or possibly 4.
   The old RALCGM viewer/translator partially breaks on a value of 3 or 4.
   (It can display binary-encoded CGM files that use the value of `3', but
   when it translates such files to the clear text encoding, it produces a
   bogus [zero] value for the metric scaling factor.) */

#define CGM_BINARY_BYTES_PER_INTEGER 2

/* In the binary encoding, how many bytes we use to represent an RGB color
   component.  In this is 1, then 24-bit color will be used; if 2, then
   48-bit color will be used.  Valid values are 1, 2, 3, 4, but our
   code in c_color.c assumes that the value is 1 or 2. */

#define CGM_BINARY_BYTES_PER_COLOR_COMPONENT 2

/* In the binary encoding, how many bytes we use to represent a string
   parameter of a CGM command.  (See c_emit.c.)  In the binary encoding, a
   string <= 254 bytes in length is represented literally, preceded by a
   1-byte count.  Any string > 254 bytes in length is partitioned: after
   the initial byte, there are one or more partitions.  Each partition
   contains an initial byte, and then up to CGM_STRING_PARTITION_SIZE bytes
   of the string.  According to the CGM spec, CGM_STRING_PARTITION_SIZE
   could be as large as 32767.  However, since we don't wish to overrun our
   output buffer, we keep it fairly small (see comment in g_outbuf.c). */

#define CGM_STRING_PARTITION_SIZE 2000
#define CGM_BINARY_BYTES_PER_STRING(len) \
((len) <= 254 ? (1 + (len)) : \
(1 + (len) + 2 * (1 + ((len) - 1) / CGM_STRING_PARTITION_SIZE)))

/* CGM's element classes (CGM numbering) */
#define CGM_DELIMITER_ELEMENT 0
#define CGM_METAFILE_DESCRIPTOR_ELEMENT 1
#define CGM_PICTURE_DESCRIPTOR_ELEMENT 2
#define CGM_CONTROL_ELEMENT 3
#define CGM_GRAPHICAL_PRIMITIVE_ELEMENT 4
#define CGM_ATTRIBUTE_ELEMENT 5
#define CGM_ESCAPE_ELEMENT 6	/* not used by libplot */
#define CGM_EXTERNAL_ELEMENT 7	/* not used by libplot */
#define CGM_SEGMENT_ELEMENT 8	/* not used by libplot */

/* tags for CGM data types within a CGM SDR (structured data record) */
#define CGM_SDR_DATATYPE_SDR 1
#define CGM_SDR_DATATYPE_COLOR_INDEX 2
#define CGM_SDR_DATATYPE_COLOR_DIRECT 3
#define CGM_SDR_DATATYPE_ENUM 5
#define CGM_SDR_DATATYPE_INTEGER 6
#define CGM_SDR_DATATYPE_INDEX 11
#define CGM_SDR_DATATYPE_REAL 12
#define CGM_SDR_DATATYPE_STRING 13
#define CGM_SDR_DATATYPE_STRING_FIXED 14
#define CGM_SDR_DATATYPE_VDC 16
#define CGM_SDR_DATATYPE_UNSIGNED_INTEGER_8BIT 18
#define CGM_SDR_DATATYPE_COLOR_LIST 21

/* CGM font properties, from the CGM spec.  (Value of each of these font
   props is an SDR, comprising a single datum of `index' type, except for
   the FAMILY prop, for which the datum is a string, and the DESIGN_GROUP
   prop, for which the SDR comprises three 8-bit unsigned integers.) */
#define CGM_FONT_PROP_INDEX 1
#define CGM_FONT_PROP_FAMILY 4
#define CGM_FONT_PROP_POSTURE 5
#define CGM_FONT_PROP_WEIGHT 6
#define CGM_FONT_PROP_WIDTH 7
#define CGM_FONT_PROP_DESIGN_GROUP 13
#define CGM_FONT_PROP_STRUCTURE 14

/* CGM line/edge types (CGM numbering; for custom dash arrays defined by
   linedash(), negative values are used) */
#define CGM_L_SOLID 1
#define CGM_L_DASHED 2
#define CGM_L_DOTTED 3
#define CGM_L_DOTDASHED 4
#define CGM_L_DOTDOTDASHED 5

/* CGM interior styles (CGM numbering) */
#define CGM_INT_STYLE_HOLLOW 0
#define CGM_INT_STYLE_SOLID 1
#define CGM_INT_STYLE_PATTERN 2
#define CGM_INT_STYLE_HATCH 3
#define CGM_INT_STYLE_EMPTY 4
#define CGM_INT_STYLE_GEOMETRIC_PATTERN 5
#define CGM_INT_STYLE_INTERPOLATED 6

/* CGM line/edge join styles (CGM numbering) */
#define CGM_JOIN_UNSPEC 1
#define CGM_JOIN_MITER 2
#define CGM_JOIN_ROUND 3
#define CGM_JOIN_BEVEL 4

/* CGM line/edge cap styles (CGM numbering) */
#define CGM_CAP_UNSPEC 1
#define CGM_CAP_BUTT 2
#define CGM_CAP_ROUND 3
#define CGM_CAP_PROJECTING 4
#define CGM_CAP_TRIANGULAR 5

/* CGM line/edge dash cap styles (CGM numbering) */
#define CGM_DASH_CAP_UNSPEC 1
#define CGM_DASH_CAP_BUTT 2
#define CGM_DASH_CAP_MATCH 3

/* CGM marker types (CGM numbering) */
#define CGM_M_DOT 1
#define CGM_M_PLUS 2
#define CGM_M_ASTERISK 3
#define CGM_M_CIRCLE 4
#define CGM_M_CROSS 5

/* CGM object types (our numbering) */
#define CGM_OBJECT_OPEN 0
#define CGM_OBJECT_CLOSED 1
#define CGM_OBJECT_MARKER 2
#define CGM_OBJECT_TEXT 3
#define CGM_OBJECT_OTHER 4

/* CGM horizontal justification types for labels (CGM numbering) */
#define CGM_ALIGN_NORMAL_HORIZONTAL 0
#define CGM_ALIGN_LEFT 1
#define CGM_ALIGN_CENTER 2
#define CGM_ALIGN_RIGHT 3

/* CGM vertical justification types for labels (CGM numbering) */
#define CGM_ALIGN_NORMAL_VERTICAL 0
#define CGM_ALIGN_TOP 1
#define CGM_ALIGN_CAP 2
#define CGM_ALIGN_HALF 3
#define CGM_ALIGN_BASE 4
#define CGM_ALIGN_BOTTOM 5

/* CGM `restricted text' types (CGM numbering) */
#define CGM_RESTRICTED_TEXT_TYPE_BASIC 1
#define CGM_RESTRICTED_TEXT_TYPE_BOXED_CAP 2

/* mappings from internal PS font number to CGM font id, as used in output
   file; see g_fontdb.c */
extern const int _pl_g_ps_font_to_cgm_font_id[PL_NUM_PS_FONTS];
extern const int _pl_g_cgm_font_id_to_ps_font[PL_NUM_PS_FONTS];

/* structure used to store the CGM properties for a font; see g_fontdb.c */
typedef struct
{
  const char *family;
  const char *extra_style;
  const char *style;
  int posture;			/* 1=upright, 2=oblique, 4=italic */
  int weight;			/* 4=semilight, 5=normal, 6=semibold, 7=bold */
  int proportionate_width;	/* 3=condensed, 5=medium */
  int design_group[3];
  int structure;		/* 1=filled, 2=outline */
} plCGMFontProperties;

extern const plCGMFontProperties _pl_g_cgm_font_properties[PL_NUM_PS_FONTS];

/* structure used to store a user-defined line type; see g_attribs.c */
typedef struct plCGMCustomLineTypeStruct
{
  int *dashes;
  int dash_array_len;
  struct plCGMCustomLineTypeStruct *next;
} plCGMCustomLineType;

/* maximum number of line types a user can define, and the maximum dash
   array length a user can specify per line type, without violating the
   WebCGM or Model CGM profiles */
#define CGM_MAX_CUSTOM_LINE_TYPES 16
#define CGM_PL_MAX_DASH_ARRAY_LENGTH 8


/************************************************************************/
/* Postscript/idraw device driver */
/************************************************************************/

/* minimum desired resolution in device frame (i.e. in printer's points) */
#define PS_MIN_RESOLUTION 0.05

/* em size (in printer's points) for a font in which a `point' could appear
   as a symbol */
#define PS_SIZE_OF_POINT 0.5

/* PS line join and line cap styles */

#define PS_LINE_JOIN_MITER 0
#define PS_LINE_JOIN_ROUND 1
#define PS_LINE_JOIN_BEVEL 2

#define PS_LINE_CAP_BUTT 0
#define PS_LINE_CAP_ROUND 1
#define PS_LINE_CAP_PROJECT 2

/* information on colors known to idraw, see p_color2.c */

#define PS_NUM_IDRAW_STD_COLORS 12
extern const plColor _pl_p_idraw_stdcolors[PS_NUM_IDRAW_STD_COLORS];
extern const char * const _pl_p_idraw_stdcolornames[PS_NUM_IDRAW_STD_COLORS];

/* information on shadings known to idraw, see p_color2.c */

#define PS_NUM_IDRAW_STD_SHADINGS 5
extern const double _pl_p_idraw_stdshadings[PS_NUM_IDRAW_STD_SHADINGS];


/************************************************************************/
/* Adobe Illustrator device driver */
/************************************************************************/

/* types of Illustrator file format that an Illustrator Plotter can emit */
#define AI_VERSION_3 0
#define AI_VERSION_5 1

/* em size (in printer's points) for a font in which a `point' could appear
   as a symbol */
#define AI_SIZE_OF_POINT 0.5

/* AI line join and line cap styles (same as for PS) */

#define AI_LINE_JOIN_MITER 0
#define AI_LINE_JOIN_ROUND 1
#define AI_LINE_JOIN_BEVEL 2

#define AI_LINE_CAP_BUTT 0
#define AI_LINE_CAP_ROUND 1
#define AI_LINE_CAP_PROJECT 2

/* AI fill rule types (in AI version 5 and later) */
#define AI_FILL_NONZERO_WINDING 0
#define AI_FILL_ODD_WINDING 1


/************************************************************************/
/* XDrawable and X device drivers */
/************************************************************************/

#ifndef X_DISPLAY_MISSING 

/* X11 colormap types (XDrawable Plotters use only the first of these) */
#define X_CMAP_ORIG 0
#define X_CMAP_NEW 1
#define X_CMAP_BAD 2	     /* colormap full, can't allocate new colors */

/* sixteen-bit restriction on X11 protocol parameters */
#define X_OOB_UNSIGNED(x) ((x) > (int)0xffff)
#define X_OOB_INT(x) ((x) > (int)0x7fff || (x) < (int)(-0x8000))

/* double buffering types, used in XDrawablePlotter `x_double_buffering'
   data member */
#define X_DBL_BUF_NONE 0
#define X_DBL_BUF_BY_HAND 1
#define X_DBL_BUF_MBX 2		/* X11 MBX extension */
#define X_DBL_BUF_DBE 3		/* X11 DBE extension */

/* numbering of our X GC's (graphics contexts); this is the numbering we
   use when passing an argument to _x_set_attributes() indicating which GC
   should be altered */
#define X_GC_FOR_DRAWING 0
#define X_GC_FOR_FILLING 1
#define X_GC_FOR_ERASING 2

#endif /* not X_DISPLAY_MISSING */


/***********************************************************************/
/* DRAWING STATE                                                       */
/***********************************************************************/

/* Default drawing state, defined in g_defstate.c.  This is used for
   initialization of the first state on the drawing state stack that every
   Plotter maintains; see g_savestate.c. */
extern const plDrawState _default_drawstate;

/*************************************************************************/
/* PLOTTER OBJECTS (structs for libplot, class instances for libplotter) */
/*************************************************************************/

/* "_plotters" is a sparse array containing pointers to all Plotter
   instances, of size "_plotters_len".  In libplot, they're globals, but in
   in libplotter, they're static data members of the base Plotter class.
   In both libraries, they're defined in g_defplot.c.

   Similarly, "_xplotters" is a sparse array containing pointers to all
   XPlotters instances, of size "_xplotters_len".  In libplot, they're
   globals, but in libplotter, they're static data members of the XPlotter
   class.  In both libraries, they're defined in y_defplot.c. */

#ifndef LIBPLOTTER
extern Plotter **_plotters;
extern int _plotters_len;
#define XPlotter Plotter	/* crock, needed by code in y_defplot.c */
extern XPlotter **_xplotters;
extern int _xplotters_len;
#else
#define _plotters Plotter::_plotters
#define _plotters_len Plotter::_plotters_len
#define _xplotters XPlotter::_xplotters
#define _xplotters_len XPlotter::_xplotters_len
#endif

#ifndef LIBPLOTTER
/* In libplot, these are the initializations of the function-pointer parts
   of the different types of Plotter.  They are copied to the Plotter at
   creation time (in apinewc.c, which is libplot-specific). */
extern const Plotter _pl_g_default_plotter, _pl_b_default_plotter, _pl_m_default_plotter, _pl_r_default_plotter, _pl_t_default_plotter, _pl_h_default_plotter, _pl_q_default_plotter, _pl_f_default_plotter, _pl_c_default_plotter, _pl_p_default_plotter, _pl_a_default_plotter, _pl_s_default_plotter, _pl_i_default_plotter, _pl_n_default_plotter, _pl_z_default_plotter, _pl_x_default_plotter, _pl_y_default_plotter;

/* Similarly, in libplot this is the initialization of the function-pointer
   part of any PlotterParams object. */
extern const PlotterParams _default_plotter_params;
#endif /* not LIBPLOTTER */

/* The array used for storing the names of recognized Plotter parameters,
   and their default values.  (See g_params2.c.) */
struct plParamRecord
{
  const char *parameter;	/* parameter name */
  void * default_value;	     /* default value (applies if string-valued) */
  bool is_string;		/* whether or not value must be a string */
};

extern const struct plParamRecord _known_params[NUM_PLOTTER_PARAMETERS];

/* A pointer to a distinguished (global) PlotterParams object, used by the
   old C and C++ bindings.  The function parampl() sets parameters in this
   object.  (This is one reason why the old bindings are non-thread-safe.
   The new bindings allow the programmer to instantiate and use more than a
   single PlotterParams object, so they are thread-safe.)  In libplotter,
   this pointer is declared as a static member of the Plotter class. */
#ifndef LIBPLOTTER
extern PlotterParams *_old_api_global_plotter_params;
#else
#define _old_api_global_plotter_params Plotter::_old_api_global_plotter_params
#endif


/**************************************************************************/
/* PROTOTYPES ETC. for libplot and libplotter */
/**************************************************************************/

/* Miscellaneous internal functions that aren't Plotter class methods, so
   they're declared the same for both libplot and libplotter. */

/* wrappers for malloc and friends */
extern void * _pl_xcalloc (size_t nmemb, size_t size);
extern void * _pl_xmalloc (size_t size);
extern void * _pl_xrealloc (void * p, size_t size);

/* misc. utility functions, mostly geometry-related */

extern plPoint _truecenter (plPoint p0, plPoint p1, plPoint pc);
extern plVector *_vscale (plVector *v, double newlen);
extern double _angle_of_arc (plPoint p0, plPoint p1, plPoint pc);
extern double _matrix_norm (const double m[6]);
extern double _xatan2 (double y, double x);
extern int _clip_line (double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip);
extern int _codestring_len (const unsigned short *codestring);
extern int _grayscale_approx (int red, int green, int blue);
extern void _matrix_product (const double m[6], const double n[6], double product[6]);
extern void _matrix_inverse (const double m[6], double inverse[6]);
extern void _matrix_sing_vals (const double m[6], double *min_sing_val, double *max_sing_val);
extern void _set_common_mi_attributes (plDrawState *drawstate, void * ptr);
extern void * _get_default_plot_param (const char *parameter); 

/* plPlotterData methods */
/* lowest-level output routines used by Plotters */
extern void _write_byte (const plPlotterData *data, unsigned char c);
extern void _write_bytes (const plPlotterData *data, int n, const unsigned char *c);
extern void _write_string (const plPlotterData *data, const char *s);
/* other plPlotterData methods */
extern bool _compute_ndc_to_device_map (plPlotterData *data);
extern void _set_page_type (plPlotterData *data);
extern void * _get_plot_param (const plPlotterData *data, const char *parameter); 

/* plPath methods (see g_subpaths.c) */
extern plPath * _flatten_path (const plPath *path);
extern plPath * _new_plPath (void);
extern plPath ** _merge_paths (const plPath **paths, int num_paths);
extern void _add_arc (plPath *path, plPoint pc, plPoint p1);
extern void _add_arc_as_bezier3 (plPath *path, plPoint pc, plPoint p1);
extern void _add_arc_as_lines (plPath *path, plPoint pc, plPoint p1);
extern void _add_bezier2 (plPath *path, plPoint pc, plPoint p);
extern void _add_bezier2_as_lines (plPath *path, plPoint pc, plPoint p);
extern void _add_bezier3 (plPath *path, plPoint pc, plPoint pd, plPoint p);
extern void _add_bezier3_as_lines (plPath *path, plPoint pc, plPoint pd, plPoint p);
extern void _add_box (plPath *path, plPoint p0, plPoint p1, bool clockwise);
extern void _add_box_as_lines (plPath *path, plPoint p0, plPoint p1, bool clockwise);
extern void _add_circle (plPath *path, plPoint pc, double radius, bool clockwise);
extern void _add_circle_as_bezier3s (plPath *path, plPoint pc, double radius, bool clockwise);
extern void _add_circle_as_ellarcs (plPath *path, plPoint pc, double radius, bool clockwise);
extern void _add_circle_as_lines (plPath *path, plPoint pc, double radius, bool clockwise);
extern void _add_ellarc (plPath *path, plPoint pc, plPoint p1);
extern void _add_ellarc_as_bezier3 (plPath *path, plPoint pc, plPoint p1);
extern void _add_ellarc_as_lines (plPath *path, plPoint pc, plPoint p1);
extern void _add_ellipse (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise);
extern void _add_ellipse_as_bezier3s (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise);
extern void _add_ellipse_as_ellarcs (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise);
extern void _add_ellipse_as_lines (plPath *path, plPoint pc, double rx, double ry, double angle, bool clockwise);
extern void _add_line (plPath *path, plPoint p);
extern void _add_moveto (plPath *path, plPoint p);
extern void _delete_plPath (plPath *path);
extern void _reset_plPath (plPath *path);

/* plOutbuf methods (see g_outbuf.c) */
extern plOutbuf * _new_outbuf (void);
extern void _bbox_of_outbuf (plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax);
extern void _bbox_of_outbufs (plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax);
extern void _delete_outbuf (plOutbuf *outbuf);
extern void _freeze_outbuf (plOutbuf *outbuf);
extern void _reset_outbuf (plOutbuf *outbuf);
extern void _update_bbox (plOutbuf *bufp, double x, double y);
extern void _update_buffer (plOutbuf *outbuf);
extern void _update_buffer_by_added_bytes (plOutbuf *outbuf, int additional);

/* functions that update a device-frame bounding box for a page, as stored
   in a plOutbuf */
extern void _set_bezier2_bbox (plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double device_line_width, double m[6]);
extern void _set_bezier3_bbox (plOutbuf *bufp, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, double device_line_width, double m[6]);
extern void _set_ellipse_bbox (plOutbuf *bufp, double x, double y, double rx, double ry, double costheta, double sintheta, double linewidth, double m[6]);
extern void _set_line_end_bbox (plOutbuf *bufp, double x, double y, double xother, double yother, double linewidth, int capstyle, double m[6]);
extern void _set_line_join_bbox (plOutbuf *bufp, double xleft, double yleft, double x, double y, double xright, double yright, double linewidth, int joinstyle, double miterlimit, double m[6]);

/* CGMPlotter-related functions, which write a CGM command, or an argument
   of same, alternatively to a plOutbuf or to a string (see c_emit.c) */
extern void _cgm_emit_command_header (plOutbuf *outbuf, int cgm_encoding, int element_class, int id, int data_len, int *byte_count, const char *op_code);
extern void _cgm_emit_color_component (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_enum (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count, const char *text_string); 
extern void _cgm_emit_index (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_integer (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int y, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_points (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const int *x, const int *y, int npoints, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_real_fixed_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_real_floating_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_string (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const char *s, int string_length, bool use_double_quotes, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_unsigned_integer (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_unsigned_integer_8bit (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count);
extern void _cgm_emit_command_terminator (plOutbuf *outbuf, int cgm_encoding, int *byte_count);

/* SVGPlotter-related functions */
extern const char * _libplot_color_to_svg_color (plColor color_48, char charbuf[8]);

/* plColorNameCache methods */
extern bool _string_to_color (const char *name, plColor *color_p, plColorNameCache *color_name_cache);
extern plColorNameCache * _create_color_name_cache (void);
extern void _delete_color_name_cache (plColorNameCache *color_cache);

/* Renaming of the global symbols in the libxmi scan conversion library,
   which we include in libplot/libplotter as a rendering module.  We
   prepend each name with "_pl".  Doing this keeps the user-level namespace
   clean, and allows an application to link with both libplot/libplotter
   and a separate version of libxmi. */

/* libxmi API functions */

#define miClearPaintedSet _pl_miClearPaintedSet
#define miCopyCanvas _pl_miCopyCanvas
#define miCopyGC _pl_miCopyGC
#define miCopyPaintedSetToCanvas _pl_miCopyPaintedSetToCanvas
#define miDeleteCanvas _pl_miDeleteCanvas
#define miDeleteEllipseCache _pl_miDeleteEllipseCache
#define miDeleteGC _pl_miDeleteGC
#define miDeletePaintedSet _pl_miDeletePaintedSet
#define miDrawArcs_r _pl_miDrawArcs_r
#define miDrawLines _pl_miDrawLines
#define miDrawPoints _pl_miDrawPoints
#define miDrawRectangles _pl_miDrawRectangles
#define miFillArcs _pl_miFillArcs
#define miFillPolygon _pl_miFillPolygon
#define miFillRectangles _pl_miFillRectangles
#define miNewCanvas _pl_miNewCanvas
#define miNewEllipseCache _pl_miNewEllipseCache
#define miNewGC _pl_miNewGC
#define miNewPaintedSet _pl_miNewPaintedSet
#define miSetCanvasStipple _pl_miSetCanvasStipple
#define miSetCanvasTexture _pl_miSetCanvasTexture
#define miSetGCAttrib _pl_miSetGCAttrib
#define miSetGCAttribs _pl_miSetGCAttribs
#define miSetGCDashes _pl_miSetGCDashes
#define miSetGCMiterLimit _pl_miSetGCMiterLimit
#define miSetGCPixels _pl_miSetGCPixels
#define miSetPixelMerge2 _pl_miSetPixelMerge2
#define miSetPixelMerge3 _pl_miSetPixelMerge3

/* an external libxmi symbol */
#define mi_libxmi_ver _pl_mi_libxmi_ver

/* internal libxmi functions (which incidentally in libxmi are given an
   initial underscore, in much the same way) */
#define mi_xmalloc _pl_mi_xmalloc
#define mi_xcalloc _pl_mi_xcalloc
#define mi_xrealloc _pl_mi_xrealloc
#define miAddSpansToPaintedSet _pl_miAddSpansToPaintedSet
#define miDrawArcs_r_internal _pl_miDrawArcs_r_internal
#define miDrawArcs_internal _pl_miDrawArcs_internal
#define miDrawLines_internal _pl_miDrawLines_internal
#define miDrawRectangles_internal _pl_miDrawRectangles_internal
#define miPolyArc_r _pl_miPolyArc_r
#define miPolyArc _pl_miPolyArc
#define miFillArcs_internal _pl_miFillArcs_internal
#define miFillRectangles_internal _pl_miFillRectangles_internal
#define miFillSppPoly _pl_miFillSppPoly
#define miFillPolygon_internal _pl_miFillPolygon_internal
#define miFillConvexPoly _pl_miFillConvexPoly
#define miFillGeneralPoly _pl_miFillGeneralPoly
#define miDrawPoints_internal _pl_miDrawPoints_internal
#define miCreateETandAET _pl_miCreateETandAET
#define miloadAET _pl_miloadAET
#define micomputeWAET _pl_micomputeWAET
#define miInsertionSort _pl_miInsertionSort
#define miFreeStorage _pl_miFreeStorage
#define miQuickSortSpansY _pl_miQuickSortSpansY
#define miUniquifyPaintedSet _pl_miUniquifyPaintedSet
#define miWideDash _pl_miWideDash
#define miStepDash _pl_miStepDash
#define miWideLine _pl_miWideLine
#define miZeroPolyArc_r _pl_miZeroPolyArc_r
#define miZeroPolyArc _pl_miZeroPolyArc
#define miZeroLine _pl_miZeroLine
#define miZeroDash _pl_miZeroDash

/* Don't include unneeded non-reentrant libxmi functions, such as the
   function miPolyArc().  We use the reentrant version miPolyArc_r()
   instead, to avoid static data. */
#define NO_NONREENTRANT_POLYARC_SUPPORT

/* Internal functions that aren't Plotter class methods, but which need to
   be renamed in libplotter. */
#ifdef LIBPLOTTER
#define pl_libplot_warning_handler pl_libplotter_warning_handler
#define pl_libplot_error_handler pl_libplotter_error_handler
#endif

/* Declarations of forwarding functions used in libplot (not libplotter). */

/* These support the derivation of classes such as the PNMPlotter and the
   PNGPlotter classes from the BitmapPlotter class, the derivation of the
   PCLPlotter class from the HPGLPlotter class, and the derivation of the
   XPlotter class from the XDrawablePlotter class.  */

#ifndef LIBPLOTTER
extern int _maybe_output_image (Plotter *_plotter);
extern void _maybe_switch_to_hpgl (Plotter *_plotter);
extern void _maybe_switch_from_hpgl (Plotter *_plotter);
#ifndef X_DISPLAY_MISSING
extern void _maybe_get_new_colormap (Plotter *_plotter);
extern void _maybe_handle_x_events (Plotter *_plotter);
#endif /* not X_DISPLAY_MISSING */
#endif /* not LIBPLOTTER */

/* Declarations of the Plotter methods and the device-specific versions of
   same.  The initial letter indicates the Plotter class specificity:
   g=generic (i.e. base Plotter class), b=bitmap, m=metafile, t=Tektronix,
   r=ReGIS, h=HP-GL/2 and PCL 5, f=xfig, c=CGM, p=PS, a=Adobe Illustrator,
   s=SVG, i=GIF, n=PNM (i.e. PBM/PGM/PPM), z=PNG, x=X11 Drawable, y=X11.

   In libplot, these are declarations of global functions.  But in
   libplotter, we use #define and the double colon notation to make them
   function members of the appropriate Plotter classes.

   The declarations-for-libplot are encapsulated within
   ___BEGIN_DECLS...___END_DECLS pairs, which do nothing if a C compiler is
   used to compile libplot.  If on the other hand libplot is compiled by a
   C++ compiler, which is easy to arrange by doing `CC=g++ ./configure',
   then this will require each libplot function to have C linkage rather
   than C++ linkage.  Libplot functions should have C linkage, of course
   (cf. libplot's external header file plot.h).  */

#ifndef LIBPLOTTER
/* support C++ */
#ifdef ___BEGIN_DECLS
#undef ___BEGIN_DECLS
#endif
#ifdef ___END_DECLS
#undef ___END_DECLS
#endif
#ifdef __cplusplus
# define ___BEGIN_DECLS extern "C" {
# define ___END_DECLS }
#else
# define ___BEGIN_DECLS		/* empty */
# define ___END_DECLS		/* empty */
#endif
#endif /* not LIBPLOTTER */

#ifndef LIBPLOTTER
/* Plotter public methods, for libplot */
#define _API_alabel pl_alabel_r
#define _API_arc pl_arc_r
#define _API_arcrel pl_arcrel_r
#define _API_bezier2 pl_bezier2_r
#define _API_bezier2rel pl_bezier2rel_r
#define _API_bezier3 pl_bezier3_r
#define _API_bezier3rel pl_bezier3rel_r
#define _API_bgcolor pl_bgcolor_r
#define _API_bgcolorname pl_bgcolorname_r
#define _API_box pl_box_r
#define _API_boxrel pl_boxrel_r
#define _API_capmod pl_capmod_r
#define _API_circle pl_circle_r
#define _API_circlerel pl_circlerel_r
#define _API_closepath pl_closepath_r
#define _API_closepl pl_closepl_r
#define _API_color pl_color_r
#define _API_colorname pl_colorname_r
#define _API_cont pl_cont_r
#define _API_contrel pl_contrel_r
#define _API_ellarc pl_ellarc_r
#define _API_ellarcrel pl_ellarcrel_r
#define _API_ellipse pl_ellipse_r
#define _API_ellipserel pl_ellipserel_r
#define _API_endpath pl_endpath_r
#define _API_endsubpath pl_endsubpath_r
#define _API_erase pl_erase_r
#define _API_farc pl_farc_r
#define _API_farcrel pl_farcrel_r
#define _API_fbezier2 pl_fbezier2_r
#define _API_fbezier2rel pl_fbezier2rel_r
#define _API_fbezier3 pl_fbezier3_r
#define _API_fbezier3rel pl_fbezier3rel_r
#define _API_fbox pl_fbox_r
#define _API_fboxrel pl_fboxrel_r
#define _API_fcircle pl_fcircle_r
#define _API_fcirclerel pl_fcirclerel_r
#define _API_fconcat pl_fconcat_r
#define _API_fcont pl_fcont_r
#define _API_fcontrel pl_fcontrel_r
#define _API_fellarc pl_fellarc_r
#define _API_fellarcrel pl_fellarcrel_r
#define _API_fellipse pl_fellipse_r
#define _API_fellipserel pl_fellipserel_r
#define _API_ffontname pl_ffontname_r
#define _API_ffontsize pl_ffontsize_r
#define _API_fillcolor pl_fillcolor_r
#define _API_fillcolorname pl_fillcolorname_r
#define _API_fillmod pl_fillmod_r
#define _API_filltype pl_filltype_r
#define _API_flabelwidth pl_flabelwidth_r
#define _API_fline pl_fline_r
#define _API_flinedash pl_flinedash_r
#define _API_flinerel pl_flinerel_r
#define _API_flinewidth pl_flinewidth_r
#define _API_flushpl pl_flushpl_r
#define _API_fmarker pl_fmarker_r
#define _API_fmarkerrel pl_fmarkerrel_r
#define _API_fmiterlimit pl_fmiterlimit_r
#define _API_fmove pl_fmove_r
#define _API_fmoverel pl_fmoverel_r
#define _API_fontname pl_fontname_r
#define _API_fontsize pl_fontsize_r
#define _API_fpoint pl_fpoint_r
#define _API_fpointrel pl_fpointrel_r
#define _API_frotate pl_frotate_r
#define _API_fscale pl_fscale_r
#define _API_fsetmatrix pl_fsetmatrix_r
#define _API_fspace pl_fspace_r
#define _API_fspace2 pl_fspace2_r
#define _API_ftextangle pl_ftextangle_r
#define _API_ftranslate pl_ftranslate_r
#define _API_havecap pl_havecap_r
#define _API_joinmod pl_joinmod_r
#define _API_label pl_label_r
#define _API_labelwidth pl_labelwidth_r
#define _API_line pl_line_r
#define _API_linedash pl_linedash_r
#define _API_linemod pl_linemod_r
#define _API_linerel pl_linerel_r
#define _API_linewidth pl_linewidth_r
#define _API_marker pl_marker_r
#define _API_markerrel pl_markerrel_r
#define _API_move pl_move_r
#define _API_moverel pl_moverel_r
#define _API_openpl pl_openpl_r
#define _API_orientation pl_orientation_r
#define _API_outfile pl_outfile_r /* OBSOLESCENT */
#define _API_pencolor pl_pencolor_r
#define _API_pencolorname pl_pencolorname_r
#define _API_pentype pl_pentype_r
#define _API_point pl_point_r
#define _API_pointrel pl_pointrel_r
#define _API_restorestate pl_restorestate_r
#define _API_savestate pl_savestate_r
#define _API_space pl_space_r
#define _API_space2 pl_space2_r
#define _API_textangle pl_textangle_r
___BEGIN_DECLS
extern FILE* _API_outfile (Plotter *_plotter, FILE* newstream);/* OBSOLESCENT */
extern double _API_ffontname (Plotter *_plotter, const char *s);
extern double _API_ffontsize (Plotter *_plotter, double size);
extern double _API_flabelwidth (Plotter *_plotter, const char *s);
extern double _API_ftextangle (Plotter *_plotter, double angle);
extern int _API_alabel (Plotter *_plotter, int x_justify, int y_justify, const char *s);
extern int _API_arc (Plotter *_plotter, int xc, int yc, int x0, int y0, int x1, int y1);
extern int _API_arcrel (Plotter *_plotter, int dxc, int dyc, int dx0, int dy0, int dx1, int dy1);
extern int _API_bezier2 (Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2);
extern int _API_bezier2rel (Plotter *_plotter, int dx0, int dy0, int dx1, int dy1, int dx2, int dy2);
extern int _API_bezier3 (Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
extern int _API_bezier3rel (Plotter *_plotter, int dx0, int dy0, int dx1, int dy1, int dx2, int dy2, int dx3, int dy3);
extern int _API_bgcolor (Plotter *_plotter, int red, int green, int blue);
extern int _API_bgcolorname (Plotter *_plotter, const char *name);
extern int _API_box (Plotter *_plotter, int x0, int y0, int x1, int y1);
extern int _API_boxrel (Plotter *_plotter, int dx0, int dy0, int dx1, int dy1);
extern int _API_capmod (Plotter *_plotter, const char *s);
extern int _API_circle (Plotter *_plotter, int x, int y, int r);
extern int _API_circlerel (Plotter *_plotter, int dx, int dy, int r);
extern int _API_closepath (Plotter *_plotter);
extern int _API_closepl (Plotter *_plotter);
extern int _API_color (Plotter *_plotter, int red, int green, int blue);
extern int _API_colorname (Plotter *_plotter, const char *name);
extern int _API_cont (Plotter *_plotter, int x, int y);
extern int _API_contrel (Plotter *_plotter, int x, int y);
extern int _API_ellarc (Plotter *_plotter, int xc, int yc, int x0, int y0, int x1, int y1);
extern int _API_ellarcrel (Plotter *_plotter, int dxc, int dyc, int dx0, int dy0, int dx1, int dy1);
extern int _API_ellipse (Plotter *_plotter, int x, int y, int rx, int ry, int angle);
extern int _API_ellipserel (Plotter *_plotter, int dx, int dy, int rx, int ry, int angle);
extern int _API_endpath (Plotter *_plotter);
extern int _API_endsubpath (Plotter *_plotter);
extern int _API_erase (Plotter *_plotter);
extern int _API_farc (Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1);
extern int _API_farcrel (Plotter *_plotter, double dxc, double dyc, double dx0, double dy0, double dx1, double dy1);
extern int _API_fbezier2 (Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2);
extern int _API_fbezier2rel (Plotter *_plotter, double dx0, double dy0, double dx1, double dy1, double dx2, double dy2);
extern int _API_fbezier3 (Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
extern int _API_fbezier3rel (Plotter *_plotter, double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);
extern int _API_fbox (Plotter *_plotter, double x0, double y0, double x1, double y1);
extern int _API_fboxrel (Plotter *_plotter, double dx0, double dy0, double dx1, double dy1);
extern int _API_fcircle (Plotter *_plotter, double x, double y, double r);
extern int _API_fcirclerel (Plotter *_plotter, double dx, double dy, double r);
extern int _API_fconcat (Plotter *_plotter, double m0, double m1, double m2, double m3, double m4, double m5);
extern int _API_fcont (Plotter *_plotter, double x, double y);
extern int _API_fcontrel (Plotter *_plotter, double x, double y);
extern int _API_fellarc (Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1);
extern int _API_fellarcrel (Plotter *_plotter, double dxc, double dyc, double dx0, double dy0, double dx1, double dy1);
extern int _API_fellipse (Plotter *_plotter, double x, double y, double rx, double ry, double angle);
extern int _API_fellipserel (Plotter *_plotter, double dx, double dy, double rx, double ry, double angle);
extern int _API_fillcolor (Plotter *_plotter, int red, int green, int blue);
extern int _API_fillcolorname (Plotter *_plotter, const char *name);
extern int _API_fillmod (Plotter *_plotter, const char *s);
extern int _API_filltype (Plotter *_plotter, int level);
extern int _API_fline (Plotter *_plotter, double x0, double y0, double x1, double y1);
extern int _API_flinedash (Plotter *_plotter, int n, const double *dashes, double offset);
extern int _API_flinerel (Plotter *_plotter, double dx0, double dy0, double dx1, double dy1);
extern int _API_flinewidth (Plotter *_plotter, double size);
extern int _API_flushpl (Plotter *_plotter);
extern int _API_fmarker (Plotter *_plotter, double x, double y, int type, double size);
extern int _API_fmarkerrel (Plotter *_plotter, double dx, double dy, int type, double size);
extern int _API_fmiterlimit (Plotter *_plotter, double limit);
extern int _API_fmove (Plotter *_plotter, double x, double y);
extern int _API_fmoverel (Plotter *_plotter, double x, double y);
extern int _API_fontname (Plotter *_plotter, const char *s);
extern int _API_fontsize (Plotter *_plotter, int size);
extern int _API_fpoint (Plotter *_plotter, double x, double y);
extern int _API_fpointrel (Plotter *_plotter, double dx, double dy);
extern int _API_frotate (Plotter *_plotter, double theta);
extern int _API_fscale (Plotter *_plotter, double x, double y);
extern int _API_fsetmatrix (Plotter *_plotter, double m0, double m1, double m2, double m3, double m4, double m5);
extern int _API_fspace (Plotter *_plotter, double x0, double y0, double x1, double y1);
extern int _API_fspace2 (Plotter *_plotter, double x0, double y0, double x1, double y1, double x2, double y2);
extern int _API_ftranslate (Plotter *_plotter, double x, double y);
extern int _API_havecap (Plotter *_plotter, const char *s);
extern int _API_joinmod (Plotter *_plotter, const char *s);
extern int _API_label (Plotter *_plotter, const char *s);
extern int _API_labelwidth (Plotter *_plotter, const char *s);
extern int _API_line (Plotter *_plotter, int x0, int y0, int x1, int y1);
extern int _API_linedash (Plotter *_plotter, int n, const int *dashes, int offset);
extern int _API_linemod (Plotter *_plotter, const char *s);
extern int _API_linerel (Plotter *_plotter, int dx0, int dy0, int dx1, int dy1);
extern int _API_linewidth (Plotter *_plotter, int size);
extern int _API_marker (Plotter *_plotter, int x, int y, int type, int size);
extern int _API_markerrel (Plotter *_plotter, int dx, int dy, int type, int size);
extern int _API_move (Plotter *_plotter, int x, int y);
extern int _API_moverel (Plotter *_plotter, int x, int y);
extern int _API_openpl (Plotter *_plotter);
extern int _API_orientation (Plotter *_plotter, int direction);
extern int _API_pencolor (Plotter *_plotter, int red, int green, int blue);
extern int _API_pencolorname (Plotter *_plotter, const char *name);
extern int _API_pentype (Plotter *_plotter, int level);
extern int _API_point (Plotter *_plotter, int x, int y);
extern int _API_pointrel (Plotter *_plotter, int dx, int dy);
extern int _API_restorestate (Plotter *_plotter);
extern int _API_savestate (Plotter *_plotter);
extern int _API_space (Plotter *_plotter, int x0, int y0, int x1, int y1);
extern int _API_space2 (Plotter *_plotter, int x0, int y0, int x1, int y1, int x2, int y2);
extern int _API_textangle (Plotter *_plotter, int angle);
/* Plotter protected methods, for libplot */
extern bool _pl_g_begin_page (Plotter *_plotter);
extern bool _pl_g_end_page (Plotter *_plotter);
extern bool _pl_g_erase_page (Plotter *_plotter);
extern bool _pl_g_flush_output (Plotter *_plotter);
extern bool _pl_g_paint_marker (Plotter *_plotter, int type, double size);
extern bool _pl_g_paint_paths (Plotter *_plotter);
extern bool _pl_g_path_is_flushable (Plotter *_plotter);
extern bool _pl_g_retrieve_font (Plotter *_plotter);
extern double _pl_g_get_text_width (Plotter *_plotter, const unsigned char *s);
extern double _pl_g_paint_text_string (Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify);
extern void _pl_g_error (Plotter *_plotter, const char *msg);
extern void _pl_g_initialize (Plotter *_plotter);
extern void _pl_g_maybe_prepaint_segments (Plotter *_plotter, int prev_num_segments);
extern void _pl_g_paint_path (Plotter *_plotter);
extern void _pl_g_paint_point (Plotter *_plotter);
extern void _pl_g_paint_text_string_with_escapes (Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify);
extern void _pl_g_pop_state (Plotter *_plotter);
extern void _pl_g_push_state (Plotter *_plotter);
extern void _pl_g_terminate (Plotter *_plotter);
extern void _pl_g_warning (Plotter *_plotter, const char *msg);
/* undocumented public methods that provide access to the font tables
   within libplot/libplotter; for libplot */
extern void * _pl_get_hershey_font_info (Plotter *_plotter);
extern void * _pl_get_ps_font_info (Plotter *_plotter);
extern void * _pl_get_pcl_font_info (Plotter *_plotter);
extern void * _pl_get_stick_font_info (Plotter *_plotter);
/* private functions related to the drawing of text strings in Hershey
   fonts (defined in g_alab_her.c); for libplot */
extern double _pl_g_alabel_hershey (Plotter *_plotter, const unsigned char *s, int x_justify, int y_justify);
extern double _pl_g_flabelwidth_hershey (Plotter *_plotter, const unsigned char *s);
extern void _pl_g_draw_hershey_glyph (Plotter *_plotter, int num, double charsize, int type, bool oblique);
extern void _pl_g_draw_hershey_penup_stroke (Plotter *_plotter, double dx, double dy, double charsize, bool oblique);
extern void _pl_g_draw_hershey_string (Plotter *_plotter, const unsigned short *string);
extern void _pl_g_draw_hershey_stroke (Plotter *_plotter, bool pendown, double deltax, double deltay);
/* other private Plotter functions (a mixed bag), for libplot */
extern double _pl_g_render_non_hershey_string (Plotter *_plotter, const char *s, bool do_render, int x_justify, int y_justify);
extern double _pl_g_render_simple_string (Plotter *_plotter, const unsigned char *s, bool do_render, int h_just, int v_just);
extern unsigned short * _pl_g_controlify (Plotter *_plotter, const unsigned char *);
extern void _pl_g_copy_params_to_plotter (Plotter *_plotter, const PlotterParams *params);
extern void _pl_g_create_first_drawing_state (Plotter *_plotter);
extern void _pl_g_delete_first_drawing_state (Plotter *_plotter);
extern void _pl_g_free_params_in_plotter (Plotter *_plotter);
extern void _pl_g_maybe_replace_arc (Plotter *_plotter);
extern void _pl_g_set_font (Plotter *_plotter);
/* other protected Plotter functions (a mixed bag), for libplot */
extern void _pl_g_flush_plotter_outstreams (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* static Plotter public method (libplotter only) */
#define parampl Plotter::parampl
/* Plotter public methods, for libplotter */
#define _API_alabel Plotter::alabel
#define _API_arc Plotter::arc
#define _API_arcrel Plotter::arcrel
#define _API_bezier2 Plotter::bezier2
#define _API_bezier2rel Plotter::bezier2rel
#define _API_bezier3 Plotter::bezier3
#define _API_bezier3rel Plotter::bezier3rel
#define _API_bgcolor Plotter::bgcolor
#define _API_bgcolorname Plotter::bgcolorname
#define _API_box Plotter::box
#define _API_boxrel Plotter::boxrel
#define _API_capmod Plotter::capmod
#define _API_circle Plotter::circle
#define _API_circlerel Plotter::circlerel
#define _API_closepath Plotter::closepath
#define _API_closepl Plotter::closepl
#define _API_color Plotter::color
#define _API_colorname Plotter::colorname
#define _API_cont Plotter::cont
#define _API_contrel Plotter::contrel
#define _API_ellarc Plotter::ellarc
#define _API_ellarcrel Plotter::ellarcrel
#define _API_ellipse Plotter::ellipse
#define _API_ellipserel Plotter::ellipserel
#define _API_endpath Plotter::endpath
#define _API_endsubpath Plotter::endsubpath
#define _API_erase Plotter::erase
#define _API_farc Plotter::farc
#define _API_farcrel Plotter::farcrel
#define _API_fbezier2 Plotter::fbezier2
#define _API_fbezier2rel Plotter::fbezier2rel
#define _API_fbezier3 Plotter::fbezier3
#define _API_fbezier3rel Plotter::fbezier3rel
#define _API_fbox Plotter::fbox
#define _API_fboxrel Plotter::fboxrel
#define _API_fcircle Plotter::fcircle
#define _API_fcirclerel Plotter::fcirclerel
#define _API_fconcat Plotter::fconcat
#define _API_fcont Plotter::fcont
#define _API_fcontrel Plotter::fcontrel
#define _API_fellarc Plotter::fellarc
#define _API_fellarcrel Plotter::fellarcrel
#define _API_fellipse Plotter::fellipse
#define _API_fellipserel Plotter::fellipserel
#define _API_ffontname Plotter::ffontname
#define _API_ffontsize Plotter::ffontsize
#define _API_fillcolor Plotter::fillcolor
#define _API_fillcolorname Plotter::fillcolorname
#define _API_fillmod Plotter::fillmod
#define _API_filltype Plotter::filltype
#define _API_flabelwidth Plotter::flabelwidth
#define _API_fline Plotter::fline
#define _API_flinedash Plotter::flinedash
#define _API_flinerel Plotter::flinerel
#define _API_flinewidth Plotter::flinewidth
#define _API_flushpl Plotter::flushpl
#define _API_fmarker Plotter::fmarker
#define _API_fmarkerrel Plotter::fmarkerrel
#define _API_fmiterlimit Plotter::fmiterlimit
#define _API_fmove Plotter::fmove
#define _API_fmoverel Plotter::fmoverel
#define _API_fontname Plotter::fontname
#define _API_fontsize Plotter::fontsize
#define _API_fpoint Plotter::fpoint
#define _API_fpointrel Plotter::fpointrel
#define _API_frotate Plotter::frotate
#define _API_fscale Plotter::fscale
#define _API_fsetmatrix Plotter::fsetmatrix
#define _API_fspace Plotter::fspace
#define _API_fspace2 Plotter::fspace2
#define _API_ftextangle Plotter::ftextangle
#define _API_ftranslate Plotter::ftranslate
#define _API_havecap Plotter::havecap
#define _API_joinmod Plotter::joinmod
#define _API_label Plotter::label
#define _API_labelwidth Plotter::labelwidth
#define _API_line Plotter::line
#define _API_linedash Plotter::linedash
#define _API_linemod Plotter::linemod
#define _API_linerel Plotter::linerel
#define _API_linewidth Plotter::linewidth
#define _API_marker Plotter::marker
#define _API_markerrel Plotter::markerrel
#define _API_move Plotter::move
#define _API_moverel Plotter::moverel
#define _API_openpl Plotter::openpl
#define _API_orientation Plotter::orientation
#define _API_outfile Plotter::outfile /* OBSOLESCENT */
#define _API_pencolor Plotter::pencolor
#define _API_pencolorname Plotter::pencolorname
#define _API_pentype Plotter::pentype
#define _API_point Plotter::point
#define _API_pointrel Plotter::pointrel
#define _API_restorestate Plotter::restorestate
#define _API_savestate Plotter::savestate
#define _API_space Plotter::space
#define _API_space2 Plotter::space2
#define _API_textangle Plotter::textangle
/* Plotter protected methods, for libplotter */
#define _pl_g_begin_page Plotter::begin_page
#define _pl_g_end_page Plotter::end_page
#define _pl_g_erase_page Plotter::erase_page
#define _pl_g_error Plotter::error
#define _pl_g_paint_text_string_with_escapes Plotter::paint_text_string_with_escapes
#define _pl_g_paint_text_string Plotter::paint_text_string
#define _pl_g_get_text_width Plotter::get_text_width
#define _pl_g_flush_output Plotter::flush_output
#define _pl_g_initialize Plotter::initialize
#define _pl_g_path_is_flushable Plotter::path_is_flushable
#define _pl_g_maybe_prepaint_segments Plotter::maybe_prepaint_segments
#define _pl_g_paint_marker Plotter::paint_marker
#define _pl_g_paint_path Plotter::paint_path
#define _pl_g_paint_paths Plotter::paint_paths
#define _pl_g_paint_point Plotter::paint_point
#define _pl_g_pop_state Plotter::pop_state
#define _pl_g_push_state Plotter::push_state
#define _pl_g_retrieve_font Plotter::retrieve_font
#define _pl_g_terminate Plotter::terminate
#define _pl_g_warning Plotter::warning
/* undocumented public methods that provide access to the font tables
   within libplot/libplotter; for libplotter */
#define _pl_get_hershey_font_info Plotter::_get_hershey_font_info
#define _pl_get_ps_font_info Plotter::_get_ps_font_info
#define _pl_get_pcl_font_info Plotter::_get_pcl_font_info
#define _pl_get_stick_font_info Plotter::_get_stick_font_info
/* private functions related to the drawing of text strings in Hershey
   fonts (defined in g_alab_her.c), for libplotter  */
#define _pl_g_alabel_hershey Plotter::_g_alabel_hershey
#define _pl_g_draw_hershey_glyph Plotter::_g_draw_hershey_glyph
#define _pl_g_draw_hershey_penup_stroke Plotter::_g_draw_hershey_penup_stroke
#define _pl_g_draw_hershey_string Plotter::_g_draw_hershey_string
#define _pl_g_draw_hershey_stroke Plotter::_g_draw_hershey_stroke
#define _pl_g_flabelwidth_hershey Plotter::_g_flabelwidth_hershey
/* other private functions (a mixed bag), for libplotter */
#define _pl_g_controlify Plotter::_g_controlify
#define _pl_g_copy_params_to_plotter Plotter::_g_copy_params_to_plotter
#define _pl_g_create_first_drawing_state Plotter::_g_create_first_drawing_state
#define _pl_g_delete_first_drawing_state Plotter::_g_delete_first_drawing_state
#define _pl_g_free_params_in_plotter Plotter::_g_free_params_in_plotter
#define _pl_g_maybe_replace_arc Plotter::_g_maybe_replace_arc
#define _pl_g_render_non_hershey_string Plotter::_g_render_non_hershey_string
#define _pl_g_render_simple_string Plotter::_g_render_simple_string
#define _pl_g_set_font Plotter::_g_set_font
/* other protected functions (a mixed bag), for libplotter */
#define _pl_g_flush_plotter_outstreams Plotter::_flush_plotter_outstreams
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* MetaPlotter protected methods, for libplot */
extern bool _pl_m_begin_page (Plotter *_plotter);
extern bool _pl_m_end_page (Plotter *_plotter);
extern bool _pl_m_erase_page (Plotter *_plotter);
extern bool _pl_m_paint_marker (Plotter *_plotter, int type, double size);
extern bool _pl_m_paint_paths (Plotter *_plotter);
extern bool _pl_m_path_is_flushable (Plotter *_plotter);
extern void _pl_m_initialize (Plotter *_plotter);
extern void _pl_m_maybe_prepaint_segments (Plotter *_plotter, int prev_num_segments);
extern void _pl_m_paint_path (Plotter *_plotter);
extern void _pl_m_paint_point (Plotter *_plotter);
extern void _pl_m_paint_text_string_with_escapes (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_m_terminate (Plotter *_plotter);
/* MetaPlotter-specific internal functions, for libplot */
extern void _pl_m_emit_integer (Plotter *_plotter, int x);
extern void _pl_m_emit_float (Plotter *_plotter, double x);
extern void _pl_m_emit_op_code (Plotter *_plotter, int c);
extern void _pl_m_emit_string (Plotter *_plotter, const char *s);
extern void _pl_m_emit_terminator (Plotter *_plotter);
extern void _pl_m_paint_path_internal (Plotter *_plotter, const plPath *path);
extern void _pl_m_set_attributes (Plotter *_plotter, unsigned int mask);
___END_DECLS
#else  /* LIBPLOTTER */
/* MetaPlotter protected methods, for libplotter */
#define _pl_m_begin_page MetaPlotter::begin_page
#define _pl_m_end_page MetaPlotter::end_page
#define _pl_m_erase_page MetaPlotter::erase_page
#define _pl_m_paint_text_string_with_escapes MetaPlotter::paint_text_string_with_escapes
#define _pl_m_initialize MetaPlotter::initialize
#define _pl_m_path_is_flushable MetaPlotter::path_is_flushable
#define _pl_m_maybe_prepaint_segments MetaPlotter::maybe_prepaint_segments
#define _pl_m_paint_marker MetaPlotter::paint_marker
#define _pl_m_paint_path MetaPlotter::paint_path
#define _pl_m_paint_paths MetaPlotter::paint_paths
#define _pl_m_paint_point MetaPlotter::paint_point
#define _pl_m_terminate MetaPlotter::terminate
/* MetaPlotter-specific internal functions, for libplotter */
#define _pl_m_emit_integer MetaPlotter::_m_emit_integer
#define _pl_m_emit_float MetaPlotter::_m_emit_float
#define _pl_m_emit_op_code MetaPlotter::_m_emit_op_code
#define _pl_m_emit_string MetaPlotter::_m_emit_string
#define _pl_m_emit_terminator MetaPlotter::_m_emit_terminator
#define _pl_m_paint_path_internal MetaPlotter::_m_paint_path_internal
#define _pl_m_set_attributes MetaPlotter::_m_set_attributes
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* BitmapPlotter protected methods, for libplot */
extern bool _pl_b_begin_page (Plotter *_plotter);
extern bool _pl_b_end_page (Plotter *_plotter);
extern bool _pl_b_erase_page (Plotter *_plotter);
extern bool _pl_b_paint_paths (Plotter *_plotter);
extern void _pl_b_initialize (Plotter *_plotter);
extern void _pl_b_paint_path (Plotter *_plotter);
extern void _pl_b_paint_point (Plotter *_plotter);
extern void _pl_b_terminate (Plotter *_plotter);
/* BitmapPlotter internal functions, for libplot (overridden in subclasses) */
extern int _pl_b_maybe_output_image (Plotter *_plotter);
/* other BitmapPlotter internal functions, for libplot */
extern void _pl_b_delete_image (Plotter *_plotter);
extern void _pl_b_draw_elliptic_arc (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_b_draw_elliptic_arc_2 (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_b_draw_elliptic_arc_internal (Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
extern void _pl_b_new_image (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* BitmapPlotter protected methods, for libplotter */
#define _pl_b_begin_page BitmapPlotter::begin_page
#define _pl_b_end_page BitmapPlotter::end_page
#define _pl_b_erase_page BitmapPlotter::erase_page
#define _pl_b_initialize BitmapPlotter::initialize
#define _pl_b_paint_path BitmapPlotter::paint_path
#define _pl_b_paint_paths BitmapPlotter::paint_paths
#define _pl_b_paint_point BitmapPlotter::paint_point
#define _pl_b_terminate BitmapPlotter::terminate
/* BitmapPlotter internal functions (overriden in subclasses) */
#define _pl_b_maybe_output_image BitmapPlotter::_maybe_output_image
/* other BitmapPlotter internal functions, for libplotter */
#define _pl_b_delete_image BitmapPlotter::_b_delete_image
#define _pl_b_draw_elliptic_arc BitmapPlotter::_b_draw_elliptic_arc
#define _pl_b_draw_elliptic_arc_2 BitmapPlotter::_b_draw_elliptic_arc_2
#define _pl_b_draw_elliptic_arc_internal BitmapPlotter::_b_draw_elliptic_arc_internal
#define _pl_b_new_image BitmapPlotter::_b_new_image 
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* TekPlotter protected methods, for libplot */
extern bool _pl_t_begin_page (Plotter *_plotter);
extern bool _pl_t_end_page (Plotter *_plotter);
extern bool _pl_t_erase_page (Plotter *_plotter);
extern bool _pl_t_path_is_flushable (Plotter *_plotter);
extern void _pl_t_initialize (Plotter *_plotter);
extern void _pl_t_maybe_prepaint_segments (Plotter *_plotter, int prev_num_segments);
extern void _pl_t_paint_point (Plotter *_plotter);
extern void _pl_t_terminate (Plotter *_plotter);
/* TekPlotter internal functions, for libplot */
extern void _pl_t_set_attributes (Plotter *_plotter);
extern void _pl_t_set_bg_color (Plotter *_plotter);
extern void _pl_t_set_pen_color (Plotter *_plotter);
extern void _pl_t_tek_mode (Plotter *_plotter, int newmode);
extern void _pl_t_tek_move (Plotter *_plotter, int xx, int yy);
extern void _pl_t_tek_vector_compressed (Plotter *_plotter, int xx, int yy, int oldxx, int oldyy, bool force);
extern void _pl_t_tek_vector (Plotter *_plotter, int xx, int yy);
___END_DECLS
#else  /* LIBPLOTTER */
/* TekPlotter protected methods, for libplotter */
#define _pl_t_begin_page TekPlotter::begin_page
#define _pl_t_end_page TekPlotter::end_page
#define _pl_t_erase_page TekPlotter::erase_page
#define _pl_t_initialize TekPlotter::initialize
#define _pl_t_path_is_flushable TekPlotter::path_is_flushable
#define _pl_t_maybe_prepaint_segments TekPlotter::maybe_prepaint_segments
#define _pl_t_paint_point TekPlotter::paint_point
#define _pl_t_terminate TekPlotter::terminate
/* TekPlotter internal functions, for libplotter */
#define _pl_t_set_attributes TekPlotter::_t_set_attributes
#define _pl_t_set_bg_color TekPlotter::_t_set_bg_color
#define _pl_t_set_pen_color TekPlotter::_t_set_pen_color
#define _pl_t_tek_mode TekPlotter::_t_tek_mode
#define _pl_t_tek_move TekPlotter::_t_tek_move
#define _pl_t_tek_vector TekPlotter::_t_tek_vector
#define _pl_t_tek_vector_compressed TekPlotter::_t_tek_vector_compressed
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* ReGISPlotter protected methods, for libplot */
extern bool _pl_r_begin_page (Plotter *_plotter);
extern bool _pl_r_end_page (Plotter *_plotter);
extern bool _pl_r_erase_page (Plotter *_plotter);
extern bool _pl_r_paint_paths (Plotter *_plotter);
extern bool _pl_r_path_is_flushable (Plotter *_plotter);
extern void _pl_r_initialize (Plotter *_plotter);
extern void _pl_r_maybe_prepaint_segments (Plotter *_plotter, int prev_num_segments);
extern void _pl_r_paint_path (Plotter *_plotter);
extern void _pl_r_paint_point (Plotter *_plotter);
extern void _pl_r_terminate (Plotter *_plotter);
/* ReGISPlotter internal functions, for libplot */
extern void _pl_r_regis_move (Plotter *_plotter, int xx, int yy);
extern void _pl_r_set_attributes (Plotter *_plotter);
extern void _pl_r_set_bg_color (Plotter *_plotter);
extern void _pl_r_set_fill_color (Plotter *_plotter);
extern void _pl_r_set_pen_color (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* ReGISPlotter protected methods, for libplotter */
#define _pl_r_begin_page ReGISPlotter::begin_page
#define _pl_r_end_page ReGISPlotter::end_page
#define _pl_r_erase_page ReGISPlotter::erase_page
#define _pl_r_initialize ReGISPlotter::initialize
#define _pl_r_path_is_flushable ReGISPlotter::path_is_flushable
#define _pl_r_maybe_prepaint_segments ReGISPlotter::maybe_prepaint_segments
#define _pl_r_paint_path ReGISPlotter::paint_path
#define _pl_r_paint_paths ReGISPlotter::paint_paths
#define _pl_r_paint_point ReGISPlotter::paint_point
#define _pl_r_terminate ReGISPlotter::terminate
/* ReGISPlotter internal functions, for libplotter */
#define _pl_r_regis_move ReGISPlotter::_r_regis_move
#define _pl_r_set_attributes ReGISPlotter::_r_set_attributes
#define _pl_r_set_bg_color ReGISPlotter::_r_set_bg_color
#define _pl_r_set_fill_color ReGISPlotter::_r_set_fill_color
#define _pl_r_set_pen_color ReGISPlotter::_r_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* HPGLPlotter/PCLPlotter protected methods, for libplot */
extern bool _pl_h_begin_page (Plotter *_plotter);
extern bool _pl_h_end_page (Plotter *_plotter);
extern bool _pl_h_erase_page (Plotter *_plotter);
extern bool _pl_h_paint_paths (Plotter *_plotter);
extern double _pl_h_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_h_paint_point (Plotter *_plotter);
extern void _pl_h_paint_path (Plotter *_plotter);
/* HPGLPlotter protected methods, for libplot */
extern void _pl_h_initialize (Plotter *_plotter);
extern void _pl_h_terminate (Plotter *_plotter);
/* PCLPlotter protected methods, for libplot */
extern void _pl_q_initialize (Plotter *_plotter);
extern void _pl_q_terminate (Plotter *_plotter);
/* HPGLPlotter/PCLPlotter internal functions, for libplot */
extern bool _pl_h_hpgl2_maybe_update_font (Plotter *_plotter);
extern bool _pl_h_hpgl_maybe_update_font (Plotter *_plotter);
extern bool _pl_h_parse_pen_string (Plotter *_plotter, const char *pen_s);
extern int _pl_h_hpgl_pseudocolor (Plotter *_plotter, int red, int green, int blue, bool restrict_white);
extern void _pl_h_hpgl_shaded_pseudocolor (Plotter *_plotter, int red, int green, int blue, int *pen, double *shading);
extern void _pl_h_set_attributes (Plotter *_plotter);
extern void _pl_h_set_fill_color (Plotter *_plotter, bool force_pen_color);
extern void _pl_h_set_font (Plotter *_plotter);
extern void _pl_h_set_hpgl_fill_type (Plotter *_plotter, int fill_type, double option1, double option2);
extern void _pl_h_set_hpgl_pen (Plotter *_plotter, int pen);
extern void _pl_h_set_hpgl_pen_type (Plotter *_plotter, int pen_type, double option1, double option2);
extern void _pl_h_set_pen_color (Plotter *_plotter, int hpgl_object_type);
extern void _pl_h_set_position (Plotter *_plotter);
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
extern void _pl_h_maybe_switch_to_hpgl (Plotter *_plotter); 
extern void _pl_h_maybe_switch_from_hpgl (Plotter *_plotter); 
/* PCLPlotter functions (overriding the above), for libplotter */
extern void _pl_q_maybe_switch_to_hpgl (Plotter *_plotter); 
extern void _pl_q_maybe_switch_from_hpgl (Plotter *_plotter); 
___END_DECLS
#else  /* LIBPLOTTER */
/* HPGLPlotter/PCLPlotter protected methods, for libplotter */
#define _pl_h_begin_page HPGLPlotter::begin_page
#define _pl_h_end_page HPGLPlotter::end_page
#define _pl_h_erase_page HPGLPlotter::erase_page
#define _pl_h_paint_text_string HPGLPlotter::paint_text_string
#define _pl_h_paint_path HPGLPlotter::paint_path
#define _pl_h_paint_paths HPGLPlotter::paint_paths
#define _pl_h_paint_point HPGLPlotter::paint_point
/* HPGLPlotter protected methods, for libplotter */
#define _pl_h_initialize HPGLPlotter::initialize
#define _pl_h_terminate HPGLPlotter::terminate
/* PCLPlotter protected methods, for libplotter */
#define _pl_q_initialize PCLPlotter::initialize
#define _pl_q_terminate PCLPlotter::terminate
/* HPGLPlotter/PCLPlotter internal functions, for libplotter */
#define _pl_h_hpgl2_maybe_update_font HPGLPlotter::_h_hpgl2_maybe_update_font
#define _pl_h_hpgl_maybe_update_font HPGLPlotter::_h_hpgl_maybe_update_font
#define _pl_h_hpgl_pseudocolor HPGLPlotter::_h_hpgl_pseudocolor
#define _pl_h_hpgl_shaded_pseudocolor HPGLPlotter::_h_hpgl_shaded_pseudocolor
#define _pl_h_parse_pen_string HPGLPlotter::_h_parse_pen_string
#define _pl_h_set_attributes HPGLPlotter::_h_set_attributes
#define _pl_h_set_fill_color HPGLPlotter::_h_set_fill_color
#define _pl_h_set_font HPGLPlotter::_h_set_font
#define _pl_h_set_hpgl_fill_type HPGLPlotter::_h_set_hpgl_fill_type
#define _pl_h_set_hpgl_pen HPGLPlotter::_h_set_hpgl_pen
#define _pl_h_set_hpgl_pen_type HPGLPlotter::_h_set_hpgl_pen_type
#define _pl_h_set_pen_color HPGLPlotter::_h_set_pen_color
#define _pl_h_set_position HPGLPlotter::_h_set_position
/* HPGLPlotter functions (overridden in PCLPlotter class), for libplotter */
#define _pl_h_maybe_switch_to_hpgl HPGLPlotter::_maybe_switch_to_hpgl
#define _pl_h_maybe_switch_from_hpgl HPGLPlotter::_maybe_switch_from_hpgl
/* PCLPlotter functions (overriding the above), for libplotter */
#define _pl_q_maybe_switch_to_hpgl PCLPlotter::_maybe_switch_to_hpgl
#define _pl_q_maybe_switch_from_hpgl PCLPlotter::_maybe_switch_from_hpgl
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* FigPlotter protected methods, for libplot */
extern bool _pl_f_begin_page (Plotter *_plotter);
extern bool _pl_f_end_page (Plotter *_plotter);
extern bool _pl_f_erase_page (Plotter *_plotter);
extern bool _pl_f_paint_paths (Plotter *_plotter);
extern bool _pl_f_retrieve_font (Plotter *_plotter);
extern double _pl_f_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_f_initialize (Plotter *_plotter);
extern void _pl_f_paint_path (Plotter *_plotter);
extern void _pl_f_paint_point (Plotter *_plotter);
extern void _pl_f_terminate (Plotter *_plotter);
/* FigPlotter internal functions, for libplot */
extern int _pl_f_fig_color (Plotter *_plotter, int red, int green, int blue);
extern void _pl_f_compute_line_style (Plotter *_plotter, int *style, double *spacing);
extern void _pl_f_draw_arc_internal (Plotter *_plotter, double xc, double yc, double x0, double y0, double x1, double y1);
extern void _pl_f_draw_box_internal (Plotter *_plotter, plPoint p0, plPoint p1);
extern void _pl_f_draw_ellipse_internal (Plotter *_plotter, double x, double y, double rx, double ry, double angle, int subtype);
extern void _pl_f_set_fill_color (Plotter *_plotter);
extern void _pl_f_set_pen_color (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* FigPlotter protected methods, for libplotter */
#define _pl_f_begin_page FigPlotter::begin_page
#define _pl_f_end_page FigPlotter::end_page
#define _pl_f_erase_page FigPlotter::erase_page
#define _pl_f_paint_text_string FigPlotter::paint_text_string
#define _pl_f_initialize FigPlotter::initialize
#define _pl_f_paint_path FigPlotter::paint_path
#define _pl_f_paint_paths FigPlotter::paint_paths
#define _pl_f_paint_point FigPlotter::paint_point
#define _pl_f_retrieve_font FigPlotter::retrieve_font
#define _pl_f_terminate FigPlotter::terminate
/* FigPlotter internal functions, for libplotter */
#define _pl_f_compute_line_style FigPlotter::_f_compute_line_style
#define _pl_f_draw_arc_internal FigPlotter::_f_draw_arc_internal
#define _pl_f_draw_box_internal FigPlotter::_f_draw_box_internal
#define _pl_f_draw_ellipse_internal FigPlotter::_f_draw_ellipse_internal
#define _pl_f_fig_color FigPlotter::_f_fig_color
#define _pl_f_set_fill_color FigPlotter::_f_set_fill_color
#define _pl_f_set_pen_color FigPlotter::_f_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* CGMPlotter protected methods, for libplot */
extern bool _pl_c_begin_page (Plotter *_plotter);
extern bool _pl_c_end_page (Plotter *_plotter);
extern bool _pl_c_erase_page (Plotter *_plotter);
extern bool _pl_c_paint_marker (Plotter *_plotter, int type, double size);
extern bool _pl_c_paint_paths (Plotter *_plotter);
extern double _pl_c_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_c_initialize (Plotter *_plotter);
extern void _pl_c_paint_path (Plotter *_plotter);
extern void _pl_c_paint_point (Plotter *_plotter);
extern void _pl_c_terminate (Plotter *_plotter);
/* CGMPlotter internal functions, for libplot */
extern void _pl_c_set_attributes (Plotter *_plotter, int cgm_object_type);
extern void _pl_c_set_bg_color (Plotter *_plotter);
extern void _pl_c_set_fill_color (Plotter *_plotter, int cgm_object_type);
extern void _pl_c_set_pen_color (Plotter *_plotter, int cgm_object_type);
___END_DECLS
#else  /* LIBPLOTTER */
/* CGMPlotter protected methods, for libplotter */
#define _pl_c_begin_page CGMPlotter::begin_page
#define _pl_c_end_page CGMPlotter::end_page
#define _pl_c_erase_page CGMPlotter::erase_page
#define _pl_c_paint_text_string CGMPlotter::paint_text_string
#define _pl_c_initialize CGMPlotter::initialize
#define _pl_c_paint_marker CGMPlotter::paint_marker
#define _pl_c_paint_path CGMPlotter::paint_path
#define _pl_c_paint_paths CGMPlotter::paint_paths
#define _pl_c_paint_point CGMPlotter::paint_point
#define _pl_c_terminate CGMPlotter::terminate
/* CGMPlotter internal functions, for libplotter */
#define _pl_c_set_attributes CGMPlotter::_c_set_attributes
#define _pl_c_set_bg_color CGMPlotter::_c_set_bg_color
#define _pl_c_set_fill_color CGMPlotter::_c_set_fill_color
#define _pl_c_set_pen_color CGMPlotter::_c_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* PSPlotter protected methods, for libplot */
extern bool _pl_p_begin_page (Plotter *_plotter);
extern bool _pl_p_end_page (Plotter *_plotter);
extern bool _pl_p_erase_page (Plotter *_plotter);
extern bool _pl_p_paint_paths (Plotter *_plotter);
extern double _pl_p_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_p_initialize (Plotter *_plotter);
extern void _pl_p_paint_path (Plotter *_plotter);
extern void _pl_p_paint_point (Plotter *_plotter);
extern void _pl_p_terminate (Plotter *_plotter);
/* PSPlotter internal functions, for libplot */
extern double _pl_p_emit_common_attributes (Plotter *_plotter);
extern void _pl_p_compute_idraw_bgcolor (Plotter *_plotter);
extern void _pl_p_fellipse_internal (Plotter *_plotter, double x, double y, double rx, double ry, double angle, bool circlep);
extern void _pl_p_set_fill_color (Plotter *_plotter);
extern void _pl_p_set_pen_color (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* PSPlotter protected methods, for libplotter */
#define _pl_p_begin_page PSPlotter::begin_page
#define _pl_p_end_page PSPlotter::end_page
#define _pl_p_erase_page PSPlotter::erase_page
#define _pl_p_paint_text_string PSPlotter::paint_text_string
#define _pl_p_initialize PSPlotter::initialize
#define _pl_p_paint_path PSPlotter::paint_path
#define _pl_p_paint_paths PSPlotter::paint_paths
#define _pl_p_paint_point PSPlotter::paint_point
#define _pl_p_terminate PSPlotter::terminate
/* PSPlotter internal functions, for libplotter */
#define _pl_p_compute_idraw_bgcolor PSPlotter::_p_compute_idraw_bgcolor
#define _pl_p_emit_common_attributes PSPlotter::_p_emit_common_attributes
#define _pl_p_fellipse_internal PSPlotter::_p_fellipse_internal
#define _pl_p_set_fill_color PSPlotter::_p_set_fill_color
#define _pl_p_set_pen_color PSPlotter::_p_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* AIPlotter protected methods, for libplot */
extern bool _pl_a_begin_page (Plotter *_plotter);
extern bool _pl_a_end_page (Plotter *_plotter);
extern bool _pl_a_erase_page (Plotter *_plotter);
extern bool _pl_a_paint_paths (Plotter *_plotter);
extern double _pl_a_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_a_initialize (Plotter *_plotter);
extern void _pl_a_paint_path (Plotter *_plotter);
extern void _pl_a_paint_point (Plotter *_plotter);
extern void _pl_a_terminate (Plotter *_plotter);
/* AIPlotter internal functions, for libplot */
extern void _pl_a_set_attributes (Plotter *_plotter);
extern void _pl_a_set_fill_color (Plotter *_plotter, bool force_pen_color);
extern void _pl_a_set_pen_color (Plotter *_plotter);
___END_DECLS
#else /* LIBPLOTTER */
/* AIPlotter protected methods, for libplotter */
#define _pl_a_begin_page AIPlotter::begin_page
#define _pl_a_end_page AIPlotter::end_page
#define _pl_a_erase_page AIPlotter::erase_page
#define _pl_a_paint_text_string AIPlotter::paint_text_string
#define _pl_a_initialize AIPlotter::initialize
#define _pl_a_paint_path AIPlotter::paint_path
#define _pl_a_paint_paths AIPlotter::paint_paths
#define _pl_a_paint_point AIPlotter::paint_point
#define _pl_a_terminate AIPlotter::terminate
/* AIPlotter internal functions, for libplotter */
#define _pl_a_set_attributes AIPlotter::_a_set_attributes
#define _pl_a_set_fill_color AIPlotter::_a_set_fill_color
#define _pl_a_set_pen_color AIPlotter::_a_set_pen_color
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* SVGPlotter protected methods, for libplot */
extern bool _pl_s_begin_page (Plotter *_plotter);
extern bool _pl_s_end_page (Plotter *_plotter);
extern bool _pl_s_erase_page (Plotter *_plotter);
extern bool _pl_s_paint_paths (Plotter *_plotter);
extern double _pl_s_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern void _pl_s_initialize (Plotter *_plotter);
extern void _pl_s_paint_path (Plotter *_plotter);
extern void _pl_s_paint_point (Plotter *_plotter);
extern void _pl_s_terminate (Plotter *_plotter);
/* PSPlotter internal functions, for libplot */
extern void _pl_s_set_matrix (Plotter *_plotter, const double m_local[6]);
___END_DECLS
#else  /* LIBPLOTTER */
/* SVGPlotter protected methods, for libplotter */
#define _pl_s_begin_page SVGPlotter::begin_page
#define _pl_s_end_page SVGPlotter::end_page
#define _pl_s_erase_page SVGPlotter::erase_page
#define _pl_s_paint_text_string SVGPlotter::paint_text_string
#define _pl_s_initialize SVGPlotter::initialize
#define _pl_s_paint_path SVGPlotter::paint_path
#define _pl_s_paint_paths SVGPlotter::paint_paths
#define _pl_s_paint_point SVGPlotter::paint_point
#define _pl_s_terminate SVGPlotter::terminate
/* SVGPlotter internal functions, for libplotter */
#define _pl_s_set_matrix SVGPlotter::_s_set_matrix
#endif /* LIBPLOTTER */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* PNMPlotter protected methods, for libplot */
extern void _pl_n_initialize (Plotter *_plotter);
extern void _pl_n_terminate (Plotter *_plotter);
/* PNMPlotter internal functions (which override BitmapPlotter functions) */
extern int _pl_n_maybe_output_image (Plotter *_plotter);
/* other PNMPlotter internal functions, for libplot */
extern void _pl_n_write_pnm (Plotter *_plotter);
extern void _pl_n_write_pbm (Plotter *_plotter);
extern void _pl_n_write_pgm (Plotter *_plotter);
extern void _pl_n_write_ppm (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* PNMPlotter protected methods, for libplotter */
#define _pl_n_initialize PNMPlotter::initialize
#define _pl_n_terminate PNMPlotter::terminate
/* PNMPlotter internal methods (which override BitmapPlotter methods) */
#define _pl_n_maybe_output_image PNMPlotter::_maybe_output_image
/* other PNMPlotter internal functions, for libplotter */
#define _pl_n_write_pnm PNMPlotter::_n_write_pnm
#define _pl_n_write_pbm PNMPlotter::_n_write_pbm
#define _pl_n_write_pgm PNMPlotter::_n_write_pgm
#define _pl_n_write_ppm PNMPlotter::_n_write_ppm
#endif /* LIBPLOTTER */

#ifdef INCLUDE_PNG_SUPPORT
#ifndef LIBPLOTTER
___BEGIN_DECLS
/* PNGPlotter protected methods, for libplot */
extern void _pl_z_initialize (Plotter *_plotter);
extern void _pl_z_terminate (Plotter *_plotter);
/* PNGPlotter internal functions (which override BitmapPlotter functions) */
extern int _pl_z_maybe_output_image (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* PNGPlotter protected methods, for libplotter */
#define _pl_z_initialize PNGPlotter::initialize
#define _pl_z_terminate PNGPlotter::terminate
/* PNGPlotter internal methods (which override BitmapPlotter methods) */
#define _pl_z_maybe_output_image PNGPlotter::_maybe_output_image
#endif /* LIBPLOTTER */
#endif /* INCLUDE_PNG_SUPPORT */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* GIFPlotter protected methods, for libplot */
extern bool _pl_i_begin_page (Plotter *_plotter);
extern bool _pl_i_end_page (Plotter *_plotter);
extern bool _pl_i_erase_page (Plotter *_plotter);
extern bool _pl_i_paint_paths (Plotter *_plotter);
extern void _pl_i_initialize (Plotter *_plotter);
extern void _pl_i_paint_path (Plotter *_plotter);
extern void _pl_i_paint_point (Plotter *_plotter);
extern void _pl_i_terminate (Plotter *_plotter);
/* GIFPlotter internal functions, for libplot */
extern int _pl_i_scan_pixel (Plotter *_plotter);
extern unsigned char _pl_i_new_color_index (Plotter *_plotter, int red, int green, int blue);
extern void _pl_i_delete_image (Plotter *_plotter);
extern void _pl_i_draw_elliptic_arc (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_i_draw_elliptic_arc_2 (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_i_draw_elliptic_arc_internal (Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
extern void _pl_i_new_image (Plotter *_plotter);
extern void _pl_i_set_bg_color (Plotter *_plotter);
extern void _pl_i_set_fill_color (Plotter *_plotter);
extern void _pl_i_set_pen_color (Plotter *_plotter);
extern void _pl_i_start_scan (Plotter *_plotter);
extern void _pl_i_write_gif_header (Plotter *_plotter);
extern void _pl_i_write_gif_image (Plotter *_plotter);
extern void _pl_i_write_gif_trailer (Plotter *_plotter);
extern void _pl_i_write_short_int (Plotter *_plotter, unsigned int i);
___END_DECLS
#else  /* LIBPLOTTER */
/* GIFPlotter protected methods, for libplotter */
#define _pl_i_begin_page GIFPlotter::begin_page
#define _pl_i_end_page GIFPlotter::end_page
#define _pl_i_erase_page GIFPlotter::erase_page
#define _pl_i_initialize GIFPlotter::initialize
#define _pl_i_paint_path GIFPlotter::paint_path
#define _pl_i_paint_paths GIFPlotter::paint_paths
#define _pl_i_paint_point GIFPlotter::paint_point
#define _pl_i_terminate GIFPlotter::terminate
/* GIFPlotter internal functions, for libplotter */
#define _pl_i_scan_pixel GIFPlotter::_i_scan_pixel
#define _pl_i_new_color_index GIFPlotter::_i_new_color_index
#define _pl_i_delete_image GIFPlotter::_i_delete_image
#define _pl_i_draw_elliptic_arc GIFPlotter::_i_draw_elliptic_arc
#define _pl_i_draw_elliptic_arc_2 GIFPlotter::_i_draw_elliptic_arc_2
#define _pl_i_draw_elliptic_arc_internal GIFPlotter::_i_draw_elliptic_arc_internal
#define _pl_i_new_image GIFPlotter::_i_new_image 
#define _pl_i_set_bg_color GIFPlotter::_i_set_bg_color
#define _pl_i_set_fill_color GIFPlotter::_i_set_fill_color
#define _pl_i_set_pen_color GIFPlotter::_i_set_pen_color
#define _pl_i_start_scan GIFPlotter::_i_start_scan
#define _pl_i_write_gif_header GIFPlotter::_i_write_gif_header
#define _pl_i_write_gif_image GIFPlotter::_i_write_gif_image
#define _pl_i_write_gif_trailer GIFPlotter::_i_write_gif_trailer
#define _pl_i_write_short_int GIFPlotter::_i_write_short_int
#endif /* LIBPLOTTER */

#ifndef X_DISPLAY_MISSING
#ifndef LIBPLOTTER
___BEGIN_DECLS
/* XDrawablePlotter/XPlotter protected methods, for libplot */
extern bool _pl_x_begin_page (Plotter *_plotter);
extern bool _pl_x_end_page (Plotter *_plotter);
extern bool _pl_x_erase_page (Plotter *_plotter);
extern bool _pl_x_flush_output (Plotter *_plotter);
extern bool _pl_x_paint_paths (Plotter *_plotter);
extern bool _pl_x_path_is_flushable (Plotter *_plotter);
extern bool _pl_x_retrieve_font (Plotter *_plotter);
extern double _pl_x_paint_text_string (Plotter *_plotter, const unsigned char *s, int h_just, int v_just);
extern double _pl_x_get_text_width (Plotter *_plotter, const unsigned char *s);
extern void _pl_x_initialize (Plotter *_plotter);
extern void _pl_x_maybe_prepaint_segments (Plotter *_plotter, int prev_num_segments);
extern void _pl_x_paint_path (Plotter *_plotter);
extern void _pl_x_paint_point (Plotter *_plotter);
extern void _pl_x_pop_state (Plotter *_plotter);
extern void _pl_x_push_state (Plotter *_plotter);
extern void _pl_x_terminate (Plotter *_plotter);
/* XDrawablePlotter/XPlotter internal functions, for libplot */
extern bool _pl_x_retrieve_color (Plotter *_plotter, XColor *rgb_ptr);
extern bool _pl_x_select_font_carefully (Plotter *_plotter, const char *name, const unsigned char *s, bool subsetting);
extern bool _pl_x_select_xlfd_font_carefully (Plotter *_plotter, const char *x_name, const char *x_name_alt, const char *x_name_alt2, const char *x_name_alt3);
extern void _pl_x_add_gcs_to_first_drawing_state (Plotter *_plotter);
extern void _pl_x_delete_gcs_from_first_drawing_state (Plotter *_plotter);
extern void _pl_x_draw_elliptic_arc (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_x_draw_elliptic_arc_2 (Plotter *_plotter, plPoint p0, plPoint p1, plPoint pc);
extern void _pl_x_draw_elliptic_arc_internal (Plotter *_plotter, int xorigin, int yorigin, unsigned int squaresize_x, unsigned int squaresize_y, int startangle, int anglerange);
extern void _pl_x_set_attributes (Plotter *_plotter, int x_gc_type);
extern void _pl_x_set_bg_color (Plotter *_plotter);
extern void _pl_x_set_fill_color (Plotter *_plotter);
extern void _pl_x_set_pen_color (Plotter *_plotter);
/* XDrawablePlotter internal functions, for libplot */
extern void _pl_x_maybe_get_new_colormap (Plotter *_plotter);
extern void _pl_x_maybe_handle_x_events (Plotter *_plotter);
/* XPlotter protected methods, for libplot */
extern bool _pl_y_begin_page (Plotter *_plotter);
extern bool _pl_y_end_page (Plotter *_plotter);
extern bool _pl_y_erase_page (Plotter *_plotter);
extern void _pl_y_initialize (Plotter *_plotter);
extern void _pl_y_terminate (Plotter *_plotter);
/* XPlotter internal functions, for libplot */
extern void _pl_y_flush_plotter_outstreams (Plotter *_plotter);
extern void _pl_y_maybe_get_new_colormap (Plotter *_plotter);
extern void _pl_y_maybe_handle_x_events (Plotter *_plotter);
extern void _pl_y_set_data_for_quitting (Plotter *_plotter);
___END_DECLS
#else  /* LIBPLOTTER */
/* XDrawablePlotter/XPlotter protected methods, for libplotter */
#define _pl_x_begin_page XDrawablePlotter::begin_page
#define _pl_x_end_page XDrawablePlotter::end_page
#define _pl_x_erase_page XDrawablePlotter::erase_page
#define _pl_x_paint_text_string XDrawablePlotter::paint_text_string
#define _pl_x_get_text_width XDrawablePlotter::get_text_width
#define _pl_x_flush_output XDrawablePlotter::flush_output
#define _pl_x_path_is_flushable XDrawablePlotter::path_is_flushable
#define _pl_x_maybe_prepaint_segments XDrawablePlotter::maybe_prepaint_segments
#define _pl_x_paint_path XDrawablePlotter::paint_path
#define _pl_x_paint_paths XDrawablePlotter::paint_paths
#define _pl_x_paint_point XDrawablePlotter::paint_point
#define _pl_x_pop_state XDrawablePlotter::pop_state
#define _pl_x_push_state XDrawablePlotter::push_state
#define _pl_x_retrieve_font XDrawablePlotter::retrieve_font
/* XDrawablePlotter protected methods (overridden in XPlotter class) */
#define _pl_x_initialize XDrawablePlotter::initialize
#define _pl_x_terminate XDrawablePlotter::terminate
/* XPlotter protected methods (which override the preceding) */
#define _pl_y_begin_page XPlotter::begin_page
#define _pl_y_end_page XPlotter::end_page
#define _pl_y_erase_page XPlotter::erase_page
#define _pl_y_initialize XPlotter::initialize
#define _pl_y_terminate XPlotter::terminate
/* XDrawablePlotter/XPlotter internal functions, for libplotter */
#define _pl_x_add_gcs_to_first_drawing_state XDrawablePlotter::_x_add_gcs_to_first_drawing_state
#define _pl_x_delete_gcs_from_first_drawing_state XDrawablePlotter::_x_delete_gcs_from_first_drawing_state
#define _pl_x_draw_elliptic_arc XDrawablePlotter::_x_draw_elliptic_arc
#define _pl_x_draw_elliptic_arc_2 XDrawablePlotter::_x_draw_elliptic_arc_2
#define _pl_x_draw_elliptic_arc_internal XDrawablePlotter::_x_draw_elliptic_arc_internal
#define _pl_x_retrieve_color XDrawablePlotter::_x_retrieve_color
#define _pl_x_select_font XDrawablePlotter::_x_select_font
#define _pl_x_select_font_carefully XDrawablePlotter::_x_select_font_carefully
#define _pl_x_select_xlfd_font_carefully XDrawablePlotter::_x_select_xlfd_font_carefully
#define _pl_x_set_attributes XDrawablePlotter::_x_set_attributes
#define _pl_x_set_bg_color XDrawablePlotter::_x_set_bg_color
#define _pl_x_set_fill_color XDrawablePlotter::_x_set_fill_color
#define _pl_x_set_font_dimensions XDrawablePlotter::_x_set_font_dimensions
#define _pl_x_set_pen_color XDrawablePlotter::_x_set_pen_color
/* XDrawablePlotter internal functions (overridden in XPlotter class) */
#define _pl_x_maybe_get_new_colormap XDrawablePlotter::_maybe_get_new_colormap
#define _pl_x_maybe_handle_x_events XDrawablePlotter::_maybe_handle_x_events
/* XPlotter internal functions (which override the preceding) */
#define _pl_y_maybe_get_new_colormap XPlotter::_maybe_get_new_colormap
#define _pl_y_maybe_handle_x_events XPlotter::_maybe_handle_x_events
/* other XPlotter internal functions, for libplotter */
#define _pl_y_flush_plotter_outstreams XPlotter::_y_flush_plotter_outstreams
#define _pl_y_set_data_for_quitting XPlotter::_y_set_data_for_quitting
#endif  /* LIBPLOTTER */
#endif /* not X_DISPLAY_MISSING */

/* Declarations of the PlotterParams methods.  In libplot, these are
   declarations of global functions.  But in libplotter, we use #define and
   the double colon notation to make them function members of the
   PlotterParams class.

   The ___BEGIN_DECLS...___END_DECLS is to support compilation of libplot
   by a C++ compiler; see the remarks above. */

#ifndef LIBPLOTTER
___BEGIN_DECLS
/* PlotterParams public methods, for libplot */
extern int _setplparam (PlotterParams *_plotter_params, const char *parameter, void * value);
extern int _pushplparams (PlotterParams *_plotter_params);
extern int _popplparams (PlotterParams *_plotter_params);
___END_DECLS
#else /* LIBPLOTTER */
/* PlotterParams public methods, for libplotter */
#define _setplparam PlotterParams::setplparam
#define _pushplparams PlotterParams::pushplparams
#define _popplparams PlotterParams::popplparams
#endif /* LIBPLOTTER */
