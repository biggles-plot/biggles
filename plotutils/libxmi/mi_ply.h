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

/*
 * Created by Brian Kelleher; Oct 1985
 *
 * Include file for filled polygon routines.
 *
 * These are the data structures needed to scan convert regions.  Two
 * different scan conversion methods are available: the even-odd method,
 * and the winding number method.  The even-odd rule states that a point is
 * inside the polygon if a ray drawn from that point in any direction will
 * pass through an odd number of path segments.  By the winding number
 * rule, a point is decided to be inside the polygon if a ray drawn from
 * that point in any direction will pass through a different number of
 * clockwise and counter-clockwise path segments.
 *
 * These data structures are adapted somewhat from the algorithm in
 * (Foley/Van Dam) for scan converting polygons.  The basic algorithm is to
 * start at the top (smallest y) of the polygon, stepping down to the
 * bottom of the polygon by incrementing the y coordinate.  We keep a list
 * of edges which the current scanline crosses, sorted by x.  This list is
 * called the Active Edge Table (AET).  As we increment the y-coordinate,
 * we update each entry in the AET to reflect the edges' new xcoords.  This
 * list must be sorted at each scanline in case two edges intersect.  We
 * also keep a data structure known as the Edge Table (ET), which keeps
 * track of all edges that the current scanline has not yet reached.  The
 * ET is basically a list of ScanLineList structures containing a list of
 * edges which are entered at a given scanline.  There is one ScanLineList
 * per scanline at which an edge is entered.  When we enter a new edge, we
 * move it from the ET to the AET.
 *
 * From the AET, we can implement the even-odd rule as in (Foley/Van Dam).
 * The winding number rule is a little trickier.  We also keep the
 * EdgeTableEntries in the AET linked by the nextWETE (winding
 * EdgeTableEntry) link.  This allows the edges to be linked just as before
 * for updating purposes, but only uses the edges linked by the nextWETE
 * link as edges representing spans of the polygon to drawn (as with the
 * even-odd rule).
 */

typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     bool ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200


/*
 *
 *     A few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = true; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}

extern bool miInsertionSort (EdgeTableEntry *AET);
extern void miCreateETandAET (int count, const miPoint *pts, EdgeTable *ET, EdgeTableEntry *AET, EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock);
extern void miloadAET (EdgeTableEntry *AET, EdgeTableEntry *ETEs);
extern void micomputeWAET (EdgeTableEntry *AET);
extern void miFreeStorage (ScanLineListBlock *pSLLBlock);
