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

/* forward references */
static void write_svg_transform (plOutbuf *outbuf, const double m[6]);

bool
_pl_s_end_page (S___(Plotter *_plotter))
{
  plOutbuf *svg_header, *svg_trailer;
      
  /* SVG files contain only one page of graphics so this is a sanity check */
  if (_plotter->data->page_number != 1)	
    return true;

  /* prepare SVG header (i.e. page header), write it to a plOutbuf */
  svg_header = _new_outbuf ();
      
  /* start with DTD */
  sprintf (svg_header->point, "\
<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\"?>\n\
<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
  _update_buffer (svg_header);

  /* Emit nominal physical size of the device-frame viewport (and specify
     that in the device-frame coordinates we use, it's a unit square).
     viewport_{x,y}size are set from the PAGESIZE Plotter parameter, via
     the xsize and ysize options, and either or both may be negative.  If
     they are, we flipped the NDC_frame->device_frame map to compensate
     (see s_defplot.c).  Which is why we can take absolute values here. */

  if (_plotter->data->page_data->metric)
    sprintf (svg_header->point, 
	     "<svg version=\"1.1\" baseProfile=\"full\" id=\"body\" width=\"%.5gcm\" height=\"%.5gcm\" ",
	     2.54 * FABS(_plotter->data->viewport_xsize),
	     2.54 * FABS(_plotter->data->viewport_ysize));
  else
    sprintf (svg_header->point, 
	     "<svg version=\"1.1\" baseProfile=\"full\" id=\"body\" width=\"%.5gin\" height=\"%.5gin\" ",
	     FABS(_plotter->data->viewport_xsize),
	     FABS(_plotter->data->viewport_ysize));
  _update_buffer (svg_header);
  sprintf (svg_header->point, 
	   "%s %s %s %s %s>\n",
	   "viewBox=\"0 0 1 1\"",
	   "preserveAspectRatio=\"none\"",
	   /* bind SVG namespace */
	   "xmlns=\"http://www.w3.org/2000/svg\"",
	   /* bind XLink and XML Events namespaces for good measure */
	   "xmlns:xlink=\"http://www.w3.org/1999/xlink\"",
	   "xmlns:ev=\"http://www.w3.org/2001/xml-events\"");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "<title>SVG drawing</title>\n");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "<desc>This was produced by version %s of GNU libplot, a free library for exporting 2-D vector graphics.</desc>\n", 
	   PL_LIBPLOT_VER_STRING);
  _update_buffer (svg_header);

  if (_plotter->s_bgcolor_suppressed == false)
  /* place a background rectangle behind, covering entire viewport */
    {
      char color_buf[8];	/* enough room for "#ffffff", incl. NUL */

      sprintf (svg_header->point, 
	       "<rect id=\"background\" x=\"0\" y=\"0\" width=\"1\" height=\"1\" stroke=\"none\" fill=\"%s\"/>\n",
	       _libplot_color_to_svg_color (_plotter->s_bgcolor, color_buf));
      _update_buffer (svg_header);
    }

  /* enclose everything else in a container */
  sprintf (svg_header->point, "<g id=\"content\" ");
  _update_buffer (svg_header);
      
  if (_plotter->s_matrix_is_unknown == false
      && _plotter->s_matrix_is_bogus == false)
    /* Place a transform in the container: this page's default
       transformation matrix, which is simply the transformation matrix
       attribute of the very first graphical object plotted on the page.

       In libplot, `transformation matrix attribute' refers to the affine
       map from user space to NDC space.  So we're careful to multiply by
       `m_ndc_to_device', which transforms NDC space to device space.
       Because SVG uses a flipped-y convention, `m_ndc_to_device' flips the
       y coordinate.  (There will be additional flipping if the
       user-specified xsize, ysize are negative; see s_defplot.c.  Also, if
       the ROTATION Plotter parameter is specified by the user, it may
       rotate.) */
    {
      double product[6];

      _matrix_product (_plotter->s_matrix, _plotter->data->m_ndc_to_device,
		       product);
      write_svg_transform (svg_header, product);
    }

  /* turn off SVG's default [unfortunate] XML-inherited treatment of spaces */
  sprintf (svg_header->point, "xml:space=\"preserve\" ");
  _update_buffer (svg_header);

  /* specify style properties (all libplot defaults) */

  sprintf (svg_header->point, "stroke=\"%s\" ",
	   "black");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-linecap=\"%s\" ",
	   "butt");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-linejoin=\"%s\" ",
	   "miter");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-miterlimit=\"%.5g\" ",
	   PL_DEFAULT_MITER_LIMIT);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-dasharray=\"%s\" ",
	   "none");
  _update_buffer (svg_header);

  /* should use `px' here to specify user units, per the SVG Authoring
     Guide, but ImageMagick objects to that */
  sprintf (svg_header->point, "stroke-dashoffset=\"%.5g\" ",
	   0.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "stroke-opacity=\"%.5g\" ",
	   1.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill=\"%s\" ",
	   "none");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill-rule=\"%s\" ",
	   "evenodd");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "fill-opacity=\"%.5g\" ",
	   1.0);
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-style=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-variant=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-weight=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-stretch=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "font-size-adjust=\"%s\" ",
	   "none");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "letter-spacing=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "word-spacing=\"%s\" ",
	   "normal");
  _update_buffer (svg_header);

  sprintf (svg_header->point, "text-anchor=\"%s\"",
	   "start");
  _update_buffer (svg_header);

  sprintf (svg_header->point, ">\n");
  _update_buffer (svg_header);

  /* place SVG header in this page's plOutbuf */
  _plotter->data->page->header = svg_header;

  /* prepare SVG trailer too, write it to a plOutbuf */
  svg_trailer = _new_outbuf ();
  
  sprintf (svg_trailer->point, "</g>\n");
  _update_buffer (svg_trailer);

  sprintf (svg_trailer->point, "</svg>\n");
  _update_buffer (svg_trailer);
  
  /* place SVG trailer in this page's plOutbuf */
  _plotter->data->page->trailer = svg_trailer;

  return true;
}

