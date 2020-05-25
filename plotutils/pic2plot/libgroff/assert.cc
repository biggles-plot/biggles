/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include <stdio.h>
#include <stdlib.h>

extern const char *program_name;

void 
assertion_failed (int lineno, const char *filename)
{
  if (program_name != 0)
    fprintf(stderr, "%s: ", program_name);
  fprintf(stderr, "Failed assertion at line %d, file `%s'.\n",
	  lineno, filename);
  fflush(stderr);
  abort();
}
