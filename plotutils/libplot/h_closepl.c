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

/* This version is for both HPGLPlotters and PCLPlotters.  

   For HPGL Plotter objects, we output all plotted objects, which we have
   saved in a resizable outbuf structure for the current page.  An HP-GL or
   HP-GL/2 prologue and epilogue are included.  We then flush the output
   stream, and reset all datastructures. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_h_end_page (S___(Plotter *_plotter))
{
  /* output HP-GL epilogue to page buffer */

  if (_plotter->hpgl_pendown == true)
    /* lift pen */
    {
      sprintf (_plotter->data->page->point, "PU;");
      _update_buffer (_plotter->data->page);
    }
  /* move to lower left hand corner */
  sprintf (_plotter->data->page->point, "PA0,0;");
  _update_buffer (_plotter->data->page);

  /* select pen zero, i.e. return pen to carousel */
  if (_plotter->hpgl_pen != 0)
    {
      sprintf (_plotter->data->page->point, "SP0;");
      _update_buffer (_plotter->data->page);
    }

  if (_plotter->hpgl_version >= 1)
    /* have a `page advance' command, so use it */
    {
      sprintf (_plotter->data->page->point, "PG0;");
      _update_buffer (_plotter->data->page);
    }

  /* add newline at end */
  sprintf (_plotter->data->page->point, "\n");
  _update_buffer (_plotter->data->page);

  /* if a PCL Plotter, switch back from HP-GL/2 mode to PCL mode */
  _maybe_switch_from_hpgl (S___(_plotter));
  
  /* set this, so that no drawing on the next page will take place without
     a pen advance */
  _plotter->hpgl_position_is_unknown = true;

  _plotter->hpgl_pendown = false; /* be on the safe side */

  return true;
}

void
_pl_h_maybe_switch_from_hpgl (S___(Plotter *_plotter))
{
}

void
_pl_q_maybe_switch_from_hpgl (S___(Plotter *_plotter))
{
  /* switch back from HP-GL/2 to PCL 5 mode */
  strcpy (_plotter->data->page->point, "\033%0A");
  _update_buffer (_plotter->data->page);
}
