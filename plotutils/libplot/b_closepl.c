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
#include "xmi.h"

bool
_pl_b_end_page (S___(Plotter *_plotter))
{
  int retval;

  /* Possibly output the page's bitmap.  In the base BitmapPlotter class
     this is a no-op (see below), but it may do something in derived
     classes. */
  retval = _maybe_output_image (S___(_plotter));

  /* tear down */
  _pl_b_delete_image (S___(_plotter));

  return (retval < 0 ? false : true);
}

/* tear down image, i.e. deallocate libxmi canvas */
void
_pl_b_delete_image (S___(Plotter *_plotter))
{
  /* deallocate libxmi's drawing canvas (and painted set struct too) */
  miDeleteCanvas ((miCanvas *)_plotter->b_canvas);
  _plotter->b_canvas = (void *)NULL;
  miDeletePaintedSet ((miPaintedSet *)_plotter->b_painted_set);
  _plotter->b_painted_set = (void *)NULL;
}

/* This is the BitmapPlotter-specific version of the _maybe_output_image()
   method, which is invoked when a page is finished.  It's a no-op; in
   derived classes such as the PNMPlotter and PNGPlotter classes, it's
   overridden by a version that actually does something.  */

int
_pl_b_maybe_output_image (S___(Plotter *_plotter))
{
  return 0;
}  

