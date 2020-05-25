/* This file is part of the GNU plotutils package. */

/* Bessel function approximations, as given in the book "Computer
 * Approximations" by Hart, Cheney et al., Wiley, 1968.  Taken in part from
 * the file standard.c in the gnuplot 3.5 distribution. */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * Copyright (C) 1986 - 1993   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and 
 * that both that copyright notice and this permission notice appear 
 * in supporting documentation.
 */

/*
 * AUTHORS
 * 
 *   Original Software:
 *     Thomas Williams,  Colin Kelley.
 * 
 *   Gnuplot 2.0 additions:
 *       Russell Lang, Dave Kotz, John Campbell.
 *
 *   Gnuplot 3.0 additions:
 *       Gershon Elber and many others.
 * 
 */

/*
 * There appears to be a mistake in Hart, Cheney et al. on page 149.
 * Where it lists Qn(x)/x ~ P(z*z)/Q(z*z), z = 8/x, it should read
 *                Qn(x)/z ~ P(z*z)/Q(z*z), z = 8/x
 * In the functions below, Qn(x) is implemented using the later
 * equation.
 * These Bessel functions are accurate to about 1e-13.
 */

#ifndef HAVE_J0

#define PI_ON_FOUR       0.78539816339744830961566084581987572
#define PI_ON_TWO        1.57079632679489661923131269163975144
#define THREE_PI_ON_FOUR 2.35619449019234492884698253745962716
#define TWO_ON_PI        0.63661977236758134307553505349005744

static const double dzero = 0.0;

/* jzero for x in [0,8]
 * Index 5849, 19.22 digits precision
 */
static const double pjzero[9] = 
{
  0.4933787251794133561816813446e+21,
  -0.11791576291076105360384408e+21,
  0.6382059341072356562289432465e+19,
  -0.1367620353088171386865416609e+18,
  0.1434354939140346111664316553e+16,
  -0.8085222034853793871199468171e+13,
  0.2507158285536881945555156435e+11,
  -0.4050412371833132706360663322e+8,
  0.2685786856980014981415848441e+5
};

static const double qjzero[9] = 
{
  0.4933787251794133562113278438e+21,
  0.5428918384092285160200195092e+19,
  0.3024635616709462698627330784e+17,
  0.1127756739679798507056031594e+15,
  0.3123043114941213172572469442e+12,
  0.669998767298223967181402866e+9,
  0.1114636098462985378182402543e+7,
  0.1363063652328970604442810507e+4,
  0.1e+1
};

/* pzero for x in [8,inf]
 * Index 6548, 18.16 digits precision
 */
static const double ppzero[6] = 
{
  0.2277909019730468430227002627e+5,
  0.4134538663958076579678016384e+5,
  0.2117052338086494432193395727e+5,
  0.348064864432492703474453111e+4,
  0.15376201909008354295771715e+3,
  0.889615484242104552360748e+0
};

static const double qpzero[6] = 
{
  0.2277909019730468431768423768e+5,
  0.4137041249551041663989198384e+5,
  0.2121535056188011573042256764e+5,
  0.350287351382356082073561423e+4,
  0.15711159858080893649068482e+3,
  0.1e+1
};

/* qzero for x in [8,inf]
 * Index 6948, 18.33 digits precision
 */
static const double pqzero[6] = 
{
  -0.8922660020080009409846916e+2,
  -0.18591953644342993800252169e+3,
  -0.11183429920482737611262123e+3,
  -0.2230026166621419847169915e+2,
  -0.124410267458356384591379e+1,
  -0.8803330304868075181663e-2,
};

static const double qqzero[6] = 
{
  0.571050241285120619052476459e+4,
  0.1195113154343461364695265329e+5,
  0.726427801692110188369134506e+4,
  0.148872312322837565816134698e+4,
  0.9059376959499312585881878e+2,
  0.1e+1
};

/* yzero for x in [0,8]
 * Index 6245, 18.78 digits precision
 */
static const double pyzero[9] = 
{
  -0.2750286678629109583701933175e+20,
  0.6587473275719554925999402049e+20,
  -0.5247065581112764941297350814e+19,
  0.1375624316399344078571335453e+18,
  -0.1648605817185729473122082537e+16,
  0.1025520859686394284509167421e+14,
  -0.3436371222979040378171030138e+11,
  0.5915213465686889654273830069e+8,
  -0.4137035497933148554125235152e+5
};

static const double qyzero[9] = 
{
  0.3726458838986165881989980739e+21,
  0.4192417043410839973904769661e+19,
  0.2392883043499781857439356652e+17,
  0.9162038034075185262489147968e+14,
  0.2613065755041081249568482092e+12,
  0.5795122640700729537380087915e+9,
  0.1001702641288906265666651753e+7,
  0.1282452772478993804176329391e+4,
  0.1e+1
};

/* jone for x in [0,8]
 * Index 6050, 20.98 digits precision
 */
