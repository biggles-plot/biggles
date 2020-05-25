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

/* This file contains the versions of the Plotter methods that (i) take as
   arguments, and/or (ii) return integers rather than doubles. */

#include "sys-defines.h"
#include "extern.h"

int
_API_arc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
{
  return _API_farc (R___(_plotter) 
		    (double)xc, (double)yc, 
		    (double)x0, (double)y0, 
		    (double)x1, (double)y1);
}

int
_API_arcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
{
  return _API_farcrel (R___(_plotter) 
		       (double)dxc, (double)dyc, 
		       (double)dx0, (double)dy0, 
		       (double)dx1, (double)dy1);
}

int
_API_bezier2 (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
{
  return _API_fbezier2 (R___(_plotter) 
			(double)xc, (double)yc, 
			(double)x0, (double)y0, 
			(double)x1, (double)y1);
}

int
_API_bezier2rel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
{
  return _API_fbezier2rel (R___(_plotter) 
			   (double)dxc, (double)dyc, 
			   (double)dx0, (double)dy0, 
			   (double)dx1, (double)dy1);
}

int
_API_bezier3 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  return _API_fbezier3 (R___(_plotter) 
			(double)x0, (double)y0, 
			(double)x1, (double)y1, 
			(double)x2, (double)y2, 
			(double)x3, (double)y3);
}

int
_API_bezier3rel (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  return _API_fbezier3rel (R___(_plotter) 
			   (double)x0, (double)y0, 
			   (double)x1, (double)y1, 
			   (double)x2, (double)y2, 
			   (double)x3, (double)y3);
}

int
_API_box (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
{
  return _API_fbox (R___(_plotter) 
		    (double)x0, (double)y0, (double)x1, (double)y1);
}

int
_API_boxrel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
{
  return _API_fboxrel (R___(_plotter) 
		       (double)dx0, (double)dy0, 
		       (double)dx1, (double)dy1);
}

int
_API_circle (R___(Plotter *_plotter) int x, int y, int r)
{
  return _API_fcircle (R___(_plotter) 
		       (double)x, (double)y, (double)r);
}

int
_API_circlerel (R___(Plotter *_plotter) int dx, int dy, int r)
{
  return _API_fcirclerel (R___(_plotter) 
			  (double)dx, (double)dy, (double)r);
}

int
_API_cont (R___(Plotter *_plotter) int x, int y)
{
  return _API_fcont (R___(_plotter) 
		     (double)x, (double)y);
}

int
_API_contrel (R___(Plotter *_plotter) int dx, int dy)
{
  return _API_fcontrel (R___(_plotter) 
			(double)dx, (double)dy);
}

int
_API_ellarc (R___(Plotter *_plotter) int xc, int yc, int x0, int y0, int x1, int y1)
{
  return _API_fellarc (R___(_plotter) 
		       (double)xc, (double)yc, 
		       (double)x0, (double)y0, 
		       (double)x1, (double)y1);
}

int
_API_ellarcrel (R___(Plotter *_plotter) int dxc, int dyc, int dx0, int dy0, int dx1, int dy1)
{
  return _API_fellarcrel (R___(_plotter) 
			  (double)dxc, (double)dyc, 
			  (double)dx0, (double)dy0, 
			  (double)dx1, (double)dy1);
}

int
_API_ellipse (R___(Plotter *_plotter) int x, int y, int rx, int ry, int angle)
{
  return _API_fellipse (R___(_plotter) 
			(double)x, (double)y, 
			(double)rx, (double)ry, (double)angle);
}

int
_API_ellipserel (R___(Plotter *_plotter) int dx, int dy, int rx, int ry, int angle)
{
  return _API_fellipserel (R___(_plotter) 
			   (double)dx, (double)dy, 
			   (double)rx, (double)ry, 
			   (double)angle);
}

int 
_API_fontname (R___(Plotter *_plotter) const char *s)
{
  double new_size = _API_ffontname (R___(_plotter) s);
  
  return IROUND(new_size);
}

int
_API_fontsize (R___(Plotter *_plotter) int size)
{
  double new_size = _API_ffontsize (R___(_plotter) (double)size);

  return IROUND(new_size);
}

int
_API_line (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
{
  return _API_fline (R___(_plotter) 
		     (double)x0, (double)y0, 
		     (double)x1, (double)y1);
}

int
_API_linedash (R___(Plotter *_plotter) int n, const int *dashes, int offset)
{
  double *idashes;
  int i, retval;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "linedash: invalid operation");
      return -1;
    }

  /* sanity checks */
  if (n < 0 || (n > 0 && dashes == NULL))
    return -1;
  for (i = 0; i < n; i++)
    if (dashes[i] < 0)
      return -1;

  idashes = (double *)_pl_xmalloc ((unsigned int)n * sizeof(double));
  for (i = 0; i < n; i++)
    idashes[i] = dashes[i];
  retval = _API_flinedash (R___(_plotter) 
			   n, idashes, (double)offset);
  free (idashes);

  return retval;
}

int
_API_linerel (R___(Plotter *_plotter) int dx0, int dy0, int dx1, int dy1)
{
  return _API_flinerel (R___(_plotter) 
			(double)dx0, (double)dy0, 
			(double)dx1, (double)dy1);
}

int
_API_labelwidth (R___(Plotter *_plotter) const char *s)
{
  double width = _API_flabelwidth (R___(_plotter) s);
  
  return IROUND(width);
}

int
_API_linewidth (R___(Plotter *_plotter) int new_line_width)
{
  return _API_flinewidth (R___(_plotter) (double)new_line_width);
}

int
_API_marker (R___(Plotter *_plotter) int x, int y, int type, int size)
{
  return _API_fmarker (R___(_plotter) 
		       (double)x, (double)y, 
		       type, (double)size);
}

int
_API_markerrel (R___(Plotter *_plotter) int dx, int dy, int type, int size)
{
  return _API_fmarkerrel (R___(_plotter) 
			  (double)dx, (double)dy, 
			  type, (double)size);
}

int
_API_move (R___(Plotter *_plotter) int x, int y)
{
  return _API_fmove (R___(_plotter) 
		     (double)x, (double)y);
}

int
_API_moverel (R___(Plotter *_plotter) int x, int y)
{
  return _API_fmoverel (R___(_plotter) 
			(double)x, (double)y);
}

int
_API_point (R___(Plotter *_plotter) int x, int y)
{
  return _API_fpoint (R___(_plotter) 
		      (double)x, (double)y);
}

int
_API_pointrel (R___(Plotter *_plotter) int dx, int dy)
{
  return _API_fpointrel (R___(_plotter) 
			 (double)dx, (double)dy);
}

int
_API_space (R___(Plotter *_plotter) int x0, int y0, int x1, int y1)
{
  return _API_fspace (R___(_plotter) 
		      (double)x0, (double)y0, 
		      (double)x1, (double)y1);
}

int
_API_space2 (R___(Plotter *_plotter) int x0, int y0, int x1, int y1, int x2, int y2)
{
  return _API_fspace2 (R___(_plotter) 
		       (double)x0, (double)y0, 
		       (double)x1, (double)y1, 
		       (double)x2, (double)y2);
}

int
_API_textangle (R___(Plotter *_plotter) int angle)
{
  double new_size = _API_ftextangle (R___(_plotter) 
				     (double)angle);

  return IROUND(new_size);
}
