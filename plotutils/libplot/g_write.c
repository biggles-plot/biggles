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

/* These are the lowest-level output routines in libplot/libplotter.
   Plotters that write to output streams use these. */

#include "sys-defines.h"
#include "extern.h"

void
_write_byte (const plPlotterData *data, unsigned char c)
{
  if (data->outfp)
    putc ((int)c, data->outfp);
#ifdef LIBPLOTTER
  else if (data->outstream)
    data->outstream->put (c);
#endif
}

void
_write_bytes (const plPlotterData *data, int n, const unsigned char *c)
{
  int i;

  if (data->outfp)
    {
      for (i = 0; i < n; i++)
	putc ((int)(c[i]), data->outfp);
    }
#ifdef LIBPLOTTER
  else if (data->outstream)
    data->outstream->write((const char *)c, n);
#endif
}

void
_write_string (const plPlotterData *data, const char *s)
{
  if (data->outfp)
    fputs (s, data->outfp);
#ifdef LIBPLOTTER
  else if (data->outstream)
    (*(data->outstream)) << s;
#endif
}