static const double pjone[9] = 
{
  0.581199354001606143928050809e+21,
  -0.6672106568924916298020941484e+20,
  0.2316433580634002297931815435e+19,
  -0.3588817569910106050743641413e+17,
  0.2908795263834775409737601689e+15,
  -0.1322983480332126453125473247e+13,
  0.3413234182301700539091292655e+10,
  -0.4695753530642995859767162166e+7,
  0.270112271089232341485679099e+4
};

static const double qjone[9] = 
{
  0.11623987080032122878585294e+22,
  0.1185770712190320999837113348e+20,
  0.6092061398917521746105196863e+17,
  0.2081661221307607351240184229e+15,
  0.5243710262167649715406728642e+12,
  0.1013863514358673989967045588e+10,
  0.1501793594998585505921097578e+7,
  0.1606931573481487801970916749e+4,
  0.1e+1
};

/* pone for x in [8,inf]
 * Index 6749, 18.11 digits precision
 */
static const double ppone[6] = 
{
  0.352246649133679798341724373e+5,
  0.62758845247161281269005675e+5,
  0.313539631109159574238669888e+5,
  0.49854832060594338434500455e+4,
  0.2111529182853962382105718e+3,
  0.12571716929145341558495e+1
};

static const double qpone[6] = 
{
  0.352246649133679798068390431e+5,
  0.626943469593560511888833731e+5,
  0.312404063819041039923015703e+5,
  0.4930396490181088979386097e+4,
  0.2030775189134759322293574e+3,
  0.1e+1
};

/* qone for x in [8,inf]
 * Index 7149, 18.28 digits precision
 */
static const double pqone[6] = 
{
  0.3511751914303552822533318e+3,
  0.7210391804904475039280863e+3,
  0.4259873011654442389886993e+3,
  0.831898957673850827325226e+2,
  0.45681716295512267064405e+1,
  0.3532840052740123642735e-1
};

static const double qqone[6] = 
{
  0.74917374171809127714519505e+4,
  0.154141773392650970499848051e+5,
  0.91522317015169922705904727e+4,
  0.18111867005523513506724158e+4,
  0.1038187585462133728776636e+3,
  0.1e+1
};

/* yone for x in [0,8]
 * Index 6444, 18.24 digits precision
 */
static const double pyone[8] = 
{
  -0.2923821961532962543101048748e+20,
  0.7748520682186839645088094202e+19,
  -0.3441048063084114446185461344e+18,
  0.5915160760490070618496315281e+16,
  -0.4863316942567175074828129117e+14,
  0.2049696673745662182619800495e+12,
  -0.4289471968855248801821819588e+9,
  0.3556924009830526056691325215e+6
};

static const double qyone[9] = 
{
  0.1491311511302920350174081355e+21,
  0.1818662841706134986885065935e+19,
  0.113163938269888452690508283e+17,
  0.4755173588888137713092774006e+14,
  0.1500221699156708987166369115e+12,
  0.3716660798621930285596927703e+9,
  0.726914730719888456980191315e+6,
  0.10726961437789255233221267e+4,
  0.1e+1
};

/* Bessel function approximations */

double 
jzero (double x)
{
  double p, q, x2;
  int n;

  x2 = x * x;
  p = pjzero[8];
  q = qjzero[8];
  for (n=7; n>=0; n--) 
    {
      p = p*x2 + pjzero[n];
      q = q*x2 + qjzero[n];
    }
  return (p/q);
}

static double 
pzero (double x)
{
  double p, q, z, z2;
  int n;
  
  z = 8.0 / x;
  z2 = z * z;
  p = ppzero[5];
  q = qpzero[5];
  for (n=4; n>=0; n--) 
    {
      p = p*z2 + ppzero[n];
      q = q*z2 + qpzero[n];
    }
  return (p/q);
}

static double 
qzero (double x)
{
  double p, q, z, z2;
  int n;
  
  z = 8.0 / x;
  z2 = z * z;
  p = pqzero[5];
  q = qqzero[5];
  for (n=4; n>=0; n--) 
    {
      p = p*z2 + pqzero[n];
      q = q*z2 + qqzero[n];
    }
  return (p/q);
}

static double 
yzero (double x)
{
  double p, q, x2;
  int n;
  
  x2 = x * x;
  p = pyzero[8];
  q = qyzero[8];
  for (n=7; n>=0; n--) 
    {
      p = p*x2 + pyzero[n];
      q = q*x2 + qyzero[n];
    }
  return p/q;
}

double 
j0 (double x)
{
  if (x <= 0.0)
    x = -x;
  if (x < 8.0)
    return jzero(x);
  else
    return (sqrt(TWO_ON_PI/x) 
	    * (pzero(x) * cos (x - PI_ON_FOUR) 
	       - 8.0/x * qzero(x) * sin (x - PI_ON_FOUR)));
}

