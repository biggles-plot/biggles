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

/* This file contains the internal path_is_flushable() method, which is
   invoked after any path segment is added to the segment list, provided
   (0) the segment list has become greater than or equal to the
   `max_unfilled_path_length' Plotter parameter, (1) the path isn't to be
   filled.  In most Plotters, this operation simply returns true. */

/* This file also contains the internal maybe_prepaint_segments() method.
   It is called immediately after any segment is added to a path.  Some
   Plotters, at least under some circumstances, treat endpath() as a no-op.
   Instead, they plot the segments of a path in real time.  */

/**********************************************************************/

/* This version of the internal method path_is_flushable() is for Tektronix
   Plotters, which (1) reduce all paths to line segments as they are
   constructed, and (2) plot the resulting line segments ("vectors") in
   real time.  Unlike the generic version, which returns true, this version
   returns false. */

#include "sys-defines.h"
#include "extern.h"

bool
_pl_t_path_is_flushable (S___(Plotter *_plotter))
{
  return false;
}

/**********************************************************************/

/* This version of the internal method maybe_prepaint_segments() is for
   Tektronix Plotters, which (1) reduce all paths to line segments as they
   are constructed, and (2) plot the resulting line segments ("vectors") in
   real time.  This is what does all the painting of line segments on a
   Tektronix display. */

/* for Cohen-Sutherland clipper, see g_clipper.c */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

void
_pl_t_maybe_prepaint_segments (R___(Plotter *_plotter) int prev_num_segments)
{
  int i;

  /* sanity check */
  if (_plotter->drawstate->path->num_segments < 2)
    return;

  if (_plotter->drawstate->path->num_segments == prev_num_segments)
    /* nothing to paint */
    return;

  /* skip drawing if pen level is set to `0' */
  if (_plotter->drawstate->pen_type == 0)
    return;

  /* Skip drawing if the pen color is white.  Since our TekPlotter class
     doesn't support filling, this is ok to do if the Tektronix isn't a
     kermit emulator (the kermit emulator supports color). */
  if (_plotter->tek_display_type != TEK_DPY_KERMIT 
      && _plotter->drawstate->fgcolor.red == 0xffff
      && _plotter->drawstate->fgcolor.green == 0xffff
      && _plotter->drawstate->fgcolor.blue == 0xffff)
    return;

  /* iterate over all new segments, i.e. segments to be painted */

  for (i = IMAX(1, prev_num_segments);
       i < _plotter->drawstate->path->num_segments;
       i++)
    {
      plPoint start, end;	/* endpoints of line seg. (in device coors) */
      plIntPoint istart, iend;	/* same, quantized to integer Tek coors */
      int clipval;
      bool same_point, force;

      /* nominal starting point and ending point for new line segment, in
	 floating point device coordinates */
      start.x = XD(_plotter->drawstate->path->segments[i-1].p.x,
		   _plotter->drawstate->path->segments[i-1].p.y);
      start.y = YD(_plotter->drawstate->path->segments[i-1].p.x,
		   _plotter->drawstate->path->segments[i-1].p.y);
      end.x = XD(_plotter->drawstate->path->segments[i].p.x,
		 _plotter->drawstate->path->segments[i].p.y);
      end.y = YD(_plotter->drawstate->path->segments[i].p.x,
		 _plotter->drawstate->path->segments[i].p.y);
      same_point = (start.x == end.x && start.y == end.y) ? true : false;

      /* clip line segment to rectangular clipping region in device frame */
      clipval = _clip_line (&start.x, &start.y, &end.x, &end.y,
			    TEK_DEVICE_X_MIN_CLIP, TEK_DEVICE_X_MAX_CLIP,
			    TEK_DEVICE_Y_MIN_CLIP, TEK_DEVICE_Y_MAX_CLIP);
      if (!(clipval & ACCEPTED))	/* line segment is OOB */
	continue;

      /* convert clipped starting point, ending point to integer Tek coors */
      istart.x = IROUND(start.x);
      istart.y = IROUND(start.y);
      iend.x = IROUND(end.x);
      iend.y = IROUND(end.y);

      if (i == 1)
	/* New polyline is beginning, so start to draw it on the display:
	   move to starting point of the first line segment, in Tek space.
	   As a side-effect, the escape sequence emitted by _pl_t_tek_move()
	   will shift the Tektronix to the desired mode, either PLOT or
	   POINT.  Note that if we are already in the desired mode,
	   emitting the escape sequence will prevent a line being drawn at
	   the time of the move (the "dark vector" concept).  That is of
	   course what we want. */
	_pl_t_tek_move (R___(_plotter) istart.x, istart.y);
      else
	/* A polyline is underway, >=1 line segments already.  So check
	   whether the position on the Tektronix is the same as the
	   starting point of the new line segment; if it differs, move to
	   the latter.  Such a difference can occur on account of clipping.
	   Also the Tektronix position could have changed on us if a
	   savestate()...restorestate() occurred since the last call to
	   cont(). */
	{
	  int correct_tek_mode = 
	    _plotter->drawstate->points_are_connected ? TEK_MODE_PLOT : TEK_MODE_POINT;

	  if (_plotter->tek_position_is_unknown
	      || _plotter->tek_pos.x != istart.x
	      || _plotter->tek_pos.y != istart.y
	      || _plotter->tek_mode_is_unknown
	      || _plotter->tek_mode != correct_tek_mode)
	    /* Move to desired position.  This automatically shifts the
	       Tektronix to correct mode, PLOT or POINT; see comment
	       above. */
	    _pl_t_tek_move (R___(_plotter) istart.x, istart.y);
	}
  
      /* Sync Tek's linestyle with ours; an escape sequence is emitted only
	 if necessary.  Linestyle could have changed on us if a
	 savestate()...restorestate() occurred since the last call to
	 cont().  Sync Tek's color and background color too (significant
	 only for kermit Tek emulator). */
      _pl_t_set_attributes (S___(_plotter));  
      _pl_t_set_pen_color (S___(_plotter));
      _pl_t_set_bg_color (S___(_plotter));

      /* If this is initial line segment of a polyline, force output of a
	 vector even if line segment has zero length, so that something
	 visible will appear on the display.  We do this only if (1) the
	 line segment in the user frame was of nonzero length, or (2) it
	 was of zero length but the cap mode is "round".  This more or less
	 agrees with our convention, on bitmap Plotters (X, PNM, GIF,
	 etc.), for dealing with device-frame vectors that are of
	 (quantized) zero length.  */
      if (i == 1
	  && (same_point == false 
	      || (same_point == true 
		  && _plotter->drawstate->cap_type == PL_CAP_ROUND)))
	force = true;
      else 
	force = false;

      /* continue polyline by drawing vector on Tek display */
      _pl_t_tek_vector_compressed (R___(_plotter) 
				   iend.x, iend.y, istart.x, istart.y, force);
      
      /* update our notion of Tek's notion of position */
      _plotter->tek_pos.x = iend.x;
      _plotter->tek_pos.y = iend.y;
    }
}
