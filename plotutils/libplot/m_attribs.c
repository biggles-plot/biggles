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

#include "sys-defines.h"
#include "extern.h"

/* ARGS: mask = attributes to be updated */
void
_pl_m_set_attributes (R___(Plotter *_plotter) unsigned int mask)
{
  if (mask & PL_ATTR_POSITION)
    {
      if (_plotter->meta_pos.x != _plotter->drawstate->pos.x
	  || _plotter->meta_pos.y != _plotter->drawstate->pos.y)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FMOVE);
	  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->pos.x);
	  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->pos.y);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_pos = _plotter->drawstate->pos;
	}
    }

  if (mask & PL_ATTR_TRANSFORMATION_MATRIX)
    {
      bool need_change = false;
      int i;
      
      for (i = 0; i < 6; i++)
	{
	  if (_plotter->meta_m_user_to_ndc[i]
	      != _plotter->drawstate->transform.m_user_to_ndc[i])
	    {
	      need_change = true;
	      break;
	    }
	}
      if (need_change)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FSETMATRIX);
	  for (i = 0; i < 6; i++)
	    {
	      _pl_m_emit_float (R___(_plotter) _plotter->drawstate->transform.m_user_to_ndc[i]);
	      _plotter->meta_m_user_to_ndc[i] =
		_plotter->drawstate->transform.m_user_to_ndc[i];
	    }
	  _pl_m_emit_terminator (S___(_plotter));
	}
    }

  if (mask & PL_ATTR_PEN_COLOR)
    {
      if (_plotter->meta_fgcolor.red != _plotter->drawstate->fgcolor.red
	  || _plotter->meta_fgcolor.green != _plotter->drawstate->fgcolor.green
	  || _plotter->meta_fgcolor.blue != _plotter->drawstate->fgcolor.blue)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_PENCOLOR);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fgcolor.red);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fgcolor.green);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fgcolor.blue);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_fgcolor = _plotter->drawstate->fgcolor;
	}
    }

  if (mask & PL_ATTR_FILL_COLOR)
    {
      if (_plotter->meta_fillcolor_base.red != _plotter->drawstate->fillcolor_base.red
	  || _plotter->meta_fillcolor_base.green != _plotter->drawstate->fillcolor_base.green
	  || _plotter->meta_fillcolor_base.blue != _plotter->drawstate->fillcolor_base.blue)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FILLCOLOR);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fillcolor_base.red);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fillcolor_base.green);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fillcolor_base.blue);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_fillcolor_base = _plotter->drawstate->fillcolor_base;
	}
    }

  if (mask & PL_ATTR_BG_COLOR)
    {
      if (_plotter->meta_bgcolor.red != _plotter->drawstate->bgcolor.red
	  || _plotter->meta_bgcolor.green != _plotter->drawstate->bgcolor.green
	  || _plotter->meta_bgcolor.blue != _plotter->drawstate->bgcolor.blue)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_BGCOLOR);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->bgcolor.red);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->bgcolor.green);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->bgcolor.blue);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_bgcolor = _plotter->drawstate->bgcolor;
	}
    }

  if (mask & PL_ATTR_PEN_TYPE)
    {
      if (_plotter->meta_pen_type != _plotter->drawstate->pen_type)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_PENTYPE);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->pen_type);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_pen_type = _plotter->drawstate->pen_type;
	}
    }

  if (mask & PL_ATTR_FILL_TYPE)
    {
      if (_plotter->meta_fill_type != _plotter->drawstate->fill_type)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FILLTYPE);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->fill_type);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_fill_type = _plotter->drawstate->fill_type;
	}
    }

  if (mask & PL_ATTR_LINE_STYLE)
    {
      if (_plotter->drawstate->dash_array_in_effect)
	/* desired line style specified by a dashing pattern */
	{
	  bool array_ok = true, offset_ok = true;
	  int i;
	  
	  if (_plotter->meta_dash_array_in_effect == false
	      || (_plotter->meta_dash_array_len != 
		  _plotter->drawstate->dash_array_len))
	    array_ok = false;
	  else
	    {
	      for (i = 0; i < _plotter->meta_dash_array_len; i++)
		{
		  if (_plotter->meta_dash_array[i] !=
		      _plotter->drawstate->dash_array[i])
		    {
		      array_ok = false;
		      break;
		    }
		}
	    }
      
	  if (_plotter->meta_dash_offset != _plotter->drawstate->dash_offset)
	    offset_ok = false;

	  if (array_ok == false || offset_ok == false)
	    {
	      _pl_m_emit_op_code (R___(_plotter) O_FLINEDASH);
	      _pl_m_emit_integer (R___(_plotter) 
			       _plotter->drawstate->dash_array_len);
	      for (i = 0; i < _plotter->drawstate->dash_array_len; i++)
		_pl_m_emit_float (R___(_plotter) 
			       _plotter->drawstate->dash_array[i]);
	      _pl_m_emit_float (R___(_plotter) _plotter->drawstate->dash_offset);
	      _pl_m_emit_terminator (S___(_plotter));

	      if (array_ok == false)
		{
		  double *new_dash_array;

		  if (_plotter->meta_dash_array != (const double *)NULL)
		    free ((double *)_plotter->meta_dash_array);
		    
		  new_dash_array = (double *)_pl_xmalloc (_plotter->drawstate->dash_array_len * sizeof (double));
		  for (i = 0; i < _plotter->drawstate->dash_array_len; i++)
		    new_dash_array[i] = _plotter->drawstate->dash_array[i];
		    _plotter->meta_dash_array = new_dash_array;
		    _plotter->meta_dash_array_len = 
		      _plotter->drawstate->dash_array_len;
		}

	      if (offset_ok == false)
		_plotter->meta_dash_offset = _plotter->drawstate->dash_offset;

	      _plotter->meta_dash_array_in_effect = true;
	    }
	}
      else
	/* desired line style is a builtin line mode */
	{
	  if (_plotter->drawstate->points_are_connected == false)
	    /* select special "disconnected" line mode */
	    {
	      if (_plotter->meta_dash_array_in_effect
		  || _plotter->meta_points_are_connected)
		{
		  _pl_m_emit_op_code (R___(_plotter) O_LINEMOD);
		  _pl_m_emit_string (R___(_plotter) "disconnected");
		  _pl_m_emit_terminator (S___(_plotter));
		  _plotter->meta_points_are_connected = false;
		  _plotter->meta_line_type = PL_L_SOLID;
		}
	    }
	  else
	    /* select a normal line mode */
	    {
	      if (_plotter->meta_dash_array_in_effect
		  || _plotter->meta_points_are_connected == false
		  || (_plotter->meta_line_type !=
		      _plotter->drawstate->line_type))
		{
		  const char *line_mode;

		  _pl_m_emit_op_code (R___(_plotter) O_LINEMOD);
		  switch (_plotter->drawstate->line_type)
		    {
		    case PL_L_SOLID:
		    default:
		      line_mode = "solid";
		      break;
		    case PL_L_DOTTED:
		      line_mode = "dotted";
		      break;
		    case PL_L_DOTDASHED:
		      line_mode = "dotdashed";
		      break;
		    case PL_L_SHORTDASHED:
		      line_mode = "shortdashed";
		      break;
		    case PL_L_LONGDASHED:
		      line_mode = "longdashed";
		      break;
		    case PL_L_DOTDOTDASHED:
		      line_mode = "dotdotdashed";
		      break;
		    case PL_L_DOTDOTDOTDASHED:
		      line_mode = "dotdotdotdashed";
		      break;
		    }
		  _pl_m_emit_string (R___(_plotter) line_mode);
		  _pl_m_emit_terminator (S___(_plotter));
		  _plotter->meta_points_are_connected = true;
		  _plotter->meta_line_type = _plotter->drawstate->line_type;
		}
	    }

	  /* discard current dash array if any, since we've selected a
	     builtin line mode rather than a user-specified dashing mode */
	  _plotter->meta_dash_array_in_effect = false;
	  if (_plotter->meta_dash_array != (const double *)NULL)
	    {
	      free ((double *)_plotter->meta_dash_array);
	      _plotter->meta_dash_array = (const double *)NULL;
	    }
	}
    }

  if (mask & PL_ATTR_LINE_WIDTH)
    {
      if ((_plotter->meta_line_width_is_default == false 
	   && _plotter->drawstate->line_width_is_default == false
	   && _plotter->meta_line_width != _plotter->drawstate->line_width)
	  ||
	  (_plotter->meta_line_width_is_default !=
	   _plotter->drawstate->line_width_is_default))
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FLINEWIDTH);
	  if (_plotter->drawstate->line_width_is_default)
	    /* switch to default by emitting negative line width */
	    _pl_m_emit_float (R___(_plotter) -1.0);
	  else
	    _pl_m_emit_float (R___(_plotter) _plotter->drawstate->line_width);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_line_width = _plotter->drawstate->line_width;
	  _plotter->meta_line_width_is_default = 
	    _plotter->drawstate->line_width_is_default;
	}
    }

  if (mask & PL_ATTR_ORIENTATION)
    {
      if (_plotter->meta_orientation != _plotter->drawstate->orientation)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_ORIENTATION);
	  _pl_m_emit_integer (R___(_plotter) _plotter->drawstate->orientation);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_orientation = _plotter->drawstate->orientation;
	}
    }

  if (mask & PL_ATTR_MITER_LIMIT)
    {
      if (_plotter->meta_miter_limit != _plotter->drawstate->miter_limit)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FMITERLIMIT);
	  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->miter_limit);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_miter_limit = _plotter->drawstate->miter_limit;
	}
    }

  if (mask & PL_ATTR_FILL_RULE)
    {
      if (_plotter->meta_fill_rule_type != _plotter->drawstate->fill_rule_type)
	{
	  const char *fill_mode;

	  _pl_m_emit_op_code (R___(_plotter) O_FILLMOD);
	  switch (_plotter->drawstate->fill_rule_type)
	    {
	    case PL_FILL_ODD_WINDING:
	    default:
	      fill_mode = "even-odd";
	      break;
	    case PL_FILL_NONZERO_WINDING:
	      fill_mode = "nonzero-winding";
	      break;
	    }
	  _pl_m_emit_string (R___(_plotter) fill_mode);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_fill_rule_type = _plotter->drawstate->fill_rule_type;
	}
    }

  if (mask & PL_ATTR_JOIN_STYLE)
    {
      if (_plotter->meta_join_type != _plotter->drawstate->join_type)
	{
	  const char *join_mode;

	  _pl_m_emit_op_code (R___(_plotter) O_JOINMOD);
	  switch (_plotter->drawstate->join_type)
	    {
	    case PL_JOIN_MITER:
	    default:
	      join_mode = "miter";
	      break;
	    case PL_JOIN_ROUND:
	      join_mode = "round";
	      break;
	    case PL_JOIN_BEVEL:
	      join_mode = "bevel";
	      break;
	    case PL_JOIN_TRIANGULAR:
	      join_mode = "triangular";
	      break;
	    }
	  _pl_m_emit_string (R___(_plotter) join_mode);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_join_type = _plotter->drawstate->join_type;
	}
    }

  if (mask & PL_ATTR_CAP_STYLE)
    {
      if (_plotter->meta_cap_type != _plotter->drawstate->cap_type)
	{
	  const char *cap_mode;

	  _pl_m_emit_op_code (R___(_plotter) O_CAPMOD);
	  switch (_plotter->drawstate->cap_type)
	    {
	    case PL_CAP_BUTT:
	    default:
	      cap_mode = "butt";
	      break;
	    case PL_CAP_ROUND:
	      cap_mode = "round";
	      break;
	    case PL_CAP_PROJECT:
	      cap_mode = "project";
	      break;
	    case PL_CAP_TRIANGULAR:
	      cap_mode = "triangular";
	      break;
	    }
	  _pl_m_emit_string (R___(_plotter) cap_mode);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_cap_type = _plotter->drawstate->cap_type;
	}
    }

  if (mask & PL_ATTR_FONT_NAME)
    {
      const char *font_name = _plotter->drawstate->font_name;
      
      if (_plotter->meta_font_name == (const char *)NULL
	  || strcasecmp (_plotter->meta_font_name, font_name) != 0)
	{
	  char *copied_font_name;

	  copied_font_name = (char *)_pl_xmalloc (strlen (font_name) + 1);
	  strcpy (copied_font_name, font_name);

	  _pl_m_emit_op_code (R___(_plotter) O_FONTNAME);
	  _pl_m_emit_string (R___(_plotter) copied_font_name);
	  _pl_m_emit_terminator (S___(_plotter));
	  if (_plotter->meta_font_name != (const char *)NULL)
	    free ((char *)_plotter->meta_font_name);
	  _plotter->meta_font_name = copied_font_name;
	}
    }

  if (mask & PL_ATTR_FONT_SIZE)
    {
      if ((_plotter->meta_font_size_is_default == false 
	   && _plotter->drawstate->font_size_is_default == false
	   && _plotter->meta_font_size != _plotter->drawstate->font_size)
	  ||
	  (_plotter->meta_font_size_is_default !=
	   _plotter->drawstate->font_size_is_default))
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FFONTSIZE);
	  if (_plotter->drawstate->font_size_is_default)
	    /* switch to default by emitting negative font size */
	    _pl_m_emit_float (R___(_plotter) -1.0);
	  else
	    _pl_m_emit_float (R___(_plotter) _plotter->drawstate->font_size);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_font_size = _plotter->drawstate->font_size;
	  _plotter->meta_font_size_is_default = 
	    _plotter->drawstate->font_size_is_default;
	}
    }

  if (mask & PL_ATTR_TEXT_ANGLE)
    {
      if (_plotter->meta_text_rotation != _plotter->drawstate->text_rotation)
	{
	  _pl_m_emit_op_code (R___(_plotter) O_FTEXTANGLE);
	  _pl_m_emit_float (R___(_plotter) _plotter->drawstate->text_rotation);
	  _pl_m_emit_terminator (S___(_plotter));
	  _plotter->meta_text_rotation = _plotter->drawstate->text_rotation;
	}
    }
}