double 
y0 (double x)
{
  if (x < 0.0)
    return (dzero/dzero);	/* IEEE machines: invalid operation */
  if (x < 8.0)
    return yzero(x) + TWO_ON_PI * j0(x) * log(x);
  else
    return (sqrt (TWO_ON_PI/x) 
	    * (pzero(x) * sin (x - PI_ON_FOUR) 
	       + (8.0/x) * qzero(x) * cos(x - PI_ON_FOUR)));

}

static double 
jone (double x)
{
  double p, q, x2;
  int n;
  
  x2 = x * x;
  p = pjone[8];
  q = qjone[8];
  for (n=7; n>=0; n--) 
    {
      p = p*x2 + pjone[n];
      q = q*x2 + qjone[n];
    }
  return (p/q);
}

static double 
pone (double x)
{
  double p, q, z, z2;
  int n;
  
  z = 8.0 / x;
  z2 = z * z;
  p = ppone[5];
  q = qpone[5];
  for (n=4; n>=0; n--) 
    {
      p = p*z2 + ppone[n];
      q = q*z2 + qpone[n];
    }
  return (p/q);
}

static double 
qone (double x)
{
  double p, q, z, z2;
  int n;
  
  z = 8.0 / x;
  z2 = z * z;
  p = pqone[5];
  q = qqone[5];
  for (n=4; n>=0; n--) 
    {
      p = p*z2 + pqone[n];
      q = q*z2 + qqone[n];
    }
  return p/q;
}

static double 
yone (double x)
{
  double p, q, x2;
  int n;
  
  x2 = x * x;
  p = 0.0;
  q = qyone[8];
  for (n=7; n>=0; n--) 
    {
      p = p*x2 + pyone[n];
      q = q*x2 + qyone[n];
    }
  return p/q;
}

double 
j1 (double x)
{
  double v,w;
  v = x;
  if (x < 0.0)
    x = -x;
  if (x < 8.0)
    return v * jone(x);
  else 
    {
      w = (sqrt(TWO_ON_PI/x) 
	   * (pone(x) * cos(x - THREE_PI_ON_FOUR) 
	      - 8.0 / x * qone(x) * sin (x - THREE_PI_ON_FOUR)));
      if (v < 0.0)
	w = -w;
      return w;
    }
}

double 
y1 (double x)
{
  if (x <= 0.0)
    return (dzero/dzero);	/* IEEE machines: invalid operation */
  if (x < 8.0)
    return x * yone(x) + TWO_ON_PI * (j1(x) * log(x) - 1.0/x);
  else
    return (sqrt(TWO_ON_PI/x) 
	    * (pone(x) * sin (x - THREE_PI_ON_FOUR) 
	       + (8.0/x) * qone(x) * cos(x - THREE_PI_ON_FOUR)));
}

/* Computation of jn(n,x), i.e., a Bessel function of arbitrary
   non-negative index, is as follows.

   For n=0, j0() is called.
   For n=1, j1() is called.
   For n<x, forward recursion is used, starting	from values of j0(x) 
	and j1(x).
   For n>x, a continued fraction approximation to jn(n,x)/jn(n-1,x) is 
        evaluated, and then backward recursion is used starting from a
        supposed value for jn(n,x).  The resulting value of jn(0,x) is
        compared with the actual value, to correct the supposed value of
        jn(n,x).

   Computation of yn(n,x) is similar in all respects, except that forward
   recursion is used for all positive values of n.
*/

double
jn (int n, double x) 
{
  int i;
  
  if (n < 0)
    {
      n = -n;
      x = -x;
    }
  if (n == 0) 
    return j0(x);
  if (n == 1) 
    return j1(x);
  if (x == 0.0) 
    return 0.0;

  if (n <= x)
    {
      double a = j0(x), b = j1(x), tmp;

      for (i = 1; i < n; i++)
	{
	  tmp = b;
	  b = (2.0*i / x) * b - a;
	  a = tmp;
	}
      return b;
    }
  else	/* n > x */
    {
      double a, b, xsq, t, tmp;

      xsq = x*x;
      for (t=0, i=n+16; i > n; i--)
	t = xsq / (2.0*i - t);
      t = x / (2.0*n - t);
      a = t;
      b = 1.0;
      for (i = n - 1; i > 0; i--)
	{
	  tmp = b;
	  b = (2.0*i / x ) * b - a;
	  a = tmp;
	}
      return t*j0(x)/b;
    }
}

double
yn (int n, double x) 
{
  int i, sign;
  double a, b, tmp;
  
  if (x <= 0)
    return (dzero/dzero);	/* IEEE machines: invalid operation */

  sign = 1;
  if (n < 0)
    {
      n = -n;
      if (n%2 == 1) 
	sign = -1;
    }
  if (n == 0) 
    return y0(x);
  if (n == 1) 
    return sign*y1(x);

  a = y0(x);
  b = y1(x);
  for (i = 1; i<n; i++)
    {
      tmp = b;
      b = (2.0*i / x) * b - a;
      a = tmp;
    }
  return sign*b;
}

#endif /* HAVE_J0 */
