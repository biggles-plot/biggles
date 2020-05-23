/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free
 * Software Foundation, Inc.
 */

/*
 * declarations of external variables and functions for ode
 */

/*
 * external variable declarations
 */

/* variables defined and initted in global.c */
extern const char     *progname, *written, *copyright;
extern int            prec;
extern long           it;
extern double         hmin, hmax, ssmin, ssmax, abmin, abmax, acmax;
extern struct sym     *symtab, *fsp;
extern struct sym     *dqueue;
extern struct prt     *pqueue;
extern struct expr    exprzero, exprone;
extern bool        sawstep, sawprint, sawevery, sawfrom;
extern bool        tflag, pflag, sflag, eflag, rflag, hflag, conflag;
extern integration_type	algorithm;

/* variables defined but not initted in global.c */
extern char    *filename;
extern jmp_buf mark;
extern int     fwd;
extern int     tevery;
extern double  tstart, tstop, tstep, tfrom;
extern bool printnum, prerr;

/* in parser */
extern FILE    *yyin;

/* in scanner */
extern int     curline;

/*
 * external function declarations
 */
bool check (void);
bool hierror (void);
bool intpr (double t);
bool lowerror (void);
double eval (const struct expr *ep);
void am (void);
void ama (void);
void args (int ac, char **av);
void defalt (void);
void eu (void);
void efree (struct expr *ep);
void field (void);
void maxerr (void);
void panic (const char *s);
void panicn (const char *fmt, int n);
void pfree (struct prt *pp);
void printq (void);
void prval (double x);
void maxerror (void);
void resetflt (void);
void rk (void);
void rka (void);
void rterror (const char *s);
void rterrors (const char *fmt, const char *s);
void rtsquawks (const char *fmt, const char *s);
void setflt (void);
void sfree (struct sym *sp);
void solve (void);
void startstep (void);
void title (void);
void trivial (void);
struct expr * ealloc (void);
struct prt * palloc (void);
struct sym * lookup (const char *nam);
struct sym * salloc (void);
RETSIGTYPE fptrap (int sig);

/* in scanner or parser */
int yyerror (const char *msg);
int yylex (void);
int yyparse (void);
struct lex * lalloc (void);
void concat (struct expr *e0, struct expr *e1);
void lfree (struct lex *lp);
void prexq (const struct expr *ep);

/* math library exception handling */
#ifdef HAVE_MATHERR
# ifdef __cplusplus
int matherr (struct __exception *x);
#else
int matherr (struct exception *x);
#endif
#endif

/* math functions in bessel.c and specfun.c */
#ifndef HAVE_J0
double j0 (double x);
double j1 (double x);
double y0 (double x);
double y1 (double x);
#endif
#ifdef NO_SYSTEM_GAMMA
double f_lgamma (double x);
#else  /* not NO_SYSTEM_GAMMA, we link in vendor code */
#ifdef HAVE_LGAMMA
extern double lgamma (double x); /* declaration may be gratuitous */
#endif
#ifdef HAVE_GAMMA
extern double gamma (double x); /* declaration may be gratuitous */
#endif
#endif
double f_gamma (double x);
#ifndef HAVE_ERF
double erf (double x);
double erfc (double x);
#endif
double ibeta (double a, double b, double x);
double igamma (double a, double x);
double inverf (double p);
double invnorm (double p);
double norm (double x);

/* declare functions in libcommon */
#include "libcommon.h"

/* support C++ */
#ifdef __BEGIN_DECLS
#undef __BEGIN_DECLS
#endif
#ifdef __END_DECLS
#undef __END_DECLS
#endif
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS		/* empty */
# define __END_DECLS		/* empty */
#endif

__BEGIN_DECLS
int yywrap (void);
__END_DECLS     
