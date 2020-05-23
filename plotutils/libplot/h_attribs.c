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

/* This internal method is invoked before drawing any path.  It sets the
   relevant attributes of an HP-GL or HP-GL/2 plotter (fill rule, line
   type, cap type, join type, line width) to what they should be. */

#include "sys-defines.h"
#include "extern.h"

/* Each dash and gap in our canonical line modes ("shortdashed",
   "dotdashed") etc. has length that we take to be an integer multiple of
   the line width.  (For the integers, see g_dash2.c).  Actually, when
   performing this computation we impose a floor on the line width
   (measured in the device frame, in scaled HP-GL coordinates. */
#define MIN_DASH_UNIT (PL_MIN_DASH_UNIT_AS_FRACTION_OF_DISPLAY_SIZE * (HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT))

/* HP-GL's native line types, indexed into by our internal line style
   number (PL_L_SOLID/PL_L_DOTTED/
   PL_L_DOTDASHED/PL_L_SHORTDASHED/PL_L_LONGDASHED/PL_L_DOTDOTDASHED/PL_L_DOTDOTDOTDASHED).
   We use HP-GL's native line types only if we aren't emitting HP-GL/2,
   since HP-GL/2 supports programmatic definition of line styles. */
static const int _hpgl_line_type[PL_NUM_LINE_TYPES] =
{ HPGL_L_SOLID, HPGL_L_DOTTED, HPGL_L_DOTDASHED,
    HPGL_L_SHORTDASHED, HPGL_L_LONGDASHED, HPGL_L_DOTDOTDASHED,
    HPGL_L_DOTDOTDOTDASHED };

/* In HP-GL/2, native line type (HP-GL/2 numbering) that will be redefined
   programmatically as the dashed line style which we'll use.  Should not
   be any of the preceding, and should be in range 1..8. */
#define SPECIAL_HPGL_LINE_TYPE 8

/* HP-GL/2 joinstyles, indexed by internal number(miter/rd./bevel/triangular)*/
static const int _hpgl_join_style[] =
{ HPGL_JOIN_MITER_BEVEL, HPGL_JOIN_ROUND, HPGL_JOIN_BEVEL, HPGL_JOIN_TRIANGULAR };

/* HP-GL/2 capstyles, indexed by internal number(butt/rd./project/triangular)*/
static const int _hpgl_cap_style[] =
{ HPGL_CAP_BUTT, HPGL_CAP_ROUND, HPGL_CAP_PROJECT, HPGL_CAP_TRIANGULAR };

#define FUZZ 0.0000001

