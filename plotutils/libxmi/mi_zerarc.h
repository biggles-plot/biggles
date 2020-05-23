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

/* This header file is included by mi_zerarc.c, which draws a single-pixel
   (i.e. Bresenham) poly-arc using a fast integer algorithm.  It defines
   structures and macros used in the algorithm. */

typedef struct 
{
  int x;
  int y;
  unsigned int mask;
} miZeroArcPt;

typedef struct 
{
  int x, y, k1, k3, a, b, d, dx, dy;
  int alpha, beta;
  int xorg, yorg;		/* upper left corner */
  int xorgo, yorgo;
  unsigned int w, h;
  unsigned int initialMask;
  miZeroArcPt start, altstart, end, altend;
  int firstx, firsty;
  int startAngle, endAngle;	/* in 1/64 degrees */
} miZeroArc;

/* miZeroPolyArc() draws an arc only if it satisfies the following size
   constraint.  If it doesn't, miZeroPolyArc() hands it off to miPolyArc(),
   which uses a floating point algorithm. */
#define MI_CAN_ZERO_ARC(arc) (((arc)->width == (arc)->height) || \
			     (((arc)->width <= 800) && ((arc)->height <= 800)))

/* used for setup only */
#define MIARCSETUP(info, x, y, k1, k3, a, b, d, dx, dy) \
x = info.x; \
y = info.y; \
k1 = info.k1; \
k3 = info.k3; \
a = info.a; \
b = info.b; \
d = info.d; \
dx = info.dx; \
dy = info.dy

#define MIARCOCTANTSHIFT(info, x, y, dx, dy, a, b, d, k1, k3, clause) \
if (a < 0) \
{ \
    if (y == (int)info.h) \
      { \
	d = -1; \
	a = b = k1 = 0; \
      } \
  else \
    { \
      dx = (k1 << 1) - k3; \
      k1 = dx - k1; \
      k3 = -k3; \
      b = b + a - (k1 >> 1); \
      d = b + ((-a) >> 1) - d + (k3 >> 3); \
      if (dx < 0) \
	  a = -((-dx) >> 1) - a; \
      else \
	  a = (dx >> 1) - a; \
      dx = 0; \
      dy = 1; \
      clause \
    } \
}

#define MIARCSTEP(x, y, dx, dy, a, b, d, k1, k3, move1, move2) \
b -= k1; \
if (d < 0) \
{ \
    x += dx; \
    y += dy; \
    a += k1; \
    d += b; \
    move1 \
} \
else \
{ \
    x++; \
    y++; \
    a += k3; \
    d -= a; \
    move2 \
}

#define MIARCCIRCLESTEP(x, y, a, b, d, k1, k3, clause) \
b -= k1; \
x++; \
if (d < 0) \
{ \
  a += k1; \
  d += b; \
} \
else \
{ \
  y++; \
  a += k3; \
  d -= a; \
  clause \
}
