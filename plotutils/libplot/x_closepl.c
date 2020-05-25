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

bool
_pl_x_end_page (S___(Plotter *_plotter))
{
  /* Xdrawable Plotters support double buffering `by hand', so check for it */

  if (_plotter->x_double_buffering == X_DBL_BUF_BY_HAND)
    /* copy final frame of buffered graphics from pixmap serving as
       graphics buffer, to window */
    {
      /* compute rectangle size; note flipped-y convention */
      int window_width = (_plotter->data->imax - _plotter->data->imin) + 1;
      int window_height = (_plotter->data->jmin - _plotter->data->jmax) + 1;
      
      if (_plotter->x_drawable1)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable1,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      if (_plotter->x_drawable2)
	XCopyArea (_plotter->x_dpy, _plotter->x_drawable3, _plotter->x_drawable2,
		   _plotter->drawstate->x_gc_bg,		   
		   0, 0,
		   (unsigned int)window_width, (unsigned int)window_height,
		   0, 0);
      
      /* no more need for pixmap, so free it (if there is one) */
      if (_plotter->x_drawable1 || _plotter->x_drawable2)
	XFreePixmap (_plotter->x_dpy, _plotter->x_drawable3);
    }

  /* do teardown of X-specific elements of the first drawing state on the
     drawing state stack */
  _pl_x_delete_gcs_from_first_drawing_state (S___(_plotter));
  
  return true;
}

void
_pl_x_delete_gcs_from_first_drawing_state (S___(Plotter *_plotter))
{
  /* free graphics contexts, if we have them -- and to have them, must have
     at least one drawable (see x_savestate.c) */
  if (_plotter->x_drawable1 || _plotter->x_drawable2)
    {
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill);
      XFreeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_bg);
    }
}
