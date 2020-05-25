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

/* This file is the driving routine for the GNU `plot' program.  It
   includes code to read a stream of commands, in GNU metafile format, and
   call libplot functions to draw the graphics. */

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"
#include "fontlist.h"
#include "plot.h"

/* Obsolete op codes (no longer listed in plot.h) */
#define O_COLOR 'C'
#define O_FROTATE 'V'
#define O_FSCALE 'X'
#define O_FTRANSLATE 'Q'

/* The six input formats we recognize */
typedef enum 
{
  /* There are two GNU metafile formats: binary and portable (ascii). */
  GNU_BINARY, GNU_PORTABLE, 

  /* PLOT5_HIGH and PLOT5_LOW are the two distinct versions of Unix plot(5)
     format (high/low byte first), which we also support.  They are
     requested by the -h and -l options respectively.  The user may not
     need to specify either of those options explicitly, since if
     sizeof(short int)=2 then plot(5) input format is subsumed by
     GNU_OLD_BINARY format (see below). */
  PLOT5_HIGH, PLOT5_LOW,

  /* GNU_OLD_BINARY [obsolete] is the binary format used in pre-2.0
     releases, with no initial magic string, short ints instead of ints,
     and no OPENPL or CLOSEPL directives.  By default, we assume that the
     input format is GNU_OLD_BINARY, and we switch to GNU_BINARY or
     GNU_PORTABLE if we see the appropriate magic header string.

     GNU_OLD_PORTABLE [obsolete] is the ascii format used in pre-2.0
     releases, with no initial magic string, and no OPENPL or CLOSEPL
     directives.  It subsumes the ascii version of plot(5) format, found on
     some Unix systems.  If the user wishes to parse GNU_OLD_PORTABLE
     format, he/she should use the -A option.  */
  GNU_OLD_BINARY, GNU_OLD_PORTABLE

} plot_format;

const char *progname = "plot";	/* name of this program */
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

bool single_page_is_requested = false; /* set if user uses -p option */
char *bg_color = NULL;		/* initial bg color, can be spec'd by user */
char *font_name = NULL;		/* initial font name, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
double font_size = -1.0;	/* initial fractional size, <0 means default */
double line_width = -1.0;	/* initial line width, <0 means default */
int requested_page = 0;		/* user sets this via -p option */

/* Default input file format (see list of supported formats above).  Don't
   change this (GNU_OLD_BINARY is an obsolete format, but it subsumes
   plot(5) format on many operating systems).  We'll switch to the
   appropriate modern format by peeking at the first line of the input file. */
plot_format user_specified_input_format = GNU_OLD_BINARY;
plot_format input_format = GNU_OLD_BINARY;

/* Whether to remove all page breaks and frame breaks (i.e. invocations
   of erase()) from the output */
bool merge_pages = false;

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "shlAIOp:F:f:W:T:";

