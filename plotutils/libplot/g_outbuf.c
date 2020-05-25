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

/* This file contains routines for creating, manipulating, and deleting a
   special sort of output buffer: a plOutbuf.  They are invoked by drawing
   methods for Plotters that do not do real-time output.  Such Plotters
   store the device code for each page of graphics in a plOutbuf.  Along
   with a page of device code, a plOutbuf optionally stores bounding box
   information for the page.

   plOutbufs are resized when they are too full.  The strange resizing
   method (_UPDATE_BUFFER) is needed because on many systems, sprintf()
   does not return the number of characters it writes.  _UPDATE_BUFFER must
   be called after each call to sprintf(); it is not invoked automatically.
   
   Output buffers of this sort are a bit of a kludge.  They may eventually
   be replaced or supplemented by an in-core object hierarchy, which
   deletepl() will scan.

   When erase() is invoked on the Plotter, _RESET_OUTBUF is called, to
   remove all graphics from the plOutbuf.  There is provision for keeping a
   section of initialization code in the plOutbuf, untouched.  This is
   arranged by a previous call to _FREEZE_OUTBUF.  Anything in the plOutbuf
   at that time will be untouched by a later call to _RESET_OUTBUF. */

#include "sys-defines.h"
#include "extern.h"

/* Initial length for a plOutbuf (should be large enough to handle any
   single one of our sprintf's or strcpy's [including final NUL], or other
   write operations, without overflow).  Note: in p_defplot.c we write long
   blocks of Postscript initialization code (see p_header.h) into a
   plOutbuf, so this should be quite large.  We may also write large
   substrings into a plOutbuf in c_emit.c. */
#define INITIAL_OUTBUF_LEN 8192

/* New (larger) length of a plOutbuf, as function of the old; used when
   reallocating due to exhaustion of storage. */
#define NEW_OUTBUF_LEN(old_outbuf_len) ((old_outbuf_len) < 10000000 ? 2 * (old_outbuf_len) : (old_outbuf_len) + 10000000)

plOutbuf *
_new_outbuf (void)
{
  plOutbuf *bufp;

  bufp = (plOutbuf *)_pl_xmalloc(sizeof(plOutbuf));
  bufp->header = (plOutbuf *)NULL;
  bufp->trailer = (plOutbuf *)NULL;
  bufp->base = (char *)_pl_xmalloc(INITIAL_OUTBUF_LEN * sizeof(char));
  bufp->len = (unsigned long)INITIAL_OUTBUF_LEN;
  bufp->next = NULL;
  bufp->reset_point = bufp->base;
  bufp->reset_contents = (unsigned long)0L;
  _reset_outbuf (bufp);

  return bufp;
}

void
_reset_outbuf (plOutbuf *bufp)
{
  int i;

  *(bufp->reset_point) = '\0';
  bufp->point = bufp->reset_point;
  bufp->contents = bufp->reset_contents;

  /* also initialize elements used by some drivers */

  /* initialize bounding box to an empty (self-contradictory) box */
  bufp->xrange_min = DBL_MAX;
  bufp->xrange_max = -(DBL_MAX);
  bufp->yrange_min = DBL_MAX;
  bufp->yrange_max = -(DBL_MAX);

  /* initialize `font used' arrays for the page */
  for (i = 0; i < PL_NUM_PS_FONTS; i++)
    bufp->ps_font_used[i] = false;
  for (i = 0; i < PL_NUM_PCL_FONTS; i++)
    bufp->pcl_font_used[i] = false;
}

void
_freeze_outbuf (plOutbuf *bufp)
{
  bufp->reset_point = bufp->point;
  bufp->reset_contents = bufp->contents;
}

void
_delete_outbuf (plOutbuf *bufp)
{
  if (bufp)
    {
      free (bufp->base);
      free (bufp);
    }
}

/* UPDATE_BUFFER is called manually, after each sprintf() and other object
   write operation.  It assumes that the buffer is always a null-terminated
   string, so that strlen() can be used, to determine how many additional
   characters were added.  */

