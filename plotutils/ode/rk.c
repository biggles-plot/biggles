/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, Free Software
 * Foundation, Inc.
 */

/*
 * Fourth-Order Runge-Kutta
 *
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

void
rk (void)
{
  double t;
  double halfstep = HALF * tstep;
  double onesixth = 1.0 / 6.0;

  for (it = 0, t = tstart; !STOPR; t = tstart + (++it) * tstep) 
    {
      symtab->sy_val[0] = symtab->sy_value = t;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_val[0] = fsp->sy_value;
	  fsp->sy_pri[0] = fsp->sy_prime;
	}
      /* output */
      printq();
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
	fsp->sy_value = fsp->sy_val[0] 
	  + onesixth * (fsp->sy_k[0]
			+ TWO * fsp->sy_k[1]
			+ TWO * fsp->sy_k[2]
			+ fsp->sy_k[3]);
    }
}
