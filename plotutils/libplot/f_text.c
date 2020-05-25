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

/* This file contains the Fig-driver-specific version of the low-level
   paint_text_string() method, which is called to plot a label in the
   current PS font, at the current fontsize and textangle.  The label is
   just a string: no control codes (font switching or sub/superscripts).

   The width of the string in user units is returned.

   This version does not support `sheared' fonts, since xfig does not
   support them.  But it does support center-justification and right
   justification as well as the default left-justification, since the xfig
   format supports them. */

#include "sys-defines.h"
#include "extern.h"

#define FONT_TYPE_LATEX 0	/* don't support LaTeX fonts currently */
#define FONT_TYPE_PS 4

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* Fig horizontal alignment styles, indexed by internal number
   (left/center/right) */
#define FIG_ALIGN_LEFT 0
#define FIG_ALIGN_CENTER 1
#define FIG_ALIGN_RIGHT 2
static const int fig_horizontal_alignment_style[PL_NUM_HORIZ_JUST_TYPES] =
{ FIG_ALIGN_LEFT, FIG_ALIGN_CENTER, FIG_ALIGN_RIGHT };

/* This prints a single-font, single-font-size label.  When this is called,
   the current point is on the intended baseline of the label.  */

/* ARGS: h_just,v_just are PL_JUST_{LEFT|CENTER|RIGHT}, PL_JUST_{TOP etc.} */
double
_pl_f_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  int len, master_font_index;
  unsigned char *ptr, *t;
  double theta, costheta, sintheta;
  double label_width, label_ascent;
  double initial_x, initial_y;
  double horizontal_x, horizontal_y, vertical_x, vertical_y;
  double horizontal_fig_length, vertical_fig_length;
  double horizontal_fig_x, vertical_fig_x;
  double horizontal_fig_y, vertical_fig_y;
  double angle_device;
  
  /* sanity check */
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT)
    return 0.0;

  /* sanity check; this routine supports only baseline positioning */
  if (v_just != PL_JUST_BASE)
    return 0.0;

  /* if empty string, nothing to do */
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* if font (previously retrieved) has a font size of zero in terms of
     integer `Fig points', bail out right now, since xfig can't handle text
     strings with zero point size */
  if (_plotter->drawstate->fig_font_point_size == 0)
    return 0.0;

  /* label rotation angle in radians */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* compute index of font in master table of PS fonts, in g_fontdb.h */
  master_font_index =
    (_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* compute label height and width, in user units */
  label_width = _plotter->get_text_width (R___(_plotter) s);
  label_ascent  = _plotter->drawstate->true_font_size * (_pl_g_ps_font_info[master_font_index]).font_ascent / 1000.0;
  
  /* vector along baseline of label, and an orthogonal vector which is the
     other side of a rectangle containing the portion of the string above
     the baseline (both in the user frame) */

  horizontal_x = costheta * label_width;
  horizontal_y = sintheta * label_width;

  vertical_x =  - sintheta * label_ascent;
  vertical_y =    costheta * label_ascent;
  
  /* Convert two above orthogonal vectors to the device frame, and compute
     their lengths.  In the device frame they may no longer be orthogonal.
     But xfig supports setting up only rectangular `hot regions', so we'll
     use their lengths as the sides of a rectangle.  If user coor->device
     coor map is highly sheared, this would be inappropriate. 

     Incidentally, the height of the rectangular hot region should really
     be the string ascent (from its bounding box), not the font ascent.
     But since we don't include the bounding boxes of individual characters
     in our g_fontdb.c, we have no way of computing the former. */

  horizontal_fig_x = XDV(horizontal_x, horizontal_y);
  horizontal_fig_y = YDV(horizontal_x, horizontal_y);  
  horizontal_fig_length = sqrt(horizontal_fig_x * horizontal_fig_x
				+ horizontal_fig_y * horizontal_fig_y);

  /* text angle in device frame (note flipped-y convention) */
  angle_device = - _xatan2 (horizontal_fig_y, horizontal_fig_x);
  if (angle_device == 0.0)
    angle_device = 0.0;		/* remove sign bit if any */
  
  /* avoid triggering a bug in xfig, which as of release 3.2.2 can't handle
     rotated text strings consisting of a single space character (they
     cause it to crash) */
  if (angle_device != 0.0 && strcmp ((const char *)s, " ") == 0)
    return _plotter->get_text_width (R___(_plotter) s);

  vertical_fig_x = XDV(vertical_x, vertical_y);
  vertical_fig_y = YDV(vertical_x, vertical_y);  
  vertical_fig_length = sqrt(vertical_fig_x * vertical_fig_x
				+ vertical_fig_y * vertical_fig_y);
  
  /* where we should start from, in device frame (i.e. in Fig units) */
  initial_x = XD((_plotter->drawstate->pos).x, (_plotter->drawstate->pos).y);
  initial_y = YD((_plotter->drawstate->pos).x, (_plotter->drawstate->pos).y);

  /* evaluate fig colors lazily, i.e. only when needed */
  _pl_f_set_pen_color (S___(_plotter));
  
  /* escape all backslashes in the text string, before output */
  len = strlen ((char *)s);
  ptr = (unsigned char *)_pl_xmalloc ((4 * len + 1) * sizeof(char));
  t = ptr;
  while (*s)
    {
      switch (*s)
	{
	case '\\':
	  *ptr++ = (unsigned char)'\\';
	  *ptr++ = *s++;
          break;
	default:
          if GOOD_PRINTABLE_ASCII (*s)
	    *ptr++ = *s++;
          else
            {	    
               sprintf ((char *)ptr, "\\%03o", (unsigned int)*s);
               ptr += 4;
               s++;
            }
          break;
	}
    }
  *ptr = (unsigned char)'\0';

  /* update xfig's `depth' attribute */
    if (_plotter->fig_drawing_depth > 0)
      (_plotter->fig_drawing_depth)--;

  sprintf(_plotter->data->page->point,
	  "#TEXT\n%d %d %d %d %d %d %.3f %.3f %d %.3f %.3f %d %d %s\\001\n",
	  4,			/* text object */
	  /* xfig supports 3 justification types: left, center, or right. */
	  fig_horizontal_alignment_style[h_just],/* horizontal just. type */
	  _plotter->drawstate->fig_fgcolor, /* pen color */
	  _plotter->fig_drawing_depth, /* depth */
	  0,			/* pen style, ignored */
	  _pl_g_ps_font_info[master_font_index].fig_id, /* Fig font id */
	  (double)_plotter->drawstate->fig_font_point_size, /* point size (float) */
	  angle_device,		/* text rotation in radians (float) */
	  FONT_TYPE_PS,		/* Fig font type */
	  /* these next two are used only for setting up `hot spots' */
	  vertical_fig_length, /* string height, Fig units (float) */
	  horizontal_fig_length, /* string width, Fig units (float) */
	  /* coors of origin of label, in Fig units */
	  IROUND(initial_x), 
	  IROUND(initial_y),
	  t);			/* munged string */
  free (t);
  _update_buffer (_plotter->data->page);

  return label_width;
}
