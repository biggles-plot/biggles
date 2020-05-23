/* This file is part of the GNU plotutils package.  Copyright (C) 1989,
   1990, 1991, 1995, 1996, 1997, 1998, 1999, 2000, 2005, 2008, 2009, Free
   Software Foundation, Inc.

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

/* This program, spline, interpolates scalar or vector-valued input data
   using splines with tension, including piecewise cubic (zero-tension)
   splines.  When acting as a real-time filter, it uses cubic Bessel
   interpolation instead.  Written by Robert S. Maier
   <rsm@math.arizona.edu>, based on earlier work by Rich Murphey.

   References:

   D. Kincaid and [E.] W. Cheney, Numerical Analysis, Brooks/Cole,
   2nd. ed., 1996, Section 6.4.

   C. de Boor, A Practical Guide to Splines, Springer-Verlag, 1978, 
   Chapter 4.

   A. K. Cline, "Scalar and Planar-Valued Curve Fitting Using Splines under
   Tension", Communications of the ACM 17 (1974), 218-223.

   The tension in a spline is set with the -T (i.e., --tension) option.  By
   definition, a one-dimensional spline with tension satisfies the
   differential equation y''''=sgn(tension)*(tension**2)y''.  The default
   value for the tension is zero.  If tension=0 then a spline with tension
   reduces to a conventional piecewise cubic spline.  In the limits
   tension->+infinity and tension->-infinity, a spline with tension reduces
   to a piecewise linear (`broken line') interpolation.

   To oversimplify a bit, 1.0/tension is the maximum abscissa range over
   which the spline likes to curve, at least when tension>0.  So increasing
   the tension far above zero tends to make the spline contain short curved
   sections, separated by sections that are almost straight.  The curved
   sections will be centered on the user-specified data points.  The
   behavior of the spline when tension<0 is altogether different: it will
   tend to oscillate, though as tension->-infinity the oscillations are
   damped out.

   Tension is a `dimensionful' quantity.  If tension=0 (the cubic spline
   case), then the computation of the spline is scale-invariant.  But if
   the tension is nonzero, then when the abscissa values are multiplied by
   some common positive factor, the tension should be divided by the same
   factor to obtain a scaled version of the original spline.

   The algorithms of Kincaid and Cheney have been extended to include
   support for periodicity.  To obtain a periodic spline, with or without
   tension, the user uses the -p (i.e., --periodic) option and supplies
   input data satisfying y[n]=y[0].  Also, in the non-periodic case the
   algorithms have been extended to include support for a parameter k,
   which appears in the two boundary conditions y''[0]=ky''[1] and
   y''[n]=ky''[n-1].  The default value of k is 1.0.  The parameter k,
   which is specified with the -k (i.e. --boundary-condition) option, is
   ignored for periodic splines (using the -k option with the -p option
   will elicit a warning).

   If the -f option is specified, then an altogether different (real-time)
   algorithm for generating interpolating points will be used, so that this
   program can be used as a real-time filter.  If -f is specified then the
   -t option, otherwise optional, must also be used.  (I.e., the minimum
   and maximum abscissa values for the interpolating points must be
   specified, and optionally the spacing between them as well.  If the
   spacing is not specified on the command line, then the interval
   [tmin,tmax] will be subdivided into a default number of intervals [100],
   unless the default number of intervals is overridden with the -n option.

   The real-time algorithm that is used when the -f option is specified is
   cubic Bessel interpolation.  (The -T, -p, and -k options are ignored
   when -f is specified; using them will elicit a warning.)  Interpolation
   in this case is piecewise cubic, and the slopes at either end of each
   sub-interval are found by fitting a parabola through each successive
   triple of points.  That is, the slope at t=t_n is found by fitting a
   parabola through the points at t_(n-1), t_n, and t_(n+1).  This
   interpolation scheme yields a spline that is only once, rather than
   twice, continuously differentiable.  However, it has the feature that
   all computations are local rather than global, so it is suitable for
   real-time work.

   Since the above was written, the -d option has been added, to permit the
   splining of multidimensional data.  All components of a d-dimensional
   data set (a d-dimensional vector y is specified at each t) are splined
   in the same way, as if they were one-dimensional functions of t.  All
   options that apply to 1-dimensional datasets, such as -T, -p, -k, -f,
   etc., apply to d-dimensional ones also. */

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"

/* states for cubic Bessel DFA; occupancy of data point queue */
enum { STATE_ZERO, STATE_ONE, STATE_TWO, STATE_THREE };

/* types of auto-abscissa */
enum { AUTO_NONE, AUTO_INCREMENT, AUTO_BY_DISTANCE };

#define FUZZ 0.0000001		/* potential roundoff error */

/* Minimum value for magnitude of x, for such functions as x-sinh(x),
   x-tanh(x), x-sin(x), and x-tan(x) to have acceptable accuracy.  If the
   magnitude of x is smaller than this value, these functions of x will be
   computed via power series to accuracy O(x**6). */
#define TRIG_ARG_MIN 0.001

/* Maximum value for magnitude of x, beyond which we approximate
   x/sinh(x) and x/tanh(x) by |x|exp(-|x|). */
#define TRIG_ARG_MAX 50.0

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "fpsAd:I:O:P:k:n:t:x:T:a::";

struct option long_options[] =
{
  {"no-of-intervals",	ARG_REQUIRED,	NULL, 'n'},
  {"periodic",		ARG_NONE,	NULL, 'p'},
  {"y-dimension",	ARG_REQUIRED,	NULL, 'd'},
  {"t-limits",		ARG_REQUIRED,	NULL, 't'}, /* 1 or 2 or 3 */
  {"t-limits",		ARG_REQUIRED,	NULL, 'x'}, /* obsolescent; hidden */
  {"tension",		ARG_REQUIRED, 	NULL, 'T'},
  {"boundary-condition",ARG_REQUIRED,	NULL, 'k'},
  {"auto-abscissa",	ARG_OPTIONAL,	NULL, 'a'}, /* 0 or 1 or 2 */
  {"auto-dist-abscissa",ARG_NONE,	NULL, 'A'},
  {"filter",		ARG_NONE,	NULL, 'f'},
  {"precision",		ARG_REQUIRED,	NULL, 'P'},
  {"suppress-abscissa",	ARG_NONE,	NULL, 's'},
  /* ascii or double */
  {"input-type",	ARG_REQUIRED,	NULL, 'I'},
  {"output-type",	ARG_REQUIRED,	NULL, 'O'},
  /* Long options with no equivalent short option alias */
  {"version",		ARG_NONE,	NULL, 'V' << 8},
  {"help",		ARG_NONE,	NULL, 'h' << 8},
  {NULL, 		0, 		0,     0}
};

/* null-terminated list of options that we don't show to the user */
const int hidden_options[] = { (int)'x', 0 };

/* type of data in input and output streams */
typedef enum
{
  T_ASCII, T_SINGLE, T_DOUBLE, T_INTEGER
}
data_type;

data_type input_type = T_ASCII;
data_type output_type = T_ASCII;

const char *progname = "spline"; /* name of this program */
const char *written = "Written by Robert S. Maier and Rich Murphey.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

/* forward references */
bool do_bessel (FILE *input, int ydimension, int auto_abscissa, double auto_t, double auto_delta, double first_t, double last_t, double spacing_t, int precision, bool suppress_abscissa);
bool is_monotonic (int n, double *t);
bool read_data (FILE *input, int *len, int *used, int auto_abscissa, double auto_t, double auto_delta, double **t, int ydimension, double **y, double **z);
bool read_float (FILE *input, double *dptr);
bool skip_whitespace (FILE *stream);
bool write_point (double t, double *y, int ydimension, int precision, bool suppress_abscissa);
double interpolate (int n, double *t, double *y, double *z, double x, double tension, bool periodic);
double quotient_sin_func (double x, double y);
double quotient_sinh_func (double x, double y);
double sin_func (double x);
double sinh_func (double x);
double tan_func (double x);
double tanh_func (double x);
int read_point (FILE *input, double *t, double *y, int ydimension, bool *first_point, int auto_abscissa, double *auto_t, double auto_delta, double *stored);
void do_bessel_range (double abscissa0, double abscissa1, double *value0, double *value1, double *slope0, double *slope1, double first_t, double last_t, double spacing_t, int ydimension, int precision, bool endit, bool suppress_abscissa);
void do_spline (int used, int len, double **t, int ydimension, double **y, double **z, double tension, bool periodic, bool spec_boundary_condition, double boundary_condition, int precision, double first_t, double last_t, double spacing_t, int no_of_intervals, bool spec_first_t, bool spec_last_t, bool spec_spacing_t, bool spec_no_of_intervals, bool suppress_abscissa);
void fit (int n, double *t, double *y, double *z, double k, double tension, bool periodic);
void maybe_emit_oob_warning (void);
void non_monotonic_error (void);
void output_dataset_separator (void);
void set_format_type (char *s, data_type *typep);