void
_update_buffer (plOutbuf *bufp)
{
  int additional;

  /* determine how many add'l chars were added */
  additional = strlen (bufp->point);
  bufp->point += additional;
  bufp->contents += additional;
  
  if (bufp->contents + 1 > bufp->len) /* need room for NUL */
    /* shouldn't happen! */
    {
      fprintf (stderr, "libplot: output buffer overrun\n");
      exit (EXIT_FAILURE);
    }
  if (bufp->contents > (bufp->len >> 1))
    /* expand buffer */
    {
      unsigned long oldlen, newlen;

      oldlen = bufp->len;
      newlen = NEW_OUTBUF_LEN(oldlen);

      bufp->base = 
	(char *)_pl_xrealloc (bufp->base, newlen * sizeof(char));
      bufp->len = newlen;
      bufp->point = bufp->base + bufp->contents;
      bufp->reset_point = bufp->base + bufp->reset_contents;
    }      
}

/* A variant of _UPDATE_BUFFER in which the caller specifies how many bytes
   have been added.  Used in cases when the buffer contains something other
   than a null-terminated string (e.g., raw binary bytes). */

void
_update_buffer_by_added_bytes (plOutbuf *bufp, int additional)
{
  bufp->point += additional;
  bufp->contents += additional;
  
  if (bufp->contents + 1 > bufp->len) /* need room for NUL */
    /* shouldn't happen! */
    {
      fprintf (stderr, "libplot: output buffer overrun\n");
      exit (EXIT_FAILURE);
    }
  if (bufp->contents > (bufp->len >> 1))
    /* expand buffer */
    {
      unsigned long oldlen, newlen;

      oldlen = bufp->len;
      newlen = NEW_OUTBUF_LEN(oldlen);

      bufp->base = 
	(char *)_pl_xrealloc (bufp->base, newlen * sizeof(char));
      bufp->len = newlen;
      bufp->point = bufp->base + bufp->contents;
      bufp->reset_point = bufp->base + bufp->reset_contents;
    }      
}

/* update bounding box information for a plOutbuf, to take account of a
   point being plotted on the associated page */
void 
_update_bbox (plOutbuf *bufp, double x, double y)
{
  if (x > bufp->xrange_max) bufp->xrange_max = x;
  if (x < bufp->xrange_min) bufp->xrange_min = x;
  if (y > bufp->yrange_max) bufp->yrange_max = y;
  if (y < bufp->yrange_min) bufp->yrange_min = y;
}

/* return bounding box information for a plOutbuf */
void 
_bbox_of_outbuf (plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
{
  double page_x_min = DBL_MAX;
  double page_y_min = DBL_MAX;  
  double page_x_max = -(DBL_MAX);
  double page_y_max = -(DBL_MAX);  

  if (bufp)
    {
      page_x_max = bufp->xrange_max;
      page_x_min = bufp->xrange_min;
      page_y_max = bufp->yrange_max;
      page_y_min = bufp->yrange_min;
    }

  *xmin = page_x_min;
  *ymin = page_y_min;
  *xmax = page_x_max;
  *ymax = page_y_max;
}

/* compute bounding box information for a linked list of plOutbufs
   (i.e. pages), starting with a specified plOutbuf (i.e., page) */
void 
_bbox_of_outbufs (plOutbuf *bufp, double *xmin, double *xmax, double *ymin, double *ymax)
{
  double doc_x_min = DBL_MAX;
  double doc_y_min = DBL_MAX;  
  double doc_x_max = -(DBL_MAX);
  double doc_y_max = -(DBL_MAX);  
  double page_x_min, page_x_max, page_y_min, page_y_max;
  plOutbuf *page = bufp;

  while (page)
    {
      page_x_max = page->xrange_max;
      page_x_min = page->xrange_min;
      page_y_max = page->yrange_max;
      page_y_min = page->yrange_min;

      if (!((page_x_max < page_x_min || page_y_max < page_y_min)))
	/* nonempty page */
	{
	  if (page_x_max > doc_x_max) doc_x_max = page_x_max;
	  if (page_y_max > doc_y_max) doc_y_max = page_y_max;
	  if (page_x_min < doc_x_min) doc_x_min = page_x_min;
	  if (page_y_min < doc_y_min) doc_y_min = page_y_min;
	}
      page = page->next;
    }

  *xmin = doc_x_min;
  *ymin = doc_y_min;
  *xmax = doc_x_max;
  *ymax = doc_y_max;
}
