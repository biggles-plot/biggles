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

/* This file contains the AI-driver-specific version of the low-level
   paint_text_string() method, which is called to plot a label in the
   current font (either PS or PCL), at the current fontsize and textangle.
   The label is just a string: no control codes (font switching or
   sub/superscripts).

   The width of the string in user units is returned. */

#include "sys-defines.h"
#include "extern.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

/* This prints a single-font, single-font-size label.  When this is called,
   the current point is on the intended baseline of the label.  */

/* ARGS: h_just = horiz justification, PL_JUST_LEFT, CENTER, or RIGHT 
   	 v_just = vert justificattion, PL_JUST_TOP, BASE, or BOTTOM */
double
_pl_a_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  int i, master_font_index;
  int justify_code;
  double width;
  unsigned char *ptr;
  double theta, costheta, sintheta;
  double norm;
  double dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3;
  double font_ascent, font_descent, up, down;
  double user_font_size = _plotter->drawstate->true_font_size;
  double device_font_size;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  double lshift;
  bool pcl_font;
  
  /* sanity check; this routine supports only baseline positioning */
  if (v_just != PL_JUST_BASE)
    return 0.0;

  /* if empty string, nothing to do */
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* sanity check */
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT
      && _plotter->drawstate->font_type != PL_F_PCL)
    return 0.0;
  pcl_font = (_plotter->drawstate->font_type == PL_F_PCL ? true : false);

  /* compute index of font in master table of PS [or PCL] fonts, in
     g_fontdb.c */
  if (pcl_font)			/* one of the 45 standard PCL fonts */
    master_font_index =
      (_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  else				/* one of the 35 standard PS fonts */
    master_font_index =
      (_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* font ascent and descent (taken from the font's bounding box) */
  if (pcl_font)
    {
      font_ascent = (double)((_pl_g_pcl_font_info[master_font_index]).font_ascent);
      font_descent = (double)((_pl_g_pcl_font_info[master_font_index]).font_descent);
    }
  else				/* PS font */
    {
      font_ascent = (double)((_pl_g_ps_font_info[master_font_index]).font_ascent);
      font_descent = (double)((_pl_g_ps_font_info[master_font_index]).font_descent);
    }
  up = user_font_size * font_ascent / 1000.0;
  down = user_font_size * font_descent / 1000.0;

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

  /* this transformation matrix rotates, and translates; it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = _plotter->drawstate->pos.x;
  user_text_transformation_matrix[5] = _plotter->drawstate->pos.y;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */
  _matrix_product (user_text_transformation_matrix, 
		   _plotter->drawstate->transform.m,
		   text_transformation_matrix);

  /* We need to extract a quantity we can call a font size in device
     coordinates, for the benefit of AI.  (AI needs to retrieve a font, and
     transform it.)

     We define this to be user_font_size (the nominal font size in user
     coordinates), times the norm of the linear tranformation contained in
     the temporary matrix we just constructed (the magnitude of its larger
     singular value).  Recall that for any square matrix M, the singular
     values are the square roots of the eigenvalues of the symmetric matrix
     M^t M. */

  norm = _matrix_norm (text_transformation_matrix);

  if (norm == 0.0)		/* avoid division by zero */
    return 0.0;

  device_font_size = norm * user_font_size;

  /* Now scale the text transformation matrix so that the linear
     transformation contained in it has unit norm (if there is no shearing,
     it will just be a rotation; if there is no rotation either, it will be
     the identity matrix). */
  for (i = 0; i < 4; i++)
    text_transformation_matrix[i] /= norm;

  /* AI directive: begin `point text' object */
  strcpy (_plotter->data->page->point, "0 To\n");
  _update_buffer (_plotter->data->page);

  /* output text transformation matrix */
  for (i = 0; i < 6; i++)
    {
      sprintf (_plotter->data->page->point, "%.4f ", 
	       text_transformation_matrix[i]);
      _update_buffer (_plotter->data->page);      
    }
  strcpy (_plotter->data->page->point, "0 Tp\nTP\n");
  _update_buffer (_plotter->data->page);
  
  /* set render mode: fill text, rather than several other possibilities */
  strcpy (_plotter->data->page->point, "0 Tr\n");
  _update_buffer (_plotter->data->page);

  /* set AI's fill color to be the same as libplot's notion of pen color
     (since letters in label will be drawn as filled outlines) */
  _pl_a_set_fill_color (R___(_plotter) true);

  /* set AI's pen color also, in particular set it to be the same as
     libplot's notion of pen color (even though we'll be filling, not
     stroking); this is a convenience for AI users who may wish e.g. to
     switch from filling letter outlines to stroking them */
  _pl_a_set_pen_color (S___(_plotter)); /* emit AI directive */

  /* AI directive: set font name and size */
  {
    const char *ps_name;

    if (pcl_font)			/* one of the 45 PCL fonts */
      ps_name = _pl_g_pcl_font_info[master_font_index].ps_name;
    else				/* one of the 35 PS fonts */
      ps_name = _pl_g_ps_font_info[master_font_index].ps_name;
    
    /* specify font name (underscore indicates reencoding), font size */
    sprintf (_plotter->data->page->point, "/_%s %.4f Tf\n", 
	     ps_name, device_font_size);
    _update_buffer (_plotter->data->page);
  }
  
  /* set line horizontal expansion factor, in percent */
  strcpy (_plotter->data->page->point, "100 Tz\n");
  _update_buffer (_plotter->data->page);

  /* NO track kerning, please */
  strcpy (_plotter->data->page->point, "0 Tt\n");
  _update_buffer (_plotter->data->page);

  /* turn off pairwise kerning (currently, a libplot convention) */
  strcpy (_plotter->data->page->point, "0 TA\n");
  _update_buffer (_plotter->data->page);

  /* turn off ALL inter-character spacing */
  strcpy (_plotter->data->page->point, "0 0 0 TC\n");
  _update_buffer (_plotter->data->page);

  /* use the default inter-word spacing; no more, no less */
  strcpy (_plotter->data->page->point, "100 100 100 TW\n");
  _update_buffer (_plotter->data->page);

  /* no indentation at beginning of `paragraphs' */
  strcpy (_plotter->data->page->point, "0 0 0 Ti\n");
  _update_buffer (_plotter->data->page);

  /* specify justification */
  switch (h_just)
    {
    case PL_JUST_LEFT:
    default:
      justify_code = 0;
      break;
    case PL_JUST_CENTER:
      justify_code = 1;
      break;
    case PL_JUST_RIGHT:
      justify_code = 2;
      break;
    }
  sprintf (_plotter->data->page->point, "%d Ta\n", justify_code);
  _update_buffer (_plotter->data->page);

  /* no hanging quotation marks */
  strcpy (_plotter->data->page->point, "0 Tq\n");
  _update_buffer (_plotter->data->page);

  /* no leading between lines of a paragraph or between paragraphs */
  strcpy (_plotter->data->page->point, "0 0 Tl\n");
  _update_buffer (_plotter->data->page);

  /* compute width of the substring in user units (used below in
     constructing a bounding box) */
  width = _plotter->get_text_width (R___(_plotter) s);

  /* for computing bounding box, compute justification-dependent leftward
     shift, as fraction of label width */
  switch (h_just)
    {
    case PL_JUST_LEFT:
    default:
      lshift = 0.0;
      break;
    case PL_JUST_CENTER:
      lshift = 0.5;
      break;
    case PL_JUST_RIGHT:
      lshift = 1.0;
      break;
    }

  /* to compute an EPS-style bounding box, first compute offsets to the
     four vertices of the smallest rectangle containing the string */

  dx0 = costheta * (- lshift) * width - sintheta * (-down);
  dy0 = sintheta * (- lshift) * width + costheta * (-down);
  
  dx1 = costheta * (- lshift) * width - sintheta * up;
  dy1 = sintheta * (- lshift) * width + costheta * up;
  
  dx2 = costheta * (1.0 - lshift) * width - sintheta * (-down);
  dy2 = sintheta * (1.0 - lshift) * width + costheta * (-down);
  
  dx3 = costheta * (1.0 - lshift) * width - sintheta * up;
  dy3 = sintheta * (1.0 - lshift) * width + costheta * up;

  /* record that we're using all four vertices (args of _update_bbox() are in
     device units, not user units) */
  _update_bbox (_plotter->data->page, XD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0),
	      YD ((_plotter->drawstate->pos).x + dx0, (_plotter->drawstate->pos).y + dy0));
  _update_bbox (_plotter->data->page, XD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1), 
	      YD ((_plotter->drawstate->pos).x + dx1, (_plotter->drawstate->pos).y + dy1));
  _update_bbox (_plotter->data->page, XD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2), 
	      YD ((_plotter->drawstate->pos).x + dx2, (_plotter->drawstate->pos).y + dy2));
  _update_bbox (_plotter->data->page, XD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3), 
	      YD ((_plotter->drawstate->pos).x + dx3, (_plotter->drawstate->pos).y + dy3));

  /* output string as a PS string (i.e. surrounded by parentheses) */
  ptr = (unsigned char *)_plotter->data->page->point;
  *ptr++ = '(';
  while (*s)
    {
      switch (*s)
	{
	case '(':		/* for PS, escape ()/ */
	case ')':
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
  *ptr++ = ')';
  *ptr = (unsigned char)'\0';
  _update_buffer (_plotter->data->page);

  /* AI directive: this is the text to be rendered */
  strcpy (_plotter->data->page->point, " Tx\n");
  _update_buffer (_plotter->data->page);

  /* AI directive: end of text object */
  strcpy (_plotter->data->page->point, "TO\n");
  _update_buffer (_plotter->data->page);

  /* flag current PS or PCL font as used */
  if (pcl_font)
    _plotter->data->page->pcl_font_used[master_font_index] = true;
  else
    _plotter->data->page->ps_font_used[master_font_index] = true;

  return width;
}
