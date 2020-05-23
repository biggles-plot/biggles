/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, 2008, Free Software
 * Foundation, Inc.
 */

/*
 * Adams-Moulton with constant step size
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

#define PASTVAL        (3)	/* previous values, val[0] is current value */

void
am (void)
{
  double t;
  double halfstep = HALF * tstep;
  double sconst = tstep / 24.0; /* step constant */
  double onesixth = 1.0 / 6.0;

  /* Runge-Kutta startup */
  for (it = 0, t = tstart; it <= PASTVAL && !STOPR; t = tstart + (++it) * tstep) 
    {
      symtab->sy_value = symtab->sy_val[0] = t;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  int j;

	  for (j = it; j > 0; j--) 
	    {
	      fsp->sy_val[j] = fsp->sy_val[j-1];
	      fsp->sy_pri[j] = fsp->sy_pri[j-1];
	    }
	  fsp->sy_pri[0] = fsp->sy_prime;
	  fsp->sy_val[0] = fsp->sy_value;
	}
      /* output */
      printq();
      if (it == PASTVAL)
	break;  /* startup complete */
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)   
	{
	  fsp->sy_k[0] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] + HALF * fsp->sy_k[0];
	}
      symtab->sy_value = t + halfstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)   
	{
	  fsp->sy_k[1] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] + HALF * fsp->sy_k[1];
	}
      symtab->sy_value = t + halfstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)   
	{
	  fsp->sy_k[2] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] + fsp->sy_k[2];
	}
      symtab->sy_value = t + tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
	fsp->sy_k[3] = tstep * fsp->sy_prime;
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_value = fsp->sy_val[0] 
	    + onesixth * (fsp->sy_k[0]
			  + TWO * fsp->sy_k[1]
			  + TWO * fsp->sy_k[2]
			  + fsp->sy_k[3]);
	}
    }

  /* predictor - corrector */
  while (!STOPA)
    {
      /* Adams-Bashforth predictor */
      for (fsp = dqueue; fsp != NULL ; fsp = fsp->sy_link) 
	{
	  fsp->sy_value = fsp->sy_val[0] 
	    + (sconst) * (55 * fsp->sy_pri[0]
			  -59 * fsp->sy_pri[1]
			  +37 * fsp->sy_pri[2]
			  -9  * fsp->sy_pri[3]);
	}
      symtab->sy_val[0] = symtab->sy_value =  t = tstart + (++it) * tstep;
      field();

      /* Adams-Moulton corrector */
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_value = fsp->sy_val[0] 
	    + (sconst) * (9  * fsp->sy_prime
			  +19 * fsp->sy_pri[0]
			  -5  * fsp->sy_pri[1]
			  +   fsp->sy_pri[2]);
	}
      field();

      /* cycle indices */
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  int j;

	  for (j = PASTVAL; j > 0; j--) 
	    {
	      fsp->sy_val[j] = fsp->sy_val[j-1];
	      fsp->sy_pri[j] = fsp->sy_pri[j-1];
	    }
	  fsp->sy_val[0] = fsp->sy_value;
	  fsp->sy_pri[0] = fsp->sy_prime;
	}

      /* output */
      printq();
    }
}
