/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free
 * Software Foundation, Inc.
 */

/*
 * find maximum errors
 *
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

static double   ssemax, abemax, acemax;
static char     *ssenam, *abenam, *acenam;

void
maxerr (void)
{
  struct sym *sp, *dq;
  
  dq = symtab->sy_link;
  ssemax = abemax = acemax = 0.0;
  for (sp = dq; sp != NULL; sp = sp->sy_link) 
    {
      if (ssemax < sp->sy_sserr) 
	{
	  ssemax = sp->sy_sserr;
	  ssenam = sp->sy_name;
	}
      if (abemax < sp->sy_aberr) 
	{
	  abemax = sp->sy_aberr;
	  abenam = sp->sy_name;
	}
      if (acmax < sp->sy_acerr) 
	{
	  acemax = sp->sy_acerr;
	  acenam = sp->sy_name;
	}
    }
}

bool
hierror (void) /* not enough accuracy */
{
  double t = symtab->sy_val[0];

  if (t + tstep == t) 
    {
      fprintf (stderr, "%s: %s\n", progname, "step size below lower limit");
      longjmp (mark, 1);
    }
  if (ssemax <= ssmax && abemax <= abmax && acemax <= acmax)
    return false;
  if (fabs(tstep) >= fabs(hmin))
    return true;
  if (sflag)
    return false;
  if (ssemax > ssmax)
    fprintf (stderr, 
	     "%s: relative error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, ssenam);
  else if (abemax > abmax)
    fprintf (stderr, 
	     "%s: absolute error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, abenam);
  else if (acemax > acmax)
    fprintf (stderr, 
	     "%s: accumulated error limit exceeded while calculating %.*s'\n",
	     progname, NAMMAX, acenam);
  longjmp (mark, 1);

  /* doesn't return, but must keep unintelligent compilers happy */
  return false;
}

bool
lowerror (void) /* more than enough accuracy */
{
  if (ssemax < ssmin || abemax < abmin)
    if (fabs(tstep) <= fabs(hmax))
      return true;
  return false;
}

/*
 * interpolate to tstop in Runge-Kutta routines
 */
#define PASTSTOP(stepvar) (t + 0.9375*stepvar > tstop && \
                                t + 0.0625*stepvar < tstop)
#define BEFORESTOP(stepvar) (t + 0.9375*stepvar < tstop && \
                                t + 0.0625*stepvar > tstop)

bool
intpr (double t)
{
  if (tstep > 0)
    if (!PASTSTOP(tstep))
      return false;
  if (tstep < 0)
    if (!BEFORESTOP(tstep))
      return false;
  if (tstep > 0)
    while (PASTSTOP(tstep))
      tstep = HALF * tstep;
  if (tstep < 0)
    while (BEFORESTOP(tstep))
      tstep = HALF * tstep;
  return true;
}
