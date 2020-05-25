/* This file is part of the GNU plotutils package. */

/* This file is part of the GNU plotutils package.  Copyright (C) 1989,
   1996, 1997, 1998, 1999, 2005, 2008, 2009, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/*
 * main() routine for ode, including command-line parser.
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"
#include "getopt.h"

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] =
{
  {"input-file",		ARG_REQUIRED,	NULL, 'f'},
  {"precision",			ARG_REQUIRED,	NULL, 'p'},
  /* integration algorithms */
  {"adams-moulton",		ARG_OPTIONAL,	NULL, 'A'}, /* 0 or 1 */
  {"euler",			ARG_OPTIONAL,	NULL, 'E'}, /* 0 or 1 */
  {"runge-kutta",		ARG_OPTIONAL,	NULL, 'R'}, /* 0 or 1 */
  /* error bounds */
  {"absolute-error-bound",	ARG_REQUIRED,	NULL, 'e'}, /* 1 or 2 */
  {"step-size-bound",		ARG_REQUIRED,	NULL, 'h'}, /* 1 or 2 */
  {"relative-error-bound",	ARG_REQUIRED,	NULL, 'r'}, /* 1 or 2 */
  {"suppress-error-bound",	ARG_NONE,	NULL, 's'},
  {"title",			ARG_NONE,	NULL, 't'},
  /* Long options with no equivalent short option alias */
  {"version",			ARG_NONE,	NULL, 'V' << 8},
  {"help",			ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { 0 };

/* forward references */
static void fatal (const char *s);

/*
 * fatal error message
 */
static void
fatal (const char *s)
{
  fprintf (stderr, "%s: %s\n", progname, s);
  exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  bool show_version = false;	/* remember to show version message */
  bool show_usage = false;	/* remember whether to output usage message. */
  double local_tstep, local_hmax;
  FILE *infile = NULL;

  for ( ; ; )
    {
      option = getopt_long (argc, argv, "e:f:h:p:r:stA::E::R::V", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 's':		/* Suppress error bound, ARG NONE */
	  sflag = true;
	  break;
	case 't':		/* Title, ARG NONE		*/
	  tflag = true;
	  if (!pflag) 
	    {
	      prec = 6;
	      fwd = 13;
	    }
	  break;
	case 'V' << 8:		/* Version, ARG NONE		*/
	  show_version = true;
	  break;
	case 'h' << 8:		/* Help, ARG NONE		*/
	  show_usage = true;
	  break;

	  /*----------- options with a single argument --------------*/

	case 'f':		/* File name, ARG REQUIRED	*/
	  filename = xstrdup (optarg);
	  break;
	case 'p':		/* Precision, ARG REQUIRED 	*/
	  pflag = true;
	  if (sscanf (optarg, "%d", &prec) <= 0)
	    fatal ("-p: bad argument");
	  prec--;
	  if (prec <= 0 || prec > 18)
	    fatal ("-p: argument must be in the range 2..19");
	  fwd = prec + 7;
	  if (fwd < 9)
	    fwd = 9;
	  break;

	  /*----------- options with 0 or 1 arguments --------------*/

	case 'A':		/* Adams-Moulton */
	  algorithm = A_ADAMS_MOULTON;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  conflag = true;
	  break;
	case 'E':		/* Euler */
	  algorithm = A_EULER;
	  conflag = true;
	  tstep = 0.1;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  break;
	case 'R':		/* Runge-Kutta-Fehlberg */
	  algorithm = A_RUNGE_KUTTA_FEHLBERG;
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv[optind], "%lf", &local_tstep) <= 0)
	    break;
	  tstep = local_tstep;
	  optind++;	/* tell getopt we recognized timestep */
	  conflag = true;
	  break;

	  /*----------- options with 1 or 2 arguments --------------*/

	case 'h':		/* Step Size Bound(s) */
	  if (sscanf (optarg, "%lf", &hmin) <= 0)
	    fatal ("-h: bad argument");
	  if (hmin < HMIN)
	    fatal ("-h: value too small");
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv [optind], "%lf", &local_hmax) <= 0)
	    break;
	  hmax = local_hmax;
	  optind++;	/* tell getopt we recognized hmax */
	  hflag = true;
	  break;

	case 'r':		/* Relative Error Bound(s) */
	  rflag = true;
	  if (sscanf (optarg, "%lf", &ssmax) <= 0)
	    fatal ("-r: bad argument");
	  if (ssmax < HMIN)
	    fatal ("-r: max value too small");
	  if (optind >= argc)
	    break;
	  /* try to parse next arg as a float */
	  if (sscanf (argv [optind], "%lf", &ssmin) <= 0)
	    {
	      ssmin = ssmax * SCALE;	      
	      break;
	    }
	  optind++;	/* tell getopt we recognized ssmin */
	  break;

	case 'e':		/* Absolute Error Bound(s) */
	  eflag = true;
	  if (sscanf (optarg, "%lf", &abmax) <= 0)
	    fatal ("-e: bad argument");
	  if (abmax < HMIN)
	    fatal ("-e: max value too small");
	  if (optind >= argc)
	  /* try to parse next arg as a float */
	    break;
	  if (sscanf (argv [optind], "%lf", &abmin) <= 0)
	    {
	      abmin = abmax * SCALE;
	      break;
	    }
	  optind++;	/* tell getopt we recognized abmin */
	  break;

	  /*---------------- End of options ----------------*/

	default:		/* Default, unknown option */
	  errcnt++;
	  break;
	}			/* endswitch */

      if ((option == EOF))
	{
	  errcnt--;
	  break;		/* break out of option processing */
	}
    }				/* endwhile */

  if (optind < argc)		/* too many arguments */
    {
      fprintf (stderr, "%s: there are too many arguments\n", progname);
      errcnt++;
    }
  
  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information\n", progname);
      return EXIT_FAILURE;
    }
  if (show_version)
    {
      display_version (progname, written, copyright);
      return EXIT_SUCCESS;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, NULL, 0);
      return EXIT_SUCCESS;
    }

  /* Some sanity checks on user-supplied options. */

  if (algorithm == A_EULER && (eflag || rflag))
    fatal ("-E [Euler] illegal with -e or -r");

  /* DO IT */

  if (filename != NULL)
    {
      infile = fopen (filename, "r");
      if (infile == NULL)
	{
	  fprintf (stderr, "%s: %s: %s\n", progname, filename, strerror(errno));
	  return EXIT_FAILURE;
	}
      yyin = infile;
      /* will switch later to stdin, in yywrap() */
    }
  else
    {
      yyin = stdin;
      filename = "";
    }
  
  yyparse();
  return EXIT_SUCCESS;
}
