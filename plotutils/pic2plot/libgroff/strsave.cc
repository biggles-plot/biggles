// -*- C++ -*-
/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include <string.h>

char *
strsave(const char *s)
{
  if (s == 0)
    return 0;
  char *p = new char[strlen(s) + 1];
  strcpy(p, s);
  return p;
}

