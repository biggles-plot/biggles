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

/* This file contains a utility function, _set_page_type(), which searches
   the database of known pagetypes for the one specified in the PAGESIZE
   parameter, sets page-related data (dimensions, size of viewport) in the
   Plotter, and also passes back the viewport offset vector.

   The PAGESIZE parameter should be of a form resembling "letter", or "a4",
   or "letter,xoffset=-1.2in", or "a4,yoffset=0.5cm,xoffset = 2mm". */

#include "sys-defines.h"
#include "extern.h"
#include "g_pagetype.h"

/* forward references */
static bool parse_page_type (const char *pagesize, const plPageData **pagedata, double *xoffset, double *yoffset, double *xorigin, double *yorigin, double *xsize, double *ysize);
static bool string_to_inches (const char *string, double *inches);

void
_set_page_type (plPlotterData *data)
{
  const char *pagesize;
  const plPageData *pagedata;
  double viewport_xoffset, viewport_yoffset;
  double viewport_xorigin, viewport_yorigin;
  double viewport_xsize, viewport_ysize;
  
  /* examine user-specified value for PAGESIZE parameter, or the default
     value if we can't parse the user-specified value */
  pagesize = (const char *)_get_plot_param (data, "PAGESIZE");
  if (!parse_page_type (pagesize, &pagedata, 
			 &viewport_xoffset, &viewport_yoffset,
			 &viewport_xorigin, &viewport_yorigin,
			 &viewport_xsize, &viewport_ysize))
    {
      pagesize = (const char *)_get_default_plot_param ("PAGESIZE");
      parse_page_type (pagesize, &pagedata, 
			&viewport_xoffset, &viewport_yoffset,
			&viewport_xorigin, &viewport_yorigin,
			&viewport_xsize, &viewport_ysize);
    }

  /* set page data in Plotter */
  data->page_data = pagedata;
  data->viewport_xoffset = viewport_xoffset;
  data->viewport_yoffset = viewport_yoffset;
  data->viewport_xorigin = viewport_xorigin;
  data->viewport_yorigin = viewport_yorigin;
  data->viewport_xsize = viewport_xsize;
  data->viewport_ysize = viewport_ysize;
}

static bool
parse_page_type (const char *pagesize, const plPageData **pagedata, double *xoffset, double *yoffset, double *xorigin, double *yorigin, double *xsize, double *ysize)
{
  const plPageData *local_pagedata = _pagedata;
  char *viewport_pagesize, *first, *next;
  char xoffset_s[32], yoffset_s[32]; /* each field should have length <=31 */
  char xorigin_s[32], yorigin_s[32];
  char xsize_s[32], ysize_s[32];
  bool anotherfield, success;
  bool got_xoffset = false, got_yoffset = false;
  bool got_xorigin = false, got_yorigin = false;
  bool got_xsize = false, got_ysize = false;
  int i;

  viewport_pagesize = (char *)_pl_xmalloc (strlen (pagesize) + 1);  
  strcpy (viewport_pagesize, pagesize);
  first = viewport_pagesize;

  next = strchr (viewport_pagesize, (int)',');
  if (next)
    {
      anotherfield = true;
      *next = '\0';
      next++;
    }
  else
    anotherfield = false;

  /* try to match page type to a page type on our list */
  success = false;

  for (i = 0; i < PL_NUM_PAGESIZES; i++, local_pagedata++)
    if (strcasecmp (local_pagedata->name, viewport_pagesize) == 0
	|| 
	(local_pagedata->alt_name 
	 && strcasecmp (local_pagedata->alt_name, viewport_pagesize) == 0))
      {
	success = true;
	break;
      }

  if (success)
    /* matched page type, at least */
    {
      /* pass back pointer to page data via pointer */
      *pagedata = local_pagedata;

      while (anotherfield && *next) /* i.e. while there's a nonempty field */
	{
	  first = next;
	  next = strchr (next, (int)',');
	  if (next)
	    {
	      anotherfield = true;
	      *next = '\0';
	      next++;
	    }
	  else
	    anotherfield = false;

	  /* try to parse field */
	  if (sscanf (first, "xoffset = %31s", xoffset_s) == 1)
	    got_xoffset = true;
	  else if (sscanf (first, "yoffset = %31s", yoffset_s) == 1)
	    got_yoffset = true;	      
	  else if (sscanf (first, "xorigin = %31s", xorigin_s) == 1)
	    got_xorigin = true;	      
	  else if (sscanf (first, "yorigin = %31s", yorigin_s) == 1)
	    got_yorigin = true;	      
	  else if (sscanf (first, "xsize = %31s", xsize_s) == 1)
	    got_xsize = true;	      
	  else if (sscanf (first, "ysize = %31s", ysize_s) == 1)
	    got_ysize = true;	      
	}
      
      /* pass back viewport size-and-location data via pointers */
      {
	double viewport_xsize, viewport_ysize;
	double viewport_xorigin, viewport_yorigin;
	double viewport_xoffset, viewport_yoffset;

	/* xsize, ysize default to this page type's default */
	if (!(got_xsize && string_to_inches (xsize_s, &viewport_xsize)))
	  viewport_xsize = local_pagedata->default_viewport_size;
	if (!(got_ysize && string_to_inches (ysize_s, &viewport_ysize)))
	  viewport_ysize = local_pagedata->default_viewport_size;

	/* xorigin, yorigin default to whatever is needed to center the
	   viewport on the page */
	if (!(got_xorigin && string_to_inches (xorigin_s, &viewport_xorigin)))
	  viewport_xorigin = 0.5 * (local_pagedata->xsize - viewport_xsize);
	if (!(got_yorigin && string_to_inches (yorigin_s, &viewport_yorigin)))
	  viewport_yorigin = 0.5 * (local_pagedata->ysize - viewport_ysize);

	/* xoffset, yoffset default to zero */
	if (!(got_xoffset && string_to_inches (xoffset_s, &viewport_xoffset)))
	  viewport_xoffset = 0.0;
	if (!(got_yoffset && string_to_inches (yoffset_s, &viewport_yoffset)))
	  viewport_yoffset = 0.0;

	*xsize = viewport_xsize;
	*ysize = viewport_ysize;
	*xorigin = viewport_xorigin;
	*yorigin = viewport_yorigin;
	*xoffset = viewport_xoffset;
	*yoffset = viewport_yoffset;
      }
    }

  free (viewport_pagesize);

  /* indicate whether we were able to match the page type */
  return success;
}

/* convert a string representing a distance measurement to inches; units
   `in', `cm', `mm' are supported */

static bool 
string_to_inches (const char *string, double *inches)
{
  double val;
  char s[4];
  
  if (sscanf (string, "%lf %3s" , &val, s) == 2)
    {
      if (strlen (s) > 2)
	return false;
      if (strcmp (s, "in") == 0)
	{
	  *inches = val;
	  return true;
	}
      else if (strcmp (s, "cm") == 0)
	{
	  *inches = val / 2.54;
	  return true;
	}
      else if (strcmp (s, "mm") == 0)      
	{
	  *inches = val / 25.4;
	  return true;
	}
    }
    return false;
}