struct option long_options[] = 
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* hidden */
  /* Other frequently used options */
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "font-size",	ARG_REQUIRED,	NULL, 'f' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  /* Long options with (mostly) no equivalent short option alias */
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "emulate-color",	ARG_REQUIRED,	NULL, 'e' << 8},  
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "merge-pages",	ARG_NONE,	NULL, 's' },
  { "page-number",	ARG_REQUIRED,	NULL, 'p' },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  /* Options relevant only to raw plot (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Old input formats, for backward compatibility */
  { "high-byte-first-input",	ARG_NONE,	NULL, 'h' },
  { "low-byte-first-input",	ARG_NONE,	NULL, 'l' },
  { "ascii-input",		ARG_NONE,	NULL, 'A' },
  /* obsolete hidden option [same as 'A'] */
  { "ascii-input",		ARG_NONE,	NULL, 'I' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "list-fonts",	ARG_NONE,	NULL, 'l' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    
/* null-terminated list of options, such as obsolete-but-still-maintained
   options or undocumented options, which we don't show to the user */
const int hidden_options[] = { (int)'I', (int)('T' << 8), 0 };


/* forward references */
bool read_plot (plPlotter *plotter, FILE *in_stream);
char *read_string (FILE *input, bool *badstatus);
double read_float (FILE *input, bool *badstatus);
double read_int (FILE *input, bool *badstatus);
int maybe_closepl (plPlotter *plotter);
int maybe_openpl (plPlotter *plotter);
int read_true_int (FILE *input, bool *badstatus);
unsigned char read_byte_as_unsigned_char (FILE *input, bool *badstatus);
unsigned int read_byte_as_unsigned_int (FILE *input, bool *badstatus);


int
main (int argc, char *argv[])
{
  plPlotter *plotter;
  plPlotterParams *plotter_params;
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  char *output_format = (char *)"meta"; /* default libplot output format */
  int errcnt = 0;		/* errors encountered */
  int local_page_number;	/* temporary storage */
  int opt_index;		/* long option index */
  int option;			/* option character */
  int retval;			/* return value */

  plotter_params = pl_newplparams ();
  while ((option = getopt_long (argc, argv, optstring, long_options, &opt_index)) != EOF)
    {
      if (option == 0)
	option = long_options[opt_index].val;
      
      switch (option) 
	{
	case 'T':		/* Output format, ARG REQUIRED      */
	case 'T' << 8:
	  output_format = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (output_format, optarg);
	  break;
	case 'O':		/* Ascii output */
	  pl_setplparam (plotter_params, "META_PORTABLE", (void *)"yes");
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
	  break;
	case 'e' << 8:		/* emulate color by grayscale */
	  pl_setplparam (plotter_params, "EMULATE_COLOR", (void *)optarg);
	  break;
	case 'C' << 8:		/* set the initial pen color */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'q' << 8:		/* set the initial background color */
	  bg_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (bg_color, optarg);
	  break;
	case 'B' << 8:		/* Bitmap size */
	  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  pl_setplparam (plotter_params, "PAGESIZE", (void *)optarg);
	  break;
	case 'f':		/* set the initial fontsize */
	  {
	    double local_font_size;

	    if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: the initial font size `%s' is bad (it should be a number)\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_font_size > 1.0)
	      fprintf (stderr, "%s: the too-large initial font size `%f' is disregarded (it should be less than 1.0)\n", 
		       progname, local_font_size);
	    else if (local_font_size < 0.0)
	      fprintf (stderr, "%s: the negative initial font size `%f' is disregarded\n",
		       progname, local_font_size);
	    else
	      font_size = local_font_size;
	    break;
	  }
	case 'p':		/* page number */
	  if (sscanf (optarg, "%d", &local_page_number) <= 0
	      || local_page_number < 1)
	    {
	      fprintf (stderr,
		       "%s: error: the page number `%s' is bad (it should be a positive integer)\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      requested_page = local_page_number;
	      single_page_is_requested = true;
	    }
	  break;
	case 'W':		/* set the initial line width */
	  {
	    double local_line_width;

	    if (sscanf (optarg, "%lf", &local_line_width) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: the initial line thickness `%s' is bad (it should be a number)\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_line_width < 0.0)
	      fprintf (stderr, "%s: the negative initial line thickness `%f' is ignored\n",
		       progname, local_line_width);
	    else
	      line_width = local_line_width;
	    break;
	  }
	case 'h':	/* High-byte-first plot(5) metafile(s) */
	  user_specified_input_format = PLOT5_HIGH;
	  break;
	case 'l':	/* Low-byte-first plot(5) metafile(s) */
	  user_specified_input_format = PLOT5_LOW;
	  break;
	case 'A':		/* Old ascii metafile(s) */
	case 'I':
	  user_specified_input_format = GNU_OLD_PORTABLE;
	  break;
	case 'r' << 8:		/* Plot rotation angle, ARG REQUIRED	*/
	  pl_setplparam (plotter_params, "ROTATION", (void *)optarg);
	  break;
	case 'M' << 8:		/* Max line length */
	  pl_setplparam (plotter_params, "MAX_LINE_LENGTH", (void *)optarg);
	  break;
	case 's':		/* Merge pages */
	  merge_pages = true;
	  break;

	case 'V' << 8:		/* Version */
	  show_version = true;
	  break;
	case 'f' << 8:		/* Fonts */
	  show_fonts = true;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = true;
	  break;
	case 'l' << 8:		/* Fonts */
	  do_list_fonts = true;
	  break;

	default:
	  errcnt++;
	  break;
	}
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
  if (do_list_fonts)
    {
      int success;

      success = list_fonts (output_format, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_fonts)
    {
      int success;

      success = display_fonts (output_format, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, usage_appendage, 2);
      return EXIT_SUCCESS;
    }

  if (bg_color)
    /* select user-specified background color */
    pl_setplparam (plotter_params, "BG_COLOR", (void *)bg_color);

  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr,
			     plotter_params)) == NULL)
    {
      fprintf (stderr, "%s: error: the plot device could not be created\n", progname);
      return EXIT_FAILURE;
    }

  if (merge_pages)
    /* we do just one openpl..closepl, wrapped around everything */
    if (pl_openpl_r (plotter) < 0)
      {
	fprintf (stderr, "%s: error: the plot device could not be opened\n",
		 progname);
	return EXIT_FAILURE;
      }

  retval = EXIT_SUCCESS;
  if (optind < argc)
    /* input files (or stdin) named explicitly on the command line */
    {
      for (; optind < argc; optind++)
	{
	  FILE *data_file;
	  
	  if (strcmp (argv[optind], "-") == 0)
	    data_file = stdin;
	  else
	    {
	      data_file = fopen (argv[optind], "r");
	      if (data_file == NULL)
		{
		  fprintf (stderr, "%s: %s: %s\n", progname, argv[optind], strerror(errno));
		  fprintf (stderr, "%s: ignoring this file\n", progname);
		  errno = 0;	/* not quite fatal */
		  retval = EXIT_FAILURE;
		  continue;	/* back to top of for loop */
		}
	    }
	  if (read_plot (plotter, data_file) == false)
	    {
		  fprintf (stderr, "%s: the input file `%s' could not be parsed\n",
			   progname, argv[optind]);
		  retval = EXIT_FAILURE;
		  break;	/* break out of for loop */
	    }

	  if (data_file != stdin) /* Don't close stdin */
	    if (fclose (data_file) < 0)
	      {
		fprintf (stderr, 
			 "%s: the input file `%s' could not be closed\n",
			 progname, argv[optind]);
		retval = EXIT_FAILURE;
		continue;	/* back to top of for loop */
	      }
	} /* endfor */
    }
  else
    /* no files/streams spec'd on the command line, just read stdin */
    {
      if (read_plot (plotter, stdin) == false)
	{
	  fprintf (stderr, "%s: the input could not be parsed\n", progname);
	  retval = EXIT_FAILURE;
	}
    }

  if (merge_pages)
    /* we do just one openpl..closepl, wrapped around everything */
    if (pl_closepl_r (plotter) < 0)
      {
	fprintf (stderr, "%s: error: the plot device could not be closed\n",
		 progname);
	return EXIT_FAILURE;
      }

  if (pl_deletepl_r (plotter) < 0)
    {
      fprintf (stderr, "%s: error: the plot device could not be deleted\n", progname);
      retval = EXIT_FAILURE;
    }
  pl_deleteplparams (plotter_params);

  return retval;
}


