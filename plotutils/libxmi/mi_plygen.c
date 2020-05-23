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
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_scanfill.h"
#include "mi_ply.h"

/*
 *
 * Original author: Brian Kelleher, Oct. 1985.
 * Hacked by Robert S. Maier, 1998-9.
 *
 * Routine to fill a general (i.e., possibly non-convex or
 * self-intersecting) polygon.  Two fill rules are supported: WINDING and
 * EVENODD.  All painting goes through the low-level MI_PAINT_SPANS()
 * macro.
 *
 * This calls utility routines in mi_plyutil.c.  See mi_scanfill.h for a
 * complete description of the algorithm. */

/* ARGS: count = number of points, ptsIn = the points */
void
miFillGeneralPoly (miPaintedSet *paintedSet, const miGC *pGC, int count, const miPoint *ptsIn)
{
  EdgeTableEntry *pAET;		/* the Active Edge Table   */
  int y;			/* the current scanline    */
  int nPts = 0;			/* number of pts in buffer */
  EdgeTableEntry *pWETE;	/* Winding Edge Table      */
  ScanLineList *pSLL;		/* Current ScanLineList    */
  miPoint *ptsOut;		/* ptr to output buffers   */
  unsigned int *width;
  miPoint FirstPoint[NUMPTSTOBUFFER]; /* the output buffers */
  unsigned int FirstWidth[NUMPTSTOBUFFER];
  EdgeTableEntry *pPrevAET;	/* previous AET entry      */
  EdgeTable ET;			/* Edge Table header node  */
  EdgeTableEntry AET;		/* Active ET header node   */
  EdgeTableEntry *pETEs;	/* Edge Table Entries buff */
  ScanLineListBlock SLLBlock;	/* header for ScanLineList */
  bool fixWAET = false;

  if (count <= 2)
    return;

  pETEs = (EdgeTableEntry *) mi_xmalloc(sizeof(EdgeTableEntry) * count);
  ptsOut = FirstPoint;
  width = FirstWidth;
  miCreateETandAET (count, ptsIn, &ET, &AET, pETEs, &SLLBlock);
  pSLL = ET.scanlines.next;

  if (pGC->fillRule == (int)MI_EVEN_ODD_RULE) 
    {
      /*
       *  for each scanline
       */
      for (y = ET.ymin; y < ET.ymax; y++) 
        {
	  /*
	   *  Add a new edge to the active edge table when we
	   *  get to the next edge.
	   */
	  if (pSLL && y == pSLL->scanline) 
            {
	      miloadAET(&AET, pSLL->edgelist);
	      pSLL = pSLL->next;
            }
	  pPrevAET = &AET;
	  pAET = AET.next;

	  /*
	   *  for each active edge
	   */
	  while (pAET) 
            {
	      ptsOut->x = pAET->bres.minor_axis;
	      ptsOut++->y = y;
	      *width++ = (unsigned int)(pAET->next->bres.minor_axis - pAET->bres.minor_axis);
	      nPts++;

	      /*
	       *  send out the buffer when its full
	       */
	      if (nPts == NUMPTSTOBUFFER) 
		{
		  MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], nPts, FirstPoint, FirstWidth)
		  ptsOut = FirstPoint;
		  width = FirstWidth;
		  nPts = 0;
                }
	      EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y);
            }
	  miInsertionSort(&AET);
        }
    }
  else				/* default to WindingNumber */
    {
      /*
       *  for each scanline
       */
      for (y = ET.ymin; y < ET.ymax; y++) 
        {
	  /*
	   *  Add a new edge to the active edge table when we
	   *  get to the next edge.
	   */
	  if (pSLL && y == pSLL->scanline) 
            {
	      miloadAET(&AET, pSLL->edgelist);
	      micomputeWAET(&AET);
	      pSLL = pSLL->next;
            }
	  pPrevAET = &AET;
	  pAET = AET.next;
	  pWETE = pAET;

	  /*
	   *  for each active edge
	   */
	  while (pAET) 
            {
	      /*
	       *  if the next edge in the active edge table is
	       *  also the next edge in the winding active edge
	       *  table.
	       */
	      if (pWETE == pAET) 
                {
		  ptsOut->x = pAET->bres.minor_axis;
		  ptsOut++->y = y;
		  *width++ = (unsigned int)(pAET->nextWETE->bres.minor_axis - pAET->bres.minor_axis);
		  nPts++;

		  /*
		   *  send out the buffer
		   */
		  if (nPts == NUMPTSTOBUFFER) 
                    {
		      MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], nPts, FirstPoint, FirstWidth)
		      ptsOut = FirstPoint;
		      width  = FirstWidth;
		      nPts = 0;
                    }

		  pWETE = pWETE->nextWETE;
		  while (pWETE != pAET)
		    EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
		  pWETE = pWETE->nextWETE;
                }
	      EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
            }

	  /*
	   *  reevaluate the Winding active edge table if we
	   *  just had to resort it or if we just exited an edge.
	   */
	  if (miInsertionSort(&AET) || fixWAET) 
            {
	      micomputeWAET(&AET);
	      fixWAET = false;
            }
        }
    }

  /*
   *     Get any spans that we missed by buffering
   */
  MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], nPts, FirstPoint, FirstWidth)
  free (pETEs);
  miFreeStorage(SLLBlock.next);
}