int
main (int argc, char *argv[])
{
  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered */
  bool show_version = false;	/* remember to show version message */
  bool show_usage = false;	/* remember to output usage message */
  bool dataset_follows;

  /* parameters controlled by command line options: */
  bool filter = false;		/* act as a filter (cubic Bessel)? */
  bool periodic = false;	/* spline should be periodic? */
  bool spec_boundary_condition = false;	/* user-specified boundary cond'n? */
  bool spec_first_t = false, spec_last_t = false, spec_spacing_t = false;
  bool spec_no_of_intervals = false; /* user-specified number of intervals? */
  bool suppress_abscissa = false; /* for each point, print ordinate only? */
  double boundary_condition = 1.0; /* force  y''_1 = k * y''_0, etc. */
  double delta_t = 1.0;		/* increment of auto abscissa */
  double first_t = 0.0, last_t = 0.0, spacing_t = 0.0; /* values of limits */
  double tension = 0.0;		/* `tension' parameter */
  double t_start = 0.0;		/* start of auto abscissa */
  int auto_abscissa = AUTO_NONE; /* automatic generation of abscissa? */
  int no_of_intervals = 100;	/* no. of intervals to divide abs. range */
  int precision = 6;		/* default no. of significant digits printed */
  int ydimension = 1;		/* dimension of each point's ordinate */

  /* used in argument parsing */
  double local_first_t, local_last_t, local_spacing_t;
  double local_t_start, local_delta_t;
  int local_precision;

  for ( ; ; )
    {
      option = getopt_long (argc, argv, optstring, long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 'p':		/* construct periodic, i.e., closed spline */
	  periodic = true;
	  break;
	case 'f':		/* act as filter */
	  filter = true;
	  break;
	case 's':		/* don't output t values */
	  suppress_abscissa = true;
	  break;
	case 'A':		/* delta t = inter-y distance */
	  auto_abscissa = AUTO_BY_DISTANCE;
	  t_start = 0.0;
	  break;
	case 'V' << 8:		/* Version */
	  show_version = true;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = true;
	  break;

	  /*--------------options with a single argument--------*/

	case 'I':
	  set_format_type (optarg, &input_type);
	  break;
	case 'O':
	  set_format_type (optarg, &output_type);
	  break;
	case 'd':		/* dimensionality of ordinate variable */
	  if (sscanf (optarg, "%d", &ydimension) <= 0 || ydimension < 1)
	    {
	      fprintf (stderr, 
		       "%s: error: the ordinate dimension `%s' is bad (it should be a positive integer)\n", 
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'k':
	  if (sscanf (optarg, "%lf", &boundary_condition) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: the boundary condition argument `%s' is bad\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    spec_boundary_condition = true;
	  break;
	case 'T':
	  if (sscanf (optarg, "%lf", &tension) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: the tension argument `%s' is bad\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'n':		/* number of intervals */
	  if (sscanf (optarg, "%d", &no_of_intervals) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: the requested number of intervals `%s' is bad\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    spec_no_of_intervals = true;
	  break;
	case 'P':		/* precision */
	  if (sscanf (optarg, "%d", &local_precision) <= 0)
	    {
	      fprintf (stderr, "%s: error: the requested precision `%s' is bad (it should be a positive integer)\n", 
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      if (local_precision <= 0)
		fprintf (stderr, 
			 "%s: the precision value `%s' is disregarded (it should be a positive integer)\n",
			 progname, optarg);
	      else
		precision = local_precision;
	    }
	  break;

	  /*------------options with 0 or more args ----------*/

	case 'a':		/* Auto-abscissa, ARG OPTIONAL [0,1,2] */
	  auto_abscissa = AUTO_INCREMENT;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_delta_t) <= 0)
	    break;
	  delta_t = local_delta_t;
	  optind++;	/* tell getopt we recognized delta_t */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_t_start) <= 0)
	    break;
	  t_start = local_t_start;
	  optind++;	/* tell getopt we recognized t_start */
	  break;

	  /*--------------options with 1 or more arguments------*/

	case 't':		/* t axis limits, ARG REQUIRED [1,2,3] */
	case 'x':		/* obsolescent variant */
	  if (sscanf (optarg, "%lf", &local_first_t) <= 0)
	    break;
	  first_t = local_first_t;
	  spec_first_t = true;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_last_t) <= 0)
	    break;
	  last_t = local_last_t;
	  spec_last_t = true;
	  optind++;	/* tell getopt we recognized last_t */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%lf", &local_spacing_t) <= 0)
	    break;
	  spacing_t = local_spacing_t;
	  spec_spacing_t = true;
	  optind++;	/* tell getopt we recognized spacing_t */
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
    }
				/* endwhile */
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
      display_usage (progname, hidden_options, usage_appendage, 0);
      return EXIT_SUCCESS;
    }

  /* Some sanity checks on user-supplied options. */

  if (no_of_intervals < 1)
    {
      fprintf (stderr, 
	       "%s: error: the abscissa range cannot be subdivided into %d intervals\n", 
	       progname, no_of_intervals);
      return EXIT_FAILURE;
    }

  if (periodic)
    {
      if (spec_boundary_condition)
	fprintf (stderr, 
		 "%s: the setting of a boundary condition is not supported for a periodic spline\n", 
		 progname);
      boundary_condition = 0.0;
    }

  if (filter)
    /* acting as a filter, so use cubic Bessel interpolation */
    {
      if (!spec_first_t || !spec_last_t)
	{
	  fprintf (stderr,
		   "%s: error: acting as a filter, so the abscissa range should be specified with the -t option\n",
		   progname);
	  return EXIT_FAILURE;
	}

      if (!spec_spacing_t) 
	spacing_t = (last_t - first_t) / no_of_intervals;
      else			/* user specified spacing */
	{
	  if (spec_no_of_intervals)
	    fprintf (stderr, "%s: the requested number of intervals is disregarded\n",
		     progname);
	  if ((last_t - first_t) * spacing_t < 0.0)
	    {
	      fprintf (stderr, "%s: the requested spacing was of the wrong sign, so it has been corrected\n",
		       progname);
	      spacing_t = -spacing_t;
	    }

	  /* N.B. if spacing specified, should optionally contract first_t and
	     last_t to make them integer multiples of spacing; cf. graph */
	}
      
      if (spec_boundary_condition)
	fprintf (stderr, 
		 "%s: acting as a filter, so the setting of a boundary condition is not supported\n",
		 progname);
      if (tension != 0.0)
	fprintf (stderr, 
		 "%s: acting as a filter, so nonzero tension is not supported\n",
		 progname);
      if (periodic)
	fprintf (stderr, 
		 "%s: acting as a filter, so periodicity is not supported\n",
		 progname);

      if (optind < argc)
	{
	  /* call do_bessel() on each file specified on the command line,
	     generating a spline from each dataset in the file */
	  for (; optind < argc; optind++)
	    {
	      FILE *data_file;
	      
	      /* open file, treating "-" as stdin */
	      if (strcmp (argv[optind], "-") == 0)
		data_file = stdin;
	      else
		{
		  data_file = fopen (argv[optind], "r");
		  if (data_file == NULL)
		    {
		      fprintf (stderr, "%s: %s: %s\n", progname, argv[optind], strerror(errno));
		      return EXIT_FAILURE;
		    }
		}		

	      /* loop through datasets in file (may be more than one) */
	      do
		{
		  dataset_follows = do_bessel (data_file, ydimension,
					       auto_abscissa, t_start, delta_t,
					       first_t, last_t, spacing_t, 
					       precision, suppress_abscissa);

		  /* output a separator between successive datasets */
		  if (dataset_follows || (optind + 1 != argc))
		    output_dataset_separator();
		  
		} while (dataset_follows);

	      /* close file */
	      if (data_file != stdin) /* don't close stdin */
		{
		  if (fclose (data_file) < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: the input file `%s' could not be closed\n",
			       progname, argv[optind]);
		      return EXIT_FAILURE;
		    }
		}
	    }
	}
      else			/* no files spec'd, read stdin instead */
	/* loop through datasets read from stdin (may be more than one) */
	do
	  {
	    dataset_follows = do_bessel (stdin, ydimension,
					 auto_abscissa, t_start, delta_t,
					 first_t, last_t, spacing_t, 
					 precision, suppress_abscissa);
	    
	    /* output a separator between successive datasets */
	    if (dataset_follows)
	      output_dataset_separator();
	  }
	while (dataset_follows);	/* keep going if no EOF yet */
    }

  else
    /* not acting as filter, so use spline interpolation (w/ tension) */
    {
      double *t, **y, **z;	/* ordinate, abscissa, 2nd derivative arrays */
      int i, len, used;

      if (optind < argc)	/* files spec'd on command line */
	{

	  /* call do_spline() on each file specified on the command line,
	     generating a spline from each dataset contained in the file */
	  for (; optind < argc; optind++)
	    {
	      FILE *data_file;
	      
	      /* open file, treat "-" as meaning stdin */
	      if (strcmp (argv[optind], "-") == 0)
		data_file = stdin;
	      else
		{
		  data_file = fopen (argv[optind], "r");
		  if (data_file == NULL)
		    {
		      fprintf (stderr, "%s: error: the file `%s' could not be opened\n",
			       progname, argv[optind]);
		      return EXIT_FAILURE;
		    }
		}
	      
	      /* loop through datasets in file (may be more than one) */
	      do
		{
		  len = 16;	/* initial value of storage length */
		  used = -1;	/* initial value of array size, minus 1 */
	      
		  t = (double *)xmalloc (sizeof(double) * len);
		  y = (double **)xmalloc (sizeof(double *) * ydimension);
		  z = (double **)xmalloc (sizeof(double *) * ydimension);
		  for (i = 0; i < ydimension; i++)
		    {
		      y[i] = (double *)xmalloc (sizeof(double) * len);
		      z[i] = (double *)xmalloc (sizeof(double) * len);
		    }
		  
		  dataset_follows = read_data (data_file, &len, &used, 
					       auto_abscissa, t_start, delta_t,
					       &t, ydimension, y, z);
		  /* read_data() may reallocate t,y[*],z[*], and update
		     len, used; on exit, used + 1 is number of data points */
		  
		  /* spline the dataset and output interpolating points */
		  do_spline (used, len, 
			     &t, ydimension, y, z, tension, periodic,
			     spec_boundary_condition, boundary_condition, 
			     precision,
			     first_t, last_t, spacing_t, no_of_intervals,
			     spec_first_t, spec_last_t, spec_spacing_t, 
			     spec_no_of_intervals, suppress_abscissa);

		  /* output a separator between successive datasets */
		  if (dataset_follows || (optind + 1 != argc))
		    output_dataset_separator();
		  
		  free (z);
		  free (y);
		  free (t);
		}
	      while (dataset_follows);	/* keep going if no EOF yet */
	      
	      /* close file */
	      if (data_file != stdin) /* don't close stdin */
		{
		  if (fclose (data_file) < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: the input file `%s' could not be closed\n",
			       progname, argv[optind]);
		      return EXIT_FAILURE;
		    }
		}
	    }
	}
      else			/* no files spec'd, read stdin instead */
	/* loop through datasets read from stdin (may be more than one) */
	do
	  {
	    len = 16;		/* initial value for array size */
	    used = -1;	/* initial number of stored points, minus 1 */
	    
	    t = (double *)xmalloc (sizeof(double) * len);
	    y = (double **)xmalloc (sizeof(double *) * ydimension);
	    z = (double **)xmalloc (sizeof(double *) * ydimension);
	    for (i = 0; i < ydimension; i++)
	      {
		y[i] = (double *)xmalloc (sizeof(double) * len);
		z[i] = (double *)xmalloc (sizeof(double) * len);
	      }
	    
	    dataset_follows = read_data (stdin, &len, &used, 
				     auto_abscissa, t_start, delta_t, 
				     &t, ydimension, y, z);
	    /* read_data() may reallocate t,y[*],z[*], and update len,
	       used; on exit, used + 1 is number of data points */
	    
	    /* spline the dataset and output interpolating points */
	    do_spline (used, len, 
		       &t, ydimension, y, z, tension, periodic,
		       spec_boundary_condition, boundary_condition, precision,
		       first_t, last_t, spacing_t, no_of_intervals,
		       spec_first_t, spec_last_t, spec_spacing_t, 
		       spec_no_of_intervals, suppress_abscissa);
	    
	    /* output a separator between successive datasets */
	    if (dataset_follows)
	      output_dataset_separator();
	    
	    for (i = 0; i < ydimension; i++)
	      {
		free (z[i]);
		free (y[i]);
	      }
	    free (z);
	    free (y);
	    free (t);
	  }
	while (dataset_follows);	/* keep going if no EOF yet */
      
    }

  return EXIT_SUCCESS;
}


void
set_format_type (char *s, data_type *typep)
{
  switch (s[0])
    {
    case 'a':
    case 'A':
      *typep = T_ASCII;
      break;
    case 'f':
    case 'F':
      *typep = T_SINGLE;
      break;
    case 'd':
    case 'D':
      *typep = T_DOUBLE;
      break;
    case 'i':
    case 'I':
      *typep = T_INTEGER;
      break;
    default:
      {
	fprintf (stderr, "%s: error: the data format type `%s' is invalid\n",
		 progname, s);
	exit (EXIT_FAILURE);
      }
      break;
    }
}