/* read_plot() reads a file in GNU metafile format or plot(5) format from a
   stream, and calls a plot function according to each instruction found in
   the file.  Return value indicates whether stream was parsed
   successfully. */
bool
read_plot (plPlotter *plotter, FILE *in_stream)
{
  bool argerr = false;	/* error occurred while reading argument? */
  bool display_open = false;	/* display device open? */
  bool first_command = true;	/* first command of file? */
  bool in_page = false;		/* within an openpl..closepl? */
  bool parameters_initted = false; /* user-specified parameters initted? */
  bool unrec = false;	/* unrecognized command seen? */
  char *s;
  double x0, y0, x1, y1, x2, y2, x3, y3;
  int i0, i1, i2;
  int instruction;
  static int current_page = 1;	/* page count is continued from file to file */
  
  /* User may specify one of the formats PLOT5_HIGH, PLOT5_LOW, and
     GNU_OLD_PORTABLE on the command line.  If user doesn't specify a
     format, this is by default set to GNU_OLD_BINARY [obsolete], and we'll
     figure out whether the file is in a modern format, and if so, 
     which one. */
  input_format = user_specified_input_format;

  /* peek at first instruction in file */
  instruction = getc (in_stream);

  /* Switch away from GNU_OLD_BINARY to GNU_BINARY if a GNU metafile magic
     string, interpreted here as a comment, is seen at top of file.  See
     also parsing of the COMMENT instruction below (we further switch to
     GNU_PORTABLE if the header line indicates we should). */
  if (input_format == GNU_OLD_BINARY && instruction == (int)O_COMMENT)
    input_format = GNU_BINARY;

/* Note: we use `input_format' as a way of working around a problem:
   absurdly large font size requests, which can crash X servers.  (You used
   to be able to crash an X server by piping any EPS file to `plot -TX',
   since the `S' on the first line was interepreted as an op code for a
   font size request!)  We no longer process the `S' op code unless we've
   seen a modern GNU metafile magic string at the beginning of the file.
   This is a kludge but adds a little safety. */

  while (instruction != EOF)
    {
      /* If a pre-modern format, OPENPL directive is not supported.  So
	 open display device if it hasn't already been opened, and
	 we're on the right page. */
      if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	if ((!single_page_is_requested || current_page == requested_page)
	    && instruction != (int)O_COMMENT && display_open == false)
	  {
	    if (maybe_openpl (plotter) < 0)
	      {
		fprintf (stderr, "%s: error: the plot device could not be opened\n", 
			 progname);
		exit (EXIT_FAILURE);
	      }
	    else
	      display_open = true;
	  }
  
      switch (instruction)
	{
	case (int)O_ALABEL:
	  {
	    char x_adjust, y_adjust;

	    x_adjust = (char)read_byte_as_unsigned_char (in_stream, &argerr);
	    y_adjust = (char)read_byte_as_unsigned_char (in_stream, &argerr); 
	    s = read_string (in_stream, &argerr);
	    if (!argerr)
	      {
		if (!single_page_is_requested || current_page == requested_page)
		  pl_alabel_r (plotter, x_adjust, y_adjust, s);
		free (s);
	      }
	  }
	  break;
	case (int)O_ARC:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_farc_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_ARCREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_farcrel_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_BEZIER2:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier2_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_BEZIER2REL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier2rel_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_BEZIER3:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  x3 = read_int (in_stream, &argerr);
	  y3 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier3_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
	  break;
	case (int)O_BEZIER3REL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  x3 = read_int (in_stream, &argerr);
	  y3 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier3rel_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
	  break;
	case (int)O_BGCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_bgcolor_r (plotter, i0, i1, i2);
	  break;
	case (int)O_BOX:
	  	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbox_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_BOXREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fboxrel_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_CAPMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_capmod_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_CIRCLE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcircle_r (plotter, x0, y0, x1);
	  break;
	case (int)O_CIRCLEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcirclerel_r (plotter, x0, y0, x1);
	  break;
	case (int)O_COLOR:	/* obsolete op code, to be removed */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_color_r (plotter, i0, i1, i2);
	  break;
	case (int)O_CLOSEPATH:
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_closepath_r (plotter);
	  break;
	case (int)O_CLOSEPL:
	  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	    /* shouldn't be seeing a CLOSEPL */
	    {
	      if (display_open && maybe_closepl (plotter) < 0)
		{
		  fprintf (stderr, "%s: error: the plot device could not be closed\n",
			   progname);
		  exit (EXIT_FAILURE);
		}
	      current_page++;
	      return false;	/* signal a parse error */
	    }
	  else
	    /* GNU_BINARY or GNU_PORTABLE format, so this may be legitimate */
	    {
	      if (in_page == false)
		/* shouldn't be seeing a CLOSEPL */
		{
		  current_page++;
		  return false;	/* signal a parse error */
		}
	      else
		/* the CLOSEPL is legitimate */
		{
		  if (!single_page_is_requested 
		      || current_page == requested_page)
		    {
		      if (maybe_closepl (plotter) < 0)
			{
			  fprintf (stderr, 
				   "%s: error: the plot device could not be closed\n", 
				   progname);
			  exit (EXIT_FAILURE);
			}
		      display_open = false;
		    }
		  in_page = false;
		  current_page++; /* `page' is an OPENPL..CLOSEPL */
		}
	    }
	  break;
	case (int)O_COMMENT:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      /* if a header line, switch to appropriate modern format */
	      if (first_command
		  && input_format != PLOT5_HIGH
		  && input_format != PLOT5_LOW
		  && (strlen (s) >= 6)
		  /* check magic number */
		  && strncmp (s, "PLOT ", 5) == 0)
		switch (s[5])
		  {
		  case '1':
		    input_format = GNU_BINARY;
		    break;		
		  case '2':
		    input_format = GNU_PORTABLE;
		    break;		
		  default:
		    fprintf (stderr, 
			     "%s: the input file is of an unrecognized metafile type\n",
			     progname);
		    break;
		  }
	      free (s);
	    }
	  break;
	case (int)O_CONT:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcont_r (plotter, x0, y0);
	  break;
	case (int)O_CONTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcontrel_r (plotter, x0, y0);
	  break;
	case (int)O_ELLARC:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellarc_r (plotter, x0, y0, x1, y1, x2, y2);	  
	  break;
	case (int)O_ELLARCREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellarcrel_r (plotter, x0, y0, x1, y1, x2, y2);	  
	  break;
	case (int)O_ELLIPSE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellipse_r (plotter, x0, y0, x1, y1, x2);
	  break;
	case (int)O_ELLIPSEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  x2 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellipserel_r (plotter, x0, y0, x1, y1, x2);
	  break;
	case (int)O_ENDPATH:
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_endpath_r (plotter);
	  break;
	case (int)O_ENDSUBPATH:
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_endsubpath_r (plotter);
	  break;
	case (int)O_ERASE:
	  if (!single_page_is_requested || current_page == requested_page)
	    if (merge_pages == false) /* i.e. not merging frames */
	      pl_erase_r (plotter);
	  break;
	case (int)O_FILLCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fillcolor_r (plotter, i0, i1, i2);
	  break;
	case (int)O_FILLMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_fillmod_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_FILLTYPE:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_filltype_r (plotter, i0);
	  break;
	case (int)O_FONTNAME:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_fontname_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_FONTSIZE:
	  x0 = read_int (in_stream, &argerr);
	  if (input_format == GNU_BINARY || input_format == GNU_PORTABLE)
	    /* workaround, see comment above */
	    {
	      if (!argerr)
		if (!single_page_is_requested 
		    || current_page == requested_page)
		  pl_ffontsize_r (plotter, x0);
	    }
	  break;
	case (int)O_JOINMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_joinmod_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_LABEL:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_label_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_LINE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fline_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_LINEDASH:
	  {
	    int n, i;
	    double *dash_array, phase;

	    n = read_true_int (in_stream, &argerr);
	    if (n > 0)
	      dash_array = (double *)xmalloc((unsigned int)n * sizeof(double));
	    else
	      dash_array = NULL;
	    for (i = 0; i < n; i++)
	      dash_array[i] = read_int (in_stream, &argerr);
	    phase = read_int (in_stream, &argerr);
	    if (!argerr)
	      if (!single_page_is_requested || current_page == requested_page)
		pl_flinedash_r (plotter, n, dash_array, phase);
	    free (dash_array);
	    break;
	  }
	case (int)O_LINEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_flinerel_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_LINEMOD:
	  s = read_string (in_stream, &argerr);
	  if (!argerr)
	    {
	      if (!single_page_is_requested || current_page == requested_page)
		pl_linemod_r (plotter, s);
	      free (s);
	    }
	  break;
	case (int)O_LINEWIDTH:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_flinewidth_r (plotter, x0);
	  break;
	case (int)O_MARKER:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmarker_r (plotter, x0, y0, i0, y1);
	  break;
	case (int)O_MARKERREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmarkerrel_r (plotter, x0, y0, i0, y1);
	  break;
	case (int)O_MOVE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmove_r (plotter, x0, y0);
	  break;
	case (int)O_MOVEREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmoverel_r (plotter, x0, y0);
	  break;
	case (int)O_OPENPL:
	  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
	    /* shouldn't be seeing an OPENPL */
	    {
	      if (display_open && maybe_closepl (plotter) < 0)
		{
		  fprintf (stderr, "%s: error: the plot device could not be closed\n",
			   progname);
		  exit (EXIT_FAILURE);
		}
	      current_page++;
	      return false;	/* signal a parse error */
	    }
	  else
	    /* GNU_BINARY or GNU_PORTABLE format, so may be legitimate */
	    {
	      if (in_page)
		/* shouldn't be seeing another OPENPL */
		{
		  if (display_open && maybe_closepl (plotter) < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: the plot device could not be closed\n",
			       progname);
		      exit (EXIT_FAILURE);
		    }
		  current_page++;
		  return false;	/* signal a parse error */
		}

	      /* this OPENPL is legitimate */
	      if (!single_page_is_requested || current_page == requested_page)
		{
		  if (maybe_openpl (plotter) < 0)
		    {
		      fprintf (stderr, 
			       "%s: error: the plot device could not be opened\n", 
			       progname);
		      exit (EXIT_FAILURE);
		    }
		  else
		    display_open = true;
		}
	      /* we're now in an openpl..closepl pair */
	      in_page = true;
	    }
	  break;
	case (int)O_ORIENTATION:
	  i0 = read_true_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_orientation_r (plotter, i0);
	  break;
	case (int)O_PENCOLOR:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i1 = read_true_int (in_stream, &argerr)&0xFFFF;
	  i2 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_pencolor_r (plotter, i0, i1, i2);
	  break;
	case (int)O_PENTYPE:
	  /* parse args as unsigned ints rather than ints */
	  i0 = read_true_int (in_stream, &argerr)&0xFFFF;
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_pentype_r (plotter, i0);
	  break;
	case (int)O_POINT:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fpoint_r (plotter, x0, y0);
	  break;
	case (int)O_POINTREL:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fpointrel_r (plotter, x0, y0);
	  break;
	case (int)O_RESTORESTATE:
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_restorestate_r (plotter);
	  break;
	case (int)O_SAVESTATE:
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_savestate_r (plotter);
	  break;
	case (int)O_SPACE:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_fspace_r (plotter, x0, y0, x1, y1);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to space(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (pen_color)
		pl_pencolorname_r (plotter, pen_color);
	      if (font_name)
		pl_fontname_r (plotter, font_name);
	      if (font_size >= 0.0)
		pl_ffontsize_r (plotter, font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		pl_flinewidth_r (plotter, line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  break;
	case (int)O_SPACE2:
	  x0 = read_int (in_stream, &argerr);
	  y0 = read_int (in_stream, &argerr);
	  x1 = read_int (in_stream, &argerr);
	  y1 = read_int (in_stream, &argerr); 
	  x2 = read_int (in_stream, &argerr);
	  y2 = read_int (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_fspace2_r (plotter, x0, y0, x1, y1, x2, y2);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to space2(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  pl_bgcolorname_r (plotter, bg_color);
		  pl_erase_r (plotter);
		}
	      if (pen_color)
		pl_pencolorname_r (plotter, pen_color);
	      if (font_name)
		pl_fontname_r (plotter, font_name);
	      if (font_size >= 0.0)
		pl_ffontsize_r (plotter, font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		pl_flinewidth_r (plotter, line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  break;
	case (int)O_TEXTANGLE:
	  x0 = read_int (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_ftextangle_r (plotter, x0);
	  break;

        /* floating point counterparts to some of the above */
	case (int)O_FARC:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_farc_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FARCREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_farcrel_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FBEZIER2:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier2_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FBEZIER2REL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier2rel_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FBEZIER3:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  x3 = read_float (in_stream, &argerr);
	  y3 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier3_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
	  break;
	case (int)O_FBEZIER3REL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  x3 = read_float (in_stream, &argerr);
	  y3 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbezier3rel_r (plotter, x0, y0, x1, y1, x2, y2, x3, y3);
	  break;
	case (int)O_FBOX:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fbox_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_FBOXREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fboxrel_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_FCIRCLE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcircle_r (plotter, x0, y0, x1);
	  break;
	case (int)O_FCIRCLEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcirclerel_r (plotter, x0, y0, x1);
	  break;
	case (int)O_FCONT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcont_r (plotter, x0, y0);
	  break;
	case (int)O_FCONTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fcontrel_r (plotter, x0, y0);
	  break;
	case (int)O_FELLARC:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellarc_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FELLARCREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellarcrel_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FELLIPSE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellipse_r (plotter, x0, y0, x1, y1, x2);
	  break;
	case (int)O_FELLIPSEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  x2 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fellipserel_r (plotter, x0, y0, x1, y1, x2);
	  break;
	case (int)O_FFONTSIZE:
	  x0 = read_float (in_stream, &argerr);
	  if (input_format == GNU_BINARY || input_format == GNU_PORTABLE)
	    /* workaround, see comment above */
	    {
	      if (!argerr)
		if (!single_page_is_requested || current_page == requested_page)
		  pl_ffontsize_r (plotter, x0);
	    }
	  break;
	case (int)O_FLINE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fline_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_FLINEDASH:
	  {
	    int n, i;
	    double *dash_array, phase;

	    n = read_true_int (in_stream, &argerr);
	    if (n > 0)
	      dash_array = (double *)xmalloc((unsigned int)n * sizeof(double));
	    else
	      dash_array = NULL;
	    for (i = 0; i < n; i++)
	      dash_array[i] = read_float (in_stream, &argerr);
	    phase = read_float (in_stream, &argerr);
	    if (!argerr)
	      if (!single_page_is_requested || current_page == requested_page)
		pl_flinedash_r (plotter, n, dash_array, phase);
	    free (dash_array);
	    break;
	  }
	case (int)O_FLINEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_flinerel_r (plotter, x0, y0, x1, y1);
	  break;
	case (int)O_FLINEWIDTH:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_flinewidth_r (plotter, x0);
	  break;
	case (int)O_FMARKER:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmarker_r (plotter, x0, y0, i0, y1);
	  break;
	case (int)O_FMARKERREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  i0 = read_true_int (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmarkerrel_r (plotter, x0, y0, i0, y1);
	  break;
	case (int)O_FMOVE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmove_r (plotter, x0, y0);
	  break;
	case (int)O_FMOVEREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmoverel_r (plotter, x0, y0);
	  break;
	case (int)O_FPOINT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fpoint_r (plotter, x0, y0);
	  break;
	case (int)O_FPOINTREL:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_fpointrel_r (plotter, x0, y0);
	  break;
	case (int)O_FSPACE:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
	    pl_fspace_r (plotter, x0, y0, x1, y1);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to fspace(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  pl_bgcolorname_r (plotter, bg_color);
		  pl_erase_r (plotter);
		}
	      if (pen_color)
		pl_pencolorname_r (plotter, pen_color);
	      if (font_name)
		pl_fontname_r (plotter, font_name);
	      if (font_size >= 0.0)
		pl_ffontsize_r (plotter, font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		pl_flinewidth_r (plotter, line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  break;
	case (int)O_FSPACE2:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (argerr)
	    break;
	  if (!single_page_is_requested || current_page == requested_page)
		pl_fspace2_r (plotter, x0, y0, x1, y1, x2, y2);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to fspace2(), if user insists on
	       including them (should estimate sizes better) */
	    {
	      if (bg_color)
		{
		  pl_bgcolorname_r (plotter, bg_color);
		  pl_erase_r (plotter);
		}
	      if (pen_color)
		pl_pencolorname_r (plotter, pen_color);
	      if (font_name)
		pl_fontname_r (plotter, font_name);
	      if (font_size >= 0.0)
		pl_ffontsize_r (plotter, font_size * fabs (x1 - x0));
	      if (line_width >= 0.0)
		pl_flinewidth_r (plotter, line_width * fabs (x1 - x0));
	      parameters_initted = true;
	    }
	  break;
	case (int)O_FTEXTANGLE:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_ftextangle_r (plotter, x0);
	  break;

        /* floating point routines with no integer counterpart */
	case (int)O_FCONCAT:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fconcat_r (plotter, x0, y0, x1, y1, x2, y2);
	  break;
	case (int)O_FMITERLIMIT:
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fmiterlimit_r (plotter, x0);
	  break;
	case (int)O_FSETMATRIX:
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  x1 = read_float (in_stream, &argerr);
	  y1 = read_float (in_stream, &argerr); 
	  x2 = read_float (in_stream, &argerr);
	  y2 = read_float (in_stream, &argerr); 
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fsetmatrix_r (plotter, x0, y0, x1, y1, x2, y2);
	  if (parameters_initted == false && 
	      ((!single_page_is_requested && current_page == 1)
	      || (single_page_is_requested && current_page == requested_page)))
	    /* insert these after the call to fsetmatrix(), if user insists
	       on including them (should estimate sizes better) */
	    {
	      if (pen_color)
		pl_pencolorname_r (plotter, pen_color);
	      if (font_name)
		pl_fontname_r (plotter, font_name);
	      if (x0 != 0.0)
		{
		  if (font_size >= 0.0)
		    pl_ffontsize_r (plotter, font_size / fabs (x0));
		  if (line_width >= 0.0)
		    pl_flinewidth_r (plotter, line_width / fabs (x0));
		}
	      parameters_initted = true;
	    }
	  break;
	case (int)O_FROTATE:	/* obsolete op code, to be removed */
	  x0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_frotate_r (plotter, x0);
	  break;
	case (int)O_FSCALE:	/* obsolete op code, to be removed */
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_fscale_r (plotter, x0, y0);
	  break;
	case (int)O_FTRANSLATE:	/* obsolete op code, to be removed */
	  x0 = read_float (in_stream, &argerr);
	  y0 = read_float (in_stream, &argerr);
	  if (!argerr)
	    if (!single_page_is_requested || current_page == requested_page)
	      pl_ftranslate_r (plotter, x0, y0);
	  break;
	case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
        case '\f':
	  /* extra whitespace is all right in portable formats */
	  if (input_format == GNU_PORTABLE
	      || input_format == GNU_OLD_PORTABLE)
	    break;
	  else			/* not harmless */
	    unrec = true;
	  break;
	default:
	  unrec = true;
	  break;
	} /* end of switch() */
      
      first_command = false;

      if (unrec)
	{
	  fprintf (stderr, "%s: an unrecognized command `0x%x' was encountered in the input\n",
		   progname, instruction);
	  break;		/* break out of while loop */
	}
      if (argerr)
	{
	  int eof = feof (in_stream);
	  
	  if (eof)
	    fprintf (stderr, "%s: the input terminated prematurely\n",
		     progname);
	  else
	    fprintf (stderr, "%s: the argument of the command `0x%x' in the input could not be parsed\n",
		     progname, instruction);
	  break;		/* break out of while loop */
	}
      
      instruction = getc (in_stream); /* get next instruction */
    } /* end of while loop, EOF reached */

  if (input_format != GNU_BINARY && input_format != GNU_PORTABLE)
    /* if a premodern format, this file contains only one page */
    {
      /* close display device at EOF, if it was ever opened */
      if (display_open && maybe_closepl (plotter) < 0)
	{
	  fprintf (stderr, "%s: error: the plot device could not be closed\n",
		   progname);
	  exit (EXIT_FAILURE);
	}
      current_page++;		/* bump page count at EOF */
    }
  else
    /* file is in a modern format, should have closed display device (if it
       was ever opened) */
    {
      if (in_page)
	/* shouldn't be the case; parse error */
	{
	  if (display_open && maybe_closepl (plotter) < 0)
	    {
	      fprintf (stderr, "%s: error: the plot device could not be closed\n",
		       progname);
	      exit (EXIT_FAILURE);
	    }
	  current_page++;
	  return false;		/* signal parse error */
	}
    }

  return ((argerr || unrec) ? false : true); /* file parsed successfully? */
}

int
maybe_openpl (plPlotter *plotter)
{
  if (merge_pages)
    return 0;
  else
    return (pl_openpl_r (plotter));
}

int
maybe_closepl (plPlotter *plotter)
{
  if (merge_pages)
    return 0;
  else
    return (pl_closepl_r (plotter));
}


/* read a single byte from input stream, return as unsigned char (0..255) */
unsigned char
read_byte_as_unsigned_char (FILE *input, bool *badstatus)
{
  int newint;

  if (*badstatus == true)
    return 0;

  newint = getc (input);
  /* have an unsigned char cast to an int, in range 0..255 */
  if (newint == EOF)
    {
      *badstatus = true;
      return 0;
    }
  else
    return (unsigned char)newint;
}

/* read a single byte from input stream, return as unsigned int (0..255) */
unsigned int
read_byte_as_unsigned_int (FILE *input, bool *badstatus)
{
  int newint;

  if (*badstatus == true)
    return 0;

  newint = getc (input);
  /* have an unsigned char cast to an int, in range 0..255 */
  if (newint == EOF)
    {
      *badstatus = true;
      return 0;
    }
  else
    return (unsigned int)newint;
}

/* read an integer from input stream (can be in ascii format, system binary
   format for integers or short integers, or perhaps in crufty old 2-byte
   format) */
int
read_true_int (FILE *input, bool *badstatus)
{
  int x, zi, returnval;
  short zs;
  unsigned int u;

  if (*badstatus == true)
    return 0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      returnval = fscanf (input, " %d", &x);
      if (returnval != 1)
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(zi), 1, input);
      if (returnval == 1)
	x = zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(zs), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      u = ((read_byte_as_unsigned_int (input, badstatus)) << 8);
      u |= read_byte_as_unsigned_int (input, badstatus);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      u = read_byte_as_unsigned_int (input, badstatus);
      u |= (read_byte_as_unsigned_int (input, badstatus) << 8);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    }

  return x;
}
  
/* a relaxed version of the preceding routine: if a portable
   (human-readable) format is used, a floating point number may substitute
   for the integer */
double
read_int (FILE *input, bool *badstatus)
{
  int x, zi, returnval;
  short zs;
  unsigned int u;

  if (*badstatus == true)
    return 0.0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      {
	double r;

	returnval = fscanf (input, " %lf", &r);
	if (returnval != 1)
	  {
	    *badstatus = true;
	    r = 0.0;
	  }
	return r;
      }
    case GNU_BINARY:		/* system format for integers */
    default:
      returnval = fread (&zi, sizeof(zi), 1, input);
      if (returnval == 1)
	x = (int)zi;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case GNU_OLD_BINARY:	/* system format for short integers */
      returnval = fread (&zs, sizeof(zs), 1, input);
      if (returnval == 1)
	x = (int)zs;
      else
	{
	  x = 0;
	  *badstatus = true;
	}
      break;
    case PLOT5_HIGH:		/* two-byte format, high byte first */
      u = ((read_byte_as_unsigned_int (input, badstatus)) << 8);
      u |= read_byte_as_unsigned_int (input, badstatus);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    case PLOT5_LOW:		/* two-byte format, low byte first */
      u = read_byte_as_unsigned_int (input, badstatus);
      u |= (read_byte_as_unsigned_int (input, badstatus) << 8);
      if (u > 0x7fff)
	x = - (int)(0x10000 - u);
      else
	x = (int)u;
      break;
    }

  return (double)x;
}
  
/* read a floating point quantity from input stream (may be in ascii format
   or system single-precision format) */
double
read_float (FILE *input, bool *badstatus)
{
  float f;
  int returnval;
  
  if (*badstatus == true)
    return 0;

  switch (input_format)
    {
    case GNU_PORTABLE:
    case GNU_OLD_PORTABLE:
      /* human-readable format */
      returnval = fscanf (input, " %f", &f);
      break;
    case GNU_BINARY:
    case GNU_OLD_BINARY:
    default:
      /* system single-precision format */
      returnval = fread (&f, sizeof(f), 1, input);
      break;
    case PLOT5_HIGH:
    case PLOT5_LOW:
      /* plot(5) didn't support floats */
      returnval = 0;
      break;
    }

  if (returnval != 1 || f != f)
    /* failure, or NaN */
    {
      *badstatus = true;
      return 0.0;
    }
  else
    return (double)f;
}

/* Read a newline-terminated string from input stream.  As returned, the
   string, with \0 replacing \n, is allocated on the heap and may be
   freed. */
char *
read_string (FILE *input, bool *badstatus)
{
  int length = 0, buffer_length = 16; /* initial length */
  char *buffer;
  char c;

  if (*badstatus == true)
    return 0;

  buffer = (char *)xmalloc (buffer_length * sizeof(char));
  for ( ; ; )
    {
      if (length >= buffer_length)
	{
	  buffer_length *= 2;
	  buffer = (char *)xrealloc (buffer, (unsigned int)(buffer_length));
	}
      c = (char)read_byte_as_unsigned_char (input, badstatus);
      if ((*badstatus == true) || (c == '\n'))
	break;
      buffer [length++] = c;
    }

  if (*badstatus)
    {
      free (buffer);
      return NULL;
    }
  else
    {
      buffer [length] = '\0';	/*  null-terminate string */
      return buffer;
    }
}

