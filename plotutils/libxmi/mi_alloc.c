/* This file is part of the GNU libxmi package.  Copyright (C) 1998, 1999,
   2000, 2005, Free Software Foundation, Inc.

   The GNU libxmi package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU libxmi package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* Wrappers for standard storage allocation functions.  The tests for zero
   size, etc., are necessitated by the way in which the original X11
   scan-conversion code was written. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"

/* wrapper for malloc() */
void * 
mi_xmalloc (size_t size)
{
  void * p;

  if (size == 0)
    return (void *)NULL;

  p = (void *) malloc (size);
  if (p == (void *)NULL)
    {
      fprintf (stderr, "libxmi: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for calloc() */
void * 
mi_xcalloc (size_t nmemb, size_t size)
{
  void * p;

  if (size == 0)
    return (void *)NULL;

  p = (void *) calloc (nmemb, size);
  if (p == (void *)NULL)
    {
      fprintf (stderr, "libxmi: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for realloc() */
void * 
mi_xrealloc (void * p, size_t size)
{
  if (!p)
    return mi_xmalloc (size);
  else
    {
      if (size == 0)
	{
	  free (p);
	  return (void *)NULL;
	}
      
      p = (void *) realloc (p, size);
      if (p == (void *)NULL)
	{
	  fprintf (stderr, "libxmi: ");
	  perror ("out of memory");
	  exit (EXIT_FAILURE);
	}
      return p;
    }
}
