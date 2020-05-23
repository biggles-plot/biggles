/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, 2008, 2009, Free Software
 * Foundation, Inc.
 */

/*
 * definitions of global variables for ode.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

/* defined and initialized */

const char    *progname	= "ode"; /* name of this program */
const char    *written =  "Written by Nicholas B. Tufillaro and Robert S. Maier.";
const char    *copyright = "Copyright (C) 1994 Nicholas B. Tufillaro,\nCopyright (C) 2009 Free Software Foundation, Inc.";

int	prec	= -1;
long	it	= 0;
double	hmin	= HMIN;
double	hmax	= HMAX;
double	ssmin	= 1e-11;
double	ssmax	= 1e-8;
double	abmin	= 1e-36;
double	abmax	= 1e36;
double	acmax	= 1e36;
struct	sym	*symtab = NULL;
struct	sym	*fsp	= NULL;
struct	sym	*dqueue	= NULL;
struct	prt	*pqueue = NULL;
struct	expr	exprzero  = 
{
  O_CONST,
  0.,
  NULL,
  NULL,
};
struct	expr	exprone = 
{
  O_CONST,
  1.,
  NULL,
  NULL,
};
bool     sawstep = false, sawprint = false;
bool	    sawevery = false, sawfrom = false;
bool     tflag = false, pflag = false, sflag = false;
bool     eflag = false, rflag = false, hflag = false, conflag = false;
integration_type algorithm = A_RUNGE_KUTTA_FEHLBERG;

/* defined but not initialized */

char	*filename;
jmp_buf mark;
int	fwd;
int     tevery;
double  tstart, tstop, tstep, tfrom;
bool printnum, prerr;
