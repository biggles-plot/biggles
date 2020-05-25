/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free
 * Software Foundation, Inc.
 */

/* stuff that doesn't go anywhere else in particular */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * check checks internal consistency prior to calling the
 * numerical routine.  Tasks it performs include:
 *	ensures one and only one independent variable
 *	the symbol table is coherent
 *	the print queue is coherent
 *	the independent variable has a derivative == 1.
 *	each dependent variable has an ODE and an initial value
 */
bool
check (void)
{
  struct sym *sp, *ivp, *prevp;
  struct prt *pp;
  
  /*
   * discard any previous entry for "(indep)"
   */
  prevp = NULL;
  for (sp = symtab; sp != NULL; sp = sp->sy_link) 
    {
      if (strncmp (sp->sy_name, "(indep)", NAMMAX) == 0) 
	{
	  if (prevp == NULL)
	    symtab = sp->sy_link;
	  else
	    prevp->sy_link = sp->sy_link;
	  sfree(sp);
	  break;
	}
      prevp = sp;
    }

  /*
   * check for only one independent variable
   */
  ivp = prevp = NULL;
  for (sp = symtab; sp != NULL; sp = sp->sy_link) 
    {
      if (!(sp->sy_flags & SF_DEPV)) 
	{
	  if (ivp != NULL) 
	    {
	      fprintf (stderr, 
		       "%s: both `%.*s' and `%.*s' are independent\n",
		       progname,
		       NAMMAX, sp->sy_name,
		       NAMMAX, ivp->sy_name);
	      return false;
	    }
	  ivp = sp;
	}
      if (ivp == NULL)
	prevp = sp;
    }

  /*
   * invent one if it's missing
   */
  if (ivp == NULL) 
    {
      ivp = salloc();
      strncpy (ivp->sy_name, "(indep)", NAMMAX);
    } 
  else if (prevp != NULL) 
    {
      
      /*
       * link the independent var at the
       * head of the symtab queue
       */
      prevp->sy_link = ivp->sy_link;
      ivp->sy_link = symtab;
      symtab = ivp;
    }

  /*
   * now ivp points to the ind. var. entry
   * make sure the independent var gets
   * printed when there's no print statement
   */
  if (!sawprint) 
    {
      for (pp = pqueue; pp != NULL; pp = pp->pr_link)
	if (pp->pr_sym == ivp)
	  goto found;
      pp = palloc();
      pp->pr_link = pqueue;
      pqueue = pp;
      pp->pr_sym = ivp;
    }
 found:
  /*
   * indep var has a derivative of 1.0
   */
  ivp->sy_expr = &exprone;
  
  /*
   * ensure an expr and value for each dep var
   */
  for (sp = symtab; sp != NULL; sp = sp->sy_link) 
    {
      switch (sp->sy_flags&SF_DEPV) 
	{
	case SF_INIT:
	  sp->sy_expr = &exprzero;
	  sp->sy_flags |= SF_ISEQN;
	  break;
	case SF_ISEQN:
	  sp->sy_value = 0;
	  sp->sy_flags |= SF_INIT;
	  break;
	}
    }

  /*
   * dependent variables start here
   */
  dqueue = symtab->sy_link;

  return true;
}

/*
 * set default values
 * determine step direction (forgive confused users)
 * initialize values for printq() and numerical routines
 */
void
defalt (void)
{
  if (!sawfrom)
    tfrom = tstart;
  if (!sawevery)
    tevery = 1;
  if (tstart>tstop && tstep>0)
    tstep = -tstep;
  else if (tstart<tstop && tstep<0)
    tstep = -tstep;
  printnum = false;
}

/*
 * evaluate all the derivatives
 */
void
field(void)
{
  for (fsp = symtab->sy_link; fsp!=NULL; fsp = fsp->sy_link)
    fsp->sy_prime = eval(fsp->sy_expr);
}

/*
 * internal error (fatal)
 */
void
panic (const char *s)
{
  fprintf (stderr, "%s panic: %s\n", progname, s);
  exit (EXIT_FAILURE);
}

void
panicn (const char *fmt, int n)
{
  fprintf (stderr, "%s panic: ", progname);
  fprintf (stderr, fmt, n);
  fprintf (stderr, "\n");
  exit (EXIT_FAILURE);
}

#define	LASTVAL (tstep>0 ? t>=tstop-0.0625*tstep : t<=tstop-0.0625*tstep)
#define	TFROM	(tfrom - 0.0625*tstep)
#define	PRFROM	(tstep>0 ? t >= TFROM : t<= TFROM)

void
printq (void)
{
  double f = 0.0;
  double t;
  struct prt *pp;

  t = symtab->sy_value;
  if (!printnum && PRFROM)
    printnum = true;
  if (((it % tevery == 0) && printnum) || LASTVAL) 
    {
      pp = pqueue;
      if (pp != NULL) 
	for (;;) 
	  {
	    switch (pp->pr_which) 
	      {
	      case P_VALUE:
		f = pp->pr_sym->sy_value;
		break;
	      case P_PRIME:
		f = pp->pr_sym->sy_prime;
		break;
	      case P_ACERR:
		f = pp->pr_sym->sy_acerr;
		break;
	      case P_ABERR:
		f = pp->pr_sym->sy_aberr;
		break;
	      case P_SSERR:
		f = pp->pr_sym->sy_sserr;
		break;
	      default:
		panicn ("bad cell spec (%d) in printq()", (int)(pp->pr_which));
		break;
	      }
	    prval (f);
	    pp = pp->pr_link;
	    if (pp == NULL)
	      break;
	    putchar (' ');
	  }
      putchar ('\n');
      fflush (stdout);
    }
  if (it == LONGMAX)
    it = 0;
}

