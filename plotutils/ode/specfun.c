/* This file is part of the GNU plotutils package. */

/* Copyright (C) 1997, 1998, 1998, 2005, 2008, 2009, Free Software
   Foundation, Inc. */

/* This file contains the five functions

   ibeta, igamma, norm, invnorm, inverf

   and versions of lgamma, erf, erfc for machines without them. */

/* The inspiration for this file was the file specfun.c that has long been
   a part of the gnuplot distribution.  However, the current version of the
   present file has been rewritten, largely from scratch, on the basis of
   published algorithms.  It is not dependent on the gnuplot specfun.c
   (which has included, and still includes, copyrighted code). */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include <errno.h>

#define ITERMAX 200

#ifdef FLT_EPSILON
#define MACHEPS FLT_EPSILON /* 1.0E-08 */
#else
#define MACHEPS 1.0E-08
#endif
#ifdef FLT_MIN_EXP
#define MINEXP  FLT_MIN_EXP /* -88.0 */
#else
#define MINEXP  -88.0
#endif
#ifdef FLT_MAX_EXP
#define MAXEXP  FLT_MAX_EXP /* +88.0 */
#else
#define MAXEXP  88.0
#endif
#ifdef FLT_MAX
#define OFLOW   FLT_MAX /* 1.0E+37 */
#else
#define OFLOW   1.0E+37
#endif
#ifdef FLT_MAX_10_EXP
#define XBIG    FLT_MAX_10_EXP /* 2.55E+305 */
#else
#define XBIG    2.55E+305
#endif

#ifndef HUGE_VAL
#ifdef HUGE
#define HUGE_VAL HUGE
#else
#ifdef INF
#define HUGE_VAL INF
#else
#define HUGE_VAL OFLOW
#endif
#endif
#endif

/*
 * Mathematical constants
 */
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288
#ifdef M_SQRT2
#undef M_SQRT2
#endif
#define M_SQRT2 1.41421356237309504880168872420969809
#define M_LNSQRT2PI 0.9189385332046727

/* Forward references */

/* The following gamma-related nonsense is necessary because (1) some
   vendors have lgamma(), some have gamma(), and some have neither [see
   include/sys-defines.h for further comments], (2) some vendors do not
   declare whichever function they have [e.g. Irix 5.3 requires an
   auxiliary preprocessing symbol to be defined for the declaration in
   math.h to be visible], and (3) some vendors supply broken versions which
   we can't use [e.g. AIX's libm.a gamma support is conspicuously broken],
   so we need to link in a replacement, but we can't use the same name for
   the external symbol `signgam'.  What a mess! -- rsm */
#ifdef NO_SYSTEM_GAMMA
#define SIGNGAM our_signgam
static int SIGNGAM;
double f_lgamma (double x);
static double lgamma_neg (double x);
static double lgamma_pos (double x);
#else  /* not NO_SYSTEM_GAMMA, we link in vendor code */
#define SIGNGAM signgam
extern int SIGNGAM;
#endif
double f_gamma (double x);

#ifndef HAVE_ERF
double erf (double x);
double erfc (double x);
#endif
double ibeta (double a, double b, double x);
double igamma (double a, double x);
double inverf (double x);
double invnorm (double x);
double norm (double x);
static double ibeta_internal (double a, double b, double x);


/*****************************************************************/
/************ Functions related to gamma function ****************/
/*****************************************************************/

/* Our gamma function.  F_LGAMMA(), which this calls, computes the log of
   the gamma function, with the sign being returned in SIGNGAM.  F_LGAMMA()
   is defined in include/sys-defines.h.  It may be a vendor-supplied
   lgamma(), a vendor-supplied gamma(), or our own f_lgamma (see below). */

double
f_gamma (double x)
{
#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  double y = F_LGAMMA(x);

  if (y > MAXEXP)
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"gamma";
      exc.arg1 = x;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "gamma: OVERFLOW error\n");
	  errno = ERANGE;
	}
      return exc.retval;
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }
  else
    return SIGNGAM * exp (y);
}

#ifdef NO_SYSTEM_GAMMA
/*
  Define our own lgamma(): compute log(Gamma(z)) for positive z as the sum
  of a suitably generated and truncated Lanczos series.  Lanczos series
  resemble Stirling (i.e. DeMoivre) asymptotic approximations, but unlike
  them are not asymptotic series; rather, they are convergent in the entire
  right half plane, on which a uniform bound on the error can be obtained.
  See C. Lanczos, "A Precision Approximation of the Gamma Function", SIAM
  J. Numerical Analysis 1B (1964), 86--96. */

