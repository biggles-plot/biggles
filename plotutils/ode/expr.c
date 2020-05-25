/* This file is part of the GNU plotutils package. */

/*
 *
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free
 * Software Foundation, Inc.
 */

/*
 * expression tree evaluator and expression space management routines
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

#define NSTACK 30
double stack[NSTACK];

double
eval (const struct expr *ep)
{
  double *sp;
  double tmp, tmp2;

  for (sp = &stack[NSTACK]; ep != NULL; ep = ep->ex_next) 
    {
#ifdef DEBUG
      if (sp > &stack[NSTACK])
	panic ("expression stack underflow");
#endif
      if (sp <= stack)
	panic ("stack overflow -- bump NSTACK and recompile");

      switch (ep->ex_oper) 
	{
	case O_CONST:
	  *--sp = ep->ex_value;
	  break;
	case O_IDENT:
	  *--sp = ep->ex_sym->sy_value;
	  break;
	case O_PLUS:
	  tmp = *sp++;
	  *sp += tmp;
	  break;
	case O_MINUS:
	  tmp = *sp++;
	  *sp -= tmp;
	  break;
	case O_MULT:
	  tmp = *sp++;
	  *sp *= tmp;
	  break;
	case O_DIV:
	  tmp = *sp++;
	  *sp /= tmp;
	  break;
	case O_POWER:
	  tmp = *sp++;
	  if ((tmp != (int)tmp) && (*sp < 0))
	    rterror("negative number to non-integer power");
	  *sp = pow(*sp,tmp);
	  break;
	case O_SQAR:
	  *sp *= *sp;
	  break;
	case O_CUBE:
	  *sp *= *sp * *sp;
	  break;
	case O_INV:
	  *sp = 1. / *sp;
	  break;
	case O_SQRT:
	  if (*sp < 0)
	    rterror("square root of a negative number");
	  *sp = sqrt(*sp);
	  break;
	case O_SIN:
	  *sp = sin(*sp);
	  break;
	case O_COS:
	  *sp = cos(*sp);
	  break;
	case O_TAN:
	  *sp = tan(*sp);
	  break;
	case O_ASIN:
	  *sp = asin(*sp);
	  break;
	case O_ACOS:
	  *sp = acos(*sp);
	  break;
	case O_ATAN:
	  *sp = atan(*sp);
	  break;
	case O_ABS:
	  if (*sp < 0)
	    *sp = -*sp;
	  break;
	case O_EXP:
	  *sp = exp(*sp);
	  break;
	case O_LOG:
	  if (*sp <= 0)
	    rterror("logarithm of non-positive number");
	  *sp = log(*sp);
	  break;
	case O_LOG10:
	  if (*sp <= 0)
	    rterror("logarithm of non-positive number");
	  *sp = log10(*sp);
	  break;
	case O_SINH:
	  *sp = sinh(*sp);
	  break;
	case O_COSH:
	  *sp = cosh(*sp);
	  break;
	case O_TANH:
	  *sp = tanh(*sp);
	  break;
	case O_ASINH:
	  *sp = asinh(*sp);
	  break;
	case O_ACOSH:
	  *sp = acosh(*sp);
	  break;
	case O_ATANH:
	  *sp = atanh(*sp);
	  break;
	case O_FLOOR:
	  *sp = floor(*sp);
	  break;
	case O_CEIL:
	  *sp = ceil(*sp);
	  break;
	case O_J0:
	  *sp = j0(*sp);
	  break;
	case O_J1:
	  *sp = j1(*sp);
	  break;
	case O_Y0:
	  *sp = y0(*sp);
	  break;
	case O_GAMMA:
	  *sp = f_gamma(*sp);
	  break;
	case O_LGAMMA:
	  *sp = F_LGAMMA(*sp);
	  break;
	case O_ERFC:
	  *sp = erfc(*sp);
	  break;
	case O_ERF:
	  *sp = erf(*sp);
	  break;
	case O_INVERF:
	  *sp = inverf(*sp);
	  break;
	case O_NORM:
	  *sp = norm(*sp);
	  break;
	case O_INVNORM:
	  *sp = invnorm(*sp);
	  break;
	case O_NEG:
	  *sp = -*sp;
	  break;
	case O_IGAMMA:
	  tmp = *sp++;
	  *sp = igamma(*sp, tmp);
	  break;
	case O_IBETA:
	  tmp2 = *sp++;
	  tmp = *sp++;
	  *sp = ibeta(*sp, tmp, tmp2);
	  break;
	default:
	  panicn ("bad op spec (%d) in eval()", (int)(ep->ex_oper));
	}
    }
  return *sp;
}

struct expr *
ealloc (void)
{
  struct expr *ep;
  
  ep = (struct expr *)xmalloc (sizeof(struct expr));
  ep->ex_next = NULL;
  ep->ex_sym = NULL;
  ep->ex_oper = O_NOOP;		/* default */
  return ep;
}

void
efree (struct expr *ep)
{
  if (ep == NULL || ep == &exprzero || ep == &exprone)
    return;
  efree (ep->ex_next);
  free ((void *)ep);
}