/*
 * print a value to current precision
 * kludge for Pascal compatibility
 */
void
prval (double x)
{
  if (prec < 0) 
    {
      char outbuf[20];
      if (x < 0) 
	{
	  putchar ('-');
	  x = -x;
	}
      sprintf (outbuf, "%.7g", x);
      if (*outbuf == '.')
	putchar ('0');
      printf ("%s", outbuf);
  } 
  else
    printf ("%*.*e", fwd, prec, x);
}

/*
 * handler for math library exceptions (`rterrors' may or may not return)
 */
#ifdef HAVE_MATHERR
int 
# ifdef __cplusplus
matherr (struct __exception *x)
#else
matherr (struct exception *x)
#endif
{
  switch (x->type) 
    {
    case DOMAIN:
      rterrors ("domain error in %s", x->name);
      break;
    case SING:
      rterrors ("singularity error in %s", x->name);
      break;
    case OVERFLOW:
      rterrors ("range error (overflow) in %s", x->name);
      break;
#ifdef TLOSS
    case TLOSS:
      rterrors ("range error (total loss of significance) in %s",
		x->name);
      break;
#endif
#ifdef PLOSS
    case PLOSS:
      rterrors ("range error (partial loss of significance) in %s", 
		x->name);
      break;
#endif
    case UNDERFLOW:		/* treat as non-fatal */
      rtsquawks ("range error (underflow) in %s", x->name);
      break;
    default:
      rterrors ("unknown error in %s", x->name);
      break;
    }
  
  return 1;			/* suppress system error message */
}
#endif

/*
 * print a diagnostic for run-time errors.
 * uses fsp to decide which dependent variable was being worked
 */
void
rterror (const char *s)
{
  if (fsp == NULL)		/* no computation, just print message */
    fprintf (stderr, "%s: %s\n", progname, s);
  else
    {
      fprintf (stderr, "%s: %s while calculating %.*s'\n", progname, s,
	       NAMMAX, fsp->sy_name);
      longjmp (mark, 1);	/* interrupt computation */
    }
}

void
rterrors (const char *fmt, const char *s)
{
  if (fsp != NULL)		/* interrupt computation */
    {
      fprintf (stderr, "%s: ", progname);
      fprintf (stderr, fmt, s);
      fprintf (stderr, " while calculating %.*s'\n", NAMMAX, fsp->sy_name);
      longjmp (mark, 1);
    }
  else				/* just print error message */
    {
      fprintf (stderr, "%s: ", progname);
      fprintf (stderr, fmt, s);
      fprintf (stderr, "\n");
    }
}

/*
 * same, but doesn't do a longjmp.
 * computation continues
 */
void
rtsquawks (const char *fmt, const char *s)
{
  fprintf (stderr, "%s: ", progname);
  fprintf (stderr, fmt, s);
  if (fsp != NULL)
    fprintf (stderr, " while calculating %.*s'", NAMMAX, fsp->sy_name);
  fprintf (stderr, "\n");
  return;
}

/*
 * Run the numerical stuff.
 * This gets called from the grammar
 * because we want to solve on each
 * 'step' statement.
 */
void
solve (void)
{
  struct sym *sp;
  bool adapt;
  
  if (check() == false)
    return;
  defalt ();
  if (tflag)
    title ();

  fflush (stderr);
  setflt ();
  if (!setjmp (mark)) 
    {
      adapt = eflag|rflag|!conflag ? true : false;
      if (tstart == tstop)
	trivial();
      else switch (algorithm)
	{
	case A_EULER:
	  eu();
	  break;
	case A_ADAMS_MOULTON:
	  if (adapt || prerr)
	    ama();
	  else
	    am();
	  break;
	case A_RUNGE_KUTTA_FEHLBERG:
	default:
	  if (adapt || prerr)
	    rka();
	  else
	    rk();
	  break;
	}
    }
  resetflt();

  /* add final newline (to aid realtime postprocessing of dataset by graph) */
  putchar ('\n');
  fflush (stdout);

  for (sp = symtab; sp != NULL; sp = sp->sy_link) 
    {
      sp->sy_prime = sp->sy_pri[0];
      sp->sy_value = sp->sy_val[0];
    }
}

/* 
 * choose step size at tstart
 */
void
startstep (void)
{
  if (!hflag)
    hmax = fabs ((tstop-tstart)/2);
  tstep = fabs ((tstop-tstart)/MESH);
  if (tstep > hmax)
    tstep = hmax;
  if (tstep < hmin)
    tstep = hmin;
  while (tstep >= HMAX)
    tstep *= HALF;
  while (tstart + tstep == tstart)
    tstep *= TWO;
}

/*
 * print a header
 * Try to center the headings over the columns
 */
void
title (void)
{
  struct prt *pp;
  char tag = '\0';
  
  pp = pqueue;
  if (pp != NULL) 
    for (;;) 
      {
	switch (pp->pr_which) 
	  {
	  case P_PRIME: 
	    tag = '\''; 
	    break;
	  case P_VALUE: 
	    tag = ' '; 
	    break;
	  case P_SSERR: 
	    tag = '?'; 
	    break;
	  case P_ABERR:
	    tag = '!';
	    break;
	  case P_ACERR: 
	    tag = '~'; 
	    break;
	  default: 
	    panicn ("bad cell spec (%d) in title()", (int)(pp->pr_which));
	    break;
	  }
	printf (" %*.*s%c", fwd - 2, NAMMAX, pp->pr_sym->sy_name, tag);
	if ((pp=pp->pr_link) == NULL)
	  break;
	putchar (' ');
      }
  putchar ('\n');
  fflush (stdout);
}