double
f_lgamma (double z)
{
  SIGNGAM = 1;		      /* will return sign of Gamma(z) in SIGNGAM */

  if (z <= 0.0)
    return lgamma_neg (z);
  else
    return lgamma_pos (z);
}

/* Case I. z<=0 (if z < 0, reducible to z>0 case by reflection formula) */

static double
lgamma_neg (double z)
{
  double intpart, trigfac, retval;

#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  if (modf (-z, &intpart) == 0.0)
    /* z is nonpositive integer, so SING error */
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = z;
      exc.retval = HUGE_VAL;
      exc.type = SING;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "lgamma: SING error\n");
	  errno = EDOM;
	}
      return (exc.retval);
#else
      errno = EDOM;
      return HUGE_VAL;
#endif /* HAVE_MATHERR */
    }

  /* use Euler's reflection formula, and call lgamma_pos() */
  trigfac = sin (M_PI * z) / M_PI;
  if (trigfac < 0.0)
    {
      trigfac = - trigfac;
      SIGNGAM = -1;
    }
  retval = - lgamma_pos (1.0 - z) - log (trigfac);

  if (fabs (retval) == HUGE_VAL)
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = z;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr(&exc))
	errno = ERANGE;
      return (exc.retval);
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }

  return retval;
}

/* Case II. z>0, the primary case */

/* Lanczos parameter G (called lower-case gamma by him). "[A] large value
   of G is advocated if very high accuracy is demanded, but then the
   required number of terms will also be larger." */
#define LANCZOS_G 6

/* Values for coeffs of as many terms in the Lanczos expansion as are
   needed for this value of G, computed by Ray Toy <toy@rtp.ericsson.se>.
   (In his 1964 paper, Lanczos only went up to G=5.)  It is claimed (see
   gnuplot's specfun.c) that this value of G (i.e., 6) and number of terms
   will yield 14-digit accuracy everywhere except near z=1 and z=2. */

#define NUM_LANCZOS_TERMS 9
static const double lanczos[NUM_LANCZOS_TERMS] =
{
       .99999999999980993227684700473478296744476168282198,
    676.52036812188509856700919044401903816411251975244084,
  -1259.13921672240287047156078755282840836424300664868028,
    771.32342877765307884865282588943070775227268469602500,
   -176.61502916214059906584551353999392943274507608117860,
     12.50734327868690481445893685327104972970563021816420,
      -.13857109526572011689554706984971501358032683492780,
       .00000998436957801957085956266828104544089848531228,
       .00000015056327351493115583383579667028994545044040
};

static double
lgamma_pos (double z)
{
  double accum, retval;
  int i;

#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  accum = lanczos[0];
  for (i = 1; i < NUM_LANCZOS_TERMS; i++)
    accum += lanczos[i] / (z + i - 1);

  retval = (log (accum) + M_LNSQRT2PI - z - LANCZOS_G - 0.5
	    + (z - 0.5) * log (z + LANCZOS_G + 0.5));

  if (retval == HUGE_VAL)
    {
#ifdef HAVE_MATHERR
      exc.name = "lgamma";
      exc.arg1 = z;
      exc.retval = HUGE_VAL;
      exc.type = OVERFLOW;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "lgamma: OVERFLOW error\n");
	  errno = ERANGE;
	}
      return exc.retval;
#else
      errno = ERANGE;
      return HUGE_VAL;
#endif
    }

  return retval;
}
#endif /* NO_SYSTEM_GAMMA */



/*****************************************************************/
/************ Functions related to inverse beta function *********/
/*****************************************************************/

/* Our incomplete beta function, I_x(a,b).  Here a,b>0 and x is in [0,1].
   Returned value is in [0,1].  Note: this normalization convention is not
   universal.

   The formula given in Abramowitz & Stegun (Eq. 26.5.8) is used.  It
   includes a continued fraction expansion.  They say, "Best results are
   obtained when x < (a-1)/(a+b-2)."  We use it when x <= a/(a+b).

   This calls F_LGAMMA (e.g., lgamma()) to compute the prefactor in the
   formula. */

