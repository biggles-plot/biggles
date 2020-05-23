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

/* Wrappers for standard storage allocation functions, for
   libplot/libplotter with the exception of the libxmi scan conversion
   module, which has its own more complicated versions (see mi_alloc.c). */

#include "sys-defines.h"
#include "extern.h"

/* wrapper for malloc() */
void * 
_pl_xmalloc (size_t size)
{
  void * p;

  p = (void *) malloc (size);
  if (p == (void *)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }

#ifdef DEBUG_MALLOC
  fprintf (stderr, "malloc (%d) = %p\n", size, p);
#endif

  return p;
}

/* wrapper for calloc() */
void * 
_pl_xcalloc (size_t nmemb, size_t size)
{
  void * p;

  p = (void *) calloc (nmemb, size);
  if (p == (void *)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }

#ifdef DEBUG_MALLOC
  fprintf (stderr, "calloc (%d, %d) = %p\n", nmemb, size, p);
#endif

  return p;
}

/* wrapper for realloc() */
void * 
_pl_xrealloc (void * p, size_t size)
{
  void * q;

  q = (void *) realloc (p, size);
  if (q == (void *)NULL)
    {
      fputs ("libplot: ", stderr);
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }

#ifdef DEBUG_MALLOC
  fprintf (stderr, "realloc (%p, %d) = %p\n", p, size, q);
#endif

  return q;
}
