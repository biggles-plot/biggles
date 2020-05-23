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

/* This file contains the translate, rotate and scale methods, which are
   GNU extensions to libplot.  They affect the affine transformation from
   user coordinates to device coordinates, as in Postscript. */

#include "sys-defines.h"
#include "extern.h"

int
_API_ftranslate (R___(Plotter *_plotter) double x, double y)
{
  double m0, m1, m2, m3, m4, m5;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "ftranslate: invalid operation");
      return -1;
    }

  m0 = m3 = 1.0;
  m1 = m2 = 0.0;
  m4 = x;
  m5 = y;
  _API_fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);
  
  return 0;
}

int
_API_frotate (R___(Plotter *_plotter) double theta)
{
  double m0, m1, m2, m3, m4, m5;
  double radians = M_PI * theta / 180.0;
  
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "frotate: invalid operation");
      return -1;
    }

  m0 = m3 = cos (radians);
  m1 = sin (radians);
  m2 = - sin (radians);
  m4 = m5 = 0.0;
  _API_fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);

  return 0;
}

int
_API_fscale (R___(Plotter *_plotter) double x, double y)
{
  double m0, m1, m2, m3, m4, m5;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "fscale: invalid operation");
      return -1;
    }

  m0 = x;
  m3 = y;
  m1 = m2 = m4 = m5 = 0.0;
  _API_fconcat (R___(_plotter) m0, m1, m2, m3, m4, m5);

  return 0;
}