/* fit() computes the array z[] of second derivatives at the knots, i.e.,
   internal data points.  The abscissa array t[] and the ordinate array y[]
   are specified.  On entry, have n+1 >= 2 points in the t, y, z arrays,
   numbered 0..n.  The knots are numbered 1..n-1 as in Kincaid and Cheney.
   In the periodic case, the final knot, i.e., (t[n-1],y[n-1]), has the
   property that y[n-1]=y[0]; moreover, y[n]=y[1].  The number of points
   supplied by the user was n+1 in the non-periodic case, and n in the
   periodic case.  When this function is called, n>=1 in the non-periodic
   case, and n>=2 in the periodic case. */

/* Algorithm: the n-1 by n-1 tridiagonal matrix equation for the vector of
   2nd derivatives at the knots is reduced to upper diagonal form.  At that
   point the diagonal entries (pivots) of the upper diagonal matrix are in
   the vector u[], and the vector on the right-hand side is v[].  That is,
   the equation is of the form Ay'' = v, where a_(ii) = u[i], and a_(i,i+1)
   = alpha[i].  Here i=1..n-1 indexes the set of knots.  The matrix
   equation is solved by back-substitution for y''[], i.e., for z[]. */

/* ARGS: k = coeff in bdy condition y''_1 = k y''_0, etc. */
void
fit (int n, double *t, double *y, double *z, double k, double tension,
     bool periodic)
{
  double *h, *b, *u, *v, *alpha, *beta;
  double *uu = NULL, *vv = NULL, *s = NULL;
  int i;

  if (n == 1)			/* exactly 2 points, use straight line */
    {
      z[0] = z[1] = 0.0;
      return;
    }

  h = (double *)xmalloc (sizeof(double) * n);
  b = (double *)xmalloc (sizeof(double) * n);
  u = (double *)xmalloc (sizeof(double) * n);
  v = (double *)xmalloc (sizeof(double) * n);
  alpha = (double *)xmalloc (sizeof(double) * n);
  beta = (double *)xmalloc (sizeof(double) * n);

  if (periodic)
    {
      s = (double *)xmalloc (sizeof(double) * n); 
      uu = (double *)xmalloc (sizeof(double) * n); 
      vv = (double *)xmalloc (sizeof(double) * n); 
    }

  for (i = 0; i <= n - 1 ; ++i)
    {
      h[i] = t[i + 1] - t[i];
      b[i] = 6.0 * (y[i + 1] - y[i]) / h[i]; /* for computing RHS */
    }

  if (tension < 0.0)		/* must rule out sin(tension * h[i]) = 0 */
    {
      for (i = 0; i <= n - 1 ; ++i)
	if (sin (tension * h[i]) == 0.0)
	  {
	    fprintf (stderr, "%s: error: the specified negative tension value is singular\n", progname);
	    exit (EXIT_FAILURE);
	  }
    }
  if (tension == 0.0)
    {
      for (i = 0; i <= n - 1 ; ++i)
	{
	  alpha[i] = h[i];	/* off-diagonal = alpha[i] to right */
	  beta[i] = 2.0 * h[i];	/* diagonal = beta[i-1] + beta[i] */
	}
    }
  else
    if (tension > 0.0)
      /* `positive' (really real) tension, use hyperbolic trig funcs */
      {
	for (i = 0; i <= n - 1 ; ++i)
	  {
	    double x = tension * h[i];
	    double xabs = (x < 0.0 ? -x : x);

	    if (xabs < TRIG_ARG_MIN)
	      /* hand-compute (6/x^2)(1-x/sinh(x)) and (3/x^2)(x/tanh(x)-1)
                 to improve accuracy; here `x' is tension * h[i] */
	      {
		alpha[i] = h[i] * sinh_func(x);
		beta[i] = 2.0 * h[i] * tanh_func(x);
	      }
	    else if (xabs > TRIG_ARG_MAX)
	      /* in (6/x^2)(1-x/sinh(x)) and (3/x^2)(x/tanh(x)-1),
		 approximate x/sinh(x) and x/tanh(x) by 2|x|exp(-|x|)
		 and |x|, respectively */
	      {
		int sign = (x < 0.0 ? -1 : 1);

		alpha[i] = ((6.0 / (tension * tension))
			   * ((1.0 / h[i]) - tension * 2 * sign * exp(-xabs)));
		beta[i] = ((6.0 / (tension * tension))
			   * (tension - (1.0 / h[i])));
	      }
	    else
	      {
		alpha[i] = ((6.0 / (tension * tension))
			    * ((1.0 / h[i]) - tension / sinh(x)));
		beta[i] = ((6.0 / (tension * tension))
			   * (tension / tanh(x) - (1.0 / h[i])));
	      }
	  }
      }
    else				/* tension < 0 */
      /* `negative' (really imaginary) tension,  use circular trig funcs */
      {
	for (i = 0; i <= n - 1 ; ++i)
	  {
	    double x = tension * h[i];
	    double xabs = (x < 0.0 ? -x : x);

	    if (xabs < TRIG_ARG_MIN)
	      /* hand-compute (6/x^2)(1-x/sin(x)) and (3/x^2)(x/tan(x)-1)
                 to improve accuracy; here `x' is tension * h[i] */
	      {
		alpha[i] = h[i] * sin_func(x);
		beta[i] = 2.0 * h[i] * tan_func(x);
	      }
	    else
	      {
		alpha[i] = ((6.0 / (tension * tension))
		           * ((1.0 / h[i]) - tension / sin(x)));
		beta[i] = ((6.0 / (tension * tension))
			   * (tension / tan(x) - (1.0 / h[i])));
	      }
	  }
      }
  
  if (!periodic && n == 2)
      u[1] = beta[0] + beta[1] + 2 * k * alpha[0];
  else
    u[1] = beta[0] + beta[1] + k * alpha[0];

  v[1] = b[1] - b[0];
  
  if (u[1] == 0.0)
    {
      fprintf (stderr, 
	       "%s: error: as posed, the problem of computing a spline is singular\n",
	       progname);
      exit (EXIT_FAILURE);
    }

  if (periodic)
    {
      s[1] = alpha[0];
      uu[1] = 0.0;
      vv[1] = 0.0;
    }

  for (i = 2; i <= n - 1 ; ++i)
    {
      u[i] = (beta[i] + beta[i - 1]
	      - alpha[i - 1] * alpha[i - 1] / u[i - 1]
	      + (i == n - 1 ? k * alpha[n - 1] : 0.0));

      if (u[i] == 0.0)
	{
	  fprintf (stderr, 
		   "%s: error: as posed, the problem of computing a spline is singular\n",
		   progname);
	  exit (EXIT_FAILURE);
	}


      v[i] = b[i] - b[i - 1] - alpha[i - 1] * v[i - 1] / u[i - 1];

      if (periodic)
	{
	  s[i] = - s[i-1] * alpha[i-1] / u[i-1];
	  uu[i] = uu[i-1] - s[i-1] * s[i-1] / u[i-1];
	  vv[i] = vv[i-1] - v[i-1] * s[i-1] / u[i-1];
	}
    }
      
  if (!periodic)
    {
      /* fill in 2nd derivative array */
      z[n] = 0.0;
      for (i = n - 1; i >= 1; --i)
	z[i] = (v[i] - alpha[i] * z[i + 1]) / u[i];
      z[0] = 0.0;
      
      /* modify to include boundary condition */
      z[0] = k * z[1];
      z[n] = k * z[n - 1];
    }
  else		/* periodic */
    {
      z[n-1] = (v[n-1] + vv[n-1]) / (u[n-1] + uu[n-1] + 2 * s[n-1]);
      for (i = n - 2; i >= 1; --i)
	z[i] = ((v[i] - alpha[i] * z[i + 1]) - s[i] * z[n-1]) / u[i];

      z[0] = z[n-1];
      z[n] = z[1];
    }

  if (periodic)
    {
      free (vv);
      free (uu);
      free (s);
    }
  free (beta);
  free (alpha);
  free (v);
  free (u);
  free (b);
  free (h);
}


