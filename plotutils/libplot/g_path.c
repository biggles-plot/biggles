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

/* This file contains the internal paint_path() and paint_paths() methods,
   which the public method endpath() is a wrapper around. */

/* This file also contains the internal path_is_flushable() method, which
   is invoked after any path segment is added to the segment list, provided
   (0) the segment list has become greater than or equal to the
   `max_unfilled_path_length' Plotter parameter, (1) the path isn't to be
   filled.  In most Plotters, this operation simply returns true. */

/* This file also contains the internal maybe_prepaint_segments() method.
   It is called immediately after any segment is added to a path.  Some
   Plotters, at least under some circumstances, treat endpath() as a no-op,
   and plot the segments of a path in real time, instead.  They accomplish
   this by overloading this method. */

#include "sys-defines.h"
#include "extern.h"

/* In a generic Plotter, paint_path() does nothing. */

void
_pl_g_paint_path (S___(Plotter *_plotter))
{
  return;
}

/* In a generic Plotter, path_is_flushable() simply returns true. */

bool
_pl_g_path_is_flushable (S___(Plotter *_plotter))
{
  return true;
}

/* In a generic Plotter, maybe_prepaint_segments() does nothing. */

void
_pl_g_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
{
  return;
}

/* In a generic Plotter, paint_paths() does nothing but returns `true'. */

bool
_pl_g_paint_paths (S___(Plotter *_plotter))
{
  return true;
}
