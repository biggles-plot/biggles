/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, 2008, Free Software
 * Foundation, Inc.
 */

/*
 * Fifth-Order Runge-Kutta-Fehlberg with adaptive step size
 *
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include "num.h"

#define T_LT_TSTOP (tstep>0 ? t<tstop : t>tstop)

void
rka (void)
{
  bool gdval = true; 		/* good value to print ? */
  int overtime = 1;
  double prevstep = 0.0;
  double t;

  for (it = 0, t = tstart; T_LT_TSTOP || overtime--; ) 
    {
      symtab->sy_value = symtab->sy_val[0] = t;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_val[0] = fsp->sy_value;
	  fsp->sy_pri[0] = fsp->sy_prime;
	}
      if (gdval)
	printq();       /* output */
      if (tstep * (t+tstep-tstop) > 0)
	tstep = tstop - t;
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[0] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] 
	    + C20 * fsp->sy_k[0];
	}
      symtab->sy_value = t + C2t * tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[1] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] 
	    + (C30 * fsp->sy_k[0]
	       + C31 * fsp->sy_k[1]);
	}
      symtab->sy_value = t + C3t * tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[2] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] 
	    + (C40 * fsp->sy_k[0]
	       + C41 * fsp->sy_k[1]
	       + C42 * fsp->sy_k[2]);
	}
      symtab->sy_value = t + C4t * tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[3] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] 
	    + (C50 * fsp->sy_k[0]
	       + C51 * fsp->sy_k[1]
	       + C52 * fsp->sy_k[2]
	       + C53 * fsp->sy_k[3]);
	}
      symtab->sy_value = t + tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[4] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0]
	    + (C60 * fsp->sy_k[0]
	       + C61 * fsp->sy_k[1]
	       + C62 * fsp->sy_k[2]
	       + C63 * fsp->sy_k[3]
	       + C64 * fsp->sy_k[4]);
	}
      symtab->sy_value = t + C6t * tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
	fsp->sy_k[5] = tstep * fsp->sy_prime;
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_predi = fsp->sy_val[0] 
	    + (A0 * fsp->sy_k[0]
	       + A2 * fsp->sy_k[2]
	       + A3 * fsp->sy_k[3]
	       + A4 * fsp->sy_k[4]);
	  fsp->sy_value = fsp->sy_val[0] 
	    + (B0 * fsp->sy_k[0]
	       + B2 * fsp->sy_k[2]
	       + B3 * fsp->sy_k[3]
	       + B4 * fsp->sy_k[4]
	       + B5 * fsp->sy_k[5]);
	  if (fsp->sy_value != 0.0)
	    fsp->sy_sserr = fabs(1.0 - fsp->sy_predi / fsp->sy_value);
	  fsp->sy_aberr = fabs(fsp->sy_value - fsp->sy_predi);
	}

      if (!conflag && T_LT_TSTOP) 
	{
	  maxerr();
	  if (hierror()) 
	    {
	      tstep *= HALF;
	      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
		fsp->sy_value = fsp->sy_val[0];
	      gdval = false;
	      continue;
	    }
	  else 
	    if (lowerror() && prevstep != tstep) 
	      {
		prevstep = tstep; /* prevent infinite loops */
		tstep *= TWO;
		for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
		  fsp->sy_value = fsp->sy_val[0];
		gdval = false;
		continue;
	      }
	}
      gdval = true;
      prevstep = 0.0;
      ++it;
      t += tstep; /* the roundoff error is gross */
    }
}
