%{
/* Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1999, 2005, 2008, Free Software 
 * Foundation, Inc.
 */
/*
 * Grammar for ode:
 * Most things are self-explanatory.
 * When you're done with a lexptr-type object
 * you should free it with lfree.  They are
 * used for passing constants around while parsing
 * (computing the value of) a cexpr.  The macros
 * for evaluating operators and functions are the
 * most important thing to be familiar with before
 * toying with the semantics.
 */
#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/*
 * Value is true iff operands pass ONECON.
 */
#define TWOCON(x,y) (ONECON(x) && ONECON(y))
#define THREECON(x,y,z) (ONECON(x) && ONECON(y) && ONECON(z))
/*
 * Value must be a (struct expr *).  Returns true if its arg, when
 * evaluated, would put a constant on the stack.
 */
#define ONECON(x) (x->ex_oper == O_CONST && x->ex_next == NULL)
/*
 * Performs ordinary binary arithmetic, when there are two constants in an
 * expr (`op' is a C operator that includes an assignment, e.g., +=).
 */
#define COMBINE(x,y,r,op) {x->ex_value op y->ex_value; efree(y); r = x;}
/*
 * Generates stack code for a binary operation, as for a dyadic operator in
 * an expression.  E.g., op = O_PLUS.
 */
#define BINARY(x,y,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(y,ep);\
                concat(r=x,y);}
/*
 * Generates stack code for a ternary operation, as for a triadic operator in
 * an expression.  E.g., op = O_IBETA.
 */
#define TERNARY(x,y,z,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(z,ep);\
                concat(y,z);\
                concat(r=x,y);}
/*
 * Performs ordinary unary arithmetic, when there is a constant in an expr.
 * "-" seems to work as a monadic operator.
 */
#define CONFUNC(x,r,f) {x->ex_value = f(x->ex_value); r = x;}
/*
 * Generates stack code for a unary operation, as for a monadic operator in
 * an expression.
 */
#define UNARY(oprnd,r,op) {struct expr *ep=ealloc();\
                ep->ex_oper = op;\
                concat(r=oprnd,ep);}
/*
 * Performs binary arithmetic in a cexpr (`op' is a C operator that
 * includes an assignment, e.g. +=).
 */
#define CEXOP(x,y,r,op) {x->lx_u.lxu_value op y->lx_u.lxu_value;\
                lfree(y);\
                r = x;}
/*
 * Performs unary arithmetic in a cexpr.
 */
#define CEXFUNC(x,r,f) {x->lx_u.lxu_value = f(x->lx_u.lxu_value); r=x;}

/*
 * A hook for future upgrades in error reporting
 */
static  char    *errmess = NULL;

bool erritem;
%}
%union {
        struct  lex     *lexptr;
        struct  expr    *exprptr;
        struct  prt     *prtptr;
        int     simple;
}
%token <lexptr> NUMBER IDENT SEP
%token ABS SQRT EXP LOG LOG10 
%token SIN COS TAN ASIN ACOS ATAN 
%token SINH COSH TANH ASINH ACOSH ATANH
%token FLOOR CEIL J0 J1 Y0 Y1
%token LGAMMA GAMMA ERF ERFC INVERF NORM INVNORM
%token IGAMMA IBETA
%token EVERY FROM PRINT STEP EXAM
%start program
%type <simple> prttag
%type <lexptr> cexpr
%type <exprptr> expr
%type <prtptr> prtitem
%nonassoc '='
%left '+' '-'
%left '*' '/'
%right '^'
%right UMINUS
%%
program         : stat
                | program stat
                ;

