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

/* This file contains the fontname, fontsize, and textangle methods, which
   are a GNU extension to libplot.  They set drawing attributes: the name
   of the font used for text subsequent drawn on the graphics device, the
   size, and the text angle. 

   The fontname, fontsize, and textangle methods return the fontsize in
   user units, as an aid to vertical positioning by the user.  (The
   fontsize is normally taken to be a minimum vertical spacing between
   adjacent lines of text.)

   The return value may depend on the mapping from user coordinates to
   graphics device coordinates, and hence, e.g., on the arguments given to
   the space() method.

   Note that the size of the font may change when the rotation angle is
   changed, since some fonts may not be available at all rotation angles,
   so that a default font must be switched to.  Also, not all font sizes
   may be available (there may need to be some size quantization).  So the
   return value should always be checked. */

#include "sys-defines.h"
#include "extern.h"

double
_API_ffontname (R___(Plotter *_plotter) const char *s)
{
  char *font_name;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontname: invalid operation");
      return -1;
    }

  /* Null pointer resets to default.  (N.B. we don't look at the font_name
     field in _default_drawstate, because it's a dummy.) */
  if ((s == NULL) || (*s == '\0') || !strcmp(s, "(null)"))
    switch (_plotter->data->default_font_type)
      {
      case PL_F_HERSHEY:
      default:
	s = PL_DEFAULT_HERSHEY_FONT;
	break;
      case PL_F_POSTSCRIPT:
	s = PL_DEFAULT_POSTSCRIPT_FONT;
	break;
      case PL_F_PCL:
	s = PL_DEFAULT_PCL_FONT;
	break;
      case PL_F_STICK:
	s = PL_DEFAULT_STICK_FONT;
	break;
      }

  /* save new font name */
  free ((char *)_plotter->drawstate->font_name);
  font_name = (char *)_pl_xmalloc (strlen (s) + 1);
  strcpy (font_name, s);
  _plotter->drawstate->font_name = font_name;

  /* retrieve font and metrics; compute `true' font size (may differ) */
  _pl_g_set_font (S___(_plotter));

  /* return value is size in user units */
  return _plotter->drawstate->true_font_size;
}

double
_API_ffontsize (R___(Plotter *_plotter) double size)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ffontsize: invalid operation");
      return -1;
    }

  if (size < 0.0)		/* reset to default */
    {
      size = _plotter->drawstate->default_font_size;
      _plotter->drawstate->font_size_is_default = true;
    }
  else
    _plotter->drawstate->font_size_is_default = false;

  /* set the new nominal size in the drawing state */
  _plotter->drawstate->font_size = size;

  /* retrieve font and metrics; compute `true' font size (may differ) */
  _pl_g_set_font (S___(_plotter));
  
  /* flag fontsize as having been invoked (so that fsetmatrix will no
     longer automatically adjust the font size to a reasonable value) */
  _plotter->data->fontsize_invoked = true;

  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}

double
_API_ftextangle (R___(Plotter *_plotter) double angle)
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "ftextangle: invalid operation");
      return -1;
    }

  /* save new rotation angle */
  _plotter->drawstate->text_rotation = angle;
  
  /* retrieve font and metrics; compute `true' font size (may differ) */
  _pl_g_set_font (S___(_plotter));
  
  /* return quantized user-specified font size */
  return _plotter->drawstate->true_font_size;
}

/* Below are four rather silly Plotter methods that are an undocumented
   part of the libplot/libplotter API.  Each returns a pointer to the head
   of a font database in g_fontdb.c, so that an application program that is
   too nosy for its own good can pry out font information.

   These should be replaced by a properly crafted API for querying font
   names, font metrics, etc. */

void *
_pl_get_hershey_font_info (S___(Plotter *_plotter))
{
  return (void *)_pl_g_hershey_font_info;
}

void *
_pl_get_ps_font_info (S___(Plotter *_plotter))
{
  return (void *)_pl_g_ps_font_info;
}

void *
_pl_get_pcl_font_info (S___(Plotter *_plotter))
{
  return (void *)_pl_g_pcl_font_info;
}

void *
_pl_get_stick_font_info (S___(Plotter *_plotter))
{
  return (void *)_pl_g_stick_font_info;
}

