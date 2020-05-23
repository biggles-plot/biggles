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

/* If libxmi is compiled standalone, this header file is #included.  It
   renames the global symbols by prepending each name with an underscore,
   to avoid polluting the user-level namespace. */

/* Note: if libxmi is compiled as part of libplot/libplotter, the extern.h
   that is #included is a different file.  That file renames each global
   symbol by prepending two underscores, rather than a single underscore.
   That alteration will permit an application to link with both
   libplot/libplotter and libxmi, if desired. */

#define mi_xmalloc _mi_xmalloc
#define mi_xcalloc _mi_xcalloc
#define mi_xrealloc _mi_xrealloc
#define miAddSpansToPaintedSet _miAddSpansToPaintedSet
#define miDrawArcs_r_internal _miDrawArcs_r_internal
#define miDrawArcs_internal _miDrawArcs_internal
#define miDrawLines_internal _miDrawLines_internal
#define miDrawRectangles_internal _miDrawRectangles_internal
#define miPolyArc_r _miPolyArc_r
#define miPolyArc _miPolyArc
#define miFillArcs_internal _miFillArcs_internal
#define miFillRectangles_internal _miFillRectangles_internal
#define miFillSppPoly _miFillSppPoly
#define miFillPolygon_internal _miFillPolygon_internal
#define miFillConvexPoly _miFillConvexPoly
#define miFillGeneralPoly _miFillGeneralPoly
#define miDrawPoints_internal _miDrawPoints_internal
#define miCreateETandAET _miCreateETandAET
#define miloadAET _miloadAET
#define micomputeWAET _micomputeWAET
#define miInsertionSort _miInsertionSort
#define miFreeStorage _miFreeStorage
#define miQuickSortSpansY _miQuickSortSpansY
#define miUniquifyPaintedSet _miUniquifyPaintedSet
#define miWideDash _miWideDash
#define miStepDash _miStepDash
#define miWideLine _miWideLine
#define miZeroPolyArc_r _miZeroPolyArc_r
#define miZeroPolyArc _miZeroPolyArc
#define miZeroLine _miZeroLine
#define miZeroDash _miZeroDash
