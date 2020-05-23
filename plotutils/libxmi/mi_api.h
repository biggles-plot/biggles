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

/* Internal counterparts of libxmi's core functions, each of which
   takes a (miPaintedSet *) as first argument. */

extern void miDrawPoints_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);
extern void miDrawLines_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);
extern void miFillPolygon_internal (miPaintedSet *paintedSet, const miGC *pGC, miPolygonShape shape, miCoordMode mode, int npts, const miPoint *pPts);
extern void miDrawRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *pRects);
extern void miFillRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *pRects);
extern void miDrawArcs_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs);
extern void miFillArcs_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs);
extern void miDrawArcs_r_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipse_cache);

/* Internal functions, which are called by wrapper functions defined in
   mi_api.c. */

extern void miWideDash (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);
extern void miZeroDash (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);
extern void miWideLine (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);
extern void miZeroLine (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npts, const miPoint *pPts);

extern void miPolyArc (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs);
extern void miZeroPolyArc (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs);

extern void miPolyArc_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipse_cache);
extern void miZeroPolyArc_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipse_cache);

/* Declarations of other internal functions, which should really be moved
   elsewhere. */

/* wrappers for storage allocation functions, see mi_alloc.c */
extern void * mi_xmalloc (size_t size);
extern void * mi_xcalloc (size_t nmemb, size_t size);
extern void * mi_xrealloc (void * p, size_t size);

/* other misc. internal functions */
extern void miFillConvexPoly (miPaintedSet *paintedSet, const miGC *pGC, int count, const miPoint *ptsIn);
extern void miFillGeneralPoly (miPaintedSet *paintedSet, const miGC *pGC, int count, const miPoint *ptsIn);
extern void miStepDash (int dist, int *pDashNum, int *pDashIndex, const unsigned int *pDash, int numInDashList, int *pDashOffset);