/* interpolate() computes an approximate ordinate value for a given
   abscissa value, given an array of data points (stored in t[] and y[],
   containing abscissa and ordinate values respectively), and z[], the
   array of 2nd derivatives at the knots (i.e. internal data points).
   
   On entry, have n+1 >= 2 points in the t, y, z arrays, numbered 0..n.
   The number of knots (i.e. internal data points) is n-1; they are
   numbered 1..n-1 as in Kincaid and Cheney.  In the periodic case, the
   final knot, i.e., (t[n-1],y[n-1]), has the property that y[n-1]=y[0];
   also, y[n]=y[1].  The number of data points supplied by the user was n+1
   in the non-periodic case, and n in the periodic case.  When this
   function is called, n>=1 in the non-periodic case, and n>=2 in the
   periodic case. */

double
interpolate (int n, double *t, double *y, double *z, double x, 
	     double tension, bool periodic)
{
  double diff, updiff, reldiff, relupdiff, h;
  double value;
  int is_ascending = (t[n-1] < t[n]);
  int i = 0, k;

  /* in periodic case, map x to t[0] <= x < t[n] */
  if (periodic && (x - t[0]) * (x - t[n]) > 0.0)
    x -= ((int)(floor( (x - t[0]) / (t[n] - t[0]) )) * (t[n] - t[0]));

  /* do binary search to find interval */
  for (k = n - i; k > 1;)
    {
      if (is_ascending ? x >= t[i + (k>>1)] : x <= t[i + (k>>1)])
	{
	  i = i + (k>>1);
	  k = k - (k>>1);
	}
      else
	k = k>>1;
    }

  /* at this point, x is between t[i] and t[i+1] */
  h = t[i + 1] - t[i];
  diff = x - t[i];
  updiff = t[i+1] - x;
  reldiff = diff / h;
  relupdiff = updiff / h;

  if (tension == 0.0)
  /* evaluate cubic polynomial in nested form */
    value =  y[i] 
      + diff
	* ((y[i + 1] - y[i]) / h - h * (z[i + 1] + z[i] * 2.0) / 6.0
	   + diff * (0.5 * z[i] + diff * (z[i + 1] - z[i]) / (6.0 * h)));
  
  else if (tension > 0.0)
    /* `positive' (really real) tension, use sinh's */
    {
      if (fabs(tension * h) < TRIG_ARG_MIN)
	/* hand-compute (6/y^2)(sinh(xy)/sinh(y) - x) to improve accuracy;
	   here `x' means reldiff or relupdiff and `y' means tension*h */
	value = (y[i] * relupdiff + y[i+1] * reldiff
		 + ((z[i] * h * h / 6.0) 
		    * quotient_sinh_func (relupdiff, tension * h))
		 + ((z[i+1] * h * h / 6.0) 
		    * quotient_sinh_func (reldiff, tension * h)));
      else if (fabs(tension * h) > TRIG_ARG_MAX)
	/* approximate 1/sinh(y) by 2 sgn(y) exp(-|y|) */
	{
	  int sign = (h < 0.0 ? -1 : 1);

	  value = (((z[i] * (exp (tension * updiff - sign * tension * h) 
			     + exp (-tension * updiff - sign * tension * h))
		     + z[i + 1] * (exp (tension * diff - sign * tension * h) 
				   + exp (-tension * diff - sign * tension*h)))
		    * (sign / (tension * tension)))
		   + (y[i] - z[i] / (tension * tension)) * (updiff / h)
		   + (y[i + 1] - z[i + 1] / (tension * tension)) * (diff / h));
	}
      else
	value = (((z[i] * sinh (tension * updiff) 
		   + z[i + 1] * sinh (tension * diff))
		  / (tension * tension * sinh (tension * h)))
		 + (y[i] - z[i] / (tension * tension)) * (updiff / h)
		 + (y[i + 1] - z[i + 1] / (tension * tension)) * (diff / h));
    }
  else
    /* `negative' (really imaginary) tension, use sin's */
    {
      if (fabs(tension * h) < TRIG_ARG_MIN)
	/* hand-compute (6/y^2)(sin(xy)/sin(y) - x) to improve accuracy;
	   here `x' means reldiff or relupdiff and `y' means tension*h */
	value = (y[i] * relupdiff + y[i+1] * reldiff
		 + ((z[i] * h * h / 6.0) 
		    * quotient_sin_func (relupdiff, tension * h))
		 + ((z[i+1] * h * h / 6.0) 
		    * quotient_sin_func (reldiff, tension * h)));
      else
	value = (((z[i] * sin (tension * updiff) 
		   + z[i + 1] * sin (tension * diff))
		  / (tension * tension * sin (tension * h)))
		 + (y[i] - z[i] / (tension * tension)) * (updiff / h)
		 + (y[i + 1] - z[i + 1] / (tension * tension)) * (diff / h));
    }
  
  return value;
}


