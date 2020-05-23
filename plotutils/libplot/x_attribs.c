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

/* This internal method is invoked by an XDrawablePlotter (or XPlotter)
   before drawing any polyline.  It sets the relevant attributes in our X11
   graphics contexts (the one used for drawing, and the one used for
   filling) to what they should be.  This includes line type, cap type,
   join type, line width, and fill rule.  

   The GC's are elements of the X11-specific part of any libplot drawing
   state.  The X11-specific part also includes `non-opaque' representations
   of the attributes, which can be easily queried.  They are updated too.

   Any invoker should pass an argument to this method, indicating which of
   the two graphics contexts (the one used for drawing, and the one used
   for filling) should be updated.  The only attribute we set in the latter
   is the fill rule; all the other attributes listed above are set in the
   former. */

#include "sys-defines.h"
#include "extern.h"

/* The length of each dash must fit in an unsigned char (X11 convention) */
#define MAX_DASH_LENGTH 255

/* ARGS: x_gc_type specifies which of our X GC's to modify */
void
_pl_x_set_attributes (R___(Plotter *_plotter) int x_gc_type)
{
  int i;

  if (_plotter->x_drawable1 == (Drawable)NULL 
      && _plotter->x_drawable2 == (Drawable)NULL)
    /* no drawables, so GC's must not have been created (see
       x_savestate.c); do nothing */
    return;

  if (x_gc_type == X_GC_FOR_DRAWING)
    /* update attributes in GC used for drawing */
    {
      XGCValues gcv;
      bool have_dash_list = false;
      unsigned char *dash_list = (unsigned char *)NULL;
      int dash_list_len = 0, dash_offset = 0;
    
      if (_plotter->drawstate->dash_array_in_effect)
	/* have user-specified dash array */
	{
	  dash_list_len = _plotter->drawstate->dash_array_len;
	  if (dash_list_len > 0)
	    {
	      bool odd_length;
	      double min_sing_val, max_sing_val;
	      int i, dash_cycle_length;
	    
	      /* compute minimum singular value of user->device coordinate
		 map, which we use as a multiplicative factor to convert line
		 widths (cf. g_linewidth.c), dash lengths, etc. */
	      _matrix_sing_vals (_plotter->drawstate->transform.m, 
				 &min_sing_val, &max_sing_val);
	    
	      odd_length = (dash_list_len & 1 ? true : false);
	      dash_list = (unsigned char *)_pl_xmalloc ((odd_length ? 2 : 1) * dash_list_len * sizeof(unsigned char));
	      have_dash_list = true; /* will free.. */
	      dash_cycle_length = 0;
	      for (i = 0; i < dash_list_len; i++)
		{
		  double unrounded_dashlen;
		  int dashlen;
		
		  unrounded_dashlen = 
		    min_sing_val * _plotter->drawstate->dash_array[i];
		
		  dashlen = IROUND(unrounded_dashlen);
		  dashlen = IMAX(dashlen, 1);
		  dashlen = IMIN(dashlen, MAX_DASH_LENGTH);
		
		  /* convert dash length, int -> unsigned char */
		  dash_list[i] = (unsigned int)dashlen;
		  dash_cycle_length += dashlen;
		  if (odd_length)
		    {
		      dash_list[dash_list_len + i] = (unsigned int)dashlen;
		      dash_cycle_length += dashlen;
		    }
		}
	      if (odd_length)
		dash_list_len *= 2;
	    
	      dash_offset = IROUND(min_sing_val * _plotter->drawstate->dash_offset);
	      if (dash_cycle_length > 0)
		/* choose an offset in range 0..dash_cycle_length-1 */
		{
		  while (dash_offset < 0)
		    dash_offset += dash_cycle_length;
		  dash_offset %= dash_cycle_length;
		}
	    
	      gcv.line_style = LineOnOffDash;
	    }
	  else			/* no dashes, will draw as solid line */
	    gcv.line_style = LineSolid;
	
	}
      else
	/* have one of the canonical line types */
	{
	  if (_plotter->drawstate->line_type != PL_L_SOLID)
	    {
	      const int *dash_array;
	      int i, scale;
	    
	      dash_list_len = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array_len;	  
	      dash_array = _pl_g_line_styles[_plotter->drawstate->line_type].dash_array;
	      /* scale by line width in terms of pixels, if nonzero */
	      scale = _plotter->drawstate->quantized_device_line_width;
	      if (scale <= 0)
		scale = 1;

	      dash_list = (unsigned char *)_pl_xmalloc (PL_MAX_DASH_ARRAY_LEN * sizeof(unsigned char));
	      have_dash_list = true; /* will free.. */
	      for (i = 0; i < dash_list_len; i++)
		{
		  int dashlen;
		
		  dashlen = scale * dash_array[i];
		  dashlen = IMAX(dashlen, 1);
		  dashlen = IMIN(dashlen, MAX_DASH_LENGTH);
		  dash_list[i] = (unsigned int)dashlen; /* int->unsigned char*/
		}

	      /* use a non-solid line style */
	      gcv.line_style = LineOnOffDash;	  
	      dash_offset = 0;
	    }
	  else			/* no dash list */
	    {
	      /* use a solid line style */
	      gcv.line_style = LineSolid;
	    }
	}
  
      /* update dash style attributes (dash offset and dash list) */

      if (have_dash_list)
	{
	  bool do_it = false;

	  if (_plotter->drawstate->x_gc_dash_offset != dash_offset
	      || _plotter->drawstate->x_gc_dash_list_len != dash_list_len)
	    do_it = true;
	  if (do_it == false)
	    {
	      for (i = 0; i < dash_list_len; i++)
		{
		  if ((unsigned char)_plotter->drawstate->x_gc_dash_list[i] != dash_list[i])
		    {
		      do_it = true;
		      break;
		    }
		}
	    }

	  if (do_it)
	    {
	      /* change the GC used for drawing */
	      XSetDashes (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
			  dash_offset, (char *)dash_list, dash_list_len);

	      /* update non-opaque information on dash style, by installing
		 dash_list as our `non-opaque dash list' */

	      /* free former non-opaque dash list if any */
	      if (_plotter->drawstate->x_gc_dash_list_len > 0)
		free ((char *)_plotter->drawstate->x_gc_dash_list);

	      _plotter->drawstate->x_gc_dash_list = (char *)dash_list;
	      _plotter->drawstate->x_gc_dash_list_len = dash_list_len;
	      _plotter->drawstate->x_gc_dash_offset = dash_offset;
	    }
	  else
	    free (dash_list);
	}

      /* update line style attribute */

      if (_plotter->drawstate->x_gc_line_style != gcv.line_style)
	{
	  /* change the GC used for drawing */
	  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		     GCLineStyle, &gcv);
	  /* update non-opaque line-style element */
	  _plotter->drawstate->x_gc_line_style = gcv.line_style;
	}

      /* update cap style attribute */

      switch (_plotter->drawstate->cap_type)
	{
	case PL_CAP_BUTT:
	default:
	  gcv.cap_style = CapButt;
	  break;
	case PL_CAP_ROUND:
	  gcv.cap_style = CapRound;
	  break;
	case PL_CAP_PROJECT:
	  gcv.cap_style = CapProjecting;
	  break;
	case PL_CAP_TRIANGULAR:	/* not supported by X11 */
	  gcv.cap_style = CapRound;
	  break;
	}
      if (_plotter->drawstate->x_gc_cap_style != gcv.cap_style)
	{
	  /* change the GC used for drawing */
	  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		     GCCapStyle, &gcv);
	  /* update non-opaque cap style element */
	  _plotter->drawstate->x_gc_cap_style = gcv.cap_style;
	}
    
      /* update join style attribute */

      switch (_plotter->drawstate->join_type)
	{
	case PL_JOIN_MITER:
	default:
	  gcv.join_style = JoinMiter;
	  break;
	case PL_JOIN_ROUND:
	  gcv.join_style = JoinRound;
	  break;
	case PL_JOIN_BEVEL:
	  gcv.join_style = JoinBevel;
	  break;
	case PL_JOIN_TRIANGULAR:	/* not supported by X11 */
	  gcv.join_style = JoinRound;
	  break;
	}
      if (_plotter->drawstate->x_gc_join_style != gcv.join_style)
	{
	  /* change the GC used for drawing */
	  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		     GCJoinStyle, &gcv);
	  /* update non-opaque join style element */
	  _plotter->drawstate->x_gc_join_style = gcv.join_style;
	}
    
      /* update line width attribute */

      gcv.line_width = _plotter->drawstate->quantized_device_line_width;
      if (_plotter->drawstate->x_gc_line_width != gcv.line_width)
	{
	  /* change the GC used for drawing */
	  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fg, 
		     GCLineWidth, &gcv);
	  /* update non-opaque line-width element */
	  _plotter->drawstate->x_gc_line_width = gcv.line_width;
	}
    }
  
  else if (x_gc_type == X_GC_FOR_FILLING)
    /* update attributes in GC used for filling */
    {
      XGCValues gcv;
      
      /* update fill rule attribute */

      switch (_plotter->drawstate->fill_rule_type)
	{
	case PL_FILL_ODD_WINDING:
	default:
	  gcv.fill_rule = EvenOddRule;
	  break;
	case PL_FILL_NONZERO_WINDING:
	  gcv.fill_rule = WindingRule;
	  break;
	}
      if (_plotter->drawstate->x_gc_fill_rule != gcv.fill_rule)
	{
	  /* change the GC used for filling */
	  XChangeGC (_plotter->x_dpy, _plotter->drawstate->x_gc_fill, 
		     GCFillRule, &gcv);
	  /* update non-opaque fill-rule element */
	  _plotter->drawstate->x_gc_fill_rule = gcv.fill_rule;
	}
    }
  
  return;
}