/* This function is invoked while writing any graphical object on a page to
   the page's output buffer.  It emits the string "transform=\"...\" ",
   where the "\"...\"" is computed from a transformation matrix attribute
   of the object, which is passed.  I.e., it transforms a per-object
   transformation matrix to an SVG-style transformation matrix, and emits
   the latter as an SVG element attribute.  The per-object transformation
   matrix is always the identity, except for rotated text strings and
   ellipses.

   This code evaluates the SVG transformation matrix as the composition of
   two transformations: the local transformation, which acts first (in user
   space), which is passed as an argument; and a 2nd transformation, which
   is the current transformation from user to NDC coordinates.  Typically,
   it's the 1st which this code emits as the value of the `transform'
   attribute.  That's because when this is called for the first time on a
   page (or newly erased page), the 2nd is stored in `s_matrix', the global
   transformation matrix for the page, which will later be written at the
   head of the SVG code for the page when closepl() is invoked (see above).

   This separation of the two distinct transformations will of course work
   only if the 2nd doesn't change from object to object on the page.  For
   this reason, what's actually emitted as the value of the SVG transform
   attribute is a composite transformation, made up in succession of

   (1) the passed per-object transformation
   (2) the current value of the transformation from user to NDC coordinates
   (3) the inverse of s_matrix.

   If the user space -> NDC space map is the same for all objects on the
   page, then (2) and (3) will cancel each other out for all objects on
   the page.

   Note that in this code we flag `s_matrix' as bogus if it's singular.  If
   it's bogus, it won't be written out when closepl() is invoked, and the
   global transformation matrix of the page will effectively be the
   identity (i.e., we'll punt). */

void
_pl_s_set_matrix (R___(Plotter *_plotter) const double m_local[6])
{
  double m_base[6], m[6];
  const double *m_emitted = (const double *)NULL; /* keep compiler happy */
  bool need_transform_attribute = false;
  int i;
  
  for (i = 0; i < 6; i++)
    m_base[i] = _plotter->drawstate->transform.m_user_to_ndc[i];

  /* if this is the first time this function is invoked on a page (or newly
     erased page), store the current user-to-NDC matrix for later use as
     the global transformation matrix for the page */
  if (_plotter->s_matrix_is_unknown)
    {
      for (i = 0; i < 6; i++)
	_plotter->s_matrix[i] = m_base[i];

      _plotter->s_matrix_is_unknown = false;

      if (m_base[0] * m_base[3] - m_base[1] * m_base[2] == 0.0) 
	/* singular, won't be used even though stored */
	_plotter->s_matrix_is_bogus = true;
    }

  /* compute product: current transformation matrix (in the transformation
     from user to NDC coors, local acts first, then base)  */
  _matrix_product (m_local, m_base, m);

  /* determine whether current matrix is different from the global one that
     will be wrapped around the entire page (if there is one) */

  if (_plotter->s_matrix_is_bogus == false)
    /* have a global page-specific transformation matrix that will be
       applied, so object's transform attribute may need to compensate */
    {
      for (i = 0; i < 6; i++)
	{
	  if (m[i] != _plotter->s_matrix[i])
	    /* different, so need to compensate */
	    {
	      need_transform_attribute = true;
	      break;
	    }
	}

      if (need_transform_attribute)
	{
	  double inverse_of_global[6], product[6];

	  _matrix_inverse (_plotter->s_matrix, inverse_of_global);

	  /* emitted transform attribute of object will be a product of
	     three matrices: (1) the passed matrix, (2) the current
	     user-to-NDC transformation matrix, and (3) the inverse of the
	     global transformation matrix */
	  _matrix_product (m, inverse_of_global, product);
	  m_emitted = product;
	}
    }
  else
    /* no global transformation matrix for this page (no doubt because of
       the abovementioned non-invertibility problem), so object's transform
       attribute will simply be the current matrix */
    {
      need_transform_attribute = true;
      m_emitted = m;
    }
  
  /* emit object's transform attribute if it's not the identity */
  if (need_transform_attribute)
    write_svg_transform (_plotter->data->page, m_emitted);
}

