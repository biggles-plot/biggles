// -*- C++ -*-
/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include <string.h>
#include <ctype.h>
#include "cset.h"
#include "stringclass.h"

extern void change_filename(const char *);
extern void change_lineno(int);

int
interpret_lf_args(const char *p)
{
  while (*p == ' ')
    p++;
  if (!csdigit(*p))
    return 0;
  int ln = 0;
  do 
    {
      ln *= 10;
      ln += *p++ - '0';
    } while (csdigit(*p));
  if (*p != ' ' && *p != '\n' && *p != '\0')
    return 0;
  while (*p == ' ')
    p++;
  if (*p == '\0' || *p == '\n')  
    {
      change_lineno(ln);
      return 1;
    }
  const char *q;
  for (q = p;
       *q != '\0' && *q != ' ' && *q != '\n' && *q != '\\';
       q++)
    ;
  string tem(p, q - p);
  while (*q == ' ')
    q++;
  if (*q != '\n' && *q != '\0')
    return 0;
  tem += '\0';
  change_filename(tem.contents());
  change_lineno(ln);

  return 1;
}
