/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include "ptable.h"
#include "errarg.h"
#include "error.h"

unsigned long
hash_string(const char *s)
{
  assert(s != 0);
  unsigned long h = 0, g;
  while (*s != 0) 
    {
      h <<= 4;
      h += *s++;
      if ((g = h & 0xf0000000) != 0) 
	{
	  h ^= g >> 24;
	  h ^= g;
	}
    }
  return h;
}

static const unsigned table_sizes[] = { 
101, 503, 1009, 2003, 3001, 4001, 5003, 10007, 20011, 40009,
80021, 160001, 500009, 1000003, 2000003, 4000037, 8000009,
16000057, 32000011, 64000031, 128000003, 0 
};

unsigned 
next_ptable_size(unsigned n)
{
  const unsigned *p;  

  for (p = table_sizes; *p <= n; p++)
    if (*p == 0)
      fatal("cannot expand table");
  return *p;
}
