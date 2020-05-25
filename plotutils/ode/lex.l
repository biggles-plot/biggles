%C
%{
/*
 * Copyright Nicholas B. Tufillaro, 1982-1994. All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free 
 * Software Foundation, Inc.
 */
#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include "gram.h"

int curline = 1;
%}
Dig		[0-9]
Expo		([eE][-+]?{Dig}{Dig}?{Dig}?)
%%
\#.*$		;	/* N.B.: comments can't be continued */
\\\n		{
			curline++;
		}
[ \t]		;
[;\n]		{
			yylval.lexptr = lalloc();
			yylval.lexptr->lx_lino = curline;
			if (*yytext == '\n')
			  curline++;
			return SEP;
		}
PI		{
			yylval.lexptr = lalloc();
			yylval.lexptr->lx_u.lxu_value =	M_PI;
			return NUMBER;
		}
^\.$		{
			/* lonely . isn't EOF in a -f file */
			if (yyin == stdin)
			  return EOF;
			REJECT;
		}
every		{ return EVERY; }
from		{ return FROM; }
print		{ return PRINT; }
step		{ return STEP; }
examine		{ return EXAM; }
abs		{ return ABS; }
sqrt		{ return SQRT; }
exp		{ return EXP; }
log10		{ return LOG10; }
ln		{ return LOG; }
log		{ return LOG; }
sin		{ return SIN; }
cos		{ return COS; }
tan		{ return TAN; }
asinh		{ return ASINH; }
acosh		{ return ACOSH; }
atanh		{ return ATANH; }
asin		{ return ASIN; }
acos		{ return ACOS; }
atan		{ return ATAN; }
sinh		{ return SINH; }
cosh		{ return COSH; }
tanh		{ return TANH; }
floor		{ return FLOOR; }
ceil		{ return CEIL; }
besj0		{ return J0; }
besj1		{ return J1; }
besy0		{ return Y0; }
besy1		{ return Y1; }
erfc		{ return ERFC; }
erf		{ return ERF; }
inverf		{ return INVERF; }
lgamma		{ return LGAMMA; }
gamma		{ return GAMMA; }
norm		{ return NORM; }
invnorm		{ return INVNORM; }
ibeta		{ return IBETA; }
igamma		{ return IGAMMA; }
[_a-zA-Z][_a-zA-Z0-9]*		{
			yylval.lexptr = lalloc();
			strncpy(yylval.lexptr->lx_u.lxu_name,yytext,NAMMAX);
			return IDENT;
		}
{Dig}+{Expo}?		|
{Dig}+"."{Dig}*{Expo}?	|
{Dig}*"."{Dig}+{Expo}?	{
			/*
			 * restrictions on numbers:
			 * radix 10 only
			 * a bare Expo isn't enough (matches IDENT)
			 */
			yylval.lexptr = lalloc();
			yylval.lexptr->lx_u.lxu_value = atof(yytext);
			return NUMBER;
		}
[-+*/^()',=?~!]	{
			/*
			 * accept as few as possible so the error
			 * message can be a smart one
			 */
			return yytext[0];
		}
.		{
			if (*yytext > '~' || *yytext < ' ')
				fprintf (stderr, 
					"%s:%s:%d: bad char '\\%02o'\n",
					progname, filename, 
					curline, *yytext&0377);
			else
				fprintf (stderr, 
					"%s:%s:%d: bad char '%c'\n",
					progname, filename,
					curline, *yytext);
			return *yytext;
		}
%%
/*
 * space management for the lexer
 */
struct lex *
lalloc (void)
{
  struct lex *lp;
  
  lp = (struct lex *)xmalloc (sizeof(struct lex));
  lp->lx_u.lxu_value = 0.0;
  lp->lx_lino = 0;
  return lp;
}

void
lfree (struct lex *lp)
{
  if (lp != NULL)
    free ((void *)lp);
}
