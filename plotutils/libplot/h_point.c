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

/* size of a `point' as fraction of diagonal distance between scaling
   points P1 and P2, i.e. as fraction of distance between opposite corners
   of the viewport */
#define POINT_HPGL_SIZE 0.0001

void
_pl_h_paint_point (S___(Plotter *_plotter))
{
  int saved_join_type, saved_cap_type;

  if (_plotter->drawstate->pen_type != 0)
    /* have a pen to draw with */
    {
      /* Sync pen color.  This may set the _plotter->hpgl_bad_pen flag (if
	 optimal pen is #0 [white] and we're not allowed to use pen #0 to
	 draw with).  So we test _plotter->hpgl_bad_pen before drawing the
	 point (see below). */
      _pl_h_set_pen_color (R___(_plotter) HPGL_OBJECT_PATH);

      /* temporarily store line attributes */
      saved_join_type = _plotter->drawstate->join_type;
      saved_cap_type = _plotter->drawstate->cap_type;  
      _plotter->drawstate->join_type = PL_JOIN_ROUND;
      _plotter->drawstate->cap_type = PL_CAP_ROUND;  
      
      /* sync line attributes and pen position */
      _pl_h_set_attributes (S___(_plotter));
      _pl_h_set_position (S___(_plotter));
      
      /* we wish to set a pen width in terms of HP-GL coordinates, which
	 _pl_h_set_attributes can't do; so we do it specially */
      if (_plotter->hpgl_version == 2)
	{
	  if (_plotter->hpgl_pen_width != POINT_HPGL_SIZE)
	    {
	      sprintf (_plotter->data->page->point, "PW%.4f;", 
		       100.0 * POINT_HPGL_SIZE);
	      _update_buffer (_plotter->data->page);
	      _plotter->hpgl_pen_width = POINT_HPGL_SIZE;
	    }
	}

      if (_plotter->hpgl_bad_pen == false)
	/* no problems with nonexistent pen */
	{
	  if (_plotter->hpgl_pendown == false)
	    /* N.B. if pen were down, point would be invisible */
	    {
	      strcpy (_plotter->data->page->point, "PD;");
	      _update_buffer (_plotter->data->page);
	      _plotter->hpgl_pendown = true;
	    }
	  strcpy (_plotter->data->page->point, "PU;");
	  _update_buffer (_plotter->data->page);
	  _plotter->hpgl_pendown = false;
	}
      
      /* restore line attributes */
      _plotter->drawstate->join_type = saved_join_type;
      _plotter->drawstate->cap_type = saved_cap_type;
    }
}
