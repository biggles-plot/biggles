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

/* This header file is included by mi_fllarc.c (which contains code for
   filling a poly-arc) and mi_arc.c (which contains code for drawing a wide
   poly-arc). */


#define FULLCIRCLE (360 * 64)

/*----------------------------------------------------------------------*/
/* Structures and macros used by arc-filling code in mi_arc.c           */
/*----------------------------------------------------------------------*/

/* Structure containing variables that are updated during a scan through a
   filled arc. */
typedef struct
{
  int xorg, yorg;
  int y;
  int dx, dy;
  int e;
  int ym, yk, xm, xk;
} miFillArc;

/* A floating-point version, used for non-circular arcs one of whose
   dimensions (width or height) is greater than 800 pixels.  Could use
   64-bit integers.  */
typedef struct
{
  int xorg, yorg;		/* upper left corner */
  int y;			/* vertical semi-axis */
  int dx, dy;
  double e;
  double ym, yk, xm, xk;
} miFillArcD;

/* Which to use? */
#define MI_CAN_FILL_ARC(arc) (((arc)->width == (arc)->height) || \
			     (((arc)->width <= 800) && ((arc)->height <= 800)))

/* Nothing to draw? */
#define MI_FILLED_ARC_IS_EMPTY(arc) (!(arc)->angle2 || \
			             !(arc)->width || !(arc)->height || \
			             (((arc)->width == 1) && ((arc)->height & 1)))

/* used for setup only */
#define MIFILLARCSETUP(info, x, y, e, xk, xm, yk, ym, dx, dy, xorg, yorg) \
x = 0; \
y = info.y; \
e = info.e; \
xk = info.xk; \
xm = info.xm; \
yk = info.yk; \
ym = info.ym; \
dx = info.dx; \
dy = info.dy; \
xorg = info.xorg; \
yorg = info.yorg

#define MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw) \
	e += yk; \
	while (e >= 0) \
	{ \
	    x++; \
	      xk -= xm; \
	e += xk; \
	} \
	y--; \
	yk -= ym; \
	slw = (x << 1) + dx; \
	if ((e == xk) && (slw > 1)) \
	     slw--
     
#define MIFILLARCLOWER(e, xk, y, dy, slw) (((y + dy) != 0) && ((slw > 1) || (e != xk)))
     
/* pie-slice related things */

typedef struct
{
  int	    x;
  int       stepx;
  int	    deltax;
  int	    e;
  int	    dy;
  int	    dx;
} miSliceEdge;

typedef struct
{
  miSliceEdge	edge1, edge2;
  int		min_top_y, max_top_y;
  int		min_bot_y, max_bot_y;
  bool		edge1_top, edge2_top;
  bool		flip_top, flip_bot;
} miArcSlice;

#define MIARCSLICESTEP(edge) \
	edge.x -= edge.stepx; \
	edge.e -= edge.dx; \
	if (edge.e <= 0) \
	{ \
	    edge.x -= edge.deltax; \
	      edge.e += edge.dy; \
	}

#define MIFILLSLICEUPPER(y, slice) \
	((y >= slice.min_top_y) && (y <= slice.max_top_y))

#define MIFILLSLICELOWER(y, slice) \
	((y >= slice.min_bot_y) && (y <= slice.max_bot_y))

#define MIARCSLICEUPPER(xl,xr,slice,slw) \
	xl = xorg - x; \
	xr = xl + slw - 1; \
	if (slice.edge1_top && (slice.edge1.x < xr)) \
	     xr = slice.edge1.x; \
	     if (slice.edge2_top && (slice.edge2.x > xl)) \
	     xl = slice.edge2.x;
     
#define MIARCSLICELOWER(xl,xr,slice,slw) \
	     xl = xorg - x; \
	     xr = xl + slw - 1; \
	     if (!slice.edge1_top && (slice.edge1.x > xl)) \
	     xl = slice.edge1.x; \
	     if (!slice.edge2_top && (slice.edge2.x < xr)) \
	     xr = slice.edge2.x;
     
/*----------------------------------------------------------------------*/
/* Macros used by wide-arc-drawing code in mi_arc.c                     */
/*----------------------------------------------------------------------*/

/* used for setup only */
#define MIWIDEARCSETUP(x,y,dy,slw,e,xk,xm,yk,ym) \
     x = 0; \
     y = slw >> 1; \
     yk = y << 3; \
     xm = 8; \
     ym = 8; \
     if (dy) \
	{ \
	   xk = 0; \
	   if (slw & 1) \
	     e = -1; \
	   else \
	     e = -(y << 2) - 2; \
	} \
	else \
	{ \
	   y++; \
	   yk += 4; \
	   xk = -4; \
	   if (slw & 1) \
	     e = -(y << 2) - 3; \
	   else \
	     e = - (y << 3); \
	}

#define MIFILLINARCSTEP(inx, iny, ine, inxk, inxm, inyk, inym, dx, slw) \
	ine += inyk; \
	while (ine >= 0) \
	{ \
	    inx++; \
	    inxk -= inxm; \
	    ine += inxk; \
	} \
	iny--; \
	inyk -= inym; \
	slw = (inx << 1) + dx; \
	if ((ine == inxk) && (slw > 1)) \
	    slw--
     
/*----------------------------------------------------------------------*/
