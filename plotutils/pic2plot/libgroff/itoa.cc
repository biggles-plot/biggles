/* Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com) */

#include "lib.h"

#define INT_DIGITS 19		/* enough for 64 bit integer */

const char *
our_itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
  if (i >= 0) 
    {
      do 
	{
	  *--p = '0' + (i % 10);
	  i /= 10;
	} while (i != 0);
      return p;
    }
  else 
    {				/* i < 0 */
      do 
	{
	  *--p = '0' - (i % 10);
	  i /= 10;
	} while (i != 0);
      *--p = '-';
    }
  return p;
}