/* is_monotonic() check whether an array of data points, read in by
   read_data(), has monotonic abscissa values. */
bool
is_monotonic (int n, double *t)
{
  bool is_ascending;

  if (t[n-1] < t[n])
    is_ascending = true;
  else if (t[n-1] > t[n])
    is_ascending = false;
  else				/* equality */
    return false;

  while (n>0)
    {
      n--;
      if (is_ascending == true ? t[n] >= t[n+1] : t[n] <= t[n+1])
	return false;
    };
  return true;
}


/* read_float reads a single floating point quantity from an input file
   (in either ascii or double format).  Return value indicates whether it
   was read successfully. */
bool 
read_float (FILE *input, double *dptr)
{
  int num_read;
  double dval;
  float fval;
  int ival;

  switch (input_type)
    {
    case T_ASCII:
    default:
      num_read = fscanf (input, "%lf", &dval);
      break;
    case T_SINGLE:
      num_read = fread ((void *) &fval, sizeof (fval), 1, input);
      dval = fval;
      break;
    case T_DOUBLE:
      num_read = fread ((void *) &dval, sizeof (dval), 1, input);
      break;
    case T_INTEGER:
      num_read = fread ((void *) &ival, sizeof (ival), 1, input);
      dval = ival;
      break;
    }
  if (num_read <= 0)
    return false;
  if (dval != dval)
    {
      fprintf (stderr, "%s: a NaN (not-a-number) was encountered in a binary input file (it is treated as EOF)\n",
	       progname);
      return false;		/* effectively eof */
    }
  else
    {
      *dptr = dval;
      return true;
    }
}

/* Emit a pair of doubles, in specified output representation.  Inform user
   if any of the emitted values was out-of-bounds for single-precision or
   integer format. */
bool 
write_point (double t, double *y, int ydimension, int precision, bool suppress_abscissa)
{
  int i, num_written = 0;
  float ft, fy;
  int it, iy;

  switch (output_type)
    {
    case T_ASCII:
    default:
      if (suppress_abscissa == false)
	num_written += printf ("%.*g ", precision, t);
      for (i = 0; i < ydimension - 1; i++)
	num_written += printf ("%.*g ", precision, y[i]);
      num_written += printf ("%.*g\n", precision, y[ydimension - 1]);
      break;
    case T_SINGLE:
      if (suppress_abscissa == false)
	{
	  ft = FROUND(t);
	  if (ft == FLT_MAX || ft == -(FLT_MAX))
	    {
	      maybe_emit_oob_warning();
	      if (ft == FLT_MAX)
		ft *= 0.99999;	/* kludge */
	    }
	  num_written += fwrite ((void *) &ft, sizeof (ft), 1, stdout);
	}
      for (i = 0; i < ydimension; i++)
	{
	  fy = y[i];
	  if (fy == FLT_MAX || fy == -(FLT_MAX))
	    {
	      maybe_emit_oob_warning();
	      if (fy == FLT_MAX)
		fy *= 0.99999;	/* kludge */
	    }
	  num_written += fwrite ((void *) &fy, sizeof (fy), 1, stdout);
	}
      break;
    case T_DOUBLE:
      if (suppress_abscissa == false)
	num_written += fwrite ((void *) &t, sizeof (t), 1, stdout);
      for (i = 0; i < ydimension; i++)
	num_written += fwrite ((void *) &(y[i]), sizeof (double), 1, stdout);
      break;
    case T_INTEGER:
      if (suppress_abscissa == false)
	{
	  it = IROUND(t);
	  if (it == INT_MAX || it == -(INT_MAX))
	    {
	      maybe_emit_oob_warning();
	      if (it == INT_MAX)
		it--;
	    }
	  num_written += fwrite ((void *) &it, sizeof (it), 1, stdout);
	}
      for (i = 0; i < ydimension; i++)
	{
	  iy = IROUND(y[i]);
	  if (iy == INT_MAX || iy == -(INT_MAX))
	    {
	      maybe_emit_oob_warning();
	      if (iy == INT_MAX)
		iy--;
	    }
	  num_written += fwrite ((void *) &iy, sizeof (iy), 1, stdout);
	}
      break;
    }
  
  return (num_written > 0 ? true : false); /* i.e. return successp */
}

/* read_point() attempts to read a data point from an input file
   (auto-abscissa is supported, as are both ascii and double formats).
   Return value is 0 if a data point was read, 1 if no data point could be
   read (i.e. EOF or garbage in file).  A return value of 2 is special: it
   indicates that an explicit end-of-dataset indicator was seen in the input
   stream.  For an ascii stream this is two newlines in succession; for a
   double stream this is a DBL_MAX, etc. */
