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

/* This file contains the savestate method, which is a GNU extension to
   libplot.  It creates a new drawing state and pushes it onto the stack of
   drawing states.  By definition, a `drawing state' comprises the set of
   drawing attributes, and the state of any path being incrementally drawn.

   The new state will have the same drawing attributes as the old state.
   If a path was being drawn incrementally in the old state, the new state
   will not contain it.  The old state may be returned to by invoking the
   restorestate routine, which pops drawing states off the stack.  If the
   incremental drawing of a path was in progress, it may be returned to at
   that time.

   This version of savestate() assumes that the device-specific part of the
   drawing state contains no strings.  Plotter objects for which this is
   not true must supplement this by defining push_state() appropriately,
   since they need to call malloc() to allocate space for the string in the
   new state. */

/* This file also contains the restorestate method, which is a GNU
   extension to libplot.  It pops off the drawing state on the top of the
   stack of drawing states.  Drawing states (other than the one which is
   always present, and may not be popped off) are created and pushed onto
   the stack by invoking the savestate() routine.

   This version of restorestate() assumes that the device-specific part of
   the state contains no strings or other dynamically allocated data.
   Versions of libplot in which this is not true must supplement this by
   defining pop_state() appropriately, since they need to call free() to
   deallocate space for the strings. */

/* N.B. The drawing state stack, during user use of libplot, always
   contains at least one drawing state.  The first state is added by
   openpl() and deleted by closepl(). */

#include "sys-defines.h"
#include "extern.h"

int
_API_savestate(S___(Plotter *_plotter))
{
  plDrawState *oldstate = _plotter->drawstate; /* non-NULL */
  plDrawState *drawstate;
  char *fill_rule, *line_mode, *join_mode, *cap_mode;
  char *font_name, *true_font_name;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "savestate: invalid operation");
      return -1;
    }

  /* create a new state */
  drawstate = (plDrawState *)_pl_xmalloc (sizeof(plDrawState));
  
  /* copy from old state */
  memcpy (drawstate, oldstate, sizeof(plDrawState));

  /* elements of state that are strings are treated specially */
  fill_rule = (char *)_pl_xmalloc (strlen (oldstate->fill_rule) + 1);
  line_mode = (char *)_pl_xmalloc (strlen (oldstate->line_mode) + 1);
  join_mode = (char *)_pl_xmalloc (strlen (oldstate->join_mode) + 1);
  cap_mode = (char *)_pl_xmalloc (strlen (oldstate->cap_mode) + 1);
  strcpy (fill_rule, oldstate->fill_rule);
  strcpy (line_mode, oldstate->line_mode);
  strcpy (join_mode, oldstate->join_mode);
  strcpy (cap_mode, oldstate->cap_mode);
  drawstate->fill_rule = fill_rule;
  drawstate->line_mode = line_mode;
  drawstate->join_mode = join_mode;
  drawstate->cap_mode = cap_mode;

  /* dash array, if non-empty, is treated specially too */
  if (oldstate->dash_array_len > 0)
    {
      int i;
      double *dash_array;

      dash_array = (double *)_pl_xmalloc (oldstate->dash_array_len * sizeof(double));
      for (i = 0; i < oldstate->dash_array_len; i++)
	dash_array[i] = oldstate->dash_array[i];
      drawstate->dash_array = dash_array;
    }

  /* The font_name, true_font_name, font_type, typeface_index, and
     font_index fields are special, since for the initial drawing state
     they're Plotter-dependent.

     For later drawing states, we just copy them from the previous state.
     Since only the first two (font_name and true_font_name) are strings,
     for later states we don't worry about the other three: they've already
     been copied.

     The fill_rule_type field is also treated specially in the initial
     drawing state, because not all Plotters support both types of filling
     (odd vs. nonzero winding number). */

  font_name = (char *)_pl_xmalloc (strlen (oldstate->font_name) + 1);  
  strcpy (font_name, oldstate->font_name);  
  drawstate->font_name = font_name;

  true_font_name = (char *)_pl_xmalloc (strlen (oldstate->true_font_name) + 1);  
  strcpy (true_font_name, oldstate->true_font_name);  
  drawstate->true_font_name = true_font_name;

  /* Our memcpy copied the pointer to the compound path under construction
     (if any).  So we knock it out, to start afresh */
  drawstate->path = (plPath *)NULL;
  drawstate->paths = (plPath **)NULL;
  drawstate->num_paths = 0;

  /* install new state at head of the state list */
  drawstate->previous = oldstate;
  _plotter->drawstate = drawstate;

  /* add any device-dependent fields to new state */
  _plotter->push_state (S___(_plotter));

  return 0;
}

int
_API_restorestate(S___(Plotter *_plotter))
{
  plDrawState *oldstate = _plotter->drawstate->previous;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "restorestate: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->previous == NULL)
    /* this is an attempt to pop the lowest state off the stack */
    {
      _plotter->error (R___(_plotter) 
		       "restorestate: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* tear down any device-dependent fields in state */
  _plotter->pop_state (S___(_plotter));

  /* elements of current state that are strings are first freed */
  free ((char *)_plotter->drawstate->fill_rule);
  free ((char *)_plotter->drawstate->line_mode);
  free ((char *)_plotter->drawstate->join_mode);
  free ((char *)_plotter->drawstate->cap_mode);
  free ((char *)_plotter->drawstate->true_font_name);
  free ((char *)_plotter->drawstate->font_name);
  
  /* free dash array too, if nonempty */
  if (_plotter->drawstate->dash_array_len > 0)
    free ((double *)_plotter->drawstate->dash_array);

  /* pop state off the stack */
  free (_plotter->drawstate);
  _plotter->drawstate = oldstate;

  return 0;
}

void
_pl_g_push_state (S___(Plotter *_plotter))
{
  return;
}

void
_pl_g_pop_state (S___(Plotter *_plotter))
{
  return;
}

