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

/* This file contains the linemod method, which is a standard part of
   libplot.  It sets a drawing attribute: the line style used in subsequent
   drawing operations.

   This version searches for the specified line style in a table of known
   line styles (see g_dash2.c). */

/* This file also contains the capmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the cap mode used when
   subsequently drawing open paths. */

/* This file also contains the joinmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the join mode used when
   subsequently drawing paths consisting of more than a single segment. */

/* This file also contains the miterlimit method, which is a GNU extension
   to libplot.  It sets a drawing attribute: the miter limit of polylines
   subsequently drawn on the display device.  This attribute controls the
   treatment of corners when the join mode is set to "miter". */

/* This file contains the orientation method, which is a GNU extension to
   libplot.  It sets a drawing attribute: whether or not closed paths of
   the three built-in types (rectangles, circles, ellipses) should be drawn
   counterclockwise or clockwise.  The former is the default. */

/* This file contains the fillmod method, which is a GNU extension to
   libplot.  It sets a drawing attribute: the fill rule used when
   subsequently drawing filled objects, i.e., the rule used to determine
   which points are `inside'.

   In principle, both the `odd winding number' rule and the `nonzero
   winding number' rule are supported.  The former is the default. */

#include "sys-defines.h"
#include "extern.h"

int
_API_linemod (R___(Plotter *_plotter) const char *s)
{
  bool matched = false;
  char *line_mode;
  int i;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "linemod: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.line_mode;

  free ((char *)_plotter->drawstate->line_mode);
  line_mode = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (line_mode, s);
  _plotter->drawstate->line_mode = line_mode;
  
  if (strcmp (s, "disconnected") == 0)
     /* we'll implement disconnected lines by drawing a filled circle at
	each path join point; see g_endpath.c */
    {
      _plotter->drawstate->line_type = PL_L_SOLID;
      _plotter->drawstate->points_are_connected = false;
      matched = true;
    }
  
  else	/* search table of libplot's builtin line types */
    for (i = 0; i < PL_NUM_LINE_TYPES; i++)
      {
	if (strcmp (s, _pl_g_line_styles[i].name) == 0)
	  {
	    _plotter->drawstate->line_type =
	      _pl_g_line_styles[i].type;
	    _plotter->drawstate->points_are_connected = true;
	    matched = true;
	    break;
	  }
      }
  
  if (matched == false)
    /* don't recognize, silently switch to default mode */
    _API_linemod (R___(_plotter) _default_drawstate.line_mode);

  /* for future paths, use builtin line style rather than user-specified
     dash array */
  _plotter->drawstate->dash_array_in_effect = false;

  return 0;
}

int
_API_capmod (R___(Plotter *_plotter) const char *s)
{
  char *cap_mode;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "capmod: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.cap_mode;

  free ((char *)_plotter->drawstate->cap_mode);
  cap_mode = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (cap_mode, s);
  _plotter->drawstate->cap_mode = cap_mode;

  /* The following four cap types are now standard. */

  if (strcmp( s, "butt") == 0)
    _plotter->drawstate->cap_type = PL_CAP_BUTT;
  else if (strcmp( s, "round") == 0)
    _plotter->drawstate->cap_type = PL_CAP_ROUND;
  else if (strcmp( s, "projecting") == 0)
    _plotter->drawstate->cap_type = PL_CAP_PROJECT;
  else if (strcmp( s, "triangular") == 0)
    _plotter->drawstate->cap_type = PL_CAP_TRIANGULAR;
  else
    /* don't recognize, silently switch to default mode */
    return _API_capmod (R___(_plotter) _default_drawstate.cap_mode);
  
  return 0;
}

int
_API_joinmod (R___(Plotter *_plotter) const char *s)
{
  char *join_mode;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "joinmod: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = _default_drawstate.join_mode;

  free ((char *)_plotter->drawstate->join_mode);
  join_mode = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (join_mode, s);
  _plotter->drawstate->join_mode = join_mode;

  /* The following four join types are now standard. */

  if (strcmp( s, "miter") == 0)
    _plotter->drawstate->join_type = PL_JOIN_MITER;
  else if (strcmp( s, "mitre") == 0)
    _plotter->drawstate->join_type = PL_JOIN_MITER;
  else if (strcmp( s, "round") == 0)
    _plotter->drawstate->join_type = PL_JOIN_ROUND;
  else if (strcmp( s, "bevel") == 0)
    _plotter->drawstate->join_type = PL_JOIN_BEVEL;
  else if (strcmp( s, "triangular") == 0)
    _plotter->drawstate->join_type = PL_JOIN_TRIANGULAR;
  else
    /* unknown, so silently switch to default mode (via recursive call) */
    return _API_joinmod (R___(_plotter) _default_drawstate.join_mode);
  
  return 0;
}