int
read_point (FILE *input, double *t, double *y, int ydimension, 
	    bool *first_point,
	    int auto_abscissa, double *auto_t, double auto_delta, 
	    double *stored)
{
  bool success;
  int i, items_read, lookahead;

 head:

  if (input_type == T_ASCII)
    {
      bool two_newlines;

      /* skip whitespace, up to but not including 2nd newline */
      two_newlines = skip_whitespace (input);
      if (two_newlines)
	/* end-of-dataset indicator */
	return 2;
    }
  if (feof (input))
    return 1;

  if (input_type == T_ASCII)
    {
      lookahead = getc (input);
      ungetc (lookahead, input);
      if (lookahead == (int)'#')	/* comment line */
	{
	  char c;
	  
	  do 
	    {
	      items_read = fread (&c, sizeof (c), 1, input);
	      if (items_read <= 0)
		return 1;	/* EOF */
	    }
	  while (c != '\n');
	  ungetc ((int)'\n', input); /* push back \n at the end of # line */
	  goto head;
	}
    }

  if (auto_abscissa != AUTO_NONE) /* i.e. AUTO_INCREMENT or AUTO_BY_DISTANCE */
    {
      /* read 1st component of y */
      success = read_float (input, &(y[0]));
      if (!success)		/* e.g., EOF */
	return 1;
      if ((input_type == T_DOUBLE && y[0] == DBL_MAX)
	  || (input_type == T_SINGLE && y[0] == (double)FLT_MAX)
	  || (input_type == T_INTEGER && y[0] == (double)INT_MAX))
	/* end-of-dataset indicator */
	return 2;

      /* read other components of y */
      for (i = 1; i < ydimension; i++)
	{
	  success = read_float (input, &(y[i]));
	  if (!success)		/* effectively EOF (could be garbage) */
	    {
	      fprintf (stderr, "%s: an input file terminated prematurely\n",
		       progname);
	      return 1;
	    }
	}

      /* t is kept track of, not read from file; two different methods */
      if (auto_abscissa == AUTO_INCREMENT)
	{
	  *t = *auto_t;
	  *auto_t += auto_delta;	/* update */
	}
      else			/* AUTO_BY_DISTANCE */
	{
	  if (*first_point == true)
	    {
	      *t = *auto_t;
	      *first_point = false;
	    }
	  else		/* compute distance to previous point */
	    {
	      double distsq = 0.0;

	      for (i = 0; i < ydimension; i++)
		distsq += (y[i] - stored[i])*(y[i] - stored[i]);
	      *auto_t += sqrt (distsq);
	      *t = *auto_t;
	    }
	  for (i = 0; i < ydimension; i++)	  
	    stored[i] = y[i];	/* store current point */
	}

      /* successfully read all components of y */
      return 0;
    }
  else
    {
      /* read t */
      success = read_float (input, t);
      if (!success)		/* e.g., EOF */
	return 1;
      if ((input_type == T_DOUBLE && *t == DBL_MAX)
	  || (input_type == T_SINGLE && *t == (double)FLT_MAX)
	  || (input_type == T_INTEGER && *t == (double)INT_MAX))
	/* end-of-dataset indicator */
	return 2;

      /* read components of y */
      for (i = 0; i < ydimension; i++)
	{
	  success = read_float (input, &(y[i]));
	  if (!success)		/* effectively EOF (could be garbage) */
	    {
	      fprintf (stderr, "%s: an input file terminated prematurely\n",
		       progname);
	      return 1;
	    }
	}

      /* successfully read both t and all components of y */
      return 0;
    }
}

/* read_data() reads a single dataset from an input file, and stores it.
   If the stream is in ascii format, end-of-dataset is signalled by two
   newlines in succession.  If the stream is in double format,
   end-of-dataset is signalled by the occurrence of a DBL_MAX, etc.

   Return value is true if the dataset is ended by an explicit
   end-of-dataset, and false if the dataset is terminated by EOF.  That is,
   return value indicates whether another dataset is expected to follow. */
bool
read_data (FILE *input, int *len, int *used, int auto_abscissa,
	   double auto_t, double auto_delta, 
	   double **t, int ydimension, double **y, double **z)
{
  bool first = true;
  int i, success;
  double tt, *yy, *stored;

  yy = (double *)xmalloc (sizeof(double) * ydimension);
  stored = (double *)xmalloc (sizeof(double) * ydimension);
  for ( ; ; )
    {
      if ((++ *used) >= *len)
	{
	  *len *= 2;
	  *t = (double *)xrealloc (*t, sizeof(double) * *len);
	  for (i = 0; i < ydimension; i++)
	    {
	      y[i] = (double *)xrealloc (y[i], sizeof(double) * *len);
	      z[i] = (double *)xrealloc (z[i], sizeof(double) * *len);
	    }
	}

      success = read_point (input, &tt, yy, ydimension, &first,
			    auto_abscissa, &auto_t, auto_delta, stored);

      switch (success)
	{
	case 0:			/* good data point */
	  (*t)[*used] = tt;
	  for (i = 0; i < ydimension; i++)
	    y[i][*used] = yy[i];
	  break;
	case 1:			/* end of dataset, EOF seen */
	  (*used)--;
	  free (stored);
	  free (yy);
	  return false;
	case 2:			/* end of dataset, but input continues */
	  (*used)--;
	  free (stored);
	  free (yy);
	  return true;
	}
    }
}


/* do_spline() is the main routine for piecewise cubic spline
   interpolation, supporting both periodicity and a user-specified boundary
   condition parameter.  Nonzero tension may be specified, in which case
   the interpolate() routine, which this calls, will use not cubic
   polynomials but rather expressions involving hyperbolic sines.

   t[] and y[] are the arrays in which the abscissa and ordinate values of
   the user-specified data points are stored, and z[] is the array in which
   the 2nd derivatives at the knots (data points in the interior of the
   interval) will be stored.  used+1 is the effective size of each of these
   arrays.  The number of points supplied by the user was used+1 in the
   non-periodic case.  It was used+0 in the periodic case.  

   The reason that the number of elements is greater by one in the periodic
   case is that the first user-supplied data point occurs also at the end.
   In fact, in the periodic case this function will increment the size of
   the array once more, since the periodic interpolation algorithm requires
   the first two data points, not just the first, to appear at the end. */

/* ARGS: used = indicator that used+1 elements stored in (*t)[] etc.
   	 len = length of each array
	 y,z are ptrs-to-ptrs because we may need to realloc
	 k = coeff in bdy condition y''_1 = k y''_0, etc. */
void
do_spline (int used, int len, double **t, int ydimension, double **y, double **z, 
	   double tension, bool periodic, bool spec_boundary_condition,
	   double k, int precision, double first_t, double last_t, 
	   double spacing_t, int no_of_intervals, bool spec_first_t, 
	   bool spec_last_t, bool spec_spacing_t, 
	   bool spec_no_of_intervals, bool suppress_abscissa)
{
  int range_count = 0;		/* number of req'd datapoints out of range */
  int lastval = 0;		/* last req'd point = 1st/last data point? */
  int i;

  if (used + 1 == 0)		/* zero data points in array */
    /* don't output anything (i.e. effectively output a null dataset) */
    return;

  if (used+1 == 1)		/* a single data point in array */
    {
      fprintf (stderr, 
	       "%s: a spline cannot be constructed from a single data point\n", 
	       progname);
      /* don't output anything (i.e. effectively output a null dataset) */
      return;
    }

  if (!periodic && used+1 <= 2)
    {
      if (spec_boundary_condition)
	fprintf (stderr, 
		 "%s: the specified boundary condition is ignored, as there are only 2 data points\n", 
		 progname);
      k = 0.0;
    }

  if (!is_monotonic (used, *t))
    non_monotonic_error();	/* self-explanatory */

  if (periodic)
    {
      bool print_warning = false;
      
      for (i = 0; i < ydimension; i++)
	{
	  if (y[i][used] != y[i][0])
	    print_warning = true;
	  y[i][used] = y[i][0];
	}
      if (print_warning == true)
	fprintf (stderr, "%s: the final y value is set equal to the initial value, to ensure periodicity\n", 
		 progname); 

      /* add pseudo-point at end (to accord with periodicity) */
      if (used + 1 >= len)
	{
	  len++;
	  *t = (double *)xrealloc (*t, sizeof(double) * len);
	  for (i = 0; i < ydimension; i++)
	    {
	      y[i] = (double *)xrealloc (y[i], sizeof(double) * len);
	      z[i] = (double *)xrealloc (z[i], sizeof(double) * len);
	    }
	}
      (*t)[used + 1] = (*t)[used] + ((*t)[1] - (*t)[0]);
      for (i = 0; i < ydimension; i++)
	y[i][used + 1] = y[i][1];
    }

  /* compute z[], array of 2nd derivatives at each knot */
  for (i = 0; i < ydimension; i++)
    fit (used + (periodic ? 1 : 0), /* include pseudo-point if any */
	 *t, y[i], z[i], k, tension, periodic);

  if (!spec_first_t) 
    first_t = (*t)[0];
  if (!spec_last_t) 
    last_t = (*t)[used];	/* used+1 data points in all */
  if (!spec_spacing_t) 
    {
      if (no_of_intervals > 0)
	spacing_t = (last_t - first_t) / no_of_intervals;
      else
	spacing_t = 0;		/* won't happen */
    }
  else				/* user specified spacing */
    {
      if ((last_t - first_t) * spacing_t < 0.0)
	{
	  fprintf (stderr, "%s: the requested spacing is of the wrong sign, so it has been corrected\n",
		   progname);
	  spacing_t = -spacing_t;
	}
      if (spec_no_of_intervals)
	fprintf (stderr, "%s: the requested number of intervals is disregarded\n",
		 progname);
      no_of_intervals = (int)(fabs((last_t - first_t) / spacing_t) + FUZZ);
    }

  if (last_t == (*t)[0])
    lastval = 1;
  else if (last_t == (*t)[used])
    lastval = 2;

  for (i = 0; i <= no_of_intervals; ++i)
    {
      double x;

      x = first_t + spacing_t * i;

      if (i == no_of_intervals)
	{
	  /* avoid numerical fuzz */
	  if (lastval == 1)	/* left end of input */
	    x = (*t)[0];
	  else if (lastval == 2) /* right end of input */
	    x = (*t)[used];
	}

      if (periodic || (x - (*t)[0]) * (x - (*t)[used]) <= 0)
	{
	  int j;
	  double *yy;

	  yy = (double *)xmalloc (sizeof(double) * ydimension); 
	  for (j = 0; j < ydimension; j++)
	    yy[j] = interpolate (used, *t, y[j], z[j], x, 
				 tension, periodic);
	  write_point (x, yy, ydimension, precision, suppress_abscissa);
	  free (yy);
	}
      else
	range_count++;
    }

  switch (range_count)
    {
    case 0:
      break;
    case 1:
      fprintf (stderr, 
	       "%s: one requested point could not be computed (as it was out of the data range)\n", 
	       progname);
      break;
    default:
      fprintf (stderr, 
	       "%s: %d requested points could not be computed (as they were out of the data range)\n", 
	       progname, range_count);
      break;
    }
}


