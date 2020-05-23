/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

#include "sys-defines.h"
#include "extern.h"

/* Computes the product of two PS-style transformation matrices
   (i.e. matrix representations of affine transformations). */

void
_matrix_product (const double m[6], const double n[6], double product[6])
{
  double local_product[6];

  local_product[0] = m[0] * n[0] + m[1] * n[2];
  local_product[1] = m[0] * n[1] + m[1] * n[3];

  local_product[2] = m[2] * n[0] + m[3] * n[2];  
  local_product[3] = m[2] * n[1] + m[3] * n[3];

  local_product[4] = m[4] * n[0] + m[5] * n[2] + n[4];
  local_product[5] = m[4] * n[1] + m[5] * n[3] + n[5];

  memcpy (product, local_product, 6 * sizeof (double));

  return;
}

/* Computes the inverse of a PS-style transformation matrix, which should
   be nonsingular for correct results. */

void
_matrix_inverse (const double m[6], double inverse[6])
{
  double det = m[0] * m[3] - m[1] * m[2];

  if (det == 0.0)
    /* bogus */
    inverse[0] = inverse[1] = inverse[2] = inverse[3]
      = inverse[4] = inverse[5] = 0.0;
  else
    {
      double invdet = 1.0 / det;
      
      inverse[0] = invdet * m[3];
      inverse[1] = - invdet * m[1];
      inverse[2] = - invdet * m[2];
      inverse[3] = invdet * m[0];
      inverse[4] = invdet * (m[2] * m[5] - m[3] * m[4]);
      inverse[5] = invdet * (m[1] * m[4] - m[0] * m[5]);
    }
}

/* _matrix_norm computes the matrix norm (in the l^2 sense) of the linear
   transformation part of a PS-style transformation matrix.  Actually we
   compute instead the geometric mean of the l^1 and l^infinity norms.  By
   Hadamard's 3-line theorem, this geometric mean is an upper bound on the
   true l^2 norm.

   This function is called only to select appropriate line widths and font
   sizes.  For the purposes of those functions, the above approximation
   should suffice. */

double 
_matrix_norm (const double m[6])
{
  double mt[4], pm[4];
  double norm1, norm2;
  double a[4];
  int i;
  
  mt[0] = m[0];			/* transpose of m */
  mt[1] = m[2];
  mt[2] = m[1];
  mt[3] = m[3];
  
  pm[0] = m[0] * mt[0] + m[1] * mt[2]; /* pm = m * mt */
  pm[1] = m[0] * mt[1] + m[1] * mt[3];  
  pm[2] = m[2] * mt[0] + m[3] * mt[2];
  pm[3] = m[2] * mt[1] + m[3] * mt[3];  

  for (i = 0; i < 4; i++)
    a[i] = fabs(pm[i]);
  
  /* compute l^1 and l^infinity norms of m * mt */
  norm1 = DMAX(a[0]+a[1], a[2]+a[3]);
  norm2 = DMAX(a[0]+a[2], a[1]+a[3]);  
  
 /* l^2 norm of m is sqrt of l^2 norm of m * mt */
  return sqrt(sqrt(norm1 * norm2));
}     

/* Compute the minimum and maximum singular values of a 2-by-2 matrix M.
   The singular values are the square roots of the eigenvalues of M times
   its transpose. */

void
_matrix_sing_vals (const double m[6], double *min_sing_val, double *max_sing_val)
{
  double mt[4], pm[4];
  double trace, det, disc, sqrtdisc;
  double s1, s2;

  mt[0] = m[0];			/* transpose of m */
  mt[1] = m[2];
  mt[2] = m[1];
  mt[3] = m[3];
  
  pm[0] = m[0] * mt[0] + m[1] * mt[2]; /* pm = m * mt */
  pm[1] = m[0] * mt[1] + m[1] * mt[3];  
  pm[2] = m[2] * mt[0] + m[3] * mt[2];
  pm[3] = m[2] * mt[1] + m[3] * mt[3];  

  trace = pm[0] + pm[3];
  det = pm[0] * pm[3] - pm[1] * pm[2];
  /* s^2 + b s + c = 0, where b = -trace, c = det */
  disc = trace * trace - 4.0 * det;
  disc = DMAX(0.0, disc);	/* paranoia */
  sqrtdisc = sqrt (disc);
  s1 = 0.5 * (trace - sqrtdisc);
  s2 = 0.5 * (trace + sqrtdisc);  
  s1 = DMAX(0.0, s1);		/* paranoia */
  s2 = DMAX(0.0, s2);		/* paranoia */

  *min_sing_val = sqrt(s1);
  *max_sing_val = sqrt(s2);
}
