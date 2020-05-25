/* This file is part of the GNU libxmi package.  

   Copyright (C) 1985, 1986, 1987, 1988, 1989, X Consortium.  For an
   associated permission notice, see the accompanying file README-X.
   
   GNU enhancements Copyright (C) 1998, 1999, 2000, 2005, Free Software
   Foundation, Inc.

   The GNU libxmi package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU libxmi package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* Stuff needed for drawing thin (zero width) lines */

#define X_AXIS	0
#define Y_AXIS	1

#define OUT_LEFT  0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

#define MI_OUTCODES(outcode, x, y, xmin, ymin, xmax, ymax) \
{\
     if (x < xmin) outcode |= OUT_LEFT;\
     if (x > xmax) outcode |= OUT_RIGHT;\
     if (y < ymin) outcode |= OUT_ABOVE;\
     if (y > ymax) outcode |= OUT_BELOW;\
}

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )

#define ceiling(m,n)  (((m)-1)/(n) + 1)

#define SWAPINT(i, j) \
{  int _t = i;  i = j;  j = _t; }

#define SWAPINT_PAIR(x1, y1, x2, y2)\
{   int t = x1;  x1 = x2;  x2 = t;\
        t = y1;  y1 = y2;  y2 = t;\
}

#define AbsDeltaAndSign(_p2, _p1, _absdelta, _sign) \
    (_sign) = 1; \
    (_absdelta) = (_p2) - (_p1); \
    if ( (_absdelta) < 0) { (_absdelta) = -(_absdelta); (_sign) = -1; }

#ifndef FIXUP_X_MAJOR_ERROR
#define FIXUP_X_MAJOR_ERROR(_e, _signdx, _signdy) \
    (_e) -= ( (_signdx) < 0)
#endif

#ifndef FIXUP_Y_MAJOR_ERROR
#define FIXUP_Y_MAJOR_ERROR(_e, _signdx, _signdy) \
    (_e) -= ( (_signdy) < 0)
#endif