double
ibeta (double a, double b, double x)
{
  double retval;

#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  if (x < 0.0 || x > 1.0 || a <= 0.0 || b <= 0.0) /* DOMAIN error */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"ibeta";
      exc.arg1 = a;
      exc.arg2 = b;		/* have no arg3, can't return x (!) */
      exc.retval = HUGE_VAL;
      exc.type = DOMAIN;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "ibeta: DOMAIN error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return HUGE_VAL;
#endif
    }

  if (x == 0.0 || x == 1.0)
    return x;

  if (a < x * (a + b))
    /* interchange */
    retval = 1.0 - ibeta_internal (b, a, 1.0 - x);
  else
    retval = ibeta_internal (a, b, x);

  if (retval < 0.0)		/* error: failure of convergence */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"ibeta";
      exc.arg1 = a;
      exc.arg2 = b;		/* have no arg3, can't return x (!) */
      exc.retval = HUGE_VAL;
      exc.type = TLOSS;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "ibeta: TLOSS error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return HUGE_VAL;
#endif
    }

  return retval;
}

/* Evaluate convergents of the continued fraction by Wallis's method;
   return value will be positive, except that -1.0 is returned if there is
   no convergence. */

static double
ibeta_internal (double a, double b, double x)
{
  double A0, B0;
  double A2 = 1.0;
  double B2 = 0.0;
  double A1 = 1.0;
  double B1 = 1.0;
  double prefactor;
  double f0 = 0.0, f1 = 1.0;	/* f0 initted to quiet compiler */
  int goodf0, goodf1 = 1;
  int j;

  prefactor = exp (a * log (x) + b * log (1.0 - x)
		   + F_LGAMMA(a + b) - F_LGAMMA(a + 1.0) - F_LGAMMA(b));

  for (j = 1; j <= ITERMAX; j++)
    {
      double aj;
      int m;

      if (j % 2)		/* j odd, j = 2m + 1 */
	{
	  m = (j - 1)/2;
	  aj = - (a + m) * (a + b + m) * x / ((a + 2 * m) * (a + 2 * m + 1));
	}
      else			/* j even, j = 2m */
	{
	  m = j/2;
	  aj = m * (b - m) * x / ((a + 2 * m - 1) * (a + 2 * m));
	}

      A0 = 1.0 * A1 + aj * A2;
      B0 = 1.0 * B1 + aj * B2;
      
      if (B0 != 0.0)
	{
	  double ren;
	  
	  /* renormalize; don't really need to do this on each pass */
	  ren = 1.0 / B0;

	  A0 *= ren;
	  B0 = 1.0;
	  A1 *= ren;
	  B1 *= ren;

	  f0 = A0;
	  goodf0 = 1;
	  
	  /* test f0 = A0/B0 = A0 for exit */

	  if (goodf1 && fabs (f0 - f1) <= DMIN(MACHEPS, fabs (f0) * MACHEPS))
	    return (prefactor / f0);
	}
      else
	goodf0 = 0;

      /* shift down */
      A2 = A1;
      B2 = B1;
      A1 = A0;
      B1 = B0;
      f1 = f0;
      goodf1 = goodf0;
    }
  
  /* if we reached here, convergence failed */

  return -1.0;
}


/*****************************************************************/
/************ Functions related to incomplete gamma function *****/
/*****************************************************************/

/* Our incomplete gamma function, igamma(a,x) with a>0.0, x>=0.0.  Return
   value is in [0,1].  The algorithm is AS 239, documented in B. L. Shea,
   "Chi-Squared and Incomplete Gamma Integral", Applied Statistics 37
   (1988), 466-473.
   
   There have been claims that if 0<=x<=1, in which case Shea's algorithm
   uses Pearson's series rather than a continued fraction representation,
   an inaccurate value may result.  This has not been verified.  There have
   also been claims that the continued fraction representation is reliable
   only if x >= a+2, rather than x >= a (the latter being Shea's
   condition).  For safety, we use it only if x >= a+2. */

double
igamma (double a, double x)
{
  double arg, prefactor;
  int i;

#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  if (x < 0.0 || a <= 0.0)	/* DOMAIN error */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"igamma";
      exc.arg1 = a;
      exc.arg2 = x;
      exc.retval = HUGE_VAL;
      exc.type = DOMAIN;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "igamma: DOMAIN error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return HUGE_VAL;
#endif
    }

  if (x > XBIG)			/* TLOSS error */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"igamma";
      exc.arg1 = a;
      exc.arg2 = x;
      exc.retval = 1.0;
      exc.type = TLOSS;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "igamma: TLOSS error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return 1.0;
