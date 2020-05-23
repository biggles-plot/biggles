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

/* This file contains the generic warning and error methods.  They simply
   write the specified message to the plotter error stream, if it has one.
   There is provision for user-specifiable warning/error message handlers
   (not yet documented). */

/* All libplot warnings and error messages go through these functions, with
   the exception of libpng error messages produced by PNG Plotters (see
   z_write.c; they're different because they need to be produced by
   callbacks). */

#include "sys-defines.h"
#include "extern.h"

/* mutex for locking the warning/error message subsystem */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
pthread_mutex_t _message_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/* user-settable handlers, defined in g_defplot.c to be NULL */
extern int (*pl_libplot_warning_handler) (const char *msg);
extern int (*pl_libplot_error_handler) (const char *msg);

void
_pl_g_warning (R___(Plotter *_plotter) const char *msg)
{
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the message subsystem */
  pthread_mutex_lock (&_message_mutex);
#endif
#endif

  if (pl_libplot_warning_handler != NULL)
    (*pl_libplot_warning_handler)(msg);
  else if (_plotter->data->errfp)
    fprintf (_plotter->data->errfp, "libplot: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->data->errstream)
    (*(_plotter->data->errstream)) << "libplot: " << msg << '\n';
#endif

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the message subsystem */
  pthread_mutex_unlock (&_message_mutex);
#endif
#endif
}

void
_pl_g_error (R___(Plotter *_plotter) const char *msg)
{
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* lock the message subsystem */
  pthread_mutex_lock (&_message_mutex);
#endif
#endif

  if (pl_libplot_error_handler != NULL)
    (*pl_libplot_error_handler)(msg);
  else if (_plotter->data->errfp)
    fprintf (_plotter->data->errfp, "libplot error: %s\n", msg);
#ifdef LIBPLOTTER
  else if (_plotter->data->errstream)
    (*(_plotter->data->errstream)) << "libplot error: " << msg << '\n';
#endif

#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  /* unlock the message subsystem */
  pthread_mutex_unlock (&_message_mutex);
#endif
#endif
}
