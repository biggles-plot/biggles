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

/* bit fields for return value from Cohen-Sutherland clipper */
enum { ACCEPTED = 0x1, CLIPPED_FIRST = 0x2, CLIPPED_SECOND = 0x4 };

/* for internal clipper use */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

/* forward references */
static int compute_outcode (double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip);

/* _clip_line() takes two points, the endpoints of a line segment in the
 * device frame (expressed in terms of floating-point device coordinates),
 * and destructively passes back two points: the endpoints of the line
 * segment clipped by Cohen-Sutherland to the rectangular clipping area.
 * The return value contains bitfields ACCEPTED, CLIPPED_FIRST, and
 * CLIPPED_SECOND.
 */

int
_clip_line (double *x0_p, double *y0_p, double *x1_p, double *y1_p, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip)
{
  double x0 = *x0_p;
  double y0 = *y0_p;
  double x1 = *x1_p;
  double y1 = *y1_p;
  int outcode0, outcode1;
  bool accepted;
  int clipval = 0;
  
  outcode0 = compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
  outcode1 = compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip);  

  for ( ; ; )
    {
      if (!(outcode0 | outcode1)) /* accept */
	{
	  accepted = true;
	  break;
	}
      else if (outcode0 & outcode1) /* reject */
	{
	  accepted = false;
	  break;
	}
      else
	{
	  /* at least one endpoint is outside; choose one that is */
	  int outcode_out = (outcode0 ? outcode0 : outcode1);
	  double x, y;		/* intersection with clip edge */
	  
	  if (outcode_out & RIGHT)	  
	    {
	      x = x_max_clip;
	      y = y0 + (y1 - y0) * (x_max_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & LEFT)
	    {
	      x = x_min_clip;
	      y = y0 + (y1 - y0) * (x_min_clip - x0) / (x1 - x0);
	    }
	  else if (outcode_out & TOP)
	    {
	      x = x0 + (x1 - x0) * (y_max_clip - y0) / (y1 - y0);
	      y = y_max_clip;
	    }
	  else
	    {
	      x = x0 + (x1 - x0) * (y_min_clip - y0) / (y1 - y0);
	      y = y_min_clip;
	    }
	  
	  if (outcode_out == outcode0)
	    {
	      x0 = x;
	      y0 = y;
	      outcode0 = compute_outcode (x0, y0, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
	    }
	  else
	    {
	      x1 = x; 
	      y1 = y;
	      outcode1 = compute_outcode (x1, y1, x_min_clip, x_max_clip, y_min_clip, y_max_clip);
	    }
	}
    }

  if (accepted)
    {
      clipval |= ACCEPTED;
      if ((x0 != *x0_p) || (y0 != *y0_p))
	clipval |= CLIPPED_FIRST;
      if ((x1 != *x1_p) || (y1 != *y1_p))
	clipval |= CLIPPED_SECOND;
      *x0_p = x0;
      *y0_p = y0;
      *x1_p = x1;
      *y1_p = y1;
    }

  return clipval;
}

static int
compute_outcode (double x, double y, double x_min_clip, double x_max_clip, double y_min_clip, double y_max_clip)
{
  int code = 0;

  if (x > x_max_clip)
    code |= RIGHT;
  else if (x < x_min_clip)
    code |= LEFT;
  if (y > y_max_clip)
    code |= TOP;
  else if (y < y_min_clip)
    code |= BOTTOM;
  
  return code;
}