#endif
    }

  if (x == 0.0)
    return 0.0;

  /* check exponentiation in prefactor */
  arg = a * log (x) - x - F_LGAMMA(a + 1.0);
  if (arg < MINEXP)
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"igamma";
      exc.arg1 = a;
      exc.arg2 = x;
      exc.retval = 0.0;
      exc.type = TLOSS;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "ibeta: TLOSS error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return 0.0;
#endif
    }

  prefactor = exp (arg);

  if ((x > 1.0) && (x >= a + 2.0))
    /* use the continued fraction, not Pearson's series; generate its
       convergents by Wallis's method */
    {
      double A0, B0, A1, B1, A2, B2;
      double f0 = 0.0, f1;	/* f0 initted to quiet compiler */
      double aa, bb;
      int goodf0, goodf1 = 1;

      aa = 1.0 - a;
      bb = aa + x + 1.0;

      A2 = 1.0;
      B2 = x;
      A1 = x + 1.0;
      B1 = x * bb;
      f1 = A1 / B1;

      for (i = 1; i <= ITERMAX; i++)
	{
	  aa++;
	  bb += 2.0;
	
	  A0 = bb * A1 - i * aa * A2;
	  B0 = bb * B1 - i * aa * B2;
	
	  if (B0 != 0.0)
	    {
	      f0 = A0 / B0;
	      if (goodf1 && 
		  fabs (f0 - f1) <= DMIN(MACHEPS, fabs (f0) * MACHEPS))
		return (1.0 - prefactor * a * f0);

	      goodf0 = 1;
	    }
	  else
	    goodf0 = 0;

	  /* shift down */
	  A2 = A1;
	  B2 = B1;
	  A1 = A0;
	  B1 = B0;
	  f1 = f0;
	  goodf1 = goodf0;
	
	  if (fabs(A0) >= OFLOW)
	    /* renormalize */
	    {
	      A2 /= OFLOW;
	      B2 /= OFLOW;
	      A1 /= OFLOW;
	      B1 /= OFLOW;
	    }
	}
    }
  else
    /* use Pearson's series, not the continued fraction */
    {
      double aa, bb, cc;

      aa = a;
      bb = 1.0;
      cc = 1.0;

      for (i = 0; i <= ITERMAX; i++)
	{
	  aa++;
	  cc *= (x / aa);
	  bb += cc;
	  if (cc < bb * MACHEPS)
	    return prefactor * bb;
	}
    }

  /* if we reached here, convergence failed */

#ifdef HAVE_MATHERR
  exc.name = (char *)"igamma";
  exc.arg1 = a;
  exc.arg2 = x;
  exc.retval = HUGE_VAL;
  exc.type = TLOSS;
  if (!matherr (&exc))
    {
      fprintf (stderr, "ibeta: TLOSS error\n");
      errno = EDOM;
    }
  return exc.retval;
#else
  errno = EDOM;
  return HUGE_VAL;
#endif
}

#ifndef HAVE_ERF
double
erf (double x)
{
  return x < 0.0 ? -igamma (0.5, x * x) : igamma (0.5, x * x);
}

double
erfc (double x)
{
  return x < 0.0 ? 1.0 + igamma (0.5, x * x) : 1.0 - igamma (0.5, x * x);
}
#endif /* not HAVE_ERF */

double
norm (double x)
{
  return 0.5 * (1.0 + erf (0.5 * M_SQRT2 * x));
}


/*****************************************************************/
/************ Functions related to inverse error function ********/
/*****************************************************************/

/* Our inverse error function, inverf(x) with -1.0<x<1.0.  It approximates
   this (odd) function by four distinct degree-3 rational functions, each
   applying in a sub-interval of the right half-interval 0.0<x<1.0. */

/* rational function R0(x) */
static const double n0[4] =
  {
    -6.075593, 9.577584, -4.026908, 0.3110567
  };
static const double d0[4] =
  {
    -6.855572, 12.601905, -6.855985, 1.0
  };

/* rational function R1(w) */
static const double n1[4] =
  {
    -39.202359, 19.332985, -6.953050, 0.9360732
  };
