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

/* This file contains the PS-driver-specific version of the low-level
   paint_text_string method, which is called to plot a label in the current
   font (either PS or PCL), at the current fontsize and textangle.  The
   label is just a string: no control codes (font switching or
   sub/superscripts).

   The width of the string in user units is returned.

   This version does not support center-justification and
   right-justification; only the default left-justification.  That is
   all right, since label justification is handled at a higher level. */

#include "sys-defines.h"
#include "extern.h"

#define GOOD_PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))

double
_pl_p_paint_text_string (R___(Plotter *_plotter) const unsigned char *s, int h_just, int v_just)
{
  int i, master_font_index;
  double width;
  unsigned char *ptr;
  double theta, costheta, sintheta;
  double norm;
  double crockshift_x, crockshift_y;
  double dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3;
  double font_ascent, font_descent, up, down;
  double user_font_size = _plotter->drawstate->true_font_size;
  double device_font_size;
  double user_text_transformation_matrix[6];
  double text_transformation_matrix[6];
  bool pcl_font;
  
  /* sanity check; this routine supports only baseline positioning */
  if (v_just != PL_JUST_BASE)
    return 0.0;

  /* similarly for horizontal justification */
  if (h_just != PL_JUST_LEFT)
    /* shouldn't happen */
    return 0.0;

  /* if empty string, nothing to do */
  if (*s == (unsigned char)'\0')
    return 0.0;

  /* sanity check */
#ifndef USE_LJ_FONTS_IN_PS
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT)
    return 0.0;
#else  /* USE_LJ_FONTS_IN_PS */
  if (_plotter->drawstate->font_type != PL_F_POSTSCRIPT
      && _plotter->drawstate->font_type != PL_F_PCL)
    return 0.0;