stat            : SEP
                        { lfree($1); }
                | IDENT '=' expr SEP
                        {
			  struct sym *sp;
			  
			  sp = lookup($1->lx_u.lxu_name);
			  sp->sy_value = eval($3);
			  sp->sy_flags |= SF_INIT;
			  lfree($1);
			  efree($3);
			  lfree($4);
                        }
                | error SEP
                        {
			  if (errmess == NULL)
			    errmess = "syntax error";
			  fprintf (stderr, "%s:%s:%d: %s\n", 
				   progname, filename,
				   ($2->lx_lino), errmess);
			  errmess = NULL;
			  lfree($2);
			  yyerrok;
			  yyclearin;
                        }
                | IDENT '\'' '=' expr SEP
                        {
			  struct sym *sp;
			  struct prt *pp, *qp;
			  
			  sp = lookup($1->lx_u.lxu_name);
			  efree(sp->sy_expr);
			  sp->sy_expr = $4;
			  sp->sy_flags |= SF_ISEQN;
			  if (!sawprint) 
			    {
			      for (pp=pqueue; pp!=NULL; pp=pp->pr_link)
				if (pp->pr_sym == sp)
				  goto found;
			      pp = palloc();
			      pp->pr_sym = sp;
			      if (pqueue == NULL)
				pqueue = pp;
			      else 
				{
				  for (qp=pqueue; qp->pr_link!=NULL; )
				    qp = qp->pr_link;
				  qp->pr_link = pp;
                                }
			    }
			found:
			  lfree($1);
			  lfree($5);
                        }
                | PRINT prtlist optevery optfrom SEP
                        {
			  sawprint = true;
			  prerr = erritem;
			  erritem = false;
			  lfree($5);
                        }
                | STEP cexpr ',' cexpr SEP
                        {
			  lfree($5);
			  tstart = $2->lx_u.lxu_value;
			  lfree($2);
			  tstop = $4->lx_u.lxu_value;
			  lfree($4);
			  if (!conflag)
			    startstep();
			  solve();
			  sawstep = true;
                        }
                | STEP cexpr ',' cexpr ',' cexpr SEP
                        {
			  double savstep;
			  bool savconflag;
			  
			  lfree($7);
			  tstart = $2->lx_u.lxu_value;
			  lfree($2);
			  tstop = $4->lx_u.lxu_value;
			  lfree($4);
			  savstep = tstep;
			  tstep = $6->lx_u.lxu_value;
			  lfree($6);
			  savconflag = conflag;
			  conflag = true;
			  solve();
			  tstep = savstep;
			  conflag = savconflag;
			  sawstep = true;
                        }
                | EXAM IDENT SEP
                        {
			  struct sym *sp;
			  
			  lfree($3);
			  sp = lookup($2->lx_u.lxu_name);
			  lfree($2);
			  printf ("\"%.*s\" is ",NAMMAX,sp->sy_name);
			  switch (sp->sy_flags & SF_DEPV)
			    {
			    case SF_DEPV:
			    case SF_ISEQN:
			      printf ("a dynamic variable\n");
			      break;
			    case SF_INIT:
			      printf ("an initialized constant\n");
			      break;
			    case 0:
			      printf ("an uninitialized constant\n");
			      break;
			    default:
			      panicn ("impossible (%d) in EXAM action",
				      sp->sy_flags);
			    }
			  printf ("value:");
			  prval (sp->sy_value);
			  printf ("\nprime:");
			  prval (sp->sy_prime);
			  printf ("\nsserr:");
			  prval (sp->sy_sserr);
			  printf ("\naberr:");
			  prval (sp->sy_aberr);
			  printf ("\nacerr:");
			  prval (sp->sy_acerr);
			  putchar ('\n');
			  prexq(sp->sy_expr);
			  fflush(stdout);
                        }
                ;

prtlist         : prtitem
                        {
			  pfree(pqueue);
			  pqueue = $1;
                        }
                | prtlist ',' prtitem
                        {
			  struct prt *pp;
			  
			  for (pp=pqueue; pp->pr_link!=NULL; pp=pp->pr_link)
			    ;
			  pp->pr_link = $3;
                        }
                ;

prtitem         : IDENT prttag
                        {
			  struct prt *pp;
			  
			  pp = palloc();
			  pp->pr_sym = lookup($1->lx_u.lxu_name);
			  pp->pr_which = (ent_cell)($2);
			  lfree($1);
			  $$ = pp;
                        }
                ;

prttag          : /* empty */
                        { $$ = P_VALUE; }
                | '\''
                        { $$ = P_PRIME; }
                | '~'
                        {
			  $$ = P_ACERR;
			  erritem = true;
                        }
                | '!'
                        {
			  $$ = P_ABERR;
			  erritem = true;
                        }
                | '?'
                        {
			  $$ = P_SSERR;
			  erritem = true;
                        }
                ;

optevery        : /* empty */
                        { sawevery = false; }
                | EVERY cexpr
                        {
                        sawevery = true;
                        tevery = IROUND($2->lx_u.lxu_value);
                        lfree($2);
                        }
                ;

optfrom         : /* empty */
                        { sawfrom = false; }
                | FROM cexpr
                        {
			  sawfrom = true;
			  tfrom = $2->lx_u.lxu_value;
			  lfree($2);
                        }
                ;

