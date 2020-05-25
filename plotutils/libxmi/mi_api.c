/* This file is part of the GNU libxmi package.  Copyright (C) 1998, 1999,
   2000, 2005, Free Software Foundation, Inc.

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

/* This file defines the core libxmi API, consisting of:

   1. miDrawPoints, miDrawLines, miFillPolygon.
   2. miDrawRectangles, miFillRectangles.
   3. miDrawArcs, miFillArcs.  Also the reentrant miDrawArcs_r.

Each of these is a wrapper around an internal function that takes as first
argument a (miPaintedSet *).  A miPaintedSet struct is a structure that is
used by Joel McCormack's span-merging module to implement the
`touch-each-pixel-once' rule.  See mi_spans.c and mi_spans.h. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

#define MI_SETUP_PAINTED_SET(paintedSet, pGC) \
{\
}

#define MI_TEAR_DOWN_PAINTED_SET(paintedSet) \
{\
  miUniquifyPaintedSet (paintedSet); \
}

/* ARGS: mode = Origin or Previous */
void
miDrawPoints (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawPoints_internal (paintedSet, pGC, mode, npt, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

/* ARGS: mode = Origin or Previous */
void
miDrawLines (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawLines_internal (paintedSet, pGC, mode, npt, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

/* ARGS: mode = Origin or Previous */
void
miFillPolygon (miPaintedSet *paintedSet, const miGC *pGC, miPolygonShape shape, miCoordMode mode, int count, const miPoint *pPts)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miFillPolygon_internal (paintedSet, pGC, shape, mode, count, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
miDrawRectangles (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC);
  miDrawRectangles_internal (paintedSet, pGC, nrects, prectInit);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
miFillRectangles (miPaintedSet *paintedSet, const miGC *pGC, int nrectFill, const miRectangle *prectInit)
{
  fprintf (stderr, "miFillRectangles()\n");

  MI_SETUP_PAINTED_SET(paintedSet, pGC);
  miFillRectangles_internal (paintedSet, pGC, nrectFill, prectInit);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
void
miDrawArcs (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawArcs_internal (paintedSet, pGC, narcs, parcs);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */

void
miFillArcs (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miFillArcs_internal (paintedSet, pGC, narcs, parcs);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

/* ARGS: ellipseCache = pointer to ellipse data cache */
void
miDrawArcs_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawArcs_r_internal (paintedSet, pGC, narcs, parcs, ellipseCache);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

/**********************************************************************/
/* Further wrappers that should really be moved to other file(s). */
/**********************************************************************/

/* ARGS: ellipseCache = pointer to ellipse data cache */
void
miDrawArcs_r_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
{
  if (pGC->lineWidth == 0)
    /* use Bresenham algorithm */
    miZeroPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache);
  else
    miPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache);
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
void
miDrawArcs_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
{
  if (pGC->lineWidth == 0)
    /* use Bresenham algorithm */
    miZeroPolyArc (paintedSet, pGC, narcs, parcs);
  else
    miPolyArc (paintedSet, pGC, narcs, parcs);
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */

/* ARGS: mode = Origin or Previous */
void
miDrawLines_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
{
  if (pGC->lineWidth == 0)
    {
    /* use Bresenham algorithm */
      if (pGC->lineStyle == (int)MI_LINE_SOLID)
	miZeroLine (paintedSet, pGC, mode, npt, pPts);
      else
	miZeroDash (paintedSet, pGC, mode, npt, pPts);
    }
  else
    {
      if (pGC->lineStyle == (int)MI_LINE_SOLID)
	miWideLine (paintedSet, pGC, mode, npt, pPts);
      else
	miWideDash (paintedSet, pGC, mode, npt, pPts);
    }
}

void
miDrawRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
{
  const miRectangle *pR = prectInit;
  miPoint rect[5];
  int i;

  fprintf (stderr, "miDrawRectangles_internal()\n");

  for (i = 0; i < nrects; i++)
    {
      rect[0].x = pR->x;
      rect[0].y = pR->y;
      
      rect[1].x = pR->x + (int) pR->width;
      rect[1].y = rect[0].y;
      
      rect[2].x = rect[1].x;
      rect[2].y = pR->y + (int) pR->height;
      
      rect[3].x = rect[0].x;
      rect[3].y = rect[2].y;
      
      /* close the polyline */
      rect[4].x = rect[0].x;
      rect[4].y = rect[0].y;
      
      miDrawLines_internal (paintedSet, pGC, MI_COORD_MODE_ORIGIN, 5, rect);
      pR++;
    }
}
