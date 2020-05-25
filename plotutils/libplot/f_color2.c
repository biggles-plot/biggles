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

/* Fig's 32 standard colors, taken from xfig source.

   The list includes the 8 standard colors (vertices of the RGB cube,
   listed first).  It also includes three intermediate intensities for each
   of the 6 standard colors other than black and white (except that yellow
   is missing, so there are only 5, giving 15 additional shades in all).
   Also included are 8 random colors (LtBlue, Gold, Brown?, Pink, and
   Pink?, with ? = 2,3,4) apparently present because someone liked them.
   LtBlue is the only color in the interior of the cube. */

#include "sys-defines.h"
#include "extern.h"

const plColor _pl_f_fig_stdcolors[FIG_NUM_STD_COLORS] = 
{
  {0x00, 0x00, 0x00},		/* Black */
  {0x00, 0x00, 0xff},		/* Blue */
  {0x00, 0xff, 0x00},		/* Green */
  {0x00, 0xff, 0xff},		/* Cyan */
  {0xff, 0x00, 0x00},		/* Red */
  {0xff, 0x00, 0xff},		/* Magenta */
  {0xff, 0xff, 0x00},		/* Yellow */
  {0xff, 0xff, 0xff},		/* White */
  {0x00, 0x00, 0x90},		/* Blue4 */
  {0x00, 0x00, 0xb0},		/* Blue3 */
  {0x00, 0x00, 0xd0},		/* Blue2 */
  {0x87, 0xce, 0xff},		/* LtBlue [SkyBlue1 in rgb.txt] */
  {0x00, 0x90, 0x00},		/* Green4 */
  {0x00, 0xb0, 0x00},		/* Green3 */
  {0x00, 0xd0, 0x00},		/* Green2 */
  {0x00, 0x90, 0x90},		/* Cyan4 */
  {0x00, 0xb0, 0xb0},		/* Cyan3 */
  {0x00, 0xd0, 0xd0},		/* Cyan2 */
  {0x90, 0x00, 0x00},		/* Red4 */
  {0xb0, 0x00, 0x00},		/* Red3 */
  {0xd0, 0x00, 0x00},		/* Red2 */
  {0x90, 0x00, 0x90},		/* Magenta4 */
  {0xb0, 0x00, 0xb0},		/* Magenta3 */
  {0xd0, 0x00, 0xd0},		/* Magenta2 */
  {0x80, 0x30, 0x00},		/* Brown4, ad hoc */
  {0xa0, 0x40, 0x00},		/* Brown3, ad hoc */
  {0xc0, 0x60, 0x00},		/* Brown2, ad hoc */
  {0xff, 0x80, 0x80},		/* Pink4, ad hoc */
  {0xff, 0xa0, 0xa0},		/* Pink3, ad hoc */
  {0xff, 0xc0, 0xc0},		/* Pink2, ad hoc */
  {0xff, 0xe0, 0xe0},		/* Pink, ad hoc */
  {0xff, 0xd7, 0x00}		/* Gold [as in rgb.txt] */
};
