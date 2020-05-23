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

/* The internal point-drawing function, which point() is a wrapper around.
   It draws a point at the current location.  There is no standard
   definition of `point', so any Plotter is free to implement this as it
   sees fit. */

#include "sys-defines.h"
#include "extern.h"

static const double identity_matrix[6] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

/* forward references */
static void write_svg_point_style (plOutbuf *page, const plDrawState *drawstate);

void
_pl_s_paint_point (S___(Plotter *_plotter))
{
  sprintf (_plotter->data->page->point, "<circle ");
  _update_buffer (_plotter->data->page);

  _pl_s_set_matrix (R___(_plotter) identity_matrix); 

  sprintf (_plotter->data->page->point,
	   "cx=\"%.5g\" cy=\"%.5g\" r=\"%s\" ",
	   _plotter->drawstate->pos.x,
	   _plotter->drawstate->pos.y,
	   "0.5px");		/* diameter = 1 pixel */
  _update_buffer (_plotter->data->page);
  
  write_svg_point_style (_plotter->data->page, _plotter->drawstate);

  sprintf (_plotter->data->page->point,
	   "/>\n");
  _update_buffer (_plotter->data->page);

  return;
}

static void
write_svg_point_style (plOutbuf *page, const plDrawState *drawstate)
{
  char color_buf[8];		/* enough room for "#ffffff", incl. NUL */

  sprintf (page->point, "stroke=\"none\" ");
  _update_buffer (page);
  
  sprintf (page->point, "fill=\"%s\"",
	   _libplot_color_to_svg_color (drawstate->fgcolor, color_buf));
  _update_buffer (page);
}
