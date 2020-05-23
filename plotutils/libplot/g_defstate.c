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

/* Initialization for the first drawing state on the stack of drawing
   states maintained by any Plotter.  Its components include drawing
   attributes, and the state of any uncompleted path object.  (At
   initialization, there is none.) */
   
/* This is copied to the first state on the stack, in g_savestate.c.  The
   four fields, `font_name', `font_type', `typeface_index', and
   `font_index' are special: they are filled in at that time, since they
   are Plotter-dependent.  So the values for them below (respectively
   "HersheySerif", PL_F_HERSHEY, 0, and 1) are really dummies. */

/* Two other fields (font size and line width in user coordinates) play an
   important role at later times, e.g. a bad font size resets the font size
   to the default.  For that reason, those variables are filled in when
   space() or fsetmatrix() is called (see g_concat.c).  They are computed
   using the two quantities PL_DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE
   and PL_DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE (defined in
   extern.h). */

#include "sys-defines.h"
#include "extern.h"

/* save keystrokes */
#define DFSAFODS PL_DEFAULT_FONT_SIZE_AS_FRACTION_OF_DISPLAY_SIZE
#define DLWAFODS PL_DEFAULT_LINE_WIDTH_AS_FRACTION_OF_DISPLAY_SIZE

const plDrawState _default_drawstate = {

/***************** DEVICE-INDEPENDENT PART **************************/

/* graphics cursor position */
  {0.0, 0.0},			/* cursor position, in user coordinates */

/* affine transformation from user coordinates to normalized device
   coordinates, and affine transformation to actual device coordinates
   (derived from it) */
  {
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* user->NDC transformation matrix */
    {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, /* user->device transformation [dummy] */
    true,			/* transf. scaling is uniform? [dummy] */
    true,			/* transf. preserves axis dirs? [dummy] */
    true			/* transf. doesn't reflect? [dummy] */
  },

/* the compound path being drawn, if any */
  (plPath *)NULL,		/* simple path being drawn */
  (plPath **)NULL,		/* previously drawn simple paths */
  0,				/* number of previously drawn simple paths */
  {0.0, 0.0},			/* starting point (used by closepath()) */

/* modal drawing attributes */
  /* 1. path-related attributes */
  "even-odd",			/* fill mode ["even-odd" / "nonzero-winding"]*/
  PL_FILL_ODD_WINDING,		/* fill type, one of PL_FILL_*, det'd by mode */
  "solid",			/* line mode [must be valid] */
  PL_L_SOLID,			/* line type, one of L_*, det'd by line mode */
  true,				/* if not set, paths are "disconnected" */
  "butt",			/* cap mode [must be valid] */
  PL_CAP_BUTT,			/* cap type, one of PL_CAP_*, det'd by cap mode */
  "miter",			/* join mode [must be valid] */
  PL_JOIN_MITER,	        /* join type, one of PL_JOIN_*, det'd by mode */
  PL_DEFAULT_MITER_LIMIT,		/* miter limit for line joins */
  DLWAFODS,			/* line width in user coors [set by space()] */
  true,				/* line width is (Plotter-specific) default? */
  1.0,				/* line width in device coordinates ["] */
  1,				/* line width, quantized to integer ["] */
  (double *)NULL,		/* array of dash on/off lengths */
  0,				/* length of same */
  0.0,				/* offset distance into dash array (`phase') */
  false,			/* dash array should override line mode? */
  1,				/* pen type (0 = none, 1 = present) */
  0,				/* fill type (0 = none, 1 = present,...) */
  1,				/* orientation of circles etc.(1=c'clockwise)*/
  /* 2. text-related attributes */
  "HersheySerif",		/* font name [dummy, see g_openpl.c] */
  DFSAFODS,			/* font size in user coordinates [dummy] */
  true,				/* font size is (Plotter-specific) default? */
  0.0,				/* degrees counterclockwise, for labels */
  "HersheySerif",		/* true font name [dummy] */
  0.0,				/* true font size in user coordinates (") */  
  0.0,				/* font ascent in user coordinates (") */
  0.0,				/* font descent in user coordinates (") */
  0.0,				/* font capital height in user coors (") */
  PL_F_HERSHEY,			/* font type [dummy] */
  0,				/* typeface index (in fontdb.h typeface table; this is Hershey Serif typeface) [dummy] */
  1,				/* font index (within typeface; this is Roman variant of Hershey Serif typeface) [dummy] */
  true,				/* true means an ISO-Latin-1 font ["] */
  /* 3. color attributes (fgcolor and fillcolor are path-related; fgcolor
     affects other primitives too) */
  {0, 0, 0},			/* fg color, i.e., pen color (= black) */
  {0, 0, 0},			/* base fill color (= black) */
  {0, 0, 0},			/* true fill color (= black) */
  {65535, 65535, 65535},	/* background color for display (= white) */
  false,			/* no actual background color? */

/* Default values for certain modal attributes, used when an out-of-range
   value is requested. (These two are special because unlike all others,
   they're set by the initial call to space() or fsetmatrix(), which also
   sets the line width and font size fields above. Incidentally, space()
   and setmatrix() also invoke linewidth().) */
  0.0,				/* default line width in user coordinates */
  0.0,				/* default font size in user coordinates */

/****************** DEVICE-DEPENDENT PART ***************************/

/* elements specific to the HP-GL drawing state [DUMMY] */
  0.001,			/* pen width (frac of diag dist betw P1,P2) */
/* elements specific to the Fig drawing state [DUMMIES] */
  16,				/* font size in fig's idea of points */
  -1,				/* fig's fill level (-1 = transparent) */
  FIG_C_BLACK,			/* fig's fg color (0=black) */
  FIG_C_BLACK,			/* fig's fill color */
/* elements specific to the PS drawing state [DUMMIES] */
  0.0,				/* RGB for PS fg color (floats) */
  0.0,
  0.0,
  1.0,				/* RGB for PS fill color (floats) */
  1.0,
  1.0,
  0,				/* idraw fg color (0=black, 9=white) */
  9,				/* idraw bg color (0=black, 9=white) */
  0,				/* shading (0=fg, 4=bg), if not transparent */
/* elements specific to the GIF drawing state [all save last 3 are DUMMIES] */
  {0, 0, 0},			/* 24-bit RGB of pixel for drawing (= black) */
  {0, 0, 0},			/* 24-bit RGB of pixel for filling (= black) */
  {255, 255, 255},		/* 24-bit RGB of pixel for erasing (= white) */
  (unsigned char)0,		/* drawing color index [dummy] */
  (unsigned char)0,		/* filling color index [dummy] */
  (unsigned char)0,		/* erasing color index [dummy] */
  false,			/* drawing color index is genuine? */
  false,			/* filling color index is genuine? */
  false,			/* erasing color index is genuine? */
#ifndef X_DISPLAY_MISSING
/* elements spec. to X11, X11 Drawable drawingstates [nearly all: DUMMY] */
  14,				/* font pixel size */
  (XFontStruct *)NULL,		/* font structure (used in x_alab_X.c) */
  (const unsigned char *)NULL,	/* label (hint to font retrieval routine) */
  (GC)NULL,			/* graphics context, for drawing */
  (GC)NULL,			/* graphics context, for filling */
  (GC)NULL,			/* graphics context, for erasing */
  {0, 0, 0},			/* pen color stored in GC (= black) */
  {0, 0, 0},			/* fill color stored in GC (= black) */
  {65535, 65535, 65535},	/* bg color stored in GC (= white) */
  (unsigned long)0,		/* drawing pixel [dummy] */
  (unsigned long)0,		/* filling pixel [dummy] */
  (unsigned long)0,		/* erasing pixel [dummy] */
  false,			/* drawing pixel is genuine? */
  false,			/* filling pixel is genuine? */
  false,			/* erasing pixel is genuine? */
  LineSolid,			/* line style stored in drawing GC */
  CapButt,			/* cap style stored in drawing GC */
  JoinMiter,			/* join style stored in drawing GC */
  0,				/* line width stored in drawing GC */
  (char *)NULL,			/* dash list stored in drawing GC */
  0,				/* length of dash list stored in drawing GC */
  0,				/* offset into dash sequence, in drawing GC */
  EvenOddRule,			/* fill rule stored in filling GC */
#endif /* X_DISPLAY_MISSING */

/* pointer to previous drawing state */
  (plDrawState *)NULL		/* pointer to previous state [must be null] */
};
