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

/*
 * Written by Brian Kelleher; June 1986
 *
 * Draw a polygon (supplied as a polyline, i.e. an array of points), via
 * one of two scan conversion routines.
 */

void
miFillPolygon_internal (miPaintedSet *paintedSet, const miGC *pGC, miPolygonShape shape, miCoordMode mode, int count, const miPoint *pPts)
{
  miPoint *ppt = (miPoint *)NULL;
  const miPoint *q;
    
  /* ensure we have >=1 points */
  if (count <= 0)
    return;

  if (mode == MI_COORD_MODE_PREVIOUS)
    /* convert from relative to absolute coordinates */
    {
      int i;

      ppt = (miPoint *)mi_xmalloc (count * sizeof(miPoint));
      ppt[0] = pPts[0];
      for (i = 1; i < count; i++)
	{
	  ppt[i].x = ppt[i-1].x + pPts[i].x;
	  ppt[i].y = ppt[i-1].y + pPts[i].y;	  
	}
      q = ppt;
    }
  else
    q = pPts;

  switch ((int)shape)
    {
    case (int)MI_SHAPE_GENERAL:
    default:
      /* use general scan conversion routine */
      miFillGeneralPoly (paintedSet, pGC, count, q);
      break;
    case (int)MI_SHAPE_CONVEX:
      /* use special (faster) routine */
      miFillConvexPoly (paintedSet, pGC, count, q);
      break;
    }

  if (mode == MI_COORD_MODE_PREVIOUS)
    free (ppt);
}


