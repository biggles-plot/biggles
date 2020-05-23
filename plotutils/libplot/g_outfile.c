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

/* This file contains the outfile method, which is a GNU extension to
   libplot.  It selects an output stream for all subsequent plot commands.
   The outfile method may only be invoked outside an openpl()...closepl()
   pair. */

/* THIS METHOD IS NOW DEPRECATED.  IT WILL SOON GO AWAY. */

#include "sys-defines.h"
#include "extern.h"

/* outfile takes a single argument, a stream that has been opened for
   writing.  It may be called only outside an openpl()....closepl() pair.
   It switches all future output to the new, specified stream.  The old
   output stream, which is not closed, is returned. */

FILE *
_API_outfile(R___(Plotter *_plotter) FILE *outfile)
{
  FILE *oldoutfile;
  
  if (_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "outfile: invalid operation");
      return (FILE *)NULL;
    }

  oldoutfile = _plotter->data->outfp;
  _plotter->data->outfp = outfile;
#ifdef LIBPLOTTER
  _plotter->data->outstream = NULL;
#endif

  _plotter->data->page_number = 0;	/* reset */

  return oldoutfile;
}