#endif
  pcl_font = (_plotter->drawstate->font_type == PL_F_PCL ? true : false);

  /* compute index of font in master table of PS [or PCL] fonts, in
     g_fontdb.c */
  if (pcl_font)			/* one of the 45 standard PCL fonts */
    master_font_index =
      (_pl_g_pcl_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];
  else				/* one of the 35 standard PS fonts */
    master_font_index =
      (_pl_g_ps_typeface_info[_plotter->drawstate->typeface_index].fonts)[_plotter->drawstate->font_index];

  /* label rotation angle in radians, in user frame */
  theta = M_PI * _plotter->drawstate->text_rotation / 180.0;
  sintheta = sin (theta);
  costheta = cos (theta);

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

  /* Current point is on the baseline, but the rendering logic of the PS
     code in the idraw prologue requires us to perform a vertical shift at
     this point.  (We'll undo the vertical shift immediately.) */
  _plotter->drawstate->pos.x -= (user_font_size - down) * sintheta;
  _plotter->drawstate->pos.y += (user_font_size - down) * costheta;

  /* the idraw PS prologue (see p_header.c) performs an additional
     [gratuitous] vertical shift by 1 unit, which we must compensate for */
  {
    double ctm_norm = _matrix_norm (_plotter->drawstate->transform.m);
    
    crockshift_x = sintheta / ctm_norm;
    crockshift_y = costheta / ctm_norm;
  }
  _plotter->drawstate->pos.x += crockshift_x;
  _plotter->drawstate->pos.y -= crockshift_y;

  /* this transformation matrix rotates, and translates: it maps (0,0) to
     the origin of the string, in user coordinates */
  user_text_transformation_matrix[0] = costheta;
  user_text_transformation_matrix[1] = sintheta;
  user_text_transformation_matrix[2] = - sintheta;
  user_text_transformation_matrix[3] = costheta;
  user_text_transformation_matrix[4] = _plotter->drawstate->pos.x;
  user_text_transformation_matrix[5] = _plotter->drawstate->pos.y;

  /* undo vertical shifts performed above */
  _plotter->drawstate->pos.x += (user_font_size - down) * sintheta;
  _plotter->drawstate->pos.y -= (user_font_size - down) * costheta;
  _plotter->drawstate->pos.x -= crockshift_x;
  _plotter->drawstate->pos.y += crockshift_y;

  /* Construct a temporary matrix that rotates, translates, and then maps
     to device coordinates.  This matrix transforms from a frame in which
     nominal character sizes are roughly 1 unit in the horizontal and
     vertical directions, to device coordinates. */
  _matrix_product (user_text_transformation_matrix, 
		   _plotter->drawstate->transform.m,
		   text_transformation_matrix);

  /* We need to extract a quantity we can call a font size in device
     coordinates, for the benefit of idraw.  (Idraw needs to retrieve an X
     font, and scale it.  Idraw does not make use of modern [X11R6+] font
     scaling technology; it does its own scaling of bitmaps.)

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

  /* Many PS interpreters can't handle zero font size.  So bail if the font
     size we'll emit is zero. */
  {
    char charbuf[64];
    double emitted_device_font_size;
    
    sprintf (charbuf, "%f", device_font_size);
    sscanf (charbuf, "%lf", &emitted_device_font_size);
    if (emitted_device_font_size == 0.0)
      return 0.0;
  }

  /* Now scale the text transformation matrix so that the linear
     transformation contained in it has unit norm (if there is no shearing,
     it will just be a rotation; if there is no rotation either, it will be
     the identity matrix). */
  for (i = 0; i < 4; i++)
    text_transformation_matrix[i] /= norm;

  /* prologue instruction, plus idraw directive: start of Text */
  strcpy (_plotter->data->page->point, "Begin %I Text\n");
  _update_buffer (_plotter->data->page);

  /* idraw directive, plus prologue instruction: set foreground color */
  _pl_p_set_pen_color (S___(_plotter));	/* invoked lazily, i.e. when needed */
  sprintf (_plotter->data->page->point, "%%I cfg %s\n%g %g %g SetCFg\n",
	   _pl_p_idraw_stdcolornames[_plotter->drawstate->ps_idraw_fgcolor],
	   _plotter->drawstate->ps_fgcolor_red,
	   _plotter->drawstate->ps_fgcolor_green,
	   _plotter->drawstate->ps_fgcolor_blue);
  _update_buffer (_plotter->data->page);

  /* idraw directive: X Windows font name, which incorporates the X font
     size.  We use our primary X font name (the `x_name' field, not the
     `x_name_alt' field if any). */

  /* N.B. this directive sets the _pixel_ size of the X font to be our
     current point size.  That would really be appropriate only if the
     screen resolution is 72 dpi.  But idraw seems to prefer setting the
     pixel size to setting the point size. */

  if (pcl_font)			/* one of the 45 PCL fonts */
    {
      const char *ps_name;
      
      /* this is to support the Tidbits-is-Wingdings botch */
      if (_pl_g_pcl_font_info[master_font_index].substitute_ps_name)
	ps_name = _pl_g_pcl_font_info[master_font_index].substitute_ps_name;
      else
	ps_name = _pl_g_pcl_font_info[master_font_index].ps_name;

      sprintf (_plotter->data->page->point,
	       "%%I f -*-%s-*-%d-*-*-*-*-*-*-*\n", 
	       (_pl_g_pcl_font_info[master_font_index]).x_name, 
	       IROUND(device_font_size));
      _update_buffer (_plotter->data->page);

      /* prolog instruction: PS font name and size */
      sprintf (_plotter->data->page->point, "/%s %f SetF\n", 
	       ps_name,
	       device_font_size);
      _update_buffer (_plotter->data->page);
    }
  else				/* one of the 35 PS fonts */
    {
      sprintf (_plotter->data->page->point,
	       "%%I f -*-%s-*-%d-*-*-*-*-*-*-*\n", 
	       (_pl_g_ps_font_info[master_font_index]).x_name, 
	       IROUND(device_font_size));
      _update_buffer (_plotter->data->page);

      /* prolog instruction: PS font name and size */
      sprintf (_plotter->data->page->point, "/%s %f SetF\n", 
	       _pl_g_ps_font_info[master_font_index].ps_name,
	       device_font_size);
      _update_buffer (_plotter->data->page);
    }

  /* idraw directive and prologue instruction: text transformation matrix */
  strcpy (_plotter->data->page->point, "%I t\n[ ");
  _update_buffer (_plotter->data->page);

  for (i = 0; i < 6; i++)
    {
      sprintf (_plotter->data->page->point, "%.7g ", 
	       text_transformation_matrix[i]);
      _update_buffer (_plotter->data->page);      
    }
  
  /* width of the string in user units (used below in constructing a
     bounding box, and as return value) */
  width = _plotter->get_text_width (R___(_plotter) s);

  /* to compute an EPS-style bounding box, first compute offsets to the
     four vertices of the smallest rectangle containing the string */
  dx0 = - sintheta * (-down);
  dy0 =   costheta * (-down);
  
  dx1 = - sintheta * up;
  dy1 =   costheta * up;
  
  dx2 = costheta * width - sintheta * (-down);
  dy2 = sintheta * width + costheta * (-down);
  
  dx3 = costheta * width - sintheta * up;
  dy3 = sintheta * width + costheta * up;

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

  /* Finish outputting transformation matrix; begin outputting string. */
  /* Escape all backslashes etc. in the text string, before output. */
  strcpy (_plotter->data->page->point, " ] concat\n\
%I\n\
[\n\
(");
  _update_buffer (_plotter->data->page);

  ptr = (unsigned char *)_plotter->data->page->point;
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
  *ptr = (unsigned char)'\0';
  _update_buffer (_plotter->data->page);

  /* prologue instruction: end of text */
  strcpy (_plotter->data->page->point, ")\n\
] Text\n\
End\n\
\n");
  _update_buffer (_plotter->data->page);

  /* flag current PS or PCL font as used on this page */
#ifdef USE_LJ_FONTS_IN_PS
  if (pcl_font)
    _plotter->data->page->pcl_font_used[master_font_index] = true;

  else
    _plotter->data->page->ps_font_used[master_font_index] = true;
#else
    _plotter->data->page->ps_font_used[master_font_index] = true;
#endif

  return width;
}
