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

/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object. */

/* This generic version does only basic mechanics, since GenericPlotters
   don't emit any graphics code. */

#include "sys-defines.h"
#include "extern.h"

int
_API_closepl (S___(Plotter *_plotter))
{
  bool emit_not_just_the_first_page = true;
  bool retval1;
  int retval2 = 0;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "closepl: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* pop drawing states in progress, if any, off the stack */
  if (_plotter->drawstate->previous != NULL)
    {
      while (_plotter->drawstate->previous)
	_API_restorestate (S___(_plotter));
    }
  
  /* invoke Plotter-specific method to end the page; also do
     device-dependent teardown Plotter-specific drawing state variables, do
     reinitialization of Plotter-specific Plotter variables, and create
     page header and trailer if any */
  retval1 = _plotter->end_page (S___(_plotter));

  /* remove first drawing state too, so we can start afresh */
  _pl_g_delete_first_drawing_state (S___(_plotter));

  switch ((int)_plotter->data->output_model)
    {
    case (int)PL_OUTPUT_NONE:
      /* we don't do output, so just delete the page buffer (presumably it
	 includes neither a header nor a trailer) */
      if (_plotter->data->page)
	_delete_outbuf (_plotter->data->page);
      _plotter->data->page = (plOutbuf *)NULL;
      break;

    case (int)PL_OUTPUT_ONE_PAGE:
      emit_not_just_the_first_page = false;
      /* fall through */

    case (int)PL_OUTPUT_ONE_PAGE_AT_A_TIME:
      if (_plotter->data->page
	  && (emit_not_just_the_first_page 
	      || _plotter->data->page_number == 1))
	{
	  /* emit page header if any */
	  if (_plotter->data->page->header 
	      && _plotter->data->page->header->len > 0)
	    _write_string (_plotter->data, 
			   _plotter->data->page->header->base); 

	  /* emit all the graphics on the page */
	  if (_plotter->data->page && _plotter->data->page->len > 0)
	    _write_string (_plotter->data, _plotter->data->page->base); 

	  /* emit page trailer if any */
	  if (_plotter->data->page->trailer 
	      && _plotter->data->page->trailer->len > 0)
	    _write_string (_plotter->data, 
			   _plotter->data->page->trailer->base); 

	  /* attempt to flush (will test whether stream is jammed) */
	  retval2 = _API_flushpl (S___(_plotter));
	}
      
      /* delete page header if any */
      if (_plotter->data->page->header)
	_delete_outbuf (_plotter->data->page->header);
      _plotter->data->page->header = (plOutbuf *)NULL;

      /* delete page trailer if any */
      if (_plotter->data->page->trailer)
	_delete_outbuf (_plotter->data->page->trailer);
      _plotter->data->page->trailer = (plOutbuf *)NULL;

      /* delete page's plOutbuf */
      if (_plotter->data->page)
	_delete_outbuf (_plotter->data->page);
      _plotter->data->page = (plOutbuf *)NULL;

      break;
      
    case (int)PL_OUTPUT_PAGES_ALL_AT_ONCE:
      /* Plotter will do its own output, in its terminate() routine.  
	 It will do so by writing out the contents of all its cached pages
	 (a new one gets added to the list each time openpl() is invoked).
	 So do nothing. */
      break;

    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES:
    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_IN_REAL_TIME:
      /* Plotter does its own output, and doesn't use libplot's
	 plOutbuf-based output system.  So do nothing, except
	 attempt to flush (will test whether stream is jammed) */
      retval2 = _API_flushpl (S___(_plotter));
      break;

    case (int)PL_OUTPUT_VIA_CUSTOM_ROUTINES_TO_NON_STREAM:
      /* Plotter doesn't use libplot's plOutbuf-based output system, and in
	 fact doesn't send output to a stream at all.  So do nothing: don't
	 attempt to flush output; the drawing state [including
	 Plotter-specific data] has already been deleted, so there may not
	 be enough information to perform a flush.  This is an issue with
	 X Plotters in particular. */
      break;

    default:			/* shouldn't happen */
      break;
    }

  _plotter->data->open = false;	/* flag device as closed */

  if (retval1 == false || retval2 < 0)
    return -1;
  else return 0;
}

/* Plotter-specific teardowns and reinitializations that take place when
   closepl() is invoked.  In a generic Plotter, this does nothing. */

bool
_pl_g_end_page (S___(Plotter *_plotter))
{
  return true;
}

void
_pl_g_delete_first_drawing_state (S___(Plotter *_plotter))
{
  /* elements of state that are strings or arrays are freed separately */
  free ((char *)_plotter->drawstate->fill_rule);
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->true_font_name);
  free ((char *)_plotter->drawstate->font_name);

  /* free dash array too, if nonempty */
  if (_plotter->drawstate->dash_array_len > 0)
    free ((double *)_plotter->drawstate->dash_array);

  /* free the state itself */
  free (_plotter->drawstate);
  _plotter->drawstate = NULL;
}
