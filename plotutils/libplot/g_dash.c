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

/* This file contains the linedash method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the dash array used for
   subsequent drawing of paths. */

#include "sys-defines.h"
#include "extern.h"

int
_API_flinedash (R___(Plotter *_plotter) int n, const double *dashes, double offset)
{
  double *dash_array;
  int i;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter)
		       "flinedash: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->path)
    _API_endpath (S___(_plotter)); /* flush path if any */

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0.0)
      return -1;

  if (_plotter->drawstate->dash_array_len > 0)
    free ((double *)_plotter->drawstate->dash_array);
  if (n > 0)
    dash_array = (double *)_pl_xmalloc (n * sizeof(double));
  else
    dash_array = NULL;

  _plotter->drawstate->dash_array_len = n;
  for (i = 0; i < n; i++)
    dash_array[i] = dashes[i];
  _plotter->drawstate->dash_array = dash_array;
  _plotter->drawstate->dash_offset = offset;

  /* for future paths, use dash array rather than line mode */
  _plotter->drawstate->dash_array_in_effect = true;

  return 0;
}