cexpr           : '(' cexpr ')'
                        {
			  $$ = $2;
                        }
                | cexpr '+' cexpr
                        {
			  CEXOP($1,$3,$$,+=)
                        }
                | cexpr '-' cexpr
                        {
			  CEXOP($1,$3,$$,-=)
                        }
                | cexpr '*' cexpr
                        {
			  CEXOP($1,$3,$$,*=)
                        }
                | cexpr '/' cexpr
                        {
			  CEXOP($1,$3,$$,/=)
                        }
                | cexpr '^' cexpr
                        {
			  $1->lx_u.lxu_value =
			    pow($1->lx_u.lxu_value,$3->lx_u.lxu_value);
			  lfree($3);
			  $$ = $1;
                        }
                | SQRT '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,sqrt)
                        }
                | ABS '(' cexpr ')'
                        {
			  if ($3->lx_u.lxu_value < 0)
			    $3->lx_u.lxu_value = -($3->lx_u.lxu_value);
			  $$ = $3;
                        }
                | EXP '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,exp)
                        }
                | LOG '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,log)
                        }
                | LOG10 '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,log10)
                        }
                | SIN '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,sin)
                        }
                | COS '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,cos)
                        }
                | TAN '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,tan)
                        }
                | ASINH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,asinh)
                        }
                | ACOSH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,acosh)
                        }
                | ATANH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,atanh)
                        }
                | ASIN '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,asin)
                        }
                | ACOS '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,acos)
                        }
                | ATAN '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,atan)
                        }
                | SINH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,sinh)
                        }
                | COSH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,cosh)
                        }
                | TANH '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,tanh)
                        }
                | FLOOR '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,floor)
                        }
                | CEIL '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,ceil)
                        }
                | J0 '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,j0)
                        }
                | J1 '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,j1)
                        }
                | Y0 '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,y0)
                        }
                | Y1 '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,y1)
                        }
                | ERFC '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,erfc)
                        }
                | ERF '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,erf)
                        }
                | INVERF '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,inverf)
                        }
                | LGAMMA '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,F_LGAMMA)
                        }
                | GAMMA '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,f_gamma)
                        }
                | NORM '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,norm)
                        }
                | INVNORM '(' cexpr ')'
                        {
			  CEXFUNC($3,$$,invnorm)
                        }
                | IGAMMA '(' cexpr ',' cexpr ')'
                        {
			  $3->lx_u.lxu_value =
			    igamma($3->lx_u.lxu_value,$5->lx_u.lxu_value);
			  lfree($5);
			  $$ = $3;
                        }
                | IBETA '(' cexpr ',' cexpr ',' cexpr ')'
                        {
			  $3->lx_u.lxu_value =
			    ibeta($3->lx_u.lxu_value,$5->lx_u.lxu_value,$7->lx_u.lxu_value);
			  lfree($5);
			  lfree($7);
			  $$ = $3;
                        }
                | '-' cexpr     %prec UMINUS
                        {
			  CEXFUNC($2,$$,-)
                        }
                | NUMBER
                        { $$ = $1; }
                ;

