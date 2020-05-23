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

#define EPSILON	0.000001
#define ISEQUAL(a,b) (FABS((a) - (b)) <= EPSILON)
#define UNEQUAL(a,b) (FABS((a) - (b)) > EPSILON)
#define PTISEQUAL(a,b) (ISEQUAL(a.x,b.x) && ISEQUAL(a.y,b.y))

/* Point with sub-pixel positioning.  In this case we use doubles, but
 * see mi_fplycon.c for other possibilities.
 */
typedef struct
{
  double	x, y;
} SppPoint;

/* Arc with sub-pixel positioning. */
typedef struct 
{
  double	x, y, width, height;
  double	angle1, angle2;
} SppArc;

extern void miFillSppPoly (miPaintedSet *paintedSet, miPixel pixel, int count, const SppPoint *ptsIn, int xTrans, int yTrans, double xFtrans, double yFtrans);
