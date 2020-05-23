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

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"
#include "mi_fply.h"

/* forward references */
static int GetFPolyYBounds (const SppPoint *pts, int n, double yFtrans, int *by, int *ty);

/*
 * Written by Todd Newman; April 1987.
 * Hacked by Robert S. Maier, 1998-1999.
 *
 * Fill a convex polygon, with SPP (subpixel placement) of vertices.  If
 * the given polygon is not convex, then the result is undefined.  All
 * painting goes through the low-level MI_PAINT_SPANS() macro.
 *
 * In libxmi, this is used to draw polygonal line caps and line joins for
 * poly-arcs.  See mi_arc.c.
 *
 * The algorithm is to order the edges from smallest y to largest y, by
 * partitioning the array into a left edge list and a right edge list.  The
 * algorithm used to traverse each edge is the digital differencing
 * analyzer line algorithm, with y as the major axis. There's some funny
 * linear interpolation involved because of the subpixel postioning. */

/* ARGS: count = # points, ptsIn = points, 
   	 xTrans,yTrans = translation for each point, 
	 xFtrans,yFtrans = translation before conversion, which provides a
	 		   mechanism to match rounding errors with any
			   shape that meets the polygon exactly. */
void
miFillSppPoly (miPaintedSet *paintedSet, miPixel pixel, int count, const SppPoint *ptsIn, int xTrans, int yTrans, double xFtrans, double yFtrans)
{
  double	xl = 0.0,	/* x vals of left and right edges */
  		xr = 0.0,
  		ml = 0.0,	/* left edge slope */
		mr = 0.0,	/* right edge slope */
		dy,		/* delta y */
		i;		/* loop counter */
  int		y,		/* current scanline */
		j,
		imin,		/* index of vertex with smallest y */
		ymin,		/* y-extents of polygon */
		ymax;
  int           left, right,	/* indices to first endpoints */
		nextleft,
		nextright;	/* indices to second endpoints */
  int		*Marked;	/* set if this vertex has been used */
  unsigned int	*width,
		*FirstWidth;	/* output buffer */
  miPoint	*ptsOut,
		*FirstPoint;	/* output buffer */

  imin = GetFPolyYBounds (ptsIn, count, yFtrans, &ymin, &ymax);

  y = ymax - ymin + 1;
  if ((count < 3) || (y <= 0))
    return;
  ptsOut = FirstPoint = (miPoint *)mi_xmalloc(sizeof(miPoint) * y);
  width = FirstWidth = (unsigned int *)mi_xmalloc(sizeof(unsigned int) * y);
  Marked = (int *) mi_xmalloc(sizeof(int) * count);

  for (j = 0; j < count; j++)
    Marked[j] = 0;
  nextleft = nextright = imin;
  Marked[imin] = -1;
  y = ICEIL(ptsIn[nextleft].y + yFtrans);

  /*
   *  loop through all edges of the polygon
   */
  do
    {
      /* add a left edge if we need to */
      if ((y > (ptsIn[nextleft].y + yFtrans) ||
	   ISEQUAL(y, ptsIn[nextleft].y + yFtrans)) 
	  && Marked[nextleft] != 1)
	{
	  Marked[nextleft]++;
	  left = nextleft++;

	  /* find the next edge, considering the end conditions */
	  if (nextleft >= count)
	    nextleft = 0;

	  /* now compute the starting point and slope */
	  dy = ptsIn[nextleft].y - ptsIn[left].y;
	  if (dy != 0.0)
	    { 
	      ml = (ptsIn[nextleft].x - ptsIn[left].x) / dy;
	      dy = y - (ptsIn[left].y + yFtrans);
	      xl = (ptsIn[left].x + xFtrans) + ml * DMAX(dy, 0);
	    }
        }

      /* add a right edge if we need to */
      if ((y > ptsIn[nextright].y + yFtrans) 
	  ||
	  (ISEQUAL(y, ptsIn[nextright].y + yFtrans)
	   && Marked[nextright] != 1))
	{
	  Marked[nextright]++;
	  right = nextright--;
	    
	  /* find the next edge, considering the end conditions */
	  if (nextright < 0)
	    nextright = count - 1;
	    
	  /* now compute the starting point and slope */
	  dy = ptsIn[nextright].y - ptsIn[right].y;
	  if (dy != 0.0) 
	    { 
	      mr = (ptsIn[nextright].x - ptsIn[right].x) / dy;
	      dy = y - (ptsIn[right].y + yFtrans); 
	      xr = (ptsIn[right].x + xFtrans) + mr * DMAX(dy, 0);
	    }
	}
	
      /*
       *  generate scans to fill while we still have
       *  a right edge as well as a left edge.
       */
      i = (DMIN(ptsIn[nextleft].y, ptsIn[nextright].y) + yFtrans) - y;

      if (i < EPSILON)
	{
	  if(Marked[nextleft] && Marked[nextright])
	    {
	      /* Arrgh, we're trapped! (no more points) 
	       * Out, we've got to get out of here before this decadence saps
	       * our will completely! */
	      break;
	    }
	  continue;
	}
      else
	{
	  j = (int) i;
	  if (!j)
	    j++;
	}
      while (j > 0) 
        {
	  int cxl, cxr;

	  ptsOut->y = (y) + yTrans;

	  cxl = ICEIL(xl);
	  cxr = ICEIL(xr);
	  /* reverse the edges if necessary */
	  if (xl < xr) 
            {
	      *(width++) = (unsigned int)(cxr - cxl);
	      (ptsOut++)->x = cxl + xTrans;
            }
	  else 
            {
	      *(width++) = (unsigned int)(cxl - cxr);
	      (ptsOut++)->x = cxr + xTrans;
            }
	  y++;

	  /* increment down the edges */
	  xl += ml;
	  xr += mr;
	  j--;
        }
    } while (y <= ymax);

  free (Marked);

  /* paint the spans (to miPaintedSet, or if NULL, to the canvas) */
  MI_PAINT_SPANS(paintedSet, pixel, ptsOut - FirstPoint, FirstPoint, FirstWidth)
}

/* Find the index of the point with the smallest y.  Also return the
   smallest and largest y. */
static int
GetFPolyYBounds (const SppPoint *pts, int n, double yFtrans, int *by, int *ty)
{
  const SppPoint *ptsStart = pts;
  const SppPoint *ptMin;
  double ymin, ymax;

  ptMin = pts;
  ymin = ymax = (pts++)->y;

  while (--n > 0) 
    {
      if (pts->y < ymin)
	{
	  ptMin = pts;
	  ymin = pts->y;
	}
      if(pts->y > ymax)
	ymax = pts->y;

      pts++;
    }

  *by = ICEIL(ymin + yFtrans);
  *ty = ICEIL(ymax + yFtrans - 1);
  return (ptMin - ptsStart);
}
