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

/* This file defines GNU libplot's builtin line styles.  A line style is
   specified by invoking the linemod() operation.  The supported line
   styles are a superset of the line styles of traditional (Unix) libplot.

   Unix libplot originated at Bell Labs in the early 1970's, and the first
   supported display device was a Tektronix 611 storage scope.  The libplot
   API did not originally include linemod(), as the Unix Version 5 manual
   makes clear.  That is because the Tektronix 611 did not have any
   predefined set of line styles.  linemod() was added to the API slightly
   later, when it was extended to support the Tektronix 4010/4014 storage
   scope.  The 4010/4014 provided hardware support for the five line styles
   "solid" through "longdashed".

   GNU libplot supports the traditional five, and also two additional line
   styles, "dotdotdashed" and "dotdotdotdashed".  Each non-solid style is
   defined as a dash pattern, with the length of each dash (drawn or not
   drawn) being an integer multiple of the line width.  This `scaling by
   line width' applies for sufficiently wide lines, at least.

   GNU libplot also supports a special "disconnected" line style (if a path
   is disconnected, it's drawn as a sequence of filled circles, one at each
   of the path join points). */

#include "sys-defines.h"
#include "extern.h"

/* An array of dashes for each line type (dashes are cylically used,
   on/off/on/off...).  Types must appear in a special order: it must agree
   with our internal numbering, i.e. must agree with the definitions of
   L_{SOLID,DOTTED,DOTDASHED,SHORTDASHED,LONGDASHED,DOTDOTDASHED etc.} in
   extern.h, which are 0,1,2,3,4,5 etc. respectively. */

const plLineStyle _pl_g_line_styles[PL_NUM_LINE_TYPES] =
/* Dash arrays for "dotted" through "longdashed" below are those used by
   the Tektronix emulator in xterm(1), except that the emulator seems
   incorrectly to have on and off interchanged (!). */
{
  { "solid", 		PL_L_SOLID, 		0, {0} 		}, /* dummy */
  { "dotted", 		PL_L_DOTTED, 		2, {1, 3} 	},
  { "dotdashed", 	PL_L_DOTDASHED, 	4, {4, 3, 1, 3} },
  { "shortdashed", 	PL_L_SHORTDASHED,	2, {4, 4} 	},
  { "longdashed", 	PL_L_LONGDASHED, 	2, {7, 4} 	},
  { "dotdotdashed", 	PL_L_DOTDOTDASHED,	6, {4, 3, 1, 3, 1, 3} },
  { "dotdotdotdashed", 	PL_L_DOTDOTDOTDASHED,	8, {4, 3, 1, 3, 1, 3, 1, 3} }
};

/* N.B. `ps4014', the Tektronix->PS translator in Adobe's Transcript
   package, uses { 1, 2 }, { 8, 2, 1, 2 }, { 2, 2 }, { 12, 2 } for
   "dotted" through "longdashed", instead. */

/* N.B. A genuine Tektronix 4014 (with Enhanced Graphics Module) uses 
   { 1, 1 }, { 5, 1, 1, 1 }, { 3, 1 }, { 6, 2 } for "dotted"
   through "longdashed", instead.  See the Tektronix 4014 Service
   Instruction Manual (dated Aug. 1974) for the diode array that produces
   these patterns. */