/* Below is the miterlimit method, which is a GNU extension to libplot.  It
   sets a drawing attribute: the miter limit of polylines subsequently
   drawn on the display device.  This attribute controls the treatment of
   corners when the join mode is set to "miter".

   At a join point of a wide polyline, the `miter length' is defined to be
   the distance between the inner corner and the outer corner.  The miter
   limit is the maximum value that can be tolerated for the miter length
   divided by the line width.  If this value is exceeded, the miter will be
   `cut off': the "bevel" join mode will be used instead.

   Examples of typical values for the miter limit are 10.43 (the
   unchangeable value used by the X Window System, which cuts off miters at
   join angles less than 11 degrees), 5.0 (the default value used by
   HP-GL/2 and PCL 5 devices, which cuts off miters at join angles less
   than 22.1 degrees), 2.0 (cuts off miters at join angles less than 60
   degrees), 1.414 (cuts off miters at join angles less than 90 degrees),
   and 1.0 (cuts off all miters).  

   In general, the miter limit is the cosecant of one-half of the minimum
   join angle for mitering, and values less than 1.0 are meaningless.  For
   example, 10.43 is csc((11 degrees)/2) = 1 / sin((11 degrees)/2).
   Mitering is allowed to take place if 1 / sin(theta/2) <= MITERLIMIT,
   i.e. sin(theta/2) >= 1/MITERLIMIT, where theta > 0 is the join angle. */

int
_API_fmiterlimit (R___(Plotter *_plotter) double new_miter_limit)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "flinewidth: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  if (new_miter_limit < 1.0)	/* reset to default */
    new_miter_limit = PL_DEFAULT_MITER_LIMIT;

  /* set the new miter limit in the drawing state */
  _plotter->drawstate->miter_limit = new_miter_limit;
  
  return 0;
}

/* Below is the orientation method, which is a GNU extension to libplot.
   It sets a drawing attribute: whether or not closed paths of the three
   built-in types (rectangles, circles, ellipses) should be drawn
   counterclockwise or clockwise.  The former is the default. */

/* This attribute-setting method, though path-related, doesn't invoke
   _API_endpath() to end the compound path under construction (if any).
   That's because it makes sense to invoke this method between the simple
   paths of a compound path. */

int
_API_orientation (R___(Plotter *_plotter) int direction)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "orientation: invalid operation");
      return -1;
    }

  if (direction != 1 && direction != -1)
    /* OOB switches to default */
    direction = _default_drawstate.orientation;

  _plotter->drawstate->orientation = direction;
  
  return 0;
}

/* Below is the fillmod method, which is a GNU extension to libplot.  It
   sets a drawing attribute: the fill rule used when subsequently drawing
   filled objects, i.e., the rule used to determine which points are
   `inside'.

   In principle, both the `odd winding number' rule and the `nonzero
   winding number' rule are supported.  The former is the default. */

int
_API_fillmod (R___(Plotter *_plotter) const char *s)
{
  const char *default_s;
  char *fill_rule;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "fillmod: invalid operation");
      return -1;
    }

  _API_endpath (S___(_plotter)); /* flush path if any */

  /* determine default fill rule (can't just read from default drawing
     state, because not all Plotters support both standard rules) */
  default_s = _default_drawstate.fill_rule;
  if (strcmp (default_s, "even-odd") == 0
      && _plotter->data->have_odd_winding_fill == 0)
    default_s = "nonzero-winding";
  else if (strcmp (default_s, "nonzero-winding") == 0
	   && _plotter->data->have_nonzero_winding_fill == 0)
    default_s = "even-odd";
  
  /* null pointer resets to default */
  if ((!s) || !strcmp(s, "(null)"))
    s = default_s;

  free ((char *)_plotter->drawstate->fill_rule);
  fill_rule = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (fill_rule, s);
  _plotter->drawstate->fill_rule = fill_rule;

  if ((strcmp (s, "even-odd") == 0 || strcmp (s, "alternate") == 0)
      && _plotter->data->have_odd_winding_fill)
    _plotter->drawstate->fill_rule_type = PL_FILL_ODD_WINDING;
  else if ((strcmp (s, "nonzero-winding") == 0 || strcmp (s, "winding") == 0)
	   && _plotter->data->have_nonzero_winding_fill)
    _plotter->drawstate->fill_rule_type = PL_FILL_NONZERO_WINDING;
  else
    /* unknown, so silently switch to default fill rule (via recursive call) */
    _API_fillmod (R___(_plotter) default_s);

  return 0;
}
