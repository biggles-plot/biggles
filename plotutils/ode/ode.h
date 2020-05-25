/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997 Free Software Foundation, Inc.
 */

/*
 * preprocessor macros and structure declarations for ode.
 */

#include <setjmp.h>

#define NAMMAX  32      /* size of identifiers */
#define KMAX    7       /* size of k vector */
#define PASTMAX 7       /* size of history vector */

/*
 * symbol table entry
 */
struct sym 
{
  char    sy_name[NAMMAX];
  double  sy_value;
  double  sy_val[PASTMAX];
  double  sy_prime;
  double  sy_pri[PASTMAX];
  double  sy_predi;		/* predictor value */
  double  sy_sserr;		/* relative single step error */
  double  sy_aberr;		/* absolute single step error */
  double  sy_acerr;		/* accumulated error */
  double  sy_k[KMAX];
  int     sy_flags;
  struct  expr    *sy_expr;
  struct  sym     *sy_link;
};

/*
 * bits in the flags word
 * of a symbol table entry
 */
#define SF_INIT 1		/* set when the initial value is given */
#define SF_ISEQN 2		/* set when an ODE is given */
#define SF_DEPV (SF_INIT|SF_ISEQN)

/* 
 * enumeration of printable cells in 
 * a symbol table entry
 */
typedef enum 
{
  P_VALUE, P_PRIME, P_SSERR, P_ACERR, P_ABERR
} ent_cell;

/*
 * the print queue is made
 * of these
 */
struct prt 
{
  ent_cell    pr_which;		/* which cell to print */
  struct prt  *pr_link;
  struct sym  *pr_sym;
};

/*
 * yylex returns a pointer to this.
 * The receiving yacc action must
 * free the space when it's done with it.
 */
struct lex 
{
  union 
    {
      double  lxu_value;
      char    lxu_name[NAMMAX];
    }	lx_u;
  int	lx_lino;
};

/*
 * operation types
 */
typedef enum 
{
  O_NOOP, O_PLUS, O_MINUS, O_MULT, O_DIV, O_POWER, O_SQRT, O_EXP, O_LOG,
  O_LOG10, O_SIN, O_COS, O_TAN, O_ASIN, O_ACOS, O_ATAN, O_IDENT, O_CONST,
  O_NEG, O_ABS, O_SINH, O_COSH, O_TANH, O_ASINH, O_ACOSH, O_ATANH, O_SQAR,
  O_CUBE, O_INV, O_FLOOR, O_CEIL, O_J0, O_J1, O_Y0, O_Y1, O_ERF, O_ERFC,
  O_INVERF, O_LGAMMA, O_GAMMA, O_NORM, O_INVNORM, O_IGAMMA, O_IBETA
} op_type;

/*
 * an operation in an expression list
 */
struct expr 
{
  op_type        ex_oper;
  double         ex_value;
  struct sym     *ex_sym;
  struct expr    *ex_next;
};

/* integration algorithm type */
typedef enum 
{ 
  A_EULER, A_ADAMS_MOULTON, A_RUNGE_KUTTA_FEHLBERG
} integration_type;

/*
 * misc. consts
 */
#define        TWO     (2.0)
#define        HALF    (0.5)
#define        SCALE   (1e-4)
#define        HMIN    (1e-36) /* Minimum step size */
#define        HMAX    (0.5)	/* Maximum step size */
#define        LONGMAX (2147483647) /* 2^31-1 */
#define        MESH    (2)
#define        STOPR   (tstep>0 ? \
                        t-0.0625*tstep>=tstop : t-0.0625*tstep<=tstop )
#define        STOPA   (tstep>0 ? \
                        t+0.9375*tstep>=tstop : t+0.9375*tstep<=tstop )
