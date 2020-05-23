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

#include "sys-defines.h"
#include "extern.h"

/* idraw's 12 standard colors, taken from idraw source.  These are the only
   pen colors that idraw understands, so we must quantize.  Quantization
   of fill colors is more complicated; see below.

   The list of 12 standard colors include 6 of the 8 vertices of the RGB
   cube; Magenta and Cyan are missing.  It also includes Orange and Indigo
   (on the boundary of the cube; they more or less substitute for Magenta
   and Cyan).  Also included are four points in the interior of the cube:
   Light Gray, Dark Gray, Violet, and Brown. */

const plColor _pl_p_idraw_stdcolors[PS_NUM_IDRAW_STD_COLORS] =
{
  {0x0000, 0x0000, 0x0000},	/* Black */
  {0xa500, 0x2a00, 0x2a00},	/* Brown */
  {0xffff, 0x0000, 0x0000},	/* Red */  
  {0xffff, 0xa5a5, 0x0000},	/* Orange */
  {0xffff, 0xffff, 0x0000},	/* Yellow */
  {0x0000, 0xffff, 0x0000},	/* Green */
  {0x0000, 0x0000, 0xffff},	/* Blue */
  {0xbf00, 0x0000, 0xff00},	/* Indigo */
  {0x4f00, 0x2f00, 0x4f00},	/* Violet */
  {0xffff, 0xffff, 0xffff},	/* White */
  {0xc350, 0xc350, 0xc350},	/* LtGray */	  
  {0x80e8, 0x80e8, 0x80e8}	/* DkGray */	  
};

const char * const _pl_p_idraw_stdcolornames[PS_NUM_IDRAW_STD_COLORS] = 
{
  "Black", "Brown", "Red", "Orange", "Yellow", "Green",
  "Blue", "Indigo", "Violet", "White", "LtGray", "DkGray"
};

/* Idraw allows a fill color to be an interpolation of a pen color
   (``foreground color'') and a background color; both must be in the above
   list.  The following is a list of the interpolations (``shadings'') that
   idraw recognizes.  0.0 means use foreground color, 1.0 means use
   background color.  (Idraw shadings are actually a special case of idraw
   patterns, which include bitmap fillings as well.) */

const double _pl_p_idraw_stdshadings[PS_NUM_IDRAW_STD_SHADINGS] =
{
  0.0, 0.25, 0.5, 0.75, 1.0
};