/* do_bessel() is the main routine for doing real-time cubic Bessel
   interpolation of a dataset.  If the input stream is in ascii format,
   end-of-dataset is signalled by two newlines in succession.  If the
   stream is in double format, end-of-dataset is signalled by the
   occurrence of a DBL_MAX, etc.

   Return value is true if the dataset is ended by an explicit
   end-of-dataset, and false if the dataset is terminated by EOF.  That is,
   return value indicates whether another dataset is expected to follow. */
bool
do_bessel (FILE *input, int ydimension, int auto_abscissa, double auto_t, 
	   double auto_delta, double first_t, double last_t, 
	   double spacing_t, int precision, bool suppress_abscissa)
{
  bool first = true;
  double t, *y, *s0, *s1, *s2, *stored;
  double tt[4], **yy;
  int direction = (last_t > first_t ? 1 : -1);
  int state = STATE_ZERO;
  int i, success;

  y = (double *)xmalloc (sizeof(double) * ydimension); 
  s0 = (double *)xmalloc (sizeof(double) * ydimension); 
  s1 = (double *)xmalloc (sizeof(double) * ydimension); 
  s2 = (double *)xmalloc (sizeof(double) * ydimension); 
  yy = (double **)xmalloc (4 * sizeof(double *));
  stored = (double *)xmalloc (sizeof(double) * ydimension);
  for (i = 0; i < 4; i++)
    yy[i] = (double *)xmalloc (ydimension * sizeof(double));

  for ( ; ; )
    {
      success = read_point (input, &t, y, ydimension, &first,
			    auto_abscissa, &auto_t, auto_delta, stored);
      
      if (success == 0)		/* got a new data point */
	{
	  /* use our DFA to process the new data point */
	  switch (state)
	    {
	    case STATE_ZERO:	/* just store point */
	      tt[0] = t;
	      for (i = 0; i < ydimension; i++)
		yy[0][i] = y[i];
	      state = STATE_ONE;
	      break;
	    case STATE_ONE:	/* just store point */
	      tt[1] = t;
	      if (direction * (tt[1] - tt[0]) <= 0)
		non_monotonic_error();
	      for (i = 0; i < ydimension; i++)
		yy[1][i] = y[i];
	      state = STATE_TWO;
	      break;
	    case STATE_TWO:	/* store point, and process */
	      tt[2] = t;
	      if (direction * (tt[2] - tt[1]) <= 0)
		non_monotonic_error();
	      for (i = 0; i < ydimension; i++)
		{
		  yy[2][i] = y[i];
		  
		  /* fit parabola through 0,1,2 to compute slopes at 0,1*/
		  s0[i] = (((tt[1]-tt[0]) * ((yy[0][i]-yy[2][i]) / (tt[0]-tt[2]))
			 + (tt[0]-tt[2]) * ((yy[1][i]-yy[0][i]) / (tt[1]-tt[0])))
			/ (tt[1]-tt[2]));
		  s1[i] = (((tt[2]-tt[1]) * ((yy[1][i]-yy[0][i]) / (tt[1]-tt[0]))
			 + (tt[1]-tt[0]) * ((yy[2][i]-yy[1][i]) / (tt[2]-tt[1])))
			/ (tt[2]-tt[0]));
		}

	      /* output spline points in range between points 0, 1 */
	      do_bessel_range (tt[0], tt[1], yy[0], yy[1], s0, s1,
			       first_t, last_t, spacing_t, 
			       ydimension, precision, false,
			       suppress_abscissa);
	      
	      state = STATE_THREE;
	      break;
	    case STATE_THREE:	/* store point, and process */
	      tt[3] = t;
	      if (direction * (tt[3] - tt[2]) <= 0)
		non_monotonic_error();
	      for (i = 0; i < ydimension; i++)
		{
		  yy[3][i] = y[i];
		  
		  /* fit parabola through points 1,2,3 to compute slope at 2 */
		  s2[i] = (((tt[3]-tt[2]) * ((yy[2][i]-yy[1][i]) / (tt[2]-tt[1]))
			 + (tt[2]-tt[1]) * ((yy[3][i]-yy[2][i]) / (tt[3]-tt[2])))
			/ (tt[3]-tt[1]));
		}
	      
	      /* output spline points in range between points 1, 2 */
	      do_bessel_range (tt[1], tt[2], yy[1], yy[2], s1, s2, 
			       first_t, last_t, spacing_t, 
			       ydimension, precision, false,
			       suppress_abscissa);
	      
	      /* shift points down */
	      tt[0] = tt[1];
	      tt[1] = tt[2];
	      tt[2] = tt[3];
	      for (i = 0; i < ydimension; i++)
		{
		  yy[0][i] = yy[1][i];
		  yy[1][i] = yy[2][i];
		  yy[2][i] = yy[3][i];
		  /* shift down the only knot slope worth keeping */
		  s1[i] = s2[i];
		}

	      break;
	    }
	}
      else		/* didn't get a point, so wind things up */
	{
	  switch (state)
	    {
	    case STATE_ZERO:
	      /* silently output a null dataset (i.e., don't output anything) */
	      break;
	    case STATE_ONE:
	      fprintf (stderr, "%s: a spline cannot be constructed from a single data point\n", 
		       progname);
	      /* output a null dataset (i.e., don't output anything) */
	      break;
	    case STATE_TWO:
	      /* have two points: do linear interp between points 0, 1 */
	      for (i = 0; i < ydimension; i++)
		s0[i] = s1[i] = (yy[1][i] - yy[0][i])/(tt[1]-tt[0]);
	      do_bessel_range (tt[0], tt[1], yy[0], yy[1], s0, s1, 
			       first_t, last_t, spacing_t, 
			       ydimension, precision, true,
			       suppress_abscissa);
	      break;
	    case STATE_THREE:
	      /* already did 1st of 2 intervals, so do 2nd one too */

	      /* fit parabola through points 0,1,2 to compute slope at 2 */
	      for (i = 0; i < ydimension; i++)
		s2[i] = (((tt[0]-tt[2]) * ((yy[2][i]-yy[1][i]) / (tt[2]-tt[1]))
		       + (tt[2]-tt[1]) * ((yy[0][i]-yy[2][i]) / (tt[0]-tt[2])))
		      / (tt[0]-tt[1]));

	      /* output spline points in range between points 1, 2 */
	      do_bessel_range (tt[1], tt[2], yy[1], yy[2], s1, s2, 
			       first_t, last_t, spacing_t, 
			       ydimension, precision, true,
			       suppress_abscissa);
	      break;
	    }

	  /* free storage before return */
	  for (i = 0; i < 4; i++)
	    free (yy[i]);
	  free (stored);
	  free (yy);
	  free (s2);
	  free (s1);
	  free (s0);
	  free (y);

	  /* return indication of whether end-of-dataset was seen in stream */
	  return (success == 2 ? true : false);
	}
    }
}

