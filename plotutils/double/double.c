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

/* This program, double, is a filter for converting, scaling, joining, and
   cutting data sets that are in the format accepted by the `spline' and
   `graph' utilities.  The data sets may be in binary (either double or
   single precision floating point format, or integer format), or in ascii.

   The `I', `O', and `q' options (i.e. `--input-type', `--output-type', and
   `--precision') are similar to those accepted by `spline' and `graph'.
   For example, `-I a' specifies that the input is in ascii format, and 
   `-O i' that the output should be in binary integer format.
   The length of each record in the input is specified with the `-R' option.
   For example, `-R 2' would be appropriate if the input consists of
   pairs of numbers (x,y).  The default record length is 1. 

   By default, `double' copies all fields of each input record to a
   corresponding output record.  Since `-R 1' is the default, without
   additional options it will simply copy an input stream of numbers to an
   output stream.  You can use the `-f' option to specify which fields you
   want copied.  For example, `-R 3 -f 2 0' would interpret the input as
   being made of size-3 records, and would produce output consisting of
   size-2 records, each of which would be made from field #2 and field #0
   of the corresponding input record.  (Fields are numbered starting with
   zero.)  In the `-f' specification, fields may be repeated.

   You can use the `-t' option to multiply all the input fields by a
   constant, and `-p' to add a constant to each of the input fields.

   `double' can also join streams of records together.  The `-j' and `-J'
   options are used to specify the names of files consisting of size-1
   records, called a `pre-file' and a `post-file'.  If you use `-j
   filename' in the above example, the output records will be of size 3
   rather than size 2, and the first field of each output record will be
   taken from the corresponding record in `filename'.  So the `-j' option
   `prepends' a component to each record.  Similarly, `-J' will append a
   component.  You can also use `-T' and `-P' to specify a `times file'
   and a `plus file', which will respectively multiply each record
   by a number taken from the times file, and add to the components of
   each record a number taken from the plus file.

   The `-d' (--dataset-limits) option takes three args: min, max, and
   spacing.  It specifies a linear progression through each dataset
   that is processed, and is useful for `thinning out' large datasets.

   `double' does not require that its input file(s) consist of only a
   single dataset.  However, the lengths of the corresponding datasets
   should be equal. */

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"

/* type of data in input and output streams */
typedef enum
{
  T_ASCII, T_SINGLE, T_DOUBLE, T_INTEGER
}
data_type;

data_type input_type = T_ASCII;
data_type output_type = T_ASCII;

const char *progname = "double"; /* name of this program */
const char *written = "Written by Robert S. Maier and Rich Murphey.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

int precision = 8;		/* default no. of digits after decimal pt. */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