expr            : '(' expr ')'
                        { $$ = $2; }
                | expr '+' expr
                        {
			  if (TWOCON($1,$3))
			    COMBINE($1,$3,$$,+=)
			  else
			    BINARY($1,$3,$$,O_PLUS);
                        }
                | expr '-' expr
                        {
			  if (TWOCON($1,$3))
			    COMBINE($1,$3,$$,-=)
			  else
			    BINARY($1,$3,$$,O_MINUS);
                        }
                | expr '*' expr
                        {
			  if (TWOCON($1,$3))
			    COMBINE($1,$3,$$,*=)
			  else
			    BINARY($1,$3,$$,O_MULT);
                        }
                | expr '/' expr
                        {
			  if (TWOCON($1,$3))
			    COMBINE($1,$3,$$,/=)
			  else if (ONECON($3) && $3->ex_value!=0.) 
			    {
			      /* division by constant */
			      $3->ex_value = 1./$3->ex_value;
			      BINARY($1,$3,$$,O_MULT);
			    } 
			  else
			    BINARY($1,$3,$$,O_DIV);
                        }
                | expr '^' expr
                        {
			  double f;
			  bool invert = false;
			  
			  if (TWOCON($1,$3)) 
			    {
			      /* case const ^ const */
			      $1->ex_value = pow($1->ex_value,$3->ex_value);
			      efree($3);
			    } 
			  else if (ONECON($1)) 
			    {
			      if ($1->ex_value == 1.)
				{
				  /* case 1 ^ x */
				  efree($3);
				  $$ = $1;
                                }
			      else
				goto other;
			    }
			  else if (!ONECON($3))
			    goto other;
			  else 
			    {
			      f = $3->ex_value;
			      if (f < 0.) 
				{
				  /*
				   * negative exponent means
				   * to append an invert cmd
				   */
				  f = -f;
				  invert = true;
                                }
			      if (f == 2.) 
				{
				  /* case x ^ 2 */
				  $3->ex_oper = O_SQAR;
				  concat($1,$3);
				  $$ = $1;
                                }
			      else if (f == 3.) 
				{
				  /* case x ^ 3 */
				  $3->ex_oper = O_CUBE;
				  concat($1,$3);
				  $$ = $1;
                                }
			      else if (f == 0.5) 
				{
				  /* case x ^ .5 */
				  $3->ex_oper = O_SQRT;
				  concat($1,$3);
				  $$ = $1;
                                }
			      else if (f == 1.5) 
				{
				  /* case x ^ 1.5 */
				  $3->ex_oper = O_CUBE;
				  BINARY($1,$3,$$,O_SQRT);
                                }
			      else if (f == 1.) 
				{
				  /* case x ^ 1 */
				  efree($3);
				  $$ = $1;
                                }
			      else if (f == 0.) 
				{
				  /* case x ^ 0 */
				  efree($1);
				  $3->ex_value = 1.;
				  $$ = $3;
                                } 
			      else 
				{
				other:
				  /* default */
				  invert = false;
				  BINARY($1,$3,$$,O_POWER);
                                }
			      if (invert)
				UNARY($$,$$,O_INV)
			    }
                        }
                | SQRT '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,sqrt)
			  else
			    UNARY($3,$$,O_SQRT);
                        }
                | ABS '(' expr ')'
                        {
			  if (ONECON($3)) 
			    {
			      if ($3->ex_value < 0)
				$3->ex_value = -($3->ex_value);
			      $$ = $3;
			  } 
			  else
			    UNARY($3,$$,O_ABS);
                        }
                | EXP '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,exp)
			  else
			    UNARY($3,$$,O_EXP);
                        }
                | LOG '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,log)
			  else
			    UNARY($3,$$,O_LOG);
                        }
                | LOG10 '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,log10)
			  else
			    UNARY($3,$$,O_LOG10);
                        }
                | SIN '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,sin)
			  else
			    UNARY($3,$$,O_SIN);
                        }
                | COS '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,cos)
			  else
			    UNARY($3,$$,O_COS);
                        }
                | TAN '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,tan)
			  else
			    UNARY($3,$$,O_TAN);
                        }
                | ASINH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,asinh)
			  else
			    UNARY($3,$$,O_ASINH);
                        }
                | ACOSH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,acosh)
			  else
			    UNARY($3,$$,O_ACOSH);
                        }
                | ATANH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,atanh)
			  else
			    UNARY($3,$$,O_ATANH);
                        }
                | ASIN '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,asin)
			  else
			    UNARY($3,$$,O_ASIN);
                        }
                | ACOS '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,acos)
			  else
			    UNARY($3,$$,O_ACOS);
                        }
                | ATAN '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,atan)
			  else
			    UNARY($3,$$,O_ATAN);
                        }
                | SINH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,sinh)
			  else
			    UNARY($3,$$,O_SINH);
                        }
                | COSH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,cosh)
			  else
			    UNARY($3,$$,O_COSH);
                        }
                | TANH '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,tanh)
			  else
			    UNARY($3,$$,O_TANH);
                        }
                | FLOOR '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,floor)
			  else
			    UNARY($3,$$,O_FLOOR);
                        }
                | CEIL '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,ceil)
			  else
			    UNARY($3,$$,O_CEIL);
                        }
                | J0 '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,j0)
			  else
			    UNARY($3,$$,O_J0);
                        }
                | J1 '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,j1)
			  else
			    UNARY($3,$$,O_J1);
                        }
                | Y0 '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,y0)
			  else
			    UNARY($3,$$,O_Y0);
                        }
                | Y1 '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,y1)
			  else
			    UNARY($3,$$,O_Y1);
                        }
                | LGAMMA '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,F_LGAMMA)
			  else
			    UNARY($3,$$,O_LGAMMA);
                        }
                | GAMMA '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,f_gamma)
			  else
			    UNARY($3,$$,O_GAMMA);
                        }
                | ERFC '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,erfc)
			  else
			    UNARY($3,$$,O_ERFC);
                        }
                | ERF '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,erf)
			  else
			    UNARY($3,$$,O_ERF);
                        }
                | INVERF '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,inverf)
			  else
			    UNARY($3,$$,O_INVERF);
                        }
                | NORM '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,norm)
			  else
			    UNARY($3,$$,O_NORM);
                        }
                | INVNORM '(' expr ')'
                        {
			  if (ONECON($3))
			    CONFUNC($3,$$,invnorm)
			  else
			    UNARY($3,$$,O_INVNORM);
                        }
                | IGAMMA '(' expr ',' expr ')'
                        {
			  if (TWOCON($3,$5)) 
			    {
			      $3->ex_value = 
				igamma($3->ex_value,$5->ex_value);
			      efree($5);
			      $$ = $3;
			    }
			  else 
			    BINARY($3,$5,$$,O_IGAMMA);
		        }
                | IBETA '(' expr ',' expr ',' expr ')'
                        {
			  if (THREECON($3,$5,$7)) 
			    {
			      $3->ex_value = 
				ibeta($3->ex_value,$5->ex_value,$7->ex_value);
			      efree($5);
			      efree($7);
			      $$ = $3;
			    }
			  else 
			    TERNARY($3,$5,$7,$$,O_IBETA);
		        }
                | '-' expr      %prec UMINUS
                        {
			  if (ONECON($2))
			    CONFUNC($2,$$,-)
			  else
			    UNARY($2,$$,O_NEG);
                        }
                | NUMBER
                        {
			  $$ = ealloc();
			  $$->ex_oper = O_CONST;
			  $$->ex_value = $1->lx_u.lxu_value;
			  lfree($1);
                        }
                | IDENT
                        {
			  $$ = ealloc();
			  $$->ex_oper = O_IDENT;
			  $$->ex_sym = lookup($1->lx_u.lxu_name);
			  lfree($1);
                        }
                ;