void
non_monotonic_error (void)
{
  fprintf (stderr, "%s: error: the abscissa values are not monotonic\n",
	   progname);
  exit (EXIT_FAILURE);
}


/* do_bessel_range() computes spline points separated by spacing_t, within
   the abscissa interval abscissa0 <= t < abscissa1, that happen to lie in
   the desired range first_t <= t <= last_t.  It writes them to standard
   output.  The ordinate values value0 and value1, and endpoint slopes
   slope0 and slope1, are specified.  If `endit' is set, then the intervals
   stretch slightly farther than abscissa1 and last_t, to compensate for
   roundoff error. */

/* ARGS: endit = last interval to be treated? */
void
do_bessel_range (double abscissa0, double abscissa1, double *value0, 
		 double *value1, double *slope0, double *slope1, 
		 double first_t, double last_t, double spacing_t, 
		 int ydimension, int precision, bool endit,
		 bool suppress_abscissa)
{
  int direction = ((last_t > first_t) ? 1 : -1); /* sign of spacing_t */
  int i, j;
  int imin1 = (int)((abscissa0 - first_t) / spacing_t - 1);
  int imax1 = (int)((abscissa1 - first_t) / spacing_t + 1);
  int imin2 = 0;
  int imax2 = (int)((last_t - first_t) / spacing_t + 1);
  int imin, imax;
  
  /* compute maximum interval over which i must range */
  imin = IMAX (imin1, imin2);
  imax = IMIN (imax1, imax2);
  for (i = imin; i <= imax; i++)
    {
      double t;

      t = first_t + i * spacing_t;

      if ((direction * t >= direction * abscissa0)
	  && (direction * t >= direction * first_t)
	  /* stretch slightly if `endit' is set */
	  && ((direction * t < (direction 
				* (abscissa1 
				   + (endit ? 
				      FUZZ * (abscissa1 - abscissa0) : 0.)))))
	  && (direction * t <= (direction
			       * (last_t
				  + (endit ? FUZZ * (last_t - first_t) : 0.)))))
	{
	  double diff = t - abscissa0;
	  double updiff = abscissa1 - t;
	  double h = abscissa1 - abscissa0;
	  double *y;
	  bool success;

	  y = (double *)xmalloc (sizeof(double) * ydimension); 
	  for (j = 0; j < ydimension; j++)
	    {
	      /* should use a nested form */
	      y[j] = (value1[j] * (-2.0 * diff * diff * diff / (h * h * h)
				   + 3.0 * diff * diff / (h * h))
		+ value0[j] * (-2.0 * updiff * updiff * updiff / (h * h * h)
			           + 3.0 * updiff * updiff / (h * h)))
		+ ((slope1[j] * (diff * diff * diff / (h * h) 
			      - diff * diff / h)
		- (slope0[j] * (updiff * updiff * updiff / (h * h) 
				 - updiff * updiff / h))));
	    }
	  
	  success = write_point (t, y, 
				 ydimension, precision, suppress_abscissa);
	  if (!success)
	    {
	      fprintf (stderr, 
		       "%s: error: standard output cannot be written to\n",
		       progname);
	      exit (EXIT_FAILURE);
	    }
	  free (y);
	}	  
    }
}


/* Output a separator between datasets.  For ascii-format output streams
   this is an extra newline (after the one that the spline ended with,
   yielding two newlines in succession).  For double-format output streams
   this is a DBL_MAX, etc. */

void
output_dataset_separator (void)
{
  double ddummy;
  float fdummy;
  int idummy;

  switch (output_type)
    {
    case T_ASCII:
    default:
      printf ("\n");
      break;
    case T_DOUBLE:
      ddummy = DBL_MAX;
      fwrite ((void *) &ddummy, sizeof(ddummy), 1, stdout);
      break;
    case T_SINGLE:
      fdummy = FLT_MAX;
      fwrite ((void *) &fdummy, sizeof(fdummy), 1, stdout);
      break;
    case T_INTEGER:
      idummy = INT_MAX;
      fwrite ((void *) &idummy, sizeof(idummy), 1, stdout);
      break;
    }
}

/* skip_whitespace() skips whitespace in an ascii-format input file,
   up to but not including a second newline.  Return value indicates
   whether or not two newlines were in fact seen.  (For ascii-format
   input files, two newlines signals an end-of-dataset.) */

bool
skip_whitespace (FILE *stream)
{
  int lookahead;
  int nlcount = 0;
  
  do 
    {
      lookahead = getc (stream);
      if (lookahead == (int)'\n')
	  nlcount++;
    }
  while (lookahead != EOF 
	 && isspace((unsigned char)lookahead)
	 && nlcount < 2);

  if (lookahead == EOF)
    return false;
  
  ungetc (lookahead, stream);
  return (nlcount == 2 ? true : false);
}

void
maybe_emit_oob_warning (void)
{
  static bool warning_written = false;

  if (!warning_written)
    {
      fprintf (stderr, "%s: one or more out-of-bounds output values are approximated\n", progname);
      warning_written = true;
    }
}


/* Following four functions compute (6/x^2)(1-x/sinh(x)),
   (3/x^2)(x/tanh(x)-1), (6/x^2)(1-x/sin(x)), and (3/x^2)(x/tan(x)-1) via
   the first three terms of the appropriate power series.  They are used
   when |x|<TRIG_ARG_MIN, to avoid loss of significance.  Errors are
   O(x**6). */
double
sinh_func (double x) 
{
  /* use 1-(7/60)x**2+(31/2520)x**4 */
  return 1.0 - (7.0/60.0)*x*x + (31.0/2520.0)*x*x*x*x;
}

double
tanh_func (double x) 
{
  /* use 1-(1/15)x**2+(2/315)x**4 */
  return 1.0 - (1.0/15.0)*x*x + (2.0/315.0)*x*x*x*x;
}

double
sin_func (double x) 
{
  /* use -1-(7/60)x**2-(31/2520)x**4 */
  return -1.0 - (7.0/60.0)*x*x - (31.0/2520.0)*x*x*x*x;
}

double
tan_func (double x) 
{
  /* use -1-(1/15)x**2-(2/315)x**4 */
  return -1.0 - (1.0/15.0)*x*x - (2.0/315.0)*x*x*x*x;
}


/* Following two functions compute (6/y^2)(sinh(xy)/sinh(y)-x) and
   (6/y^2)(sin(xy)/sin(y)-x), via the first three terms of the appropriate
   power series in y.  They are used when |y|<TRIG_ARG_MIN, to avoid loss
   of significance.  Errors are O(y**6). */
double
quotient_sinh_func (double x, double y) 
{
  return ((x*x*x-x) + (x*x*x*x*x/20.0 - x*x*x/6.0 + 7.0*x/60.0)*(y*y)
	  + (x*x*x*x*x*x*x/840.0 - x*x*x*x*x/120.0 + 7.0*x*x*x/360.0
	     -31.0*x/2520.0)*(y*y*y*y));
}

double
quotient_sin_func (double x, double y) 
{
  return (- (x*x*x-x) + (x*x*x*x*x/20.0 - x*x*x/6.0 + 7.0*x/60.0)*(y*y)
	  - (x*x*x*x*x*x*x/840.0 - x*x*x*x*x/120.0 + 7.0*x*x*x/360.0
	     -31.0*x/2520.0)*(y*y*y*y));
}
