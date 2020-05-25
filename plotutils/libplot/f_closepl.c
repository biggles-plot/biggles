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

/* For FigPlotter objects, we place all user-defined colors [``color
   pseudo-objects''], which must appear first in the .fig file, in a header
   for the page.

   Note that a Fig file may contain no more than a single page of graphics.
   Later pages are simply deallocated by closepl(), even though we go to
   som trouble to prepare them. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_f_end_page (S___(Plotter *_plotter))
{
  int i;
  const char *units;
  plOutbuf *fig_header;
      
  /* prepare Fig header, write it to a plOutbuf */
  fig_header = _new_outbuf ();
      
  units = (_plotter->data->page_data->metric ? "Metric" : "Inches");
  sprintf (fig_header->point,
	   "#FIG 3.2\n%s\n%s\n%s\n%s\n%.2f\n%s\n%d\n%d %d\n",
	   "Portrait",		/* portrait mode, not landscape */
	   "Flush Left",	/* justification */
	   units,		/* "Metric" or "Inches" */
	   _plotter->data->page_data->fig_name, /* paper size */
	   100.00,		/* export and print magnification */
	   "Single",		/* "Single" or "Multiple" pages */
	   -2,			/* color number for transparent color */
	   IROUND(FIG_UNITS_PER_INCH), /* Fig units per inch */
	   2			/* origin in lower left corner (ignored) */
	   );
  _update_buffer (fig_header);
      
  /* output user-defined colors if any */
  for (i = 0; i < _plotter->fig_num_usercolors; i++)
    {
      sprintf (fig_header->point,
	       "#COLOR\n%d %d #%06lx\n",
	       0,	               /* color pseudo-object */
	       FIG_USER_COLOR_MIN + i, /* color num, in xfig's range */
	       _plotter->fig_usercolors[i] /* 24-bit RGB value */
	       );
      _update_buffer (fig_header);
    }
  
  /* place header in the plOutbuf for the page */
  _plotter->data->page->header = fig_header;
  
  return true;
}
