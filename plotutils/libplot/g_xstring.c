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

/* Replacement for system strcasecmp() if it doesn't exist. */

#include "sys-defines.h"
#include "extern.h"

#ifndef HAVE_STRCASECMP
int
strcasecmp(const char *s1, const char *s2)
{
  bool retval_set = false;
  int retval = 0;
  char *t1, *t2, *t1_base, *t2_base;
  
  t1 = t1_base = (char *)_pl_xmalloc (strlen (s1) + 1);
  t2 = t2_base = (char *)_pl_xmalloc (strlen (s2) + 1);
  strcpy (t1, s1);
  strcpy (t2, s2);  

  while (*t1 && *t2)
    {
      unsigned int c1 = tolower ((int)(unsigned char)*t1);
      unsigned int c2 = tolower ((int)(unsigned char)*t2);
      
      if (c1 > c2)
	{
	  retval = 1;
	  retval_set = true;
	  break;
	}
      else if (c1 < c2)
	{
	  retval = -1;
	  retval_set = true;
	  break;
	}
      else
	{
	  t1++; 
	  t2++;
	}
    }      
  
  if (!retval_set)
    {
      if (*t1)
	retval = 1;
      else if (*t2)
	retval = -1;
      else
	retval = 0;
    }
  
  free (t1_base);
  free (t2_base);
  
  return retval;
}
#endif /* not HAVE_STRCASECMP */
