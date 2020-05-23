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

/* This file contains the main routine, and a few support subroutines, for
   GNU graph. */

#include "sys-defines.h"
#include "extern.h"
#include "libcommon.h"
#include "getopt.h"
#include "fontlist.h"

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "-BCHOQVstE:F:f:g:h:k:K:I:l:L:m:N:q:R:r:T:u:w:W:X:Y:a::x::y::S::"; /* initial hyphen requests no reordering */

struct option long_options[] =
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* hidden */
  /* Other frequently used options */
  {"auto-abscissa",	ARG_OPTIONAL,	NULL, 'a'}, /* 0 or 1 or 2 */
  {"clip-mode",		ARG_REQUIRED,	NULL, 'K'},
  {"fill-fraction",	ARG_REQUIRED,	NULL, 'q'},
  {"font-name",		ARG_REQUIRED,	NULL, 'F'},
  {"font-size",		ARG_REQUIRED,	NULL, 'f'},
  {"grid-style",	ARG_REQUIRED,	NULL, 'g'},
  {"height-of-plot",	ARG_REQUIRED,	NULL, 'h'},
  {"input-format",	ARG_REQUIRED,	NULL, 'I'},
  {"line-mode",		ARG_REQUIRED,	NULL, 'm'},
  {"line-width",	ARG_REQUIRED,	NULL, 'W'},
  {"right-shift",	ARG_REQUIRED,	NULL, 'r'},
  {"save-screen",	ARG_NONE,	NULL, 's'},
  {"symbol",		ARG_OPTIONAL,	NULL, 'S'}, /* 0 or 1 or 2 */
  {"tick-size",		ARG_REQUIRED,	NULL, 'k'},
  {"toggle-auto-bump",	ARG_NONE,	NULL, 'B'},
  {"toggle-axis-end",	ARG_REQUIRED,	NULL, 'E'},
  {"toggle-frame-on-top",	ARG_NONE,	NULL, 'H'},
  {"toggle-log-axis",	ARG_REQUIRED,	NULL, 'l'},
  {"toggle-no-ticks",	ARG_REQUIRED,	NULL, 'N'},
  {"toggle-rotate-y-label",	ARG_NONE,	NULL, 'Q'},
  {"toggle-round-to-next-tick",	ARG_REQUIRED,	NULL, 'R'},
  {"toggle-transpose-axes",	ARG_NONE,	NULL, 't'},
  {"toggle-use-color",	ARG_NONE,	NULL, 'C'},
  {"top-label",		ARG_REQUIRED,	NULL, 'L'},
  {"upward-shift",      ARG_REQUIRED,	NULL, 'u'},
  {"width-of-plot",	ARG_REQUIRED,	NULL, 'w'},
  {"x-label",		ARG_REQUIRED,	NULL, 'X'},
  {"x-limits",		ARG_OPTIONAL,	NULL, 'x'}, /* 0, 1, 2, or 3 */
  {"y-label",		ARG_REQUIRED,	NULL, 'Y'},
  {"y-limits",		ARG_OPTIONAL,	NULL, 'y'}, /* 0, 1, 2, or 3 */
  /* Long options with no equivalent short option alias */
  {"bg-color",		ARG_REQUIRED,	NULL, 'q' << 8},
  {"bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8},
  {"blankout",		ARG_REQUIRED,	NULL, 'b' << 8},  
  {"emulate-color",	ARG_REQUIRED,	NULL, 'e' << 8},  
  {"frame-line-width",	ARG_REQUIRED,	NULL, 'W' << 8},
  {"frame-color",	ARG_REQUIRED,	NULL, 'C' << 8},
  {"max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8},
  {"pen-colors",	ARG_REQUIRED,	NULL, 'p' << 8},
  {"reposition",	ARG_REQUIRED,	NULL, 'R' << 8}, /* 3 */
  {"rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  {"symbol-font-name",	ARG_REQUIRED,	NULL, 'G' << 8},
  {"title-font-name",	ARG_REQUIRED,	NULL, 'Z' << 8},
  {"title-font-size",	ARG_REQUIRED,	NULL, 'F' << 8},
  {"page-size",		ARG_REQUIRED,	NULL, 'P' << 8},
  /* Options relevant only to raw graph (refers to plot(5) output) */
  {"portable-output",	ARG_NONE,	NULL, 'O'},
  /* Documentation options */
  {"help-fonts",	ARG_NONE,	NULL, 'f' << 8},
  {"list-fonts",	ARG_NONE,	NULL, 'l' << 8},
  {"version",		ARG_NONE,	NULL, 'V' << 8},
  {"help",		ARG_NONE,	NULL, 'h' << 8},
  {NULL, 0, 0, 0}
};

/* null-terminated list of options, such as obsolete-but-still-maintained
   options or undocumented options, which we don't show to the user */
const int hidden_options[] = { (int)('T' << 8), 0 };

const char *progname = "graph";	/* name of this program */
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

/* forward references */
static void close_file (char *filename, FILE *stream);
static void open_file_for_reading (char *filename, FILE **input);
static bool parse_pen_string (const char *pen_s);

int
main (int argc, char *argv[])
{
  /* Variables related to getopt parsing */

  int option;
  int opt_index;
  int errcnt = 0;		/* errors encountered in getopt parsing */
  int matched;
  bool using_getopt = true;	/* true until end of command-line options */
  bool continue_parse = true;	/* reset e.g. when --help or --version seen */
  bool show_version = false;	/* show version message? */
  bool show_usage = false;	/* show usage message? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool filter = false;		/* will we act as a filter? */
  bool new_symbol = false;
  bool new_symbol_size = false;
  bool new_symbol_font_name = false;
  bool new_linemode = false;
  bool new_plot_line_width = false;
  bool new_fill_fraction = false;
  bool new_use_color = false;
  bool first_file_of_graph = true;
  bool first_graph_of_multigraph = true;
  FILE *data_file = NULL;

  /* Variables related to the point reader */

  Reader *reader = NULL;
  data_type input_type = T_ASCII; /* by default we read ascii data */
  bool auto_bump = true;	/* auto-bump linemode between polylines? */
  bool auto_abscissa = false;	/* generate abscissa values automatically? */
  double x_start = 0.;		/* start and increment, for auto-abscissa */
  double delta_x = 1.;
  /* polyline attributes */
  int linemode_index = 1;	/* linemode for polylines, 1=solid, etc. */
  double plot_line_width = -0.001; /* polyline width (as frac. of display width), negative means default provided by libplot) */
  int symbol_index = 0;		/* 0=none, 1=dot, 2=plus, 3=asterisk, etc. */
  double symbol_size = .03;	/* symbol size (frac. of plotting box size) */
  double fill_fraction = -1.0;	/* negative means regions aren't filled */
  bool use_color = false;	/* color / monochrome */

  /* Variables related to both the point reader and the point plotter */

  bool transpose_axes = false;	/* true means -x applies to y axis, etc. */

  /* Variables related to the multigrapher, i.e. point plotter */

  Multigrapher *multigrapher = NULL;
  
  /* command-line parameters (constant over multigrapher operation) */
  const char *output_format = "meta";/* libplot output format */
  const char *bg_color = NULL;	/* color of background, if non-NULL */
  const char *bitmap_size = NULL;
  const char *emulate_color = NULL;
  const char *max_line_length = NULL;
  const char *meta_portable = NULL;
  const char *page_size = NULL;
  const char *rotation_angle = NULL;
  bool save_screen = false;	/* save screen, i.e. no erase before plot? */

  /* graph-specific parameters (may change from graph to graph) */

  grid_type grid_spec = AXES_AND_BOX; /* frame type for current graph */
  bool no_rotate_y_label = false; /* used for pre-X11R6 servers */
  const char *frame_color = "black"; /* color of frame (and graph, if no -C)*/
  int clip_mode = 1;		/* clipping mode (cf. gnuplot) */
  /* following variables are portmanteau: x and y are included as bitfields*/
  int log_axis = 0;		/* log axes or linear axes? */
  int round_to_next_tick = 0;	/* round axis limits to nearest tick? */
  int switch_axis_end = 0;	/* axis at top/right instead of bottom/left? */
  int omit_ticks = 0;		/* omit ticks and tick labels from an axis? */

  /* graph dimensions, expressed as fractions of the width of the libplot
     graphics display [by convention square]; <0.0 means use libplot default */
  double frame_line_width = -0.001; /* width of lines in the graph frame */

  /* dimensions of graphing area, expressed as fractions of the width of
     the libplot graphics display [by convention square] */
  double margin_below = .2;	/* margin below the plot */
  double margin_left = .2;	/* margin left of the plot */
  double plot_height = .6;	/* height of the plot */
  double plot_width = .6;	/* width of the plot */

  /* dimensions, expressed as fractions of the size of the plotting area */
  double tick_size = .02;	/* size of tick marks (< 0.0 allowed) */
  double font_size = 0.0525;	/* fontsize */
  double title_font_size = 0.07; /* title fontsize */
  double blankout_fraction = 1.3; /* this fraction of size of plotting box
				   is erased before the plot is drawn */

  /* text-related */
  const char *font_name = NULL;	/* font name, NULL -> device default */
  const char *title_font_name = NULL; /* title font name, NULL -> default */
  const char *symbol_font_name = "ZapfDingbats"; /* symbol font name, NULL -> default */
  const char *x_label = NULL;	/* label for the x axis, NULL -> no label */
  const char *y_label = NULL;	/* label for the y axis, NULL -> no label */
  const char *top_label = NULL;	/* title above the plot, NULL -> no title */

  /* user-specified limits on the axes */
  double min_x = 0.0, min_y = 0.0, max_x = 0.0, max_y = 0.0;
  double spacing_x = 0.0, spacing_y = 0.0;

  /* flags indicating which axis limits the user has specified */
  bool spec_min_x = false, spec_min_y = false;
  bool spec_max_x = false, spec_max_y = false;
  bool spec_spacing_x = false, spec_spacing_y = false;

  /* misc. local variables used in getopt parsing, counterparts to the above */
  double local_x_start, local_delta_x;
  int local_grid_style;
  int local_symbol_index;
  int local_clip_mode;
  double local_symbol_size, local_font_size, local_title_font_size;
  double local_frame_line_width, local_plot_line_width;
  double local_min_x, local_min_y;
  double local_max_x, local_max_y;
  double local_spacing_x, local_spacing_y;
  double local_fill_fraction;
  
  /* `finalized' arguments to set_graph_parameters() (computed at the time
     the first file of a graph is seen, and continuing in effect over the
     duration of the graph) */
  int final_log_axis = 0;
  int final_round_to_next_tick = 0;
  double final_min_x = 0.0, final_max_x = 0.0, final_spacing_x = 0.0;
  double final_min_y = 0.0, final_max_y = 0.0, final_spacing_y = 0.0;  
  bool final_spec_min_x = false, final_spec_min_y = false;
  bool final_spec_max_x = false, final_spec_max_y = false;
  bool final_spec_spacing_x = false, final_spec_spacing_y = false;
  bool final_transpose_axes = false;

  /* for storage of data points (if we're not acting as a filter) */
  Point *p;			/* points array */
  int points_length = 1024;	/* length of the points array, in points */
  int no_of_points = 0;		/* number of points stored in it */

  /* support for multigraphing */
  double reposition_trans_x = 0.0, reposition_trans_y = 0.0;
  double reposition_scale = 1.0;
  double old_reposition_trans_x, old_reposition_trans_y;
  double old_reposition_scale;

  /* sui generis */
  bool frame_on_top = false;

  /* The main command-line parsing loop, which uses getopt to scan argv[]
     without reordering, i.e. to process command-line arguments (options
     and filenames) sequentially.

     From a logical point of view, a multigraph consists of a sequence of
     graphs, with a `--reposition' flag serving as a separator between
     graphs.  A graph is drawn from one or more files.

     So this parsing loop invokes Multigrapher methods (1) when a file name
     is seen, (2) when a `--reposition' directive is seen, and (3) at the
     end of the scan over argv[].

     If at the end of the scan no file names have been seen, stdin is used
     instead as an input stream.  (As a file name, `-' means stdin.) */

  while (continue_parse)
    {
      if (using_getopt)
	/* end of options not reached yet */
	{
	  option = getopt_long (argc, argv, 
				/* initial hyphen requests no reordering */
				optstring, 
				long_options, &opt_index);
	  if (option == EOF)	/* end of options */
	    {
	      using_getopt = false;
	      continue;		/* back to top of while loop */
	    }
	  if (option == 1)	/* filename embedded among options */
	    {
	      if (strcmp (optarg, "-") == 0)
		data_file = stdin; /* interpret "-" as stdin */
	      else
		open_file_for_reading (optarg, &data_file);
	    }
	}
      else
	/* end of options reached, processing filenames manually */
	{
	  if (optind >= argc)	/* all files processed */
	    {
	      if (first_graph_of_multigraph && first_file_of_graph)
		/* no file appeared on command line, read stdin instead */
		{
		  data_file = stdin;
		  option = 1;	/* code for pseudo-option */
		}
	      else
		break;		/* all files done, break out of while loop */
	    }
	  else			/* have files yet to process */
	    {
	      if (strcmp (argv[optind], "-") == 0)
		data_file = stdin;
	      else
		open_file_for_reading (argv[optind], &data_file);
	      optarg = argv[optind]; /* keep track of name of opened file */
	      optind++;
	      option = 1;	/* code for pseudo-option */
	    }
	}

      /* Parse an option flag, which may be a genuine option flag obtained
	 from getopt, or a fake (a `1', indicating that a filename has been
	 seen by getopt, or that filename has been seen on the command line
	 after all genuine options have been processed, or that stdin
	 must be read because no filenames have been seen). */

      switch (option)
	{
	  /* ----------- options with no argument --------------*/

	case 's':		/* Don't erase display before plot, ARG NONE */
	  save_screen = true;
	  break;
	case 't':		/* Toggle transposition of axes, ARG NONE */
	  transpose_axes = (transpose_axes == true ? false : true);
	  break;
	case 'B':		/* Toggle linemode auto-bumping, ARG NONE */
	  auto_bump = (auto_bump == true ? false : true);
	  break;
	case 'C':		/* Toggle color/monochrome, ARG NONE */
	  new_use_color = true;
	  use_color = (use_color == true ? false : true);
	  break;
	case 'H':		/* Toggle frame-on-top, ARG NONE */
	  frame_on_top = (frame_on_top == true ? false : true);
	  break;
	case 'O':		/* portable format, ARG NONE */
	  meta_portable = "yes";
	  break;
	case 'e' << 8:		/* emulate color, ARG NONE */
	  emulate_color = xstrdup (optarg);
	  break;
	case 'V' << 8:		/* Version, ARG NONE		*/
	  show_version = true;
	  continue_parse = false;
	  break;
	case 'h' << 8:		/* Help, ARG NONE		*/
	  show_usage = true;
	  continue_parse = false;
	  break;
	case 'f' << 8:		/* Help on fonts, ARG NONE	*/
	  show_fonts = true;
	  continue_parse = false;
	  break;
	case 'l' << 8:		/* List fonts, ARG NONE		*/
	  do_list_fonts = true;
	  continue_parse = false;
	  break;
	case 'Q':		/* Toggle rotation of y-label, ARG NONE */
	  no_rotate_y_label = (no_rotate_y_label == true ? false : true);
	  break;

	  /*----------- options with a single argument --------------*/

	case 'I':		/* Input format, ARG REQUIRED	*/
	  switch (*optarg)
	    {
	    case 'a':
	    case 'A':
	      /* ASCII format, records and fields within records are
		 separated by whitespace, and datasets are separated by a
		 pair of newlines.  Record length = 2. */
	      input_type = T_ASCII;
	      break;
	    case 'f':
	    case 'F':
	      /* Binary single precision, records and fields within records
		 are contiguous, and datasets are separated by a FLT_MAX.
		 Record length = 2. */
	      input_type = T_SINGLE;
	      break;
	    case 'd':
	    case 'D':
	      /* Binary double precision, records and fields within records
		 are contiguous, and datasets are separated by a DBL_MAX.
		 Record length = 2. */
	      input_type = T_DOUBLE;
	      break;
	    case 'i':
	    case 'I':
	      /* Binary integer, records and fields within records are
		 contiguous, and datasets are separated by an occurrence of
		 INT_MAX.  Record length = 2. */
	      input_type = T_INTEGER;
	      break;
	    case 'e':
	    case 'E':
	      /* Same as T_ASCII, but record length = 3. */
	      input_type = T_ASCII_ERRORBAR;
	      break;
	    case 'g':
	    case 'G':
	      /* Sui generis. */
	      input_type = T_GNUPLOT;	/* gnuplot `table' format */
	      break;
	    default:
	      fprintf (stderr,
		       "%s: error: `%s' is an unrecognized data option\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'f':		/* Font size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the font size should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      if (local_font_size >= 1.0)
		fprintf (stderr, "%s: the too-large font size `%f' is disregarded (it should be less than 1.0)\n",
			 progname, local_font_size);
	      else if (local_font_size < 0.0)
		fprintf (stderr, "%s: the negative font size `%f' is disregarded\n",
			 progname, local_font_size);
	      else
		font_size = local_font_size;
	    }
	  break;
	case 'g':		/* Grid style, ARG REQUIRED	*/
	  if (sscanf (optarg, "%d", &local_grid_style) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the grid style should be a (small) integer, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	      break;
	    }
	  switch (local_grid_style)
	    /* the subset ordering is: 0 < 1 < 2 < 3; 4 is different */
	    {
	    case 0:
	      /* no frame at all; just the plot */
	      grid_spec = NO_AXES;
	      break;
	    case 1:
	      /* box, ticks, gridlines, labels */
	      grid_spec = AXES;
	      break;
	    case 2:
	      /* box, ticks, no gridlines, labels */
	      grid_spec = AXES_AND_BOX;
	      break;
	    case 3:
	      /* `half-box', partial ticks, no gridlines, labels */
	      grid_spec = AXES_AND_BOX_AND_GRID;
	      break;
	    case 4:
	      /* no box, no gridlines; specially positioned axes, labels */
	      grid_spec = AXES_AT_ORIGIN;
	      break;
	    default:
	      fprintf (stderr,
		       "%s: error: the grid style number `%s' is out of bounds\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'h':		/* Height of plot, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &plot_height) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the plot height should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'K':		/* Clip mode, ARG REQUIRED */
	  if ((sscanf (optarg, "%d", &local_clip_mode) <= 0)
	      || local_clip_mode < 0 || local_clip_mode > 2)
	    fprintf (stderr,
		     "%s: the bad clip mode `%s' is disregarded (it should be 0, 1, or 2)\n",
		     progname, optarg);
	  else
	    clip_mode = local_clip_mode;
	  break;
	case 'l':		/* Toggle log/linear axis, ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      log_axis ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      log_axis ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: the unrecognized axis specification `%s' is disregarded\n",
		       progname, optarg);
	      break;
	    }
	  break;
	case 'N':		/* Toggle omission of labels, ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      omit_ticks ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      omit_ticks ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: the unrecognized axis specification `%s' is disregarded\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'm':		/* Linemode, ARG REQUIRED	*/
	  new_linemode = true;
	  if (sscanf (optarg, "%d", &linemode_index) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the linemode should be a (small) integer, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'q':		/* Fill fraction, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_fill_fraction) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the fill fraction should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else
	    {
	      if (local_fill_fraction > 1.0)
		fprintf (stderr, 
			 "%s: the region fill fraction `%f' was disregarded (it should be less than or equal to 1.0)\n",
			 progname, local_fill_fraction);
	      else
		{
		  fill_fraction = local_fill_fraction;
		  new_fill_fraction = true;
		}
	    }
	  break;
	case 'r':		/* Right shift, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &margin_left) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the rightward displacement for the plot should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'u':		/* Upward shift, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &margin_below) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the upward displacement for the plot should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'w':		/* Width of plot, ARG REQUIRED 	*/
	  if (sscanf (optarg, "%lf", &plot_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the plot width should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'T':		/* Output format, ARG REQUIRED      */
	case 'T' << 8:
	  output_format = xstrdup (optarg);
	  break;
	case 'F':		/* Font name, ARG REQUIRED      */
	  font_name = xstrdup (optarg);
	  break;
	case 'r' << 8:		/* Rotation angle, ARG REQUIRED      */
	  rotation_angle = xstrdup (optarg);
	  break;
	case 'Z' << 8:		/* Title Font name, ARG REQUIRED      */
	  title_font_name = xstrdup (optarg);
	  break;
	case 'G' << 8:		/* Symbol Font name, ARG REQUIRED      */
	  symbol_font_name = xstrdup (optarg);
	  new_symbol_font_name = true;
	  break;
	case 'R':		/* Toggle rounding to next tick, ARG REQUIRED*/
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      round_to_next_tick ^= X_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      round_to_next_tick ^= Y_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: the unrecognized axis specification `%s' is disregarded\n",
		       progname, optarg);
	      break;
	    }
	  break;
	case 'L':		/* Top title, ARG REQUIRED	*/
	  top_label = xstrdup (optarg);
	  break;
	case 'k':		/* Tick size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &tick_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the tick size should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'W':		/* Line width, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_plot_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the line thickness for the plot should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_plot_line_width < 0.0)
	    fprintf (stderr, "%s: the negative plot line thickness `%f' is disregarded\n",
		     progname, local_plot_line_width);
	  else
	    {
	      plot_line_width = local_plot_line_width;
	      new_plot_line_width = true;
	    }
	  break;
	case 'X':		/* X axis title, ARG REQUIRED	*/
	  x_label = xstrdup (optarg);
	  break;
	case 'Y':		/* Y axis title, ARG REQUIRED	*/
	  y_label = xstrdup (optarg);
	  break;
	case 'E':		/* Toggle switching of axis to other end, 
				   ARG REQUIRED */
	  switch (*optarg)
	    {
	    case 'x':
	    case 'X':
	      switch_axis_end ^= Y_AXIS;
	      break;
	    case 'y':
	    case 'Y':
	      switch_axis_end ^= X_AXIS;
	      break;
	    default:
	      fprintf (stderr, 
		       "%s: the unrecognized axis specification `%s' is disregarded\n", 
		       progname, optarg);
	      break;
	    }
	  break;
	case 'b' << 8:		/* Blankout fraction, ARG REQUIRED */
	  if (sscanf (optarg, "%lf", &blankout_fraction) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the fractional blankout should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  break;
	case 'B' << 8:		/* Bitmap size, ARG REQUIRED	*/
	  bitmap_size = xstrdup (optarg);
	  break;
	case 'F' << 8:		/* Title font size, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_title_font_size) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the font size for the title should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  else if (local_title_font_size >= 1.0)
	    fprintf (stderr, "%s: the too-large title font size `%f' is disregarded (it should be less than 1.0)\n",
		     progname, local_title_font_size);
	  else if (local_title_font_size < 0.0)
	    fprintf (stderr, "%s: the negative title font size `%f' is disregarded\n",
		     progname, local_title_font_size);
	  if (local_title_font_size == 0.0)
	    fprintf (stderr, "%s: the request for a zero title font size is disregarded\n",
		     progname);
	  else
	    title_font_size = local_title_font_size;
	  break;
	case 'W' << 8:		/* Frame line width, ARG REQUIRED	*/
	  if (sscanf (optarg, "%lf", &local_frame_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the line thickness for the frame should be a number, but it was `%s'\n",
		       progname, optarg);
	      errcnt++;
	    }
	  if (local_frame_line_width < 0.0)
	    fprintf (stderr, "%s: the negative frame line thickness `%f' is disregarded\n",
		     progname, local_frame_line_width);
	  else
	    frame_line_width = local_frame_line_width;
	  break;
	case 'M' << 8:		/* Max line length, ARG REQUIRED	*/
	  max_line_length = xstrdup (optarg);
	  break;
	case 'P' << 8:		/* Page size, ARG REQUIRED	*/
	  page_size = xstrdup (optarg);
	  break;
	case 'p' << 8:		/* Pen color string, ARG REQUIRED      */
	  if (parse_pen_string (optarg) == false)
	    {
	      fprintf (stderr, "%s: the unparseable pen string `%s' is disregarded\n",
		       progname, optarg);
	    }
	  break;
	case 'q' << 8:		/* Background color, ARG REQUIRED      */
	  bg_color = xstrdup (optarg);
	  break;
	case 'C' << 8:		/* Frame color, ARG REQUIRED      */
	  frame_color = xstrdup (optarg);
	  break;

	  
	  /*------ options with zero or more arguments ---------*/

	case 'a':		/* Auto-abscissa, ARG OPTIONAL [0,1,2] */
	  auto_abscissa = true;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_delta_x) <= 0)
	    break;
	  optind++;	/* tell getopt we recognized delta_x */
	  if (local_delta_x == 0.0)
	    /* "-a 0" turns off auto-abscissa for next file */
	    {
	      auto_abscissa = false;
	      break;
	    }
	  delta_x = local_delta_x;
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_x_start) <= 0)
	    break;
	  x_start = local_x_start;
	  optind++;	/* tell getopt we recognized x_start */
	  break;
	case 'x':		/* X limits, ARG OPTIONAL [0,1,2,3] */
	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_min_x)) <= 0))
	    {
	      spec_min_x = spec_max_x = spec_spacing_x = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_min_x = true;
	      min_x = local_min_x;
	    }
	  else
	      spec_min_x = false;
	  optind++;	/* tell getopt we recognized min_x */

	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_max_x)) <= 0))
	    {
	      spec_max_x = spec_spacing_x = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_max_x = true;
	      max_x = local_max_x;
	    }
	  else
	    spec_max_x = false;
	  optind++;	/* tell getopt we recognized max_x */

	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_spacing_x)) <= 0))
	    {
	      spec_spacing_x = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_spacing_x = true;
	      spacing_x = local_spacing_x;
	    }
	  else
	      spec_spacing_x = false;
	  optind++;	/* tell getopt we recognized spacing_x */
	  break;

	case 'y':		/* Y limits, ARG OPTIONAL [0,1,2,3] */
	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_min_y)) <= 0))
	    {
	      spec_min_y = spec_max_y = spec_spacing_y = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_min_y = true;
	      min_y = local_min_y;
	    }
	  else
	      spec_min_y = false;
	  optind++;	/* tell getopt we recognized min_y */

	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_max_y)) <= 0))
	    {
	      spec_max_y = spec_spacing_y = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_max_y = true;
	      max_y = local_max_y;
	    }
	  else
	      spec_max_y = false;
	  optind++;	/* tell getopt we recognized max_y */

	  matched = 0;
	  if (optind >= argc
	      || ((strcmp (argv[optind], "-") != 0)
		  && (matched 
		      = sscanf (argv[optind], "%lf", &local_spacing_y)) <= 0))
	    {
	      spec_spacing_y = false;
	      break;
	    }
	  if (matched > 0)
	    {
	      spec_spacing_y = true;
	      spacing_y = local_spacing_y;
	    }
	  else
	      spec_spacing_y = false;
	  optind++;	/* tell getopt we recognized spacing_y */
	  break;

	case 'S':		/* Symbol, ARG OPTIONAL	[0,1,2]		*/
	  new_symbol = true;
	  symbol_index = 1;	/* symbol # 1 is switched to by -S alone */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%d", &local_symbol_index) <= 0)
	    break;
	  if (local_symbol_index < 0 || local_symbol_index > 255)
	    fprintf (stderr, "%s: the symbol type `%d' is disregarded (it should be in the range 0..255)\n",
		     progname, local_symbol_index);
	  else
	    symbol_index = local_symbol_index;
	  optind++;		/* tell getopt we recognized symbol_index */
	  if (optind >= argc)
	    break;
	  if (sscanf (argv[optind], "%lf", &local_symbol_size) <= 0)
	    break;
	  if (local_symbol_size < 0.0)
	    fprintf (stderr, "%s: the negative symbol size `%f' is disregarded\n",
		     progname, local_symbol_size);
	  else if (local_symbol_size == 0.0)
	    fprintf (stderr, "%s: the request for a zero symbol size is disregarded\n",
		     progname);
	  else
	    {
	      symbol_size = local_symbol_size;
	      new_symbol_size = true;
	    }
	  optind++;		/* tell getopt we recognized symbol_size */
	  break;

	  /* ---------- options with one or more arguments ---------- */
	  
	case 'R' << 8:		/* End graph and reposition, ARG REQUIRED [3]*/
	  old_reposition_trans_x = reposition_trans_x;
	  old_reposition_trans_y = reposition_trans_y;
	  old_reposition_scale = reposition_scale;
	  
	  if (sscanf (optarg, "%lf", &reposition_trans_x) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the x repositioning should be a number, but it was `%s'\n",
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  if (optind >= argc)
	    {
	      fprintf (stderr,
		       "%s: error: one or more arguments to the --reposition option were missing\n",
		       progname);
	      return EXIT_FAILURE;
	    }
	  if (sscanf (argv[optind], "%lf", &reposition_trans_y) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the y repositioning should be a number, but it was `%s'\n",
		       progname, argv[optind]);
	      return EXIT_FAILURE;
	    }
	  optind++;		/* tell getopt we recognized trans_y */
	  if (optind >= argc)
	    {
	      fprintf (stderr,
		       "%s: error: one or more arguments to the --reposition option were missing\n",
		       progname);
	      return EXIT_FAILURE;
	    }
	  if (sscanf (argv[optind], "%lf", &reposition_scale) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the reposition scale factor should be a number, but it was `%s'\n",
		       progname, optarg);
	      return EXIT_FAILURE;
	    }
	  if (reposition_scale == 0.0)
	    {
	      fprintf (stderr,
		       "%s: error: the reposition scale factor should not be zero\n", progname);
	      return EXIT_FAILURE;
	    }
	  optind++;		/* tell getopt we recognized trans_x */

	  if (!first_file_of_graph)
	    /* a graph is in progress (at least one file has been read), so
	       it must be ended before we begin the next one */
	    {
	      if (!filter)
		/* We haven't been acting as a real-time filter for the
		   duration of this graph, so the graph isn't already drawn
		   on the display.  Instead, we have a points array and we
		   need to plot it, after computing bounds. */
		{
		  /* fill in any of min_? and max_? that user didn't
		     specify (the prefix "final_" means these arguments
		     were finalized at the time the first file of the plot
		     was processed) */
		  array_bounds (p, no_of_points, 
				final_transpose_axes, clip_mode,
				&final_min_x, &final_min_y, 
				&final_max_x, &final_max_y,
				final_spec_min_x, final_spec_min_y, 
				final_spec_max_x, final_spec_max_y);
		  
		  if (first_graph_of_multigraph)
		    /* haven't created multigrapher yet, do so now */
		    {
		      if ((multigrapher = new_multigrapher (output_format, bg_color, bitmap_size, emulate_color, max_line_length, meta_portable, page_size, rotation_angle, save_screen)) == NULL)
			{
			  fprintf (stderr, 
				   "%s: error: the graphing device could not be opened\n", progname);
			  return EXIT_FAILURE;
			}
		    }
		  
		  /* begin graph: push new libplot drawing state onto stack
		     of states; also concatenate the current transformation
		     matrix with a matrix formed from the repositioning
		     parameters (this will be in effect for duration of the
		     graph) */
		  begin_graph (multigrapher,
			       old_reposition_scale,
			       old_reposition_trans_x, old_reposition_trans_y);
		  
		  /* font selection, saves typing */
		  if ((title_font_name == NULL) && (font_name != NULL))
		    title_font_name = font_name;
	      
		  /* initialize, using (in part) finalized arguments */
		  set_graph_parameters (multigrapher,
					frame_line_width,
					frame_color,
					top_label,
					title_font_name, title_font_size, /* for title */
					tick_size, grid_spec,
					final_min_x, final_max_x, final_spacing_x,
					final_min_y, final_max_y, final_spacing_y,
					final_spec_spacing_x,
					final_spec_spacing_y,
					plot_width, plot_height, margin_below, margin_left,
					font_name, font_size, /* for abs. label */
					x_label, 
					font_name, font_size, /* for ord. label */
					y_label,
					no_rotate_y_label,
					/* these args are portmanteaux */
					final_log_axis, 
					final_round_to_next_tick,
					switch_axis_end, omit_ticks, 
					/* more args */
					clip_mode,
					blankout_fraction,
					final_transpose_axes);
	      
		  /* draw the graph frame (grid, ticks, etc.); draw a
		     `canvas' (a background opaque white rectangle) only if
		     this isn't the first graph */
		  draw_frame_of_graph (multigrapher,
				       (first_graph_of_multigraph ? false : true));
	      
		  /* plot the laboriously read-in array */
		  plot_point_array (multigrapher, p, no_of_points);
	      
		  /* free points array */
		  free (p);
		  no_of_points = 0;
		  first_file_of_graph = false;
	      
		} /* end of not-filter case */
	  
	      /* draw graph frame on top of graph, if user requested it */
	      if (frame_on_top)
		{
		  end_polyline_and_flush (multigrapher);
		  draw_frame_of_graph (multigrapher, false);
		}

	      /* end graph: pop the graph-specific libplot drawing state off
                 the stack of drawing states */
	      end_graph (multigrapher);

	      /* on to next graph */
	      first_graph_of_multigraph = false;
	      first_file_of_graph = true;

	    } /* end of not first-file-of-plot case */
	  
	  break;		/* end of `--reposition' option */

	  /* ---------------- pseudo-options -------------- */

	  /* File specified on command line, returned in order (along with
	     command-line options).  The first time we reach this point in
	     any plot, we perform special initializations and in particular
	     determine whether or not, for the duration of this plot, we'll
	     be acting as a filter.  We can do so if xmin, xmax, ymin, ymax
	     have all been specified, by this point, on the command line.

	     A plot may consist of many files.  A plot in progress is
	     terminated if a --reposition option (which moves us to the
	     next plot of a multiplot) is seen, or when the last
	     command-line option is processed. */
	case 1:
	  if (first_file_of_graph)
	    {
	      /* For plots with a logarithmic axis, compute logs of axis
		 limits, since coordinates along the axis, as obtained from
		 the reader, are stored in logarithmic form. */
	      if (log_axis & X_AXIS)
		{
		  if (spec_min_x)
		    {
		      if (min_x > 0.0)
			min_x = log10(min_x);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: the limit %g on a logarithmic axis is nonpositive\n", 
				  progname, min_x);
			  return EXIT_FAILURE;
			}
		    }
		  if (spec_max_x)
		    {
		      if (max_x > 0.0)
			max_x = log10(max_x);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: the limit %g on a logarithmic axis is nonpositive\n", 
				  progname, max_x);
			  return EXIT_FAILURE;
			}
		    }
		}

	      if (log_axis & Y_AXIS)
		{
		  if (spec_min_y)
		    {
		      if (min_y > 0.0)
			min_y = log10(min_y);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: the limit %g on a logarithmic axis is nonpositive\n", 
				  progname, min_y);
			  return EXIT_FAILURE;
			}
		    }
		  if (spec_max_y)
		    {
		      if (max_y > 0.0)
			max_y = log10(max_y);
		      else
			{
			  fprintf(stderr, 
				  "%s: error: the limit %g on a logarithmic axis is nonpositive\n", 
				  progname, max_y);
			  return EXIT_FAILURE;
			}
		    }
		}

	      /* We now finalize the following parameters (arguments to
		 set_graph_parameters()), even though we won't call
		 set_graph_parameters() for a while yet, if it turns out we
		 need to act as a real-time filter. */

	      /* portmanteaux */
	      final_log_axis = log_axis;
	      final_round_to_next_tick = round_to_next_tick;

	      /* bool */
	      final_transpose_axes = transpose_axes;

	      /* x-axis specific */
	      final_min_x = min_x;
	      final_max_x = max_x;
	      final_spacing_x = spacing_x;
	      final_spec_min_x = spec_min_x;
	      final_spec_max_x = spec_max_x;
	      final_spec_spacing_x = spec_spacing_x;

	      /* y-axis specific */
	      final_min_y = min_y;
	      final_max_y = max_y;
	      final_spec_min_y = spec_min_y;
	      final_spec_max_y = spec_max_y;
	      final_spacing_y = spacing_y;
	      final_spec_spacing_y = spec_spacing_y;

	      /* If user didn't specify either the lower limit or the upper
		 limit for an axis, by default we'll round the axis limits
		 to the nearest tick, after computing them.  (If either
		 limit was specified by the user, to request rounding the
		 user must specify the -R option as well.) */
	      if (!final_spec_min_x && !final_spec_max_x)
		final_round_to_next_tick |= X_AXIS;
	      if (!final_spec_min_y && !final_spec_max_y)
		final_round_to_next_tick |= Y_AXIS;
	      
	      /* The case when x_min, x_max, y_min, y_max are all specified
		 by the luser is special: we set the `filter' flag for the
		 duration of this plot, to indicate that we can function as
		 a real-time filter, calling read_and_plot_file() on each
		 file, rather than calling read_file() on each one
		 separately to create an array of points, and then calling
		 plot_point_array(). */
	      filter = ((final_spec_min_x && final_spec_max_x 
			 && final_spec_min_y && final_spec_max_y) 
			? true : false);

	    } /* end of first-file-of-plot case */

	  if (filter)
	    /* filter flag is set, will call read_and_plot() on this file */
	    {
	      if (first_file_of_graph)
		{
		  if (first_graph_of_multigraph)
		    /* need to create the multigrapher */
		    {
		      if ((multigrapher = new_multigrapher (output_format, bg_color, bitmap_size, emulate_color, max_line_length, meta_portable, page_size, rotation_angle, save_screen)) == NULL)
			{
			  fprintf (stderr, 
				   "%s: error: the graphing device could not be opened\n", 
				   progname);
			  return EXIT_FAILURE;
			}
		    }
		  
		  /* begin graph: push a graph-specific drawing state onto
		     libplot's stack of drawing states; also concatenate
		     the current transformation matrix with a matrix formed
		     from the repositioning parameters (this will take
		     effect for the duration of the graph) */
		  begin_graph (multigrapher,
			       reposition_scale,
			       reposition_trans_x, reposition_trans_y);
	      
		  /* font selection, saves typing */
		  if ((title_font_name == NULL) && (font_name != NULL))
		    title_font_name = font_name;
	      
		  /* following will be in effect for the entire plot */
		  set_graph_parameters (multigrapher,
					frame_line_width, 
					frame_color,
					top_label,
					title_font_name, title_font_size, /* for title */
					tick_size, grid_spec,
					final_min_x, final_max_x, final_spacing_x,
					final_min_y, final_max_y, final_spacing_y,
					final_spec_spacing_x,
					final_spec_spacing_y,
					plot_width, plot_height, 
					margin_below, margin_left,
					font_name, font_size, /* on abscissa */
					x_label, 
					font_name, font_size, /* on ordinate */
					y_label,
					no_rotate_y_label,
					/* these args are portmanteaux */
					final_log_axis, 
					final_round_to_next_tick,
					switch_axis_end,
					omit_ticks, 
					/* more args */
					clip_mode,
					blankout_fraction,
					final_transpose_axes);

		  /* draw the graph frame (grid, ticks, etc.); draw a
		     `canvas' (a background opaque white rectangle) only if
		     this isn't the first graph */
		  draw_frame_of_graph (multigrapher,
				       first_graph_of_multigraph ? false : true);
		  
		  reader = new_reader (data_file, input_type,
				       auto_abscissa, delta_x, x_start,
				       /* following three are graph-specific */
				       final_transpose_axes, 
				       final_log_axis, auto_bump,
				       /* following args are file-specific
					  (they set dataset attributes) */
				       symbol_index, symbol_size,
				       symbol_font_name,
				       linemode_index, plot_line_width, 
				       fill_fraction, use_color);
		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      else
		/* not first file of plot; do some things anyway */
		{
		  /* set reader parameters that may change when we move
		     from file to file within a plot */
		  alter_reader_parameters (reader,
					   data_file, input_type,
					   auto_abscissa, delta_x, x_start,
					   /* following args set dataset 
					      attributes */
					   symbol_index, symbol_size, 
					   symbol_font_name,
					   linemode_index, plot_line_width, 
					   fill_fraction, use_color,
					   /* following bools make up a mask*/
					   new_symbol, new_symbol_size,
					   new_symbol_font_name,
					   new_linemode, new_plot_line_width, 
					   new_fill_fraction, new_use_color);

		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
    
	      /* call read_and_plot_file() on the file; each dataset in the
		 file yields a polyline */
	      read_and_plot_file (reader, multigrapher);

	    } /* end of filter case */
	  
	  else
	    /* filter flag is set, will read and plot this file separately */

	    /* Luser didn't specify enough information for us to act as a
	       filter, so we do things the hard way: we call read_file() on
	       each file to create a points array, and at the end of the
	       plot we'll call plot_point_array() on the array.  For now,
	       we don't even call set_graph_parameters(). */
	    {
	      if (first_file_of_graph)	/* some additional initializations */
		{
		  p = (Point *)xmalloc (points_length * sizeof (Point));
		  
		  reader = new_reader (data_file, input_type, 
				       auto_abscissa, delta_x, x_start,
				       /* following are graph-specific */
				       final_transpose_axes, 
				       final_log_axis, auto_bump,
				       /* following args are file-specific
					  (they set dataset attributes) */
				       symbol_index, symbol_size,
				       symbol_font_name,
				       linemode_index, plot_line_width, 
				       fill_fraction, use_color);
		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      else	/* not first file of plot, but do some things anyway */
		{
		  /* set reader parameters that may change when we move
		     from file to file within a plot */
		  alter_reader_parameters (reader,
					   data_file, input_type, 
					   auto_abscissa, delta_x, x_start,
					   /* following args set dataset
					      attributes */
					   symbol_index, symbol_size, 
					   symbol_font_name,
					   linemode_index, plot_line_width, 
					   fill_fraction, use_color,
					   /* following bools make up a mask*/
					   new_symbol, new_symbol_size,
					   new_symbol_font_name,
					   new_linemode, new_plot_line_width, 
					   new_fill_fraction, new_use_color);

		  new_symbol = new_symbol_size = new_symbol_font_name = false;
		  new_linemode = new_plot_line_width = false;
		  new_fill_fraction = new_use_color = false;
		}
	      
	      /* add points to points array by calling read_file() on file */
	      read_file (reader, &p, &points_length, &no_of_points);

	    } /* end of not-filter case */

	  /* close file */
	  if (data_file != stdin)
	    close_file (optarg, data_file);

	  first_file_of_graph = false;
	  break;	/* end of `case 1' in switch() [i.e., filename seen] */
	  
	  /*---------------- End of options ----------------*/

	default:		/* Default, unknown option */
	  errcnt++;
	  continue_parse = false;
	  break;
	}			/* end of switch() */

      if (errcnt > 0)
	continue_parse = false;
    }				/* end of while loop */
  
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

  /* End of command-line parse.  At this point, we need to terminate the
     graph currently in progress, if it's nonempty (i.e. if one or more
     files have been read). */

  if (first_file_of_graph == false)
    {
      /* At least one file was read.  If we're acting as a real-time
	 filter, then the graph is already drawn on the display and there's
	 nothing for us to do.  Instead, we have a points array and we need
	 to plot it, after computing bounds. */
      if (!filter)
	{

	  /* fill in any of min_? and max_? that user didn't specify (the
	     prefix "final_" means these arguments were finalized at the
	     time the first file of the plot was processed) */
	  array_bounds (p, no_of_points,
			final_transpose_axes, clip_mode,
			&final_min_x, &final_min_y,
			&final_max_x, &final_max_y,
			final_spec_min_x, final_spec_min_y, 
			final_spec_max_x, final_spec_max_y);
	  
	  if (first_graph_of_multigraph)
	    /* still haven't created multigrapher, do so now */
	    {
	      if ((multigrapher = new_multigrapher (output_format, bg_color, bitmap_size, emulate_color, max_line_length, meta_portable, page_size, rotation_angle, save_screen)) == NULL)
		{
		  fprintf (stderr, 
			   "%s: error: the graphing device could not be opened\n", progname);
		  return EXIT_FAILURE;
		}
	    }
	  
	  /* begin graph: push new libplot drawing state onto stack of
	     states; also concatenate the current transformation matrix
	     with a matrix formed from the repositioning parameters (this
	     will take effect for the duration of the graph) */
	  begin_graph (multigrapher,
		       reposition_scale, 
		       reposition_trans_x, reposition_trans_y);
	  
	  /* font selection, saves typing */
	  if ((title_font_name == NULL) && (font_name != NULL))
	    title_font_name = font_name;
	      
	  set_graph_parameters (multigrapher,
				frame_line_width,
				frame_color,
				top_label,
				title_font_name, title_font_size, /*for title*/
				tick_size, grid_spec,
				final_min_x, final_max_x, final_spacing_x,
				final_min_y, final_max_y, final_spacing_y,
				final_spec_spacing_x,
				final_spec_spacing_y,
				plot_width, plot_height, 
				margin_below, margin_left,
				font_name, font_size, /* for abscissa label */
				x_label, 
				font_name, font_size, /* for ordinate label */
				y_label,
				no_rotate_y_label,
				/* these args are portmanteaux */
				final_log_axis,
				final_round_to_next_tick,
				switch_axis_end, omit_ticks, 
				/* more args */
				clip_mode,
				blankout_fraction,
				final_transpose_axes);
	  
	  /* draw the graph frame (grid, ticks, etc.); draw a `canvas' (a
	     background opaque white rectangle) only if this isn't the
	     first graph */
	  draw_frame_of_graph (multigrapher,
			       first_graph_of_multigraph ? false : true);
	  
	  /* plot the laboriously read-in array */
	  plot_point_array (multigrapher, p, no_of_points);
	  
	  /* free points array */
	  free (p);
	  no_of_points = 0;

	} /* end of not-filter case */

      /* draw graph frame on top of graph, if user requested it */
      if (frame_on_top)
	{
	  end_polyline_and_flush (multigrapher);
	  draw_frame_of_graph (multigrapher, false);
	}

      /* end graph: pop drawing state off the stack of drawing states */
      end_graph (multigrapher);

    } /* end of nonempty-graph case */
  
  /* finish up by deleting our multigrapher (one must have been created,
     since we always read at least stdin) */
  if (delete_multigrapher (multigrapher) < 0)
    {
      fprintf (stderr, "%s: error: the graphing device could not be closed\n", 
	       progname);
      return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}


static void
open_file_for_reading (char *filename, FILE **input)
{
  FILE *data_file;
		
  data_file = fopen (filename, "r");
  if (data_file == NULL)
    {
      fprintf (stderr, "%s: %s: %s\n", progname, filename, strerror(errno));
      exit (EXIT_FAILURE);
    }
  else
    *input = data_file;
}  

static void
close_file (char *filename, FILE *stream)
{
  if (fclose (stream) < 0)
    fprintf (stderr, 
	     "%s: the input file `%s' could not be closed\n", 
	     progname, filename);
}

static bool
parse_pen_string (const char *pen_s)
{
  const char *charp;
  char name[MAX_COLOR_NAME_LEN];
  int i;

  charp = pen_s;
  while (*charp)
    {
      int pen_num;
      bool got_digit;
      const char *tmp;

      if (*charp == ':')	/* skip any ':' */
	{
	  charp++;
	  continue;		/* back to top of while loop */
	}
      pen_num = 0;
      got_digit = false;
      while (*charp >= '0' && *charp <= '9')
	{
	  pen_num = 10 * pen_num + (int)*charp - (int)'0';
	  got_digit = true;
	  charp++;
	}
      if (!got_digit || pen_num < 1 || pen_num > NO_OF_LINEMODES)
	return false;
      if (*charp != '=')
	return false;
      charp++;
      for (tmp = charp, i = 0; i < MAX_COLOR_NAME_LEN; tmp++, i++)
	{
	  if (*tmp == ':') /* end of color name string */
	    {
	      name[i] = '\0';
	      charp = tmp + 1;
	      break;
	    }
	  else if (*tmp == '\0') /* end of name string */
	    {
	      name[i] = '\0';
	      charp = tmp;
	      break;
	    }
	  else
	    name[i] = *tmp;
	}

      /* replace pen color name by user-specified color name */
      colorstyle[pen_num - 1] = xstrdup (name);
    }
  return true;
}
