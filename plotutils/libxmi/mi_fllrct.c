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

/* mi rectangles
   written by Todd Newman, with debts to all and sundry
   */

/* Very straightforward.  We let the low-level paint function invoked by
 * MI_PAINT_SPANS() worry about clipping to the destination.
 *
 * Note libxmi's convention: right edges and bottom edges of filled
 * polygons (including rectangles) are unpainted, so that adjacent polygons
 * will abut with no overlaps or gaps. */

void
miFillRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
{
  const miRectangle *prect; 

  /* ensure we have >=1 rects to fill */
  if (nrects <= 0)
    return;

  prect = prectInit;
  while (nrects--)
    {
      miPoint *ppt;
      miPoint *pptFirst;
      int xorg, yorg;
      unsigned int *pw, *pwFirst;
      unsigned int height, width, countdown;

      height = prect->height;
      width = prect->width;
      pptFirst = (miPoint *)mi_xmalloc (height * sizeof(miPoint));
      pwFirst = (unsigned int *)mi_xmalloc (height * sizeof(unsigned int));
      ppt = pptFirst;
      pw = pwFirst;

      xorg = prect->x;
      yorg = prect->y;
      countdown = height;
      while (countdown--)
	{
	  *pw++ = width;
	  ppt->x = xorg;
	  ppt->y = yorg;
	  ppt++;
	  yorg++;
	}

      /* paint to paintedSet, or if that's NULL, to canvas */
      MI_PAINT_SPANS(paintedSet, pGC->pixels[1], (int)height, pptFirst, pwFirst)

      prect++;
    }
}
