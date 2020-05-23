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

/* This routine paints a set of points.  All painting goes through the
   low-level MI_PAINT_SPANS() macro. */

/* ARGS: mode = Origin or Previous */
void
miDrawPoints_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
{
  unsigned int	*pwidthInit, *pwidth;
  int		i;
  miPoint 	*ppt = (miPoint *)NULL;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  ppt = (miPoint *)mi_xmalloc (npt * sizeof(miPoint));
  if (mode == MI_COORD_MODE_PREVIOUS)
    /* convert from relative to absolute coordinates */
    {
      ppt[0] = pPts[0];
      for (i = 1; i < npt; i++)
	{
	  ppt[i].x = ppt[i-1].x + pPts[i].x;
	  ppt[i].y = ppt[i-1].y + pPts[i].y;	  
	}
    }
  else
    /* just copy */
    {
      for (i = 0; i < npt; i++)
	ppt[i] = pPts[i];
    }

  pwidthInit = (unsigned int *)mi_xmalloc (npt * sizeof(unsigned int));
  pwidth = pwidthInit;
  for (i = 0; i < npt; i++)
    *pwidth++ = 1;

  if (npt > 1)
    miQuickSortSpansY (ppt, pwidthInit, npt);
  MI_PAINT_SPANS(paintedSet, pGC->pixels[1], npt, ppt, pwidthInit)
}

