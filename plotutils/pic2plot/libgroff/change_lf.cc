/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include <string.h>

extern char *strsave(const char *);

extern const char *current_filename;
extern int current_lineno;

void
change_filename(const char *f)
{
  if (current_filename != 0 && strcmp(current_filename, f) == 0)
    return;
  current_filename = strsave(f);
}

void
change_lineno(int ln)
{
  current_lineno = ln;
}