void
_pl_h_set_attributes (S___(Plotter *_plotter))
{
  double desired_hpgl_pen_width;
  double width, height, diagonal_p1_p2_distance;

  /* first, compute desired linewidth in scaled HP-GL coors (i.e. as
     fraction of diagonal distance between P1,P2) */
  width = (double)(HPGL_SCALED_DEVICE_RIGHT - HPGL_SCALED_DEVICE_LEFT);
  height = (double)(HPGL_SCALED_DEVICE_TOP - HPGL_SCALED_DEVICE_BOTTOM);
  diagonal_p1_p2_distance = sqrt (width * width + height * height);
  desired_hpgl_pen_width 
    = _plotter->drawstate->device_line_width / diagonal_p1_p2_distance;

  /* if plotter's policy on dashing lines needs to be adjusted, do so */

  if (_plotter->hpgl_version == 2
      && (_plotter->drawstate->dash_array_in_effect
	  || (_plotter->hpgl_line_type != 
	      _hpgl_line_type[_plotter->drawstate->line_type])
	  || (_plotter->hpgl_pen_width != desired_hpgl_pen_width)))
    /* HP-GL/2 case, and we need to emit HP-GL/2 instructions that define a
       new line type.  Why?  Several possibilities: (1) user called
       linedash(), in which case we always define the line type here, or
       (2) user called linemod() to change the canonical line style, in
       which case we need to define a line type here containing the
       corresponding dash array, or (3) user called linewidth(), in which
       case we need to define the new line type here because (in the
       canonical line style case) the dash lengths we'll use depend on the
       line width. */
    {
      double min_sing_val, max_sing_val;
      double *dashbuf, dash_cycle_length;
      int i, num_dashes;

      /* compute minimum singular value of user->device coordinate map,
	 which we use as a multiplicative factor to convert line widths
	 (cf. g_linewidth.c), dash lengths, etc. */
      _matrix_sing_vals (_plotter->drawstate->transform.m,
			 &min_sing_val, &max_sing_val);

      if (_plotter->drawstate->dash_array_in_effect)
	/* user invoked linedash() */
	{
	  num_dashes = _plotter->drawstate->dash_array_len;
	  if (num_dashes > 0)
	    dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));
	  else
	    dashbuf = NULL;	/* solid line */
	  
	  dash_cycle_length = 0.0;
	  for (i = 0; i < num_dashes; i++)
	    {
	      /* convert dash length to device coordinates */
	      dashbuf[i] = min_sing_val * _plotter->drawstate->dash_array[i];
	      dash_cycle_length += dashbuf[i];
	    }
	}
      else
	/* have a canonical line type, but since this is HP-GL/2, rather
	   than pre-HP-GL/2 or generic HP-GL, we'll implement it as a
	   user-defined line type for accuracy */
	{
	  if (_plotter->drawstate->line_type == PL_L_SOLID)
	    {
	      num_dashes = 0;
	      dash_cycle_length = 0.0;
	      dashbuf = NULL;
	    }
	  else
	    {
	      const int *dash_array;
	      double scale;
	      
	      num_dashes =
		_pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;
	      dashbuf = (double *)_pl_xmalloc (num_dashes * sizeof(double));

	      /* scale the array of integers by line width (actually by
		 floored line width; see comments at head of file) */
	      dash_array = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
	      scale = DMAX(MIN_DASH_UNIT,_plotter->drawstate->device_line_width);

	      dash_cycle_length = 0.0;
	      for (i = 0; i < num_dashes; i++)
		{
		  dashbuf[i] = scale * dash_array[i];
		  dash_cycle_length += dashbuf[i];
		}
	    }
	}

      if (num_dashes == 0 || dash_cycle_length == 0.0)
	/* just switch to solid line type */
	{
	  strcpy (_plotter->data->page->point, "LT;");
	  _update_buffer (_plotter->data->page);      
	  _plotter->hpgl_line_type = HPGL_L_SOLID;
	}
      else
	/* create user-defined line-type, and switch to it */
	{
	  bool odd_length = (num_dashes & 1 ? true : false);

	  /* create user-defined line type */
	  sprintf (_plotter->data->page->point, "UL%d",
		   SPECIAL_HPGL_LINE_TYPE);
	  _update_buffer (_plotter->data->page);      
	  for (i = 0; i < num_dashes; i++)
	    {
	      sprintf (_plotter->data->page->point, ",%.3f", 
		       /* dash length as frac of iteration interval */
		       100.0 * (odd_length ? 0.5 : 1.0) 
		       * dashbuf[i] / dash_cycle_length);
	      _update_buffer (_plotter->data->page);      
	    }
	  if (odd_length)
	    /* if an odd number of dashes, emit the dash array twice
	       (HP-GL/2 doesn't handle odd-length patterns the way that
	       Postscript does, so an even-length pattern is better) */
	    {
	      for (i = 0; i < num_dashes; i++)
		{
		  sprintf (_plotter->data->page->point, ",%.3f", 
			   /* dash length as frac of iteration interval */
			   100.0 * (odd_length ? 0.5 : 1.0) 
			   * dashbuf[i] / dash_cycle_length);
		  _update_buffer (_plotter->data->page);      
		}
	    }
	  sprintf (_plotter->data->page->point, ";");
	  _update_buffer (_plotter->data->page);      
	  
	  /* switch to new line type */
	  {
	    double width, height, diagonal_p1_p2_distance;
	    double iter_interval;

	    /* specify iteration interval as percentage of P1-P2 distance */
	    width = (double)(HPGL_SCALED_DEVICE_RIGHT-HPGL_SCALED_DEVICE_LEFT);
	    height = (double)(HPGL_SCALED_DEVICE_TOP-HPGL_SCALED_DEVICE_BOTTOM);
	    diagonal_p1_p2_distance = sqrt (width * width + height * height);
	    iter_interval = 100 * (odd_length ? 2 : 1) * (dash_cycle_length/diagonal_p1_p2_distance);
	    sprintf (_plotter->data->page->point, "LT%d,%.4f;", 
		     SPECIAL_HPGL_LINE_TYPE, iter_interval);
	    _update_buffer (_plotter->data->page);
	    if (_plotter->drawstate->dash_array_in_effect)
	      _plotter->hpgl_line_type = SPECIAL_HPGL_LINE_TYPE;
	    else
	      /* keep track of plotter's line type as if it were
		 one of the built-in ones */
	      _plotter->hpgl_line_type = 
		_hpgl_line_type[_plotter->drawstate->line_type];
	  }
	}
      
      free (dashbuf);
    }

  /* Not HP-GL/2, so the only line types at our disposal are HP-GL's
     traditional line types.  Check whether we need to switch. */

  if (_plotter->hpgl_version < 2
      && ((_plotter->hpgl_line_type !=
	   _hpgl_line_type[_plotter->drawstate->line_type])
	  ||			/* special case #1, mapped to "shortdashed" */
	  (_plotter->drawstate->dash_array_in_effect
	   && _plotter->drawstate->dash_array_len == 2
	   && (_plotter->drawstate->dash_array[1]
	       == _plotter->drawstate->dash_array[0]))
	  ||			/* special case #2, mapped to "dotted" */
	  (_plotter->drawstate->dash_array_in_effect
	   && _plotter->drawstate->dash_array_len == 2
	   && (_plotter->drawstate->dash_array[1]
	       > (3 - FUZZ) * _plotter->drawstate->dash_array[0])
	   && (_plotter->drawstate->dash_array[1]
	       < (3 + FUZZ) * _plotter->drawstate->dash_array[0]))))
    /* switch to one of HP-GL's traditional line types */
    {
      double dash_cycle_length, iter_interval;
      double min_sing_val, max_sing_val;
      int line_type;

      if (_plotter->drawstate->dash_array_in_effect
	  && _plotter->drawstate->dash_array_len == 2
	  && (_plotter->drawstate->dash_array[1]
	      == _plotter->drawstate->dash_array[0]))
	/* special case #1, user-specified dashing (equal on/off lengths):
	   treat effectively as "shortdashed" line mode */
	{
	  /* Minimum singular value is the nominal device-frame line width
	     divided by the actual user-frame line-width (see
	     g_linewidth.c), so it's the user->device frame conversion
	     factor. */
	  _matrix_sing_vals (_plotter->drawstate->transform.m,
			     &min_sing_val, &max_sing_val);
	  dash_cycle_length = 
	    min_sing_val * 2.0 * _plotter->drawstate->dash_array[0];
	  line_type = PL_L_SHORTDASHED;
	}
      else if (_plotter->drawstate->dash_array_in_effect
	       && _plotter->drawstate->dash_array_len == 2
	       && (_plotter->drawstate->dash_array[1]
		   > (3 - FUZZ) * _plotter->drawstate->dash_array[0])
	       && (_plotter->drawstate->dash_array[1]
		   < (3 + FUZZ) * _plotter->drawstate->dash_array[0]))
	/* special case #2, user-specified dashing (dash on length = 1/4 of
	   cycle length): treat effectively as "dotted" line mode */
	{
	  /* Minimum singular value is the nominal device-frame line width
	     divided by the actual user-frame line-width (see
	     g_linewidth.c), so it's the user->device frame conversion
	     factor. */
	  _matrix_sing_vals (_plotter->drawstate->transform.m,
			     &min_sing_val, &max_sing_val);
	  dash_cycle_length = 
	    min_sing_val * 2.0 * 4.0 * _plotter->drawstate->dash_array[0];
	  line_type = PL_L_DOTTED;
	}
      else
	/* general case: user must have changed canonical line types by
	   invoking linemod(); will implement new line style as one of the
	   traditional HP-GL line types. */
	{ 
	  const int *dash_array; 
	  int i, num_dashes; 
	  double scale;
      
	  dash_array = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
	  num_dashes =
	    _pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;
      
	  /* compute iter interval in device coors, scaling by floored line
             width (see comments at head of file) */
	  scale = DMAX(MIN_DASH_UNIT,_plotter->drawstate->device_line_width);
	  if (scale < 1.0)
	    scale = 1.0;
	  dash_cycle_length = 0.0;
	  for (i = 0; i < num_dashes; i++)
	    dash_cycle_length += scale * dash_array[i];

	  line_type = _plotter->drawstate->line_type;
	}
      
      /* compute iteration interval as percentage of P1-P2 distance */
      {
	double width, height, diagonal_p1_p2_distance;
	
	width = (double)(HPGL_SCALED_DEVICE_RIGHT-HPGL_SCALED_DEVICE_LEFT);
	height = (double)(HPGL_SCALED_DEVICE_TOP-HPGL_SCALED_DEVICE_BOTTOM);
	diagonal_p1_p2_distance = sqrt (width * width + height * height);
	iter_interval = 100 * (dash_cycle_length/diagonal_p1_p2_distance);
      }
      
      switch (line_type)
	{
	case PL_L_SOLID:
	  /* "solid" */
	  strcpy (_plotter->data->page->point, "LT;");
	  break;
	case PL_L_DOTTED:
	  /* "dotted": emulate dots by selecting shortdashed pattern with a
	     short iteration interval */
	  sprintf (_plotter->data->page->point, 
		   "LT%d,%.4f;",
		   HPGL_L_SHORTDASHED,
		   0.5 * iter_interval);
	  break;
	case PL_L_DOTDOTDOTDASHED:
	  /* not a native line type before HP-GL/2; use "dotdotdashed" */
	  sprintf (_plotter->data->page->point, 
		   "LT%d,%.4f;", 
		   HPGL_L_DOTDOTDASHED,
		   iter_interval);
	  break;
	default:
	  sprintf (_plotter->data->page->point, 
		   "LT%d,%.4f;", 
		   _hpgl_line_type[_plotter->drawstate->line_type], 
		   iter_interval);
	}
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_line_type = 
	_hpgl_line_type[_plotter->drawstate->line_type];
    }
  
  /* if plotter's line attributes don't agree with what they should be,
     adjust them (HP-GL/2 only) */
  if (_plotter->hpgl_version == 2)
    {
      if ((_plotter->hpgl_cap_style 
	   != _hpgl_cap_style[_plotter->drawstate->cap_type])
	  || (_plotter->hpgl_join_style 
	      != _hpgl_join_style[_plotter->drawstate->join_type]))
	{
	  sprintf (_plotter->data->page->point, "LA1,%d,2,%d;", 
		   _hpgl_cap_style[_plotter->drawstate->cap_type],
		   _hpgl_join_style[_plotter->drawstate->join_type]);
	  _update_buffer (_plotter->data->page);
	  _plotter->hpgl_cap_style = 
	    _hpgl_cap_style[_plotter->drawstate->cap_type];
	  _plotter->hpgl_join_style = 
	    _hpgl_join_style[_plotter->drawstate->join_type];
	}
    }
  
  /* if plotter's miter limit doesn't agree with what it should be, update
     it (HP-GL/2 only) */
  if (_plotter->hpgl_version == 2 
      && _plotter->hpgl_miter_limit != _plotter->drawstate->miter_limit)
    {
      double new_limit = _plotter->drawstate->miter_limit;
      int new_limit_integer;
      
      if (new_limit > 32767.0)	/* clamp */
	new_limit = 32767.0;
      else if (new_limit < 1.0)
	new_limit = 1.0;
      new_limit_integer = (int)new_limit; /* floor */
      
      sprintf (_plotter->data->page->point, "LA3,%d;", new_limit_integer);
      _update_buffer (_plotter->data->page);
      _plotter->hpgl_miter_limit = _plotter->drawstate->miter_limit;
    }

  /* if plotter's pen width doesn't agree with what it should be (i.e. the
     device-frame version of our line width), update it (HP-GL/2 only) */
  if (_plotter->hpgl_version == 2)
    {
      if (_plotter->hpgl_pen_width != desired_hpgl_pen_width)
	{
	  sprintf (_plotter->data->page->point, "PW%.4f;", 
		   100.0 * desired_hpgl_pen_width);
	  _update_buffer (_plotter->data->page);
	  _plotter->hpgl_pen_width = desired_hpgl_pen_width;
	}
    }
}
