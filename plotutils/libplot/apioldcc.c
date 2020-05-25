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

/* This file belongs to both libplot and libplotter.  It contains a
   function that appears in both the old (non-thread-safe) C and C++
   bindings.  It is named pl_parampl() and parampl(), respectively.

   pl_parampl/parampl sets parameters in a global PlotterParams object,
   which is used as a source of parameters when any Plotter is created.
   The presence of this global state is one reason why the old API's are
   not thread-safe.

   In libplotter, parampl is a static function member of the Plotter class,
   as is the global PlotterParams.  This is arranged by #ifdef's in
   extern.h.

   In both libplot and libplotter, the pointer to the global PlotterParams,
   which is called _old_api_global_plotter_params, is defined in
   g_defplot.c. */

#include "sys-defines.h"
#include "extern.h"
#ifndef LIBPLOTTER
#include "plot.h"		/* header file for C API's */
#endif

int
#ifdef LIBPLOTTER
parampl (const char *parameter, void *value)
#else  /* not LIBPLOTTER */
pl_parampl (const char *parameter, void *value)
#endif
{
  /* create global object if necessary (via different routes for libplotter
     and libplot; for latter, call a function in new C API) */
  if (_old_api_global_plotter_params == NULL)
#ifdef LIBPLOTTER
    _old_api_global_plotter_params = new PlotterParams;
#else
    _old_api_global_plotter_params = pl_newplparams ();
#endif

  return _old_api_global_plotter_params->setplparam (R___(_old_api_global_plotter_params)
						     parameter, value);
}
