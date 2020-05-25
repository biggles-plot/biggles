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

/* This header file gives the metrics for the vector fonts. */

/* Our choice for stroke width, in terms of virtual pixels.  This number is
   magic: just slightly greater than sqrt(2), so that adjacent strokes that
   are inclined at a 45 degree angle will overlap as they should.

   (According to ``Calligraphy for Computers'', the Hershey fonts were
   designed to be drawn by an electron beam the intensity of which fell to
   50% at a transverse displacement of one unit.) */

#define HERSHEY_STROKE_WIDTH 1.42

/* This value gives good results for the Japanese characters (Kana and
   Kanji). */
#define HERSHEY_ORIENTAL_STROKE_WIDTH 1.175

/* According to Allen Hershey, 1 em = 32 virtual pixels for his alphabets
   of principal size.  But taking into account the width of the strokes
   (nominally 1 virtual pixel), if the characters are thought of as resting
   on a baseline, the baseline (and the capline, etc.) should be located at
   half-integer values of the vertical coordinate.  That changes things
   slightly (an em should be 33 virtual pixels, not 32).

   Incidentally his recommended spacing between lines is 40 pixels (24 for
   indexical size), i.e., 1.2 em or so (1 em is the minimum possible
   spacing).
*/

/* Dimensions for characters in principal [large] size.  The `centerline'
   is located at y=0, in the original coordinate system.  The `topline' and
   `bottomline' are determined by the tallest characters, which are
   parentheses, brackets, and braces. */

#define HERSHEY_LARGE_BASELINE (-9.5)	/* relative to centerline */
#define HERSHEY_LARGE_CAPLINE 12.5	/* relative to centerline */
#define HERSHEY_LARGE_TOPLINE 16.5	/* relative to centerline */
#define HERSHEY_LARGE_BOTTOMLINE -16.5 /* relative to centerline */

#define HERSHEY_LARGE_CAPHEIGHT 22 /* i.e. capline - baseline */
#define HERSHEY_LARGE_ASCENT 26	/* i.e. topline - baseline */
#define HERSHEY_LARGE_DESCENT 7	/* i.e. baseline - bottomline */
#define HERSHEY_LARGE_HEIGHT (HERSHEY_LARGE_ASCENT + HERSHEY_LARGE_DESCENT)
#define HERSHEY_LARGE_EM 33

/* Dimensions for characters in indexical [medium] size.  The `centerline'
   is located at y=0, in the original coordinate system.  The `topline' and
   `bottomline' are determined by the tallest characters, which are
   parentheses, brackets, and braces. */

#define HERSHEY_MEDIUM_BASELINE (-6.5)	/* relative to centerline */
#define HERSHEY_MEDIUM_CAPLINE 7.5	/* relative to centerline */
#define HERSHEY_MEDIUM_TOPLINE 10.5	/* relative to centerline */
#define HERSHEY_MEDIUM_BOTTOMLINE -10.5 /* relative to centerline */

#define HERSHEY_MEDIUM_CAPHEIGHT 14	/* i.e. capline - baseline */
#define HERSHEY_MEDIUM_ASCENT 17	/* i.e. topline - baseline */
#define HERSHEY_MEDIUM_DESCENT 4	/* i.e. baseline - bottomline */
#define HERSHEY_MEDIUM_HEIGHT (HERSHEY_MEDIUM_ASCENT + HERSHEY_MEDIUM_DESCENT)
#define HERSHEY_MEDIUM_EM 21

/* Dimensions for characters in cartographic [small] size.  The
   `centerline' is located at y=0, in the original coordinate system.  The
   `topline' and `bottomline' are determined by the tallest characters,
   which are parentheses, brackets, and braces.  In the cartographic size
   there are only parentheses, and unlike the other two sizes they are not
   symmetric about y=0 (since they will surround only upper-case letters;
   there are no lower-case letters in cartographic). */

#define HERSHEY_SMALL_BASELINE (-4.5)	/* relative to centerline */
#define HERSHEY_SMALL_CAPLINE 5.5	/* relative to centerline */
#define HERSHEY_SMALL_TOPLINE 6.5	/* relative to centerline */
#define HERSHEY_SMALL_BOTTOMLINE -5.5 /* relative to centerline */

#define HERSHEY_SMALL_CAPHEIGHT 10	/* i.e. capline - baseline */
#define HERSHEY_SMALL_ASCENT 11	/* i.e. topline - baseline */
#define HERSHEY_SMALL_DESCENT 1	/* i.e. baseline - bottomline */
#define HERSHEY_SMALL_HEIGHT (HERSHEY_SMALL_ASCENT + HERSHEY_SMALL_DESCENT)
#define HERSHEY_SMALL_EM 12

/* Vertical positionings (in alabel_str.c) are now based on the assumption
   that all characters we are dealing with are of principal [large] size.
   I see no graceful way to handle positionings relative to the baseline
   for the other two sizes.  Of course, centered positioning will work
   perfectly, since the Hershey glyphs were designed for that. */

#define HERSHEY_BASELINE HERSHEY_LARGE_BASELINE
#define HERSHEY_CAPHEIGHT HERSHEY_LARGE_CAPHEIGHT
#define HERSHEY_ASCENT HERSHEY_LARGE_ASCENT
#define HERSHEY_DESCENT HERSHEY_LARGE_DESCENT
#define HERSHEY_HEIGHT HERSHEY_LARGE_HEIGHT
#define HERSHEY_EM HERSHEY_LARGE_EM

/* The scaling between distances in Hershey units and distances in user
   coordinates.  Idea is that the font size (i.e. the nominal minimum
   inter-line spacing) corresponds to HERSHEY_LARGE_EM Hershey units. */

#define HERSHEY_UNITS_TO_USER_UNITS(size) \
	((size)*(_plotter->drawstate->true_font_size)/(HERSHEY_EM))


/************************************************************************/

/* Some miscellaneous information on typesetting mathematics, taken from
   Allen Hershey's 1969 TR (see g_her_glyph.c):

   Subscripts and superscripts, in math text, should be in indexical size.
   The centerline of subscripts/superscripts would be lowered/raised by 10
   vertical units.  

   [In principal size the centerline is 9.5 units above the baseline; in
   indexical size the centerline is 6.5 units above the baseline.  So when
   going to subscripts, the baseline should be lowered by 7 units; when
   going to superscripts, the baseline should be raised by 13 units.  This
   is not actually the scheme we use; see alabel_str.c.  -- rsm]

   In math text the quantity being supplied with a sub/superscript is
   typically an italic character.  The transitions principal->superscript
   and subscript->principal are accordingly accompanied by 2 add'l units of
   horizontal space.

   In math text, conjuctive/predicative signs should be given a extra
   spacing of 1/2 en (i.e. 1/4 em), i.e. 8 units in principal size, to
   either side.  This is accomplished by glyph 2198 (or 1198, in indexical
   size).

   Case fractions (e.g. \frac34) are formed by raising/lowering
   indexical-size characters by 12 vertical units; in mathematical text,
   they should be given a small (4-unit) spacing to either side.

   Simple limits, for sum and integral signs, are in indexical size, and
   are raised/lowered by 24 units. */

/************************************************************************/