static const double d1[4] =
  {
    -44.27977, 21.98546, -7.586103, 1.0
  };

/* rational function R2(w) */
static const double n2[4] =
  {
    -5.911558, 4.795462, -3.111584, 1.005405
  };
static const double d2[4] =
  {
    -6.266786, 4.666263, -2.962883, 1.0
  };

/* rational function R3(1/w) */
static const double n3[4] =
  {
    0.09952975, 0.51914515, -0.2187214, 1.0107864
  };
static const double d3[4] =
  {
    0.09952975, 0.5211733, -0.06888301, 1.0
  };


double
inverf (double x)
{
  static double num, den, retval;
  static int xsign;

#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  if (x <= -1.0 || x >= 1.0)	/* DOMAIN error */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"inverf";
      exc.arg1 = x;
      exc.retval = (x < 0.0 ? -HUGE_VAL : HUGE_VAL);
      exc.type = DOMAIN;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "inverf: DOMAIN error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return (x < 0.0 ? -HUGE_VAL : HUGE_VAL);
#endif
    }

  /* exploit oddness in x */
  xsign = (x >= 0.0 ? 1 : -1);
  x = (xsign > 0 ? x : -x);

  /* N.B. The numerator and denominator of each of these four rational
   approximants should really be written in nested polynomial form. */

  if (x <= 0.85)
    /* 0.0 <= x <= 0.85; use f = x R0(x**2), where R0 is degree-3 rational */
    {
      double y;
      
      y = x * x;
      num = n0[0] + n0[1]*y + n0[2]*y*y + n0[3]*y*y*y;
      den = d0[0] + d0[1]*y + d0[2]*y*y + d0[3]*y*y*y;

      retval = x * num / den;
    }
  else	/* x > 0.85 */
    {
      double w;

      w = sqrt (- log (1 - x * x)); /* w > 1.132 */
      
      /* note that as x->1-, i.e., w->infinity, retval is asymptotic to w,
	 to leading order */ 

      if (w <= 2.5)
	/* 1.132 < w <= 2.5; use f = w R1(w), where R1 is degree-3 rational */
	{
	  num = n1[0] + n1[1]*w + n1[2]*w*w + n1[3]*w*w*w;
	  den = d1[0] + d1[1]*w + d1[2]*w*w + d1[3]*w*w*w;
	  
	  retval = w * num / den;
	}
      
      else if (w <= 4.0)
	/* 2.5 < w <= 4.0; use f = w R2(w), where R2 is degree-3 rational */
	{
	  num = n2[0] + n2[1]*w + n2[2]*w*w + n2[3]*w*w*w;
	  den = d2[0] + d2[1]*w + d2[2]*w*w + d2[3]*w*w*w;
	  
	  retval = w * num / den;
	}
      
      else
	/* w > 4.0; use f = w R3(1/w), where R3 is degree-3 rational
	   with equal constant terms in numerator and denominator */
	{
	  double w1;
	  
	  w1 = 1.0 / w;

	  num = n3[0] + n3[1]*w1 + n3[2]*w1*w1 + n3[3]*w1*w1*w1;
	  den = d3[0] + d3[1]*w1 + d3[2]*w1*w1 + d3[3]*w1*w1*w1;
	  
	  retval = w * num / den;
	}
    }

  return (xsign > 0 ? retval : -retval);
}

/* Our inverse normal function (i.e., inverse Gaussian probability
   function), invnorm(x) with 0.0<x<1.0.  Trivially expressed in terms of
   inverf(), just as erf() can be expressed in terms of erfc().  */

double
invnorm (double x)
{
#ifdef HAVE_MATHERR
#ifdef __cplusplus
  struct __exception exc;
#else
  struct exception exc;
#endif
#endif

  if (x <= 0.0 || x >= 1.0)	/* DOMAIN error */
    {
#ifdef HAVE_MATHERR
      exc.name = (char *)"invnorm";
      exc.arg1 = x;
      exc.retval = HUGE_VAL;
      exc.type = DOMAIN;
      if (!matherr (&exc))
	{
	  fprintf (stderr, "invnorm: DOMAIN error\n");
	  errno = EDOM;
	}
      return exc.retval;
#else
      errno = EDOM;
      return HUGE_VAL;
#endif
    }

  return -M_SQRT2 * inverf (1.0 - 2 * x);
}
