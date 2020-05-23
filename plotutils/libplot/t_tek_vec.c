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

/* This file contains the low-level _pl_t_tek_vector() method, which takes
   two integer arguments and writes them, in the format used by a Tektronix
   4014 terminal for addresses while in vector or point-plot mode, to a
   specified output stream.  It is called by several TekPlotter methods.
   
   Original Tek 4014 resolution was 1024 by 1024, with 1024 by 780
   displayable, i.e. 0x400 by 0x30c.  Extended (EGM) resolution, supported
   by 4014's with the Enhanced Graphics Module, is four times that: 4096 by
   3120.  This is the resolution of the xterm Tektronix emulator.

   With EGM, have an allowed range of 12 bits (0 <= x,y <=4095).  A Tek
   code for a point address (x,y) consists of a sequence of bytes:

   	[Hi_Y] [EGM] [Lo_Y] [Hi_X] Lo_X.

   Since the EGM byte was added later, to improve resolution, it contains
   the lowest 2 bits of each of the X and Y bytes, packed into the lowest
   four bits of the byte.  The remaining 10 bits are subdivided as follows
   among the remaining four code bytes.  Lo=lowest 5 of the 10 bits,
   Hi=highest 5 of the 10 bits, all packed into the lowest 5 bits of the
   byte.  Code bytes are distinguished from each other by the pattern of
   the high three bits.  (Hi_Y = ?01, EGM = ?11, Lo_Y = ?11, Hi_X = ?01,
   Lo_X = ?10, where ? is ignored [or should be].)  If EGM is included then
   Lo_Y must be included to prevent ambiguity.  Also if Hi_X is included
   then Lo_Y must be included.

   If a byte is omitted, then its value defaults to the value of the
   corresponding byte in the most recently received point.  The exception
   is the EGM byte; if omitted, it defaults to zero.

   Warning: some emulators, the xterm emulator in particular, get this
   scheme wrong.  The xterm emulator will behave unpredicably if Lo_Y is
   omitted.  And if EGM is omitted, its value (as far as the xterm emulator
   goes) is not zero, but is equal to the value of the EGM byte in the most
   recently received point. */

#include "sys-defines.h"
#include "extern.h"

#define FIVEBITS 0x1F
#define TWOBITS 0x03		/* a dollar and six bits for a haircut... */

void
_pl_t_tek_vector (R___(Plotter *_plotter) int xx, int yy)
{
  unsigned char xx_high, yy_high;
  unsigned char xx_low, yy_low;
  unsigned char xx_topsig, yy_topsig;
  unsigned char egm;
  unsigned char byte_buf[5];
  int num_bytes = 0;
  
#ifdef NO_WRAP			/* could completely restrict to box */
  if (xx < 0)
    xx = 0;
  if (yy < 0)
    yy = 0;
#endif

  xx_high = (xx>>7) & FIVEBITS;	/* bits 11 through 7 of xx */
  yy_high = (yy>>7) & FIVEBITS;	/* bits 11 through 7 of yy */
  
  xx_low = (xx>>2) & FIVEBITS;	/* bits 6 through 2 of xx */
  yy_low = (yy>>2) & FIVEBITS;	/* bits 6 through 2 of yy */
  
  xx_topsig = xx & TWOBITS;	/* bits 1 through 0 of xx */
  yy_topsig = yy & TWOBITS;	/* bits 1 through 0 of yy */
  egm = (yy_topsig<<2) + xx_topsig;

  /* The bit patterns 0x20, 0x40, 0x60 are magic */

  byte_buf[num_bytes++] = yy_high | 0x20; /* bits 5 through 9 of yy */
#ifdef CAN_OMIT_EGM
  if (egm)
#endif
    byte_buf[num_bytes++] =   egm | 0x60;
  byte_buf[num_bytes++] = yy_low  | 0x60; /* bits 0 through 4 of yy */
  byte_buf[num_bytes++] = xx_high | 0x20; /* bits 5 through 9 of xx */
  byte_buf[num_bytes++] = xx_low  | 0x40;  /* bits 0 through 4 of xx */

  /* invoke low-level output routine */
  _write_bytes (_plotter->data, num_bytes, byte_buf);

  return;
}

/* This version checks whether the supplied x and y coordinates are similar
   to the x and y coordinates of another point, presumed to be the most
   recently output point.  If they are, the Tek code is shortened by the
   omission of optional bytes.  Hi_Y, Lo_Y and Hi_X are all held in memory,
   so need not be transmitted if they did not change.  Lo_X must always be
   transmitted.  EGM is not held in memory; if not transmitted, it defaults
   to zero.

   The `force' argument will force output even if the vector has zero
   length. */

void
_pl_t_tek_vector_compressed (R___(Plotter *_plotter) int xx, int yy, int oldxx, int oldyy, bool force)
{
  unsigned char xx_high, yy_high, oldxx_high, oldyy_high;
  unsigned char xx_low, yy_low, oldyy_low;
  unsigned char xx_top, yy_top;
  unsigned char egm;
  unsigned char byte_buf[5];
  int num_bytes = 0;
  
#ifdef NO_WRAP			/* could completely restrict to box */
  if (xx < 0)
    xx = 0;
  if (yy < 0)
    yy = 0;
#endif

  /* if line segment has zero length, do nothing unless forcing an output */
  if (!force && (xx == oldxx) && (yy == oldyy))
    return;

  xx_high = (xx>>7) & FIVEBITS;	/* bits 11 through 7 of xx */
  yy_high = (yy>>7) & FIVEBITS;	/* bits 11 through 7 of yy */
  oldxx_high = (oldxx>>7) & FIVEBITS; /* bits 11 through 7 of oldxx */
  oldyy_high = (oldyy>>7) & FIVEBITS; /* bits 11 through 7 of oldyy */
  
  xx_low = (xx>>2) & FIVEBITS;	/* bits 6 through 2 of xx */
  yy_low = (yy>>2) & FIVEBITS;	/* bits 6 through 2 of yy */
  oldyy_low = (oldyy>>2) & FIVEBITS;	/* bits 4 through 0 of oldyy */
  
  xx_top = xx & TWOBITS;	/* bits 1 through 0 of xx */
  yy_top = yy & TWOBITS;	/* bits 1 through 0 of yy */

  egm = (yy_top<<2) + xx_top;

  /* The bit patterns 0x20, 0x40, 0x60 are magic */

  if (yy_high != oldyy_high)
    byte_buf[num_bytes++] = yy_high | 0x20; /* bits 11 through 7 of yy: Hi_Y */

#ifdef CAN_OMIT_EGM
  if (egm)
#endif
    byte_buf[num_bytes++] = egm     | 0x60; /* bits 1 through 0 of xx and yy */
#ifdef CAN_OMIT_LO_Y
  if ((yy_low != oldyy_low) || (xx_high != oldxx_high) || egm)
#endif
    byte_buf[num_bytes++] = yy_low  | 0x60; /* bits 6 through 2 of yy: Lo_Y */
  if (xx_high != oldxx_high)
    byte_buf[num_bytes++] = xx_high | 0x20; /* bits 11 through 7 of xx: Hi_X */
  byte_buf[num_bytes++] = xx_low    | 0x40; /* bits 6 through 2 of xx: Lo_X */

  /* invoke low-level output routine */
  _write_bytes (_plotter->data, num_bytes, byte_buf);

  return;
}
