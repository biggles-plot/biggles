/* This file is part of the GNU libxmi package.  Copyright (C) 1998, 1999,
   2000, 2005, Free Software Foundation, Inc.

   The GNU libxmi package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU libxmi package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
/* The reentrant functions miPolyArc_r() and miZeroPolyArc_r() take a
   pointer to a user-supplied cache of rasterized elliptical arcs as an
   argument.  Their non-reentrant counterparts miPolyArc() and
   miZeroPolyArc() use a pointer to an in-library cache. */
extern miEllipseCache * _mi_ellipseCache;
#endif /* NO_NONREENTRANT_POLYARC_SUPPORT */
