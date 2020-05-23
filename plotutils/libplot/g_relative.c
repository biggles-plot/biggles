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

/* This file contains the libplot methods that take floating-point relative
   coordinates as arguments. */

#include "sys-defines.h"
#include "extern.h"

int
_API_farcrel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
{
  return _API_farc (R___(_plotter) 
		    _plotter->drawstate->pos.x + dxc, 
		    _plotter->drawstate->pos.y + dyc,
		    _plotter->drawstate->pos.x + dx0, 
		    _plotter->drawstate->pos.y + dy0,
		    _plotter->drawstate->pos.x + dx1, 
		    _plotter->drawstate->pos.y + dy1);
}

int
_API_fbezier2rel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
{
  return _API_fbezier2 (R___(_plotter) 
			_plotter->drawstate->pos.x + dxc, 
			_plotter->drawstate->pos.y + dyc,
			_plotter->drawstate->pos.x + dx0, 
			_plotter->drawstate->pos.y + dy0,
			_plotter->drawstate->pos.x + dx1, 
			_plotter->drawstate->pos.y + dy1);
}

int
_API_fbezier3rel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
{
  return _API_fbezier3 (R___(_plotter) 
			_plotter->drawstate->pos.x + dx0, 
			_plotter->drawstate->pos.y + dy0,
			_plotter->drawstate->pos.x + dx1, 
			_plotter->drawstate->pos.y + dy1,
			_plotter->drawstate->pos.x + dx2,
			_plotter->drawstate->pos.y + dy2,
			_plotter->drawstate->pos.x + dx3, 
			_plotter->drawstate->pos.y + dy3);
}

int
_API_fellarcrel (R___(Plotter *_plotter) double dxc, double dyc, double dx0, double dy0, double dx1, double dy1)
{
  return _API_fellarc (R___(_plotter) 
		       _plotter->drawstate->pos.x + dxc, 
		       _plotter->drawstate->pos.y + dyc,
		       _plotter->drawstate->pos.x + dx0, 
		       _plotter->drawstate->pos.y + dy0,
		       _plotter->drawstate->pos.x + dx1, 
		       _plotter->drawstate->pos.y + dy1);
}

int
_API_fboxrel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1)
{
  return _API_fbox (R___(_plotter) 
		    _plotter->drawstate->pos.x + dx0, 
		    _plotter->drawstate->pos.y + dy0,
		    _plotter->drawstate->pos.x + dx1, 
		    _plotter->drawstate->pos.y + dy1);
}

int
_API_fcirclerel (R___(Plotter *_plotter) double dx, double dy, double r)
{
  return _API_fcircle (R___(_plotter) 
		       _plotter->drawstate->pos.x + dx, 
		       _plotter->drawstate->pos.y + dy, r);
}

int
_API_fellipserel (R___(Plotter *_plotter) double dx, double dy, double rx, double ry, double angle)
{
  return _API_fellipse (R___(_plotter) 
			_plotter->drawstate->pos.x + dx, 
			_plotter->drawstate->pos.y + dy, 
			rx, ry, angle);
}

int
_API_fcontrel (R___(Plotter *_plotter) double dx, double dy)
{
  return _API_fcont (R___(_plotter) 
		     _plotter->drawstate->pos.x + dx, 
		     _plotter->drawstate->pos.y + dy);
}

int
_API_flinerel (R___(Plotter *_plotter) double dx0, double dy0, double dx1, double dy1)
{
  return _API_fline (R___(_plotter) 
		     _plotter->drawstate->pos.x + dx0, 
		     _plotter->drawstate->pos.y + dy0,
		     _plotter->drawstate->pos.x + dx1, 
		     _plotter->drawstate->pos.y + dy1);
}

int
_API_fmarkerrel (R___(Plotter *_plotter) double dx, double dy, int type, double size)
{
  return _API_fmarker (R___(_plotter) 
		       _plotter->drawstate->pos.x + dx, 
		       _plotter->drawstate->pos.y + dy, 
		       type, size);
}

int
_API_fmoverel (R___(Plotter *_plotter) double x, double y)
{
  return _API_fmove (R___(_plotter) 
		     _plotter->drawstate->pos.x + x, 
		     _plotter->drawstate->pos.y + y);
}

int
_API_fpointrel (R___(Plotter *_plotter) double dx, double dy)
{
  return _API_fpoint (R___(_plotter) 
		      _plotter->drawstate->pos.x + dx, 
		      _plotter->drawstate->pos.y + dy);
}