struct option long_options[] =
{
  /* string arg */
  {"input-type",		ARG_REQUIRED,	NULL, 'I'},
  {"output-type",		ARG_REQUIRED,	NULL, 'O'},
  {"precision",			ARG_REQUIRED,	NULL, 'q'},
  /* file name arg */
  {"times-file",		ARG_REQUIRED,	NULL, 'T'},
  {"plus-file",			ARG_REQUIRED,	NULL, 'P'},
  {"pre-join-file",		ARG_REQUIRED,	NULL, 'j'},
  {"post-join-file",		ARG_REQUIRED,	NULL, 'J'},
  /* floating point arg */
  {"times",			ARG_REQUIRED,	NULL, 't'},
  {"plus",			ARG_REQUIRED,	NULL, 'p'},
  /* integer arg */
  {"record-length",		ARG_REQUIRED,	NULL, 'R'},
  {"fields",			ARG_OPTIONAL,	NULL, 'f'}, /* 0,1,2, or ... */
  {"dataset-limits",		ARG_OPTIONAL,	NULL, 'd'}, /* 0,1,2,3 args*/
  /* flags */
  {"version",			ARG_NONE,	NULL, 'V' << 8},
  {"help",			ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

/* null-terminated list of options that we don't show to the user */
int hidden_options[] = { 0 };

/* forward references */
bool mung_dataset (FILE *input, int record_length, int *field_array, int field_array_len, double scale, double baseline, FILE *add_fp, FILE *mult_fp, FILE *pre_join_fp, FILE *post_join_fp, int precision, bool suppress);
bool read_float (FILE *input, double *dptr);
bool skip_whitespace (FILE *stream);
bool write_float (double data, int precision);
int get_record (FILE *input, double *record, int record_length);
void maybe_emit_oob_warning (void);
void open_file (char *name, FILE **fpp);
void output_dataset_separator (void);
void set_format_type (char *s, data_type *typep);


int
main (int argc, char **argv)
{
  int option;			/* for option parsing */
  int opt_index;
  int errcnt = 0;		/* errors encountered in parsing */
  int i;
  bool show_version = false;	/* remember to show version message */
  bool show_usage = false;	/* remember to output usage message */
  char *add_file = NULL, *mult_file = NULL;
  char *pre_join_file = NULL, *post_join_file = NULL;
  FILE *add_fp = NULL, *mult_fp = NULL; 
  FILE *pre_join_fp = NULL, *post_join_fp = NULL;
  double scale = 1.0, baseline = 0.0; /* mult., additive constants */
  int record_length = 1;	/* default record length */
  int dataset_min = 0, dataset_max = INT_MAX, dataset_spacing = 1;  
  int local_dataset_min, local_dataset_max, local_dataset_spacing;  
  int *field_array = NULL;	/* array of indices we'll extract */
  int field_array_len = 0;	/* initial size of field_array[] */
  int dataset_index = 0;	/* running count */
  bool more_points, dataset_printed = false;

  for ( ; ; )
    {
      option = getopt_long (argc, argv, "I:O:q:T:P:j:J:t:p:R:f::d::", long_options, &opt_index);
      if (option == 0)
	option = long_options[opt_index].val;

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 'V' << 8:		/* display version */
	  show_version = true;
	  break;
	case 'h' << 8:		/* help */
	  show_usage = true;
	  break;

	  /* ----------- options with a single argument --------------*/

	case 'I':
	  set_format_type (optarg, &input_type);
	  break;
	case 'O':
	  set_format_type (optarg, &output_type);
	  break;

	case 'T':		/* Times file name, ARG REQUIRED */
	  mult_file = xstrdup (optarg);
	  break;
	case 'P':		/* Plus file name, ARG REQUIRED	*/
	  add_file = xstrdup (optarg);
	  break;
	case 'j':		/* Pre-join file name, ARG REQUIRED */
	  pre_join_file = xstrdup (optarg);
	  break;
	case 'J':		/* Post-join file name, ARG REQUIRED */
	  post_join_file = xstrdup (optarg);
	  break;

	case 't':		/* Times (mult. constant), ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &scale) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: the multiplicative constant `%s' is bad\n",
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  break;
	case 'p':		/* Plus (add. constant), ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &baseline) <= 0)
	    {
	      fprintf (stderr, 
		       "%s: error: the additive constant `%s' is bad\n", 
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  break;
	case 'q':		/* Precision, ARG REQUIRED 	*/
	  if ((sscanf (optarg, "%d", &precision) <= 0)
	      || (precision < 1))
	    {
	      fprintf (stderr,
		       "%s: error: the precision `%s' is bad (it should be an integer greater than or equal to 1)\n",
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  break;

	case 'R':		/* Number of data per record, ARG REQUIRED */
	  if ((sscanf (optarg, "%d", &record_length) <= 0)
	      || (record_length < 1))
	    {
	      fprintf (stderr,
		       "%s: error: the record length `%s' is bad (it should be an integer greater than or equal to 1)\n",
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  break;

	  /* ----- Options with a variable number of arguments ----- */

	case 'd':		/* Dataset limits, ARG OPTIONAL [0,1,2,3] */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%d", &local_dataset_min) <= 0)
	    break;
	  dataset_min = local_dataset_min;
	  optind++;	/* tell getopt we recognized dataset_min */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_dataset_max) <= 0)
	    break;
	  dataset_max = local_dataset_max;
	  optind++;	/* tell getopt we recognized dataset_max */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv [optind], "%d", &local_dataset_spacing) <= 0)
	    break;
	  dataset_spacing = local_dataset_spacing;
	  optind++;	/* tell getopt we recognized dataset_spacing */
	  break;

	case 'f':
	  for ( ; ; )
	    {
	      int field_index;

	      if (optind >= argc)
		break;
	      if (sscanf (argv[optind], "%d", &field_index) <= 0)
		break;
	      if (field_index < 0)
		{
		  fprintf (stderr, "%s: error: the field index `%d' is bad (it should be greater than or equal to 0)\n",
			   progname, field_index);
		  return EXIT_FAILURE;
		}
	      if (field_array_len == 0)
		field_array = 
		  (int *)xmalloc ((++field_array_len) * sizeof(int));
	      else
		field_array = 
		  (int *)xrealloc (field_array, 
				   (++field_array_len) * sizeof(int));
	      field_array[field_array_len - 1] = field_index;
	      optind++;		/* tell getopt we recognized field index */
	    }
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

  /* Sanity checks on user-supplied options */

  if (dataset_spacing < 1)
    {
      fprintf (stderr, "%s: error: the dataset spacing `%d' is bad (it should be positive)\n",
	       progname, dataset_spacing);
      return EXIT_FAILURE;
    }

  for (i = 0; i < field_array_len; i++)
    if (field_array[i] >= record_length)
      {
	fprintf (stderr, 
		 "%s: error: at least one field index is out of bounds\n", progname);
	return EXIT_FAILURE;
      }

  /* default if no `-R' option seen: extract all fields of each record */
  if (field_array_len == 0)
    {
      field_array = 
	(int *)xmalloc ((record_length) * sizeof(int));
      field_array_len = record_length;
      for (i = 0; i < field_array_len; i++)
	field_array[i] = i;
    }

  /* open additive/multiplicative/join files. */
  if (add_file)
    open_file (add_file, &add_fp);
  if (mult_file)
    open_file (mult_file, &mult_fp);
  if (pre_join_file)
    open_file (pre_join_file, &pre_join_fp);
  if (post_join_file)
    open_file (post_join_file, &post_join_fp);
  
  if (optind < argc)
    {
      /* call mung_dataset() on all datasets contained in
	 each file specified on command line */
      for (; optind < argc; optind++)
	{
	  FILE *data_fp;
	  
	  /* open file, treat "-" as stdin */
	  if (strcmp (argv[optind], "-") == 0)
	    data_fp = stdin;
	  else
	    open_file (argv[optind], &data_fp);

	  /* loop through datasets in file (may be more than one) */
	  do
	    {
	      bool dataset_ok;
	      
	      dataset_ok = ((dataset_index >= dataset_min)
			    && (dataset_index <= dataset_max)
			    && ((dataset_index - dataset_min) 
				% dataset_spacing == 0)) ? true : false;

	      /* output a separator between successive datasets */
	      if (dataset_printed && dataset_ok)
		output_dataset_separator();

	      more_points = mung_dataset (data_fp,
					  record_length, 
					  field_array, field_array_len,
					  scale, baseline,
					  add_fp, mult_fp, 
					  pre_join_fp, post_join_fp,
					  precision, dataset_ok ? false : true);

	      if (dataset_ok)
		dataset_printed = true;
	      
	      dataset_index++;
	    } 
	  while (more_points);
	  
	  /* close file (but don't close stdin) */
	  if (data_fp != stdin && fclose (data_fp) < 0)
	    {
	      fprintf (stderr, "%s: error: the input file could not be closed\n", 
		       progname);
	      return EXIT_FAILURE;
	    }
	}
    }
  else			/* no files spec'd, read stdin instead */
    /* loop through datasets (may be more than one) */
    do
      {
	bool dataset_ok;
	
	dataset_ok = ((dataset_index >= dataset_min)
		      && (dataset_index <= dataset_max)
		      && ((dataset_index - dataset_min) 
			  % dataset_spacing == 0)) ? true : false;

	/* output a separator between successive datasets */
	if (dataset_printed && dataset_ok)
	  output_dataset_separator();

	more_points = mung_dataset (stdin,
				    record_length, 
				    field_array, field_array_len,
				    scale, baseline,
				    add_fp, mult_fp, 
				    pre_join_fp, post_join_fp,
				    precision, dataset_ok ? false : true);
	if (dataset_ok)
	  dataset_printed = true;

	dataset_index++;
      }
    while (more_points);	/* keep going if no EOF yet */

  return EXIT_SUCCESS;
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
      fprintf (stderr, "%s: a NaN (not-a-number) was encountered in a binary-format input file\n",
	       progname);
      return false;
    }
  else
    {
      *dptr = dval;
      return true;
    }
}

/* get_record() attempts to read a record (a sequence of record_length
   data, i.e., floating-point quantities) from an input file.  Return
   value is 0 if a record was successfully read, 1 if no record could be
   read (i.e. EOF or garbage in stream).  A return value of 2 is special:
   it indicates that an explicit end-of-dataset indicator was seen in the
   input file.  For an ascii stream this is two newlines in succession;
   for a stream of doubles it is a DBL_MAX appearing at what would
   otherwise have been the beginning of the record, etc. */

int
get_record (FILE *input, double *record, int record_length)
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
    return 1;			/* EOF */

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

  for (i = 0; i < record_length; i++)
    {
      double val;

      success = read_float (input, &val);
      if (i == 0 && 
	  ((input_type == T_DOUBLE && val == DBL_MAX)
	   || (input_type == T_SINGLE && val == (double)FLT_MAX)
	   || (input_type == T_INTEGER && val == (double)INT_MAX)))
	/* end-of-dataset indicator */
	return 2;
      if (!success)		/* EOF or garbage */
	{
	  if (i > 0)
	    fprintf (stderr, "%s: the input file terminated prematurely\n",
		     progname);
	  return 1;
	}
      record[i] = val;
    }
  
  return 0;
}

/* Emit a double, in specified output representation.  Be sure to inform
   user if any of the emitted values were out-of-bounds for
   single-precision or integer format. */
bool
write_float (double x, int precision)
{
  int num_written = 0;
  float fx;
  int ix;

  switch (output_type)
    {
    case T_ASCII:
    default:
      num_written = printf ("%.*g ", precision, x);
      break;
    case T_SINGLE:
      fx = FROUND(x);
      if (fx == FLT_MAX || fx == -(FLT_MAX))
	{
	  maybe_emit_oob_warning();
	  if (fx == FLT_MAX)
	    fx *= 0.99999;	/* kludge */
	}
      num_written = fwrite ((void *) &fx, sizeof (fx), 1, stdout);
      break;
    case T_DOUBLE:
      num_written = fwrite ((void *) &x, sizeof (x), 1, stdout);
      break;
    case T_INTEGER:
      ix = IROUND(x);
      if (ix == INT_MAX || ix == -(INT_MAX))
	{
	  maybe_emit_oob_warning();
	  if (ix == INT_MAX)
	    ix--;
	}
      num_written = fwrite ((void *) &ix, sizeof (ix), 1, stdout);
      break;
    }
  if (num_written < 0)
    return false;
  else
    return true;
}

void
open_file (char *name, FILE **fpp)
{
  FILE *fp;

  fp = fopen (name, "r");
  if (fp == NULL)
    {
      fprintf (stderr, "%s: %s: %s\n", progname, name, strerror(errno));
      exit (EXIT_FAILURE);
    }
  *fpp = fp;
}

void
set_format_type (char *s, data_type *typep)
{
  switch (s[0])
    {
    case 'a':
    case 'A':
      /* ASCII format: records and fields within records are separated by
	 whitespace, and datasets are separated by a pair of newlines. */
      *typep = T_ASCII;
      break;
    case 'f':
    case 'F':
      /* Binary single precision: records and fields within records are
	 contiguous, and datasets are separated by a FLT_MAX.  */
      *typep = T_SINGLE;
      break;
    case 'd':
    case 'D':
      /* Binary double precision: records and fields within records are
	 contiguous, and datasets are separated by a DBL_MAX. */
      *typep = T_DOUBLE;
      break;
    case 'i':
    case 'I':
      /* Binary integer: records and fields within records are contiguous,
	 and datasets are separated by an occurrence of INT_MAX. */
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

/* mung_dataset() is the main routine for extracting fields from records in
   a dataset, and munging them.  Its return value indicates whether the
   records in the input file ended with an explicit end-of-dataset
   indicator, i.e., whether another dataset is expected to follow.  An
   end-of-dataset indicator is two newlines in succession for an ascii
   stream, and a DBL_MAX for a stream of doubles, etc. */

/* ARGS: record_length = number of fields per record in dataset
   	 field_array = array of fields we'll extract
	 field_array_len = length of this array
	 suppress = suppress output for this dataset? */
bool
mung_dataset (FILE *input, int record_length, 
	      int *field_array, int field_array_len,
	      double scale, double baseline, FILE *add_fp, FILE *mult_fp, 
	      FILE *pre_join_fp, FILE *post_join_fp, int precision, 
	      bool suppress)
{
  double *record = (double *)xmalloc (record_length * sizeof(double));
  bool in_trouble = suppress; /* once in trouble, we never get out */
  
  if (!in_trouble)
    {
      /* rewind all fp's */
      if (add_fp)
	fseek(add_fp, 0L, 0);
      if (mult_fp)
	fseek(mult_fp, 0L, 0);
      if (pre_join_fp)
	fseek(pre_join_fp, 0L, 0);
      if (post_join_fp)
	fseek(post_join_fp, 0L, 0);
    }

  for ( ; ; )
    {
      int i;
      int success;
      double add_data, mult_data, pre_join_data, post_join_data;

      if (!in_trouble && add_fp && read_float (add_fp, &add_data) == false)
	in_trouble = true;
      if (!in_trouble && mult_fp && read_float (mult_fp, &mult_data) == false)
	in_trouble = true;
      if (!in_trouble && pre_join_fp 
	  && read_float (pre_join_fp, &pre_join_data) == false)
	in_trouble = true;
      if (!in_trouble && post_join_fp 
	  && read_float (post_join_fp, &post_join_data) == false)
	in_trouble = true;
  
      success = get_record (input, record, record_length);

      switch (success)
	{
	case 0:			/* good record */
	  if (in_trouble)	/* if in trouble, do nought till dataset end */
	    continue;

	  if (pre_join_fp)
	    write_float (pre_join_data, precision);

	  for (i = 0; i < field_array_len; i++)
	    {
	      double datum;
	      
	      datum = record[field_array[i]];
	      if (mult_fp)
		datum *= mult_data;
	      if (add_fp)
		datum += add_data;
	      datum *= scale;
	      datum += baseline;
	      
	      /* output the munged datum */
	      write_float (datum, precision);
	    }
	      
	  if (post_join_fp)
	    write_float (post_join_data, precision);

	  if (output_type == T_ASCII) /* end each record with a newline */
	    printf ("\n");

	  break;
	case 1:			/* no more records, EOF seen */
	  return false;
	case 2:			/* end of dataset, but input continues */
	  return true;
	}
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

/* Output a separator between datasets.  For ascii-format output streams
   this is an extra newline (after the one that the spline ended with,
   yielding two newlines in succession).  For double-format output streams
   this is a DBL_MAX, etc. */

void
output_dataset_separator(void)
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
