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

/* This file defines the default mapping from `virtual' linemodes (the
   linemodes reported by the point-reader) to physical linemodes.  On
   monochrome displays, a physical linemode is simply a linemode, in the
   traditional libplot sense.  On color displays, it involves a choice of
   color as well; see explanation at head of plotter.c.  NO_OF_LINEMODES is
   defined in extern.h. */

#include "sys-defines.h"
#include "plot.h"
#include "extern.h"

/* following line types are the five used by Unix graph(1) */

const char *linemodes[NO_OF_LINEMODES] =
{
  "solid", "dotted", "dotdashed", "shortdashed", "longdashed"
};

/* following colors are the first five used by the gnuplot X11 driver */

const char *colorstyle[NO_OF_LINEMODES] =
{
  "red", "green", "blue", "magenta", "cyan"
};
