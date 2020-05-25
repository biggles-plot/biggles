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

/* This file contains the flushpl method, which is a GNU extension to
   libplot.  It flushes (i.e. pushes onward) all plot commands sent to the
   graphics device. */

/* This file also contains the internal _pl_g_flush_plotter_outstreams()
   method, which should be invoked by any Plotter that forks an auxiliary
   process.  (E.g., an XPlotter forks off an auxiliary process when
   closepl() is invoked, to manage its `spun-off' window.) */

#include "sys-defines.h"
#include "extern.h"

#ifdef MSDOS
#include <unistd.h>		/* for fsync() */
#endif

/* Mutex for locking _plotters[] and _plotters_len.  Defined in
   g_defplot.c. */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
extern pthread_mutex_t _plotters_mutex;
#endif
#endif

int
_API_flushpl (S___(Plotter *_plotter))
{
  int retval = 0;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "flushpl: invalid operation");
      return -1;
    }

  switch ((int)_plotter->data->output_model)
    {
    case (int)PL_OUTPUT_NONE:
      /* Plotter doesn't do any output, so do nothing */
      break;

    case (int)PL_OUTPUT_ONE_PAGE:
    case (int)PL_OUTPUT_ONE_PAGE_AT_A_TIME:
    case (int)PL_OUTPUT_PAGES_ALL_AT_ONCE:
      /* Plotter may have an output stream that can be flushed.  Should
	 really distinguish here between Plotters that write graphics in
	 real time, and those that wait until the end of a page, or of all
	 pages, before writing graphics.  */
      if (_plotter->data->outfp)
	{
	  if (fflush(_plotter->data->outfp) < 0
#ifdef MSDOS
	      /* data can be caught in DOS buffers, so do an fsync() too */
	      || fsync (_plotter->data->outfp) < 0
#endif
	      )
	    retval = -1;
	  }
#ifdef LIBPLOTTER
      if (_plotter->data->outstream)
	{
	  _plotter->data->outstream->flush ();
	  if (!(*(_plotter->data->outstream)))
	    retval = -1;
	}
#endif
      break;
      
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM:
      /* Plotter does its own output, so invoke Plotter-specific flush
	 method */
	if (_plotter->flush_output (S___(_plotter)) == false)
	  retval = -1;

      break;

    default:			/* shouldn't happen */
      break;
    }

  if (retval < 0)
    _plotter->error (R___(_plotter) "the output stream is jammed");
  return retval;
}

/* An internal method that's called when any Plotter that does its own
   output to a non-stream is flushed.  Actually, this generic version does
   nothing; it'll need to be overridden by any Plotter that wishes to make
   use of this feature.  Return value indicates success. */
bool
_pl_g_flush_output (S___(Plotter *_plotter))
{
  return true;
}

/* Flush output streams for all Plotters.  Plotters that fork when the
   closepl() operation is invoked should call this before forking.

   This code is messy, because there are four cases to cover, and in the
   final three, the global variables _plotters[] and _plotters_len need to
   be locked and unlocked. */
void
_pl_g_flush_plotter_outstreams (S___(Plotter *_plotter))
{
#ifndef LIBPLOTTER 

#ifdef HAVE_NULL_FLUSH
  fflush ((FILE *)NULL);
#else  /* not HAVE_NULL_FLUSH */
  int i;
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->data->outfp)
	  fflush (_plotters[i]->data->outfp);
	if (_plotters[i]->data->errfp)
	  fflush (_plotters[i]->data->errfp);
      }
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif
#endif /* not HAVE_NULL_FLUSH */

#else  /* LIBPLOTTER */
  int i;
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&_plotters_mutex);
#endif
#endif
#ifdef HAVE_NULL_FLUSH
  fflush ((FILE *)NULL);
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->data->outstream)
	  _plotters[i]->data->outstream->flush ();
	if (_plotters[i]->data->errstream)
	  _plotters[i]->data->errstream->flush ();
      }
#else  /* not HAVE_NULL_FLUSH */
  for (i = 0; i < _plotters_len; i++)
    if (_plotters[i]) 
      {
	if (_plotters[i]->data->outfp)
	  fflush (_plotters[i]->data->outfp);
	if (_plotters[i]->data->errfp)
	  fflush (_plotters[i]->data->errfp);
	if (_plotters[i]->data->outstream)
	  _plotters[i]->data->outstream->flush ();
	if (_plotters[i]->data->errstream)
	  _plotters[i]->data->errstream->flush ();
      }
#endif /* not HAVE_NULL_FLUSH */
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&_plotters_mutex);
#endif
#endif

#endif /* LIBPLOTTER */
}