%%

int
yyerror (const char *s)
{
  return 0;
}

/*
 * tack two queues of stack code together
 * e1 is connected on the tail of e0
 * There is no good way to test for circular
 * lists, hence the silly count.
 */
void
concat (struct expr *e0, struct expr *e1)
{
  int count;
  
  if (e0 == NULL || e1 == NULL) 
    panic ("NULL expression queue");

  for (count = 0; e0->ex_next != NULL; e0 = e0->ex_next)
    if (++count > 10000) 
      panic ("circular expression queue");

  e0->ex_next = e1;
}

/*
 * print an expression queue
 * called when EXAMINE is invoked on a variable (see above)
 */
void
prexq (const struct expr *ep)
{
  const char *s;
  
  printf (" code:");
  if (ep == NULL)
    putchar ('\n');

  for (; ep != NULL; ep = ep->ex_next) 
    {
      switch (ep->ex_oper) 
	{
	case O_PLUS: s = "add"; break;
	case O_MINUS: s = "subtract"; break;
	case O_MULT: s = "multiply"; break;
	case O_DIV: s = "divide"; break;
	case O_POWER: s = "power"; break;
	case O_SQRT: s = "sqrt"; break;
	case O_EXP: s = "exp"; break;
	case O_LOG: s = "log"; break;
	case O_LOG10: s = "log10"; break;
	case O_SIN: s = "sin"; break;
	case O_COS: s = "cos"; break;
	case O_TAN: s = "cos"; break;
	case O_ASIN: s = "sin"; break;
	case O_ACOS: s = "cos"; break;
	case O_ATAN: s = "cos"; break;
	case O_NEG: s = "negate"; break;
	case O_ABS: s = "abs"; break;
	case O_SINH: s = "sinh"; break;
	case O_COSH: s = "cosh"; break;
	case O_TANH: s = "tanh"; break;
	case O_ASINH: s = "asinh"; break;
	case O_ACOSH: s = "acosh"; break;
	case O_ATANH: s = "atanh"; break;
	case O_SQAR: s = "square"; break;
	case O_CUBE: s = "cube"; break;
	case O_INV: s = "invert"; break;
	case O_FLOOR: s = "floor"; break;
	case O_CEIL: s = "ceil"; break;
	case O_J0: s = "besj0"; break;
	case O_J1: s = "besj1"; break;
	case O_Y0: s = "besy0"; break;
	case O_Y1: s = "besy1"; break;
	case O_ERF: s = "erf"; break;
	case O_ERFC: s = "erfc"; break;
	case O_INVERF: s = "inverf"; break;
	case O_LGAMMA: s = "lgamma"; break;
	case O_GAMMA: s = "gamma"; break;
	case O_NORM: s = "norm"; break;
	case O_INVNORM: s = "invnorm"; break;
	case O_IGAMMA: s = "igamma"; break;
	case O_IBETA: s = "ibeta"; break;
	case O_CONST:
	  printf ("\tpush ");
	  prval (ep->ex_value);
	  putchar ('\n');
	  s = NULL;
	  break;
	case O_IDENT:
	  printf ("\tpush \"%.*s\"\n",
		 NAMMAX, ep->ex_sym->sy_name);
	  s = NULL;
	  break;
	default: s = "unknown!";
	}

      if (s != NULL)
	printf ("\t%s\n",s);
    }
}