/* Internal function for writing out a PS-style affine transformation as a
   SVG-style affine transformation.  If matrix is the identity, nothing is
   written.  

   In SVG format, the value of the `transform' attribute is a sequence of
   transformations such as `rotate', `scale', and `translate', where the
   sequence (as a composite transformation from user space to device [NDC]
   space) is read from right to left.  This is the opposite of the PS
   convention.  SVG documentation uses column vectors, while PS
   documentation uses row vectors.

   Presumably the SVG convention arose from a desire to make the
   `nestedness' of the transform attribute, implemented as the computation
   of a composite transformation, more intuitive. */

static void
write_svg_transform (plOutbuf *outbuf, const double m[6])
{
  double mm[6];
  double max_value = 0.0;
  int i;
  int type = 0;			/* default */
      
  /* compensate for possible roundoff error: treat very small elements of
     linear transformation (if any) as zero */
#define VERY_SMALL_FACTOR 1e-10
  
  for (i = 0; i < 4; i++)
    max_value = DMAX(max_value, FABS(m[i]));
  for (i = 0; i < 6; i++)
    if (i < 4 && FABS(m[i]) < VERY_SMALL_FACTOR * max_value)
      mm[i] = 0;
    else
      mm[i] = m[i];

  if (mm[0] == 1.0 && mm[1] == 0.0 && mm[2] == 0.0 && mm[3] == 1.0
      && mm[4] == 0.0 && mm[5] == 0.0)
    /* identity matrix, unnecessary to write it */
    return;

  /* treat several types of affine transformation specially */

  if (mm[1] == 0.0 && mm[2] == 0.0)
    type = 1;			/* scale + translation */

  else if (mm[0] == 0.0 && mm[1] == 1.0 && mm[2] == -1.0 && mm[3] == 0.0)
    type = 2;			/* rotation by 90 + translation */
  else if (mm[0] == 0.0 && mm[1] == -1.0 && mm[2] == 1.0 && mm[3] == 0.0)
    type = 3;			/* rotation by 270 + translation */
  else if (mm[0] == 0.0 && mm[1] == 1.0 && mm[2] == 1.0 && mm[3] == 0.0)
    type = 4;			/* y-flip + rotation by 90 + translation */
  else if (mm[0] == 0.0 && mm[1] == -1.0 && mm[2] == -1.0 && mm[3] == 0.0)
    type = 5;			/* y-flip + rotation by 270 + translation */
  
  sprintf (outbuf->point, "transform=\"");
  _update_buffer (outbuf);
      
  if (type != 0)
    {
      /* emit translation if any (SVG will perform it last, since SVG uses
	 opposite order from PS for multiplying matrices) */
      if (mm[4] != 0.0 || mm[5] != 0.0)
	{
	  if (mm[5] == 0.0)
	    sprintf (outbuf->point, "translate(%.5g) ",
		     mm[4]);
	  else
	    sprintf (outbuf->point, "translate(%.5g,%.5g) ",
		     mm[4], mm[5]);
	  _update_buffer (outbuf);	      
	}

      switch (type)
	{
	case 1:
	  if (mm[0] != 1.0 || mm[3] != 1.0)
	    {
	      if (mm[3] == mm[0])
		sprintf (outbuf->point, "scale(%.5g) ",
			 mm[0]);
	      else if (mm[3] == -mm[0])
		{
		  if (mm[0] != 1.0)
		    sprintf (outbuf->point, "scale(1,-1) scale(%.5g) ",
			     mm[0]);
		  else
		    sprintf (outbuf->point, "scale(1,-1) ");
		}
	      else
		sprintf (outbuf->point, "scale(%.5g,%.5g) ",
			 mm[0], mm[3]);
	      _update_buffer (outbuf);	      
	    }
	  break;

	case 2:
	  sprintf (outbuf->point, "rotate(90) ");
	  _update_buffer (outbuf);	      
	  break;

	case 3:
	  sprintf (outbuf->point, "rotate(270) ");
	  _update_buffer (outbuf);	      
	  break;

	case 4:
	  sprintf (outbuf->point, "rotate(90) scale(1,-1) ");
	  _update_buffer (outbuf);	      
	  break;

	case 5:
	  sprintf (outbuf->point, "rotate(270) scale(1,-1) ");
	  _update_buffer (outbuf);	      
	  break;

	default:		/* shouldn't happen */
	  break;
	}
    }
  else
    /* general affine transformation */
    {
      sprintf (outbuf->point, "matrix(%.5g %.5g %.5g %.5g %.5g %.5g) ",
	       mm[0], mm[1], mm[2], mm[3], mm[4], mm[5]);
      _update_buffer (outbuf);
    }

  sprintf (outbuf->point, "\" ");
  _update_buffer (outbuf);
}
