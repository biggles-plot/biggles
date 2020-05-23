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

/* This file contains the erase method, which is a standard part of
   libplot.  It erases all objects on the graphics device display.

   This generic version is designed mostly for Plotters that do not do
   real-time plotting, and have no internal state.  It simply resets the
   output buffer for the current `page', discarding all objects written 
   to it. */

#include "sys-defines.h"
#include "extern.h"

int
_API_erase (S___(Plotter *_plotter))
{
  bool retval1;
  int retval2 = 0;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "erase: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  switch ((int)_plotter->data->output_model)
    {
    case (int)PL_OUTPUT_NONE:
      /* Plotter doesn't do output, so do nothing */
      break;

    case (int)PL_OUTPUT_ONE_PAGE:
    case (int)PL_OUTPUT_ONE_PAGE_AT_A_TIME:
    case (int)PL_OUTPUT_PAGES_ALL_AT_ONCE:
      /* the builtin plOutbuf mechanism is being used for storing pages, so
	 remove all stored graphics code from the buffer corresponding to
	 the current page; reset page bounding box, etc. */
      if (_plotter->data->page)	/* paranoid */
	_reset_outbuf (_plotter->data->page);
      break;

    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM:
      /* Plotter does its own output, and doesn't use libplot's
	 plOutbuf-based output system.  So do nothing. */
      break;

    default:			/* shouldn't happen */
      break;
    }

  /* Invoke Plotter-specific method to do device-dependent aspects of page
     erasure.  For example, reset the elements of the Plotter that keep
     track of the output device's graphics state to their default
     values. */
  retval1 = _plotter->erase_page (S___(_plotter));

  /* if Plotter is using custom output routines, and is, or could be,
     drawing graphics in real time, flush out the erasure */
  if (_plotter->data->output_model == 
      PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME
      || _plotter->data->output_model == 
      PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM)
    retval2 = _API_flushpl (S___(_plotter));

  /* on to next frame */
  _plotter->data->frame_number++;

  return (retval1 == true && retval2 == 0 ? 0 : -1);
}

/* Plotter-specific things that take place when erase() is invoked.  In a
   generic Plotter, this does nothing. */

bool
_pl_g_erase_page (S___(Plotter *_plotter))
{
  return true;
}
