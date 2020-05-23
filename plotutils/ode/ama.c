/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, 2008, Free Software
 * Foundation, Inc.
 */

/*
 * Adams-Moulton with adaptive step size
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include "num.h"

#define PASTVAL        (3)	/* past values, val[0] is current value */
#define T_LT_TSTOP (tstep>0 ? t<tstop:t>tstop)
#define NEARSTOP       (tstep > 0 ? \
                        t+0.9375*tstep > tstop && t+0.0625*tstep < tstop : \
                        t+0.9375*tstep < tstop && t+0.0625*tstep > tstop)

void
ama (void)
{
  bool gdval = true;		/* good value to print ? */
  int overtime = 1;
  long startit = 0;
  double prevstep = 0.0;
  double t = tstart;

 top:
  /* fifth-order Runge-Kutta startup */
  it = startit;
  while (it <= startit + PASTVAL&&(T_LT_TSTOP || overtime--)) 
    {
      symtab->sy_value = t;
      field();
      if (gdval) 
	{
	  for (fsp = symtab; fsp != NULL; fsp = fsp->sy_link) 
	    {
	      int j;
	      
	      for (j = it - startit; j > 0; j--) 
		{
		  fsp->sy_val[j] = fsp->sy_val[j-1];
		  fsp->sy_pri[j] = fsp->sy_pri[j-1];
		}
	      fsp->sy_val[0] = fsp->sy_value;
	      fsp->sy_pri[0] = fsp->sy_prime;
	    }
	  printq();		/* output */
	  if (it == startit + PASTVAL)
	    break;		/* startup complete */
	}
      if (tstep * (t+tstep-tstop) > 0)
	tstep = tstop - t;
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[0] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] + C20 * fsp->sy_k[0];
	}
      symtab->sy_value = t + C2t * tstep;
      field();
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_k[1] = tstep * fsp->sy_prime;
	  fsp->sy_value = fsp->sy_val[0] + C30 * fsp->sy_k[0]
	    + C31 * fsp->sy_k[1];
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
	    fsp->sy_sserr = fabs(1.0 - fsp->sy_predi /
				 fsp->sy_value);
	  fsp->sy_aberr = fabs(fsp->sy_value - fsp->sy_predi);
	}
      if (!conflag && T_LT_TSTOP) 
	{
	  maxerr();
	  if (hierror()) 
	    { 
	      tstep *= HALF;
	      for (fsp = symtab; fsp != NULL; fsp = fsp->sy_link)
		fsp->sy_value = fsp->sy_val[0];
	      gdval = false;
	      continue;
	    }
	  else if (lowerror() && prevstep != tstep) 
	    {
	      prevstep = tstep; /* prevent infinite loops */
	      tstep *= 2.0;
	      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link)
		fsp->sy_value = fsp->sy_val[0];
	      gdval = false;
	      continue;
	    }
	}
      gdval = true;
      ++it;
      t += tstep;	/* the roundoff error is gross */
    }
  /* predictor - corrector */
  while (T_LT_TSTOP ) 
    {
      /* Adams-Bashforth predictor */
      if (tstep*(t+tstep-tstop) > 0)
	{
	  startit = it;
	  goto top;
	}
      for (fsp = dqueue; fsp != NULL ; fsp = fsp->sy_link) 
	{
	  fsp->sy_predi = fsp->sy_value
	    = fsp->sy_val[0] + tstep/24.0 *
	      (55 * fsp->sy_pri[0]
	       -59 * fsp->sy_pri[1]
	       +37 * fsp->sy_pri[2]
	       -9  * fsp->sy_pri[3]);
	}
      ++it;
      symtab->sy_value = t += tstep;
      /* the roundoff error is gross */
      field();
      /* Adams-Moulton corrector */
      for (fsp = dqueue; fsp != NULL; fsp = fsp->sy_link) 
	{
	  fsp->sy_value = fsp->sy_val[0] + tstep/24.0 *
	    (9  * fsp->sy_prime
	     +19 * fsp->sy_pri[0]
	     -5  * fsp->sy_pri[1]
	     +     fsp->sy_pri[2]);
	  if (fsp->sy_value != 0.0)
	    fsp->sy_sserr = ECONST *
	      fabs(1.0 - fsp->sy_predi / fsp->sy_value);
	  fsp->sy_aberr = ECONST *
	    fabs (fsp->sy_value - fsp->sy_predi);
	  fsp->sy_value += ECONST * (fsp->sy_predi - fsp->sy_value);
	}
      if (!conflag) 
	{
	  maxerr();
	  if (hierror()) 
	    {
	      tstep *= HALF;
	      t = symtab->sy_val[0];
	      for (fsp = symtab; fsp != NULL; fsp = fsp->sy_link) 
		{
		  fsp->sy_value = fsp->sy_val[0];
		  fsp->sy_prime = fsp->sy_pri[0];
		}
	      startit = --it;
	      gdval = false;
	      goto top;
	    }
	  else if (lowerror()) 
	    {
	      tstep *= TWO;
                                        t = symtab->sy_val[0];
	      for (fsp = symtab; fsp != NULL; fsp = fsp->sy_link) 
		{
		  fsp->sy_value = fsp->sy_val[0];
		  fsp->sy_prime = fsp->sy_pri[0];
		}
	      startit = --it;
	      gdval = false;
	      goto top;
	    }
	}
      field();
      /* cycle indices */
      for (fsp = symtab; fsp != NULL; fsp = fsp->sy_link) 
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
