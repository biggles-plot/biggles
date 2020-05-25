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

/* This internal method is invoked by an Illustrator Plotter before drawing
   any object.  It sets the relevant attributes (fill rule [if filling],
   cap type, join type, miter limit, line width) to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* Pseudo line type, which we use internally when AI's line type,
   i.e. dashing style, is set to agree with what the user specifies with
   linedash(), rather than with what the user specifies with linemod().
   Should not equal any of our canonical line types, i.e. PL_L_SOLID etc. */
#define SPECIAL_AI_LINE_TYPE 100

/* AI fill rule types (in version 5 and later), indexed by our internal
   fill rule number (PL_FILL_ODD_WINDING/PL_FILL_NONZERO_WINDING) */
static const int _ai_fill_rule[PL_NUM_FILL_RULES] = 
{ AI_FILL_ODD_WINDING, AI_FILL_NONZERO_WINDING };

/* AI (i.e. PS) join styles, indexed by internal number
   (miter/rd./bevel/triangular) */
static const int _ai_join_style[PL_NUM_JOIN_TYPES] =
{ AI_LINE_JOIN_MITER, AI_LINE_JOIN_ROUND, AI_LINE_JOIN_BEVEL, AI_LINE_JOIN_ROUND };

/* AI (i.e. PS) cap styles, indexed by internal number
   (butt/rd./project/triangular) */
static const int _ai_cap_style[PL_NUM_CAP_TYPES] =
{ AI_LINE_CAP_BUTT, AI_LINE_CAP_ROUND, AI_LINE_CAP_PROJECT, AI_LINE_CAP_ROUND };

void
_pl_a_set_attributes (S___(Plotter *_plotter))
{
  bool changed_width = false;
  int desired_fill_rule = _ai_fill_rule[_plotter->drawstate->fill_rule_type];
  double desired_ai_line_width = _plotter->drawstate->device_line_width;
  int desired_ai_cap_style = _ai_cap_style[_plotter->drawstate->cap_type];
  int desired_ai_join_style = _ai_join_style[_plotter->drawstate->join_type];
  double desired_ai_miter_limit = _plotter->drawstate->miter_limit;
  int desired_ai_line_type = _plotter->drawstate->line_type;  
  int i;
  double display_size_in_points, min_dash_unit;

  if (_plotter->ai_version >= AI_VERSION_5
      && _plotter->drawstate->fill_type > 0
      && _plotter->ai_fill_rule_type != desired_fill_rule)
    {
      sprintf (_plotter->data->page->point, "%d XR\n", desired_fill_rule);
      _update_buffer (_plotter->data->page);
      _plotter->ai_fill_rule_type = desired_fill_rule;
    }
  
  if (_plotter->ai_cap_style != desired_ai_cap_style)
    {
      sprintf (_plotter->data->page->point, "%d J\n", desired_ai_cap_style);
      _update_buffer (_plotter->data->page);
      _plotter->ai_cap_style = desired_ai_cap_style;
    }
  
  if (_plotter->ai_join_style != desired_ai_join_style)
    {
      sprintf (_plotter->data->page->point, "%d j\n", desired_ai_join_style);
      _update_buffer (_plotter->data->page);
      _plotter->ai_join_style = desired_ai_join_style;
    }

  if (_plotter->drawstate->join_type == PL_JOIN_MITER
      && _plotter->ai_miter_limit != desired_ai_miter_limit)
    {
      sprintf (_plotter->data->page->point, "%.4g M\n", desired_ai_miter_limit);
      _update_buffer (_plotter->data->page);
      _plotter->ai_miter_limit = desired_ai_miter_limit;
    }

  if (_plotter->ai_line_width != desired_ai_line_width)
    {
      sprintf (_plotter->data->page->point, "%.4f w\n", desired_ai_line_width);
      _update_buffer (_plotter->data->page);
      _plotter->ai_line_width = desired_ai_line_width;
      changed_width = true;
    }

  if (_plotter->drawstate->dash_array_in_effect
      || _plotter->ai_line_type != desired_ai_line_type
      || (changed_width && desired_ai_line_type != PL_L_SOLID))
    /* must tell AI which dash array to use */
    {
      double *dashbuf;
      int num_dashes;
      double offset;
      
      if (_plotter->drawstate->dash_array_in_effect)
	/* have user-specified dash array */
	{
	  num_dashes = _plotter->drawstate->dash_array_len;

	  if (num_dashes > 0)
	    /* non-solid line type */
	    {
	      double min_sing_val, max_sing_val;
	      
	      /* compute minimum singular value of user->device coordinate
		 map, which we use as a multiplicative factor to convert
		 line widths (cf. g_linewidth.c), dash lengths, etc. */
	      _matrix_sing_vals (_plotter->drawstate->transform.m, 
				 &min_sing_val, &max_sing_val);
	      
	      dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));

	      for (i = 0; i < num_dashes; i++)
		{
		  double dashlen;

		  dashlen =
		    min_sing_val * _plotter->drawstate->dash_array[i];
		  dashbuf[i] = dashlen;
		}
	      offset = min_sing_val * _plotter->drawstate->dash_offset;
	    }
	  else
	    /* zero-length dash array, i.e. solid line type */
	    {
	      dashbuf = NULL;
	      offset = 0;
	    }

	  /* we'll keep track of the fact that AI is using a special
	     user-specified dash array by setting the `hpgl_line_type' data
	     member to this bogus value */
	  desired_ai_line_type = SPECIAL_AI_LINE_TYPE;
	}
      else
	/* dash array not in effect, have a canonical line type instead */
	{
	  if (desired_ai_line_type == PL_L_SOLID)
	    {
	      num_dashes = 0;
	      dashbuf = NULL;
	      offset = 0.0;
	    }
	  else
	    {
	      const int *dash_array;
	      double scale;
	      
	      num_dashes =
		_pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;
	      dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));

	      /* compute PS dash array for this line type */
	      dash_array = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
	      /* scale the array of integers by line width (actually by
		 floored line width; see comments at head of file) */
	      display_size_in_points = 
		DMIN(_plotter->data->xmax - _plotter->data->xmin, 
		     _plotter->data->ymax - _plotter->data->ymin);
	      min_dash_unit = (PL_MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE 
			       * display_size_in_points);
	      scale = DMAX(min_dash_unit,
			   _plotter->drawstate->device_line_width);

	      for (i = 0; i < num_dashes; i++)
		dashbuf[i] = scale * dash_array[i];
	      offset = 0.0;
	    }
	}

      /* emit dash array */
      sprintf (_plotter->data->page->point, "[");
      _update_buffer (_plotter->data->page);
      for (i = 0; i < num_dashes; i++)
	{
	  if (i == 0)
	    sprintf (_plotter->data->page->point, "%.4f", dashbuf[i]);
	  else
	    sprintf (_plotter->data->page->point, " %.4f", dashbuf[i]);	  
	  _update_buffer (_plotter->data->page);      
	}
      sprintf (_plotter->data->page->point, "] %.4f d\n", offset);
      _update_buffer (_plotter->data->page);

      /* Update our knowledge of AI's line type (i.e. dashing style). 
	 This new value will be one of PL_L_SOLID etc., or the pseudo value
	 SPECIAL_AI_LINE_TYPE. */
      _plotter->ai_line_type = desired_ai_line_type;

      free (dashbuf);
    }
  
  return;
}
