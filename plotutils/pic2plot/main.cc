/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, 2009, Free Software
   Foundation, Inc.

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

#include "pic.h"
#include "output.h"
#include "common.h"

#include "libcommon.h"
#include "getopt.h"
#include "fontlist.h"
#include "plot.h"		// libplot header file

const char *progname = "pic2plot"; // name of this program
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " FILE...\n";

// A global; we have only one of these.  Its member function are what do
// the output of objects of various kinds (they're invoked by the
// objects' `print' operations). 
output *out;			// declared in output.h

// `out' is a pointer to an instance of the plot_output class, which
// is derived from the output class.  Any instance of the plot_output
// class looks at the following global variables, which the user
// can set on the command line.
char *output_format = (char *)"meta"; // libplot output format
char *font_name = NULL;	// initial font name (if set)
char *pen_color_name = NULL; // initial pen color (if set)
double font_size = 10.0/(8.0*72.); // font size as width of graphics display
double line_width = -0.5/(8.0*72.); // line width as width of graphics display,
				// negative means use libplot default

// any plot_output instance contains a plPlotter object; the following
// PlotterParams object contains the parameters used when creating it
plPlotterParams *plotter_params;

// flags (used by lexer)
int command_char = '.';		// char that introduces pass-thru lines
int compatible_flag = 0;	// recog. PS/PE even if not foll. by ' ', '\n'?

// flags (used by parser)
int safer_flag = 0; 		// forbid shell escapes?

// flag (could be used by driver)
int flyback_flag;
int no_centering_flag = 0;	// turn off auto-centering?
int precision_dashing = 0;	// position dashes/dots individually?

// static variables
static int had_parse_error = 0;	// parse error?
static int lf_flag = 1;		// non-zero -> try to parse `.lf' lines

// options

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "T:OndF:f:W:";

struct option long_options[] = 
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* hidden */
  /* Long options, most with no equivalent short option alias */
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "emulate-color",	ARG_REQUIRED,	NULL, 'e' << 8 },
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "font-size",	ARG_REQUIRED,	NULL, 'f' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "no-centering",	ARG_NONE,	NULL, 'n' },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "precision-dashing",ARG_NONE,	NULL, 'd' },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  /* Options relevant only to raw pic2plot (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "list-fonts",	ARG_NONE,	NULL, 'l' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};
    
/* null-terminated list of options, such as obsolete-but-still-maintained
   options or undocumented options, which we don't show to the user */
const int hidden_options[] = { (int)('T' << 8), 0 };

// forward references
void do_file (const char *filename);
void do_picture (FILE *fp);

//////////////////////////////////////////////////////////////////////
// TOP_INPUT class.
//////////////////////////////////////////////////////////////////////

class top_input : public input 
{
public:
  // ctor
  top_input (FILE *);
  // public functions
  int get (void);
  int peek (void);
  int get_location (const char **filenamep, int *linenop);
private:
  FILE *fp;
  int bol;
  int eof;
  int push_back[3];
  int start_lineno;
};

top_input::top_input(FILE *p) : fp(p), bol(1), eof(0)
{
  push_back[0] = push_back[1] = push_back[2] = EOF;
  start_lineno = current_lineno;
}

int 
top_input::get(void)
{
  if (eof)
    return EOF;
  if (push_back[2] != EOF) 
    {
      int c = push_back[2];
      push_back[2] = EOF;
      return c;
    }
  else if (push_back[1] != EOF) 
    {
      int c = push_back[1];
      push_back[1] = EOF;
      return c;
    }
  else if (push_back[0] != EOF) 
    {
      int c = push_back[0];
      push_back[0] = EOF;
      return c;
    }
  int c = getc(fp);
  while (illegal_input_char(c)) 
    {
      error("illegal input character code %1", int(c));
      c = getc(fp);
      bol = 0;
    }
  if (bol && c == '.') 
    {
      c = getc(fp);
      if (c == 'P') 
	{
	  c = getc(fp);
	  if (c == 'F' || c == 'E') 
	    {
	      int d = getc(fp);
	      if (d != EOF)
		ungetc(d, fp);
	      if (d == EOF || d == ' ' || d == '\n' || compatible_flag) 
		{
		  eof = 1;
		  flyback_flag = (c == 'F');
		  return EOF;
		}
	      push_back[0] = c;
	      push_back[1] = 'P';
	      return '.';
	    }
	  if (c == 'S') 
	    {
	      c = getc(fp);
	      if (c != EOF)
		ungetc(c, fp);
	      if (c == EOF || c == ' ' || c == '\n' || compatible_flag) 
		{
		  error("nested .PS");
		  eof = 1;
		  return EOF;
		}
	      push_back[0] = 'S';
	      push_back[1] = 'P';
	      return '.';
	    }
	  if (c != EOF)
	    ungetc(c, fp);
	  push_back[0] = 'P';
	  return '.';
	}
      else 
	{
	  if (c != EOF)
	    ungetc(c, fp);
	  return '.';
	}
    }
  if (c == '\n') 
    {
      bol = 1;
      current_lineno++;
      return '\n';
    }
  bol = 0;
  if (c == EOF) 
    {
      eof = 1;
      error("end of file before .PE or .PF");
      error_with_file_and_line(current_filename, start_lineno - 1,
			       ".PS was here");
    }
  return c;
}

int 
top_input::peek(void)
{
  if (eof)
    return EOF;
  if (push_back[2] != EOF)
    return push_back[2];
  if (push_back[1] != EOF)
    return push_back[1];
  if (push_back[0] != EOF)
    return push_back[0];
  int c = getc(fp);
  while (illegal_input_char(c)) 
    {
      error("illegal input character code %1", int(c));
      c = getc(fp);
      bol = 0;
    }
  if (bol && c == '.') 
    {
      c = getc(fp);
      if (c == 'P') 
	{
	  c = getc(fp);
	  if (c == 'F' || c == 'E') 
	    {
	      int d = getc(fp);
	      if (d != EOF)
		ungetc(d, fp);
	      if (d == EOF || d == ' ' || d == '\n' || compatible_flag) 
		{
		  eof = 1;
		  flyback_flag = (c == 'F');
		  return EOF;
		}
	      push_back[0] = c;
	      push_back[1] = 'P';
	      push_back[2] = '.';
	      return '.';
	    }
	  if (c == 'S') 
	    {
	      c = getc(fp);
	      if (c != EOF)
		ungetc(c, fp);
	      if (c == EOF || c == ' ' || c == '\n' || compatible_flag) 
		{
		  error("nested .PS");
		  eof = 1;
		  return EOF;
		}
	      push_back[0] = 'S';
	      push_back[1] = 'P';
	      push_back[2] = '.';
	      return '.';
	    }
	  if (c != EOF)
	    ungetc(c, fp);
	  push_back[0] = 'P';
	  push_back[1] = '.';
	  return '.';
	}
      else 
	{
	  if (c != EOF)
	    ungetc(c, fp);
	  push_back[0] = '.';
	  return '.';
	}
    }
  if (c != EOF)
    ungetc(c, fp);
  if (c == '\n')
    return '\n';
  return c;
}

int 
top_input::get_location(const char **filenamep, int *linenop)
{
  *filenamep = current_filename;
  *linenop = current_lineno;
  return 1;
}

//////////////////////////////////////////////////////////////////////
// End of TOP_INPUT class, beginning of main().
//////////////////////////////////////////////////////////////////////

int 
main (int argc, char **argv)
{
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  int errcnt = 0;		/* errors encountered */
  int opt_index;		/* long option index */
  int option;			/* option character */

  static char stderr_buf[BUFSIZ];
  setbuf(stderr, stderr_buf);

  /* set global */
  program_name = progname;

#if 0
/* could add -C, -S options to set these */
  compatible_flag = 1;	// allow non-' ', non-'\n' after .PS ?
  safer_flag = 1;		// forbid shell escapes
#endif

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
	case 'n':		/* No centering */
	  no_centering_flag = 1;
	  break;
	case 'd':		/* Draw dots/dashes individually */
	  precision_dashing = 1;
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
	  break;
	case 'f':		/* Font size, ARG REQUIRED	*/
	  {
	    double local_font_size;
	    char s[4];
	    
	    if (sscanf (optarg, "%lf %3s", &local_font_size, s) == 2
		&& strcmp (s, "pt") == 0)
	      local_font_size /= (8.0 * 72.0); // express as width of display
	    else if (sscanf (optarg, "%lf", &local_font_size) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: font size must be a number, e.g. `0.01736' or `10pt', was `%s'\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_font_size >= 1.0)
	      fprintf (stderr, "%s: ignoring too-large font size\n",
		       progname);
	    else if (local_font_size < 0.0)
	      fprintf (stderr, "%s: ignoring negative font size\n",
		       progname);
	    else
	      font_size = local_font_size;
	    break;
	  }
	case 'W':		/* Line width, ARG REQUIRED	*/
	  {
	    double local_line_width;
	    char s[4];
	    
	    if (sscanf (optarg, "%lf %3s", &local_line_width, s) == 2
		&& strcmp (s, "pt") == 0)
	      local_line_width /= (8.0 * 72.0); // express as width of display
	    else if (sscanf (optarg, "%lf", &local_line_width) <= 0)
	      {
		fprintf (stderr,
			 "%s: error: line thickness must be a number, e.g. `0.00868' or `0.5pt', was `%s'\n",
			 progname, optarg);
		errcnt++;
		break;
	      }
	    if (local_line_width >= 1.0)
	      fprintf (stderr, "%s: ignoring too-large line thickness\n",
		       progname);

	    /* N.B. We don't rule out negative line thickness, because it's
	       interpreted as a request to use libplot's default line
	       thickness.  (Which depends on output format; for example,
	       with the `-T X' option it'll yield a zero-thickness X11
	       line, which means a Bresenham line.) */

	    else
	      line_width = local_line_width;
	    break;
	  }
	case 'e' << 8:		/* Emulate color via grayscale */
	  pl_setplparam (plotter_params, "EMULATE_COLOR", (void *)optarg);
	  break;
	case 'C' << 8:		/* set the initial pen color */
	  pen_color_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color_name, optarg);
	  break;
	case 'q' << 8:		/* set the initial background color */
	  pl_setplparam (plotter_params, "BG_COLOR", (void *)optarg);
	  break;
	case 'B' << 8:		/* Bitmap size */
	  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)optarg);
	  break;
	case 'M' << 8:		/* Max line length */
	  pl_setplparam (plotter_params, "MAX_LINE_LENGTH", (void *)optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  pl_setplparam (plotter_params, "PAGESIZE", (void *)optarg);
	  break;
	case 'r' << 8:		/* Rotation angle */
	  pl_setplparam (plotter_params, "ROTATION", (void *)optarg);
	  break;
	case 'V' << 8:		/* Version */
	  show_version = true;
	  break;
	case 'f' << 8:		/* Fonts */
	  show_fonts = true;
	  break;
	case 'l' << 8:		/* Fonts */
	  do_list_fonts = true;
	  break;
	case 'h' << 8:		/* Help */
	  show_usage = true;
	  break;
	default:
	  errcnt++;
	  break;
	}
    }

  if (errcnt > 0)
    {
      fprintf (stderr, "Try `%s --help' for more information\n", 
	       progname);
      return EXIT_FAILURE;
    }
  if (show_version)
    {
      display_version (progname, written, copyright);
      return EXIT_SUCCESS;
    }
  if (do_list_fonts)
    {
      int success = true;

      success = list_fonts (output_format, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_fonts)
    {
      int success = true;

      success = display_fonts (output_format, progname);
      if (success)
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  if (show_usage)
    {
      display_usage (progname, hidden_options, usage_appendage, true);
      return EXIT_SUCCESS;
    }

  /* at most, only file names remain, so initialize parser */
  parse_init();

  out = make_plot_output();
  command_char = 014;	// bogus to avoid seeing `commands'
  lf_flag = 0;

  /* invoke do_file() on stdin or on each remaining file */
  if (optind >= argc)
    do_file ("-");
  else
    for (int i = optind; i < argc; i++)
      do_file (argv[i]);

  delete out;
  if (ferror(stdout) || fflush(stdout) < 0)
    fatal ("output error");

  /* clean up */
  pl_deleteplparams (plotter_params);

  return had_parse_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

void 
do_file (const char *filename)
{
  FILE *fp;
  if (strcmp(filename, "-") == 0)
    fp = stdin;
  else 
    {
      errno = 0;
      fp = fopen(filename, "r");
      if (fp == 0)
	fatal ("can't open `%1': %2", filename, strerror(errno));
    }
  out->set_location (filename, 1);
  current_filename = filename;
  current_lineno = 1;
  enum { START, MIDDLE, HAD_DOT, HAD_P, HAD_PS, HAD_l, HAD_lf } state = START;
  for (;;) 
    {
      int c = getc(fp);
      if (c == EOF)		// break out of for loop on EOF, only
	break;
      switch (state) 
	{
	case START:
	  if (c == '.')
	    state = HAD_DOT;
	  else 
	    {
	      if (c == '\n') 
		{
		  current_lineno++;
		  state = START;
		}
	      else		// dnl
		state = MIDDLE;
	    }
	  break;
	case MIDDLE:
	  // discard chars until newline seen; switch back to START
	  if (c == '\n') 
	    {
	      current_lineno++;
	      state = START;
	    }
	  break;
	case HAD_DOT:
	  if (c == 'P')
	    state = HAD_P;
	  else if (lf_flag && c == 'l')
	    state = HAD_l;
	  else 
	    {
	      if (c == '\n') 
		{
		  current_lineno++;
		  state = START;
		}
	      else		// dnl
		state = MIDDLE;
	    }
	  break;
	case HAD_P:
	  if (c == 'S')
	    state = HAD_PS;
	  else  
	    {
	      if (c == '\n') 
		{
		  current_lineno++;
		  state = START;
		}
	      else		// dnl
		state = MIDDLE;
	    }
	  break;
	case HAD_PS:
	  if (c == ' ' || c == '\n' || compatible_flag) 
	    {
	      ungetc(c, fp);
	      do_picture(fp);	// do the picture, incl. args of .PS if any
	      state = START;
	    }
	  else			// dnl
	    state = MIDDLE;
	  break;
	case HAD_l:
	  if (c == 'f')
	    state = HAD_lf;
	  else 
	    {
	      if (c == '\n') 
		{
		  current_lineno++;
		  state = START;
		}
	      else		// dnl
		state = MIDDLE;
	    }
	  break;
	case HAD_lf:
	  if (c == ' ' || c == '\n' || compatible_flag) 
	    {
	      string line;

	      while (c != EOF) 
		{
		  line += c;
		  if (c == '\n') 
		    {
		      current_lineno++;
		      break;
		    }
		  c = getc(fp);
		}
	      line += '\0';
	      interpret_lf_args(line.contents());
	      state = START;
	    }
	  else			// dnl
	    state = MIDDLE;
	  break;
	default:
	  assert(0);
	}
    }

  // exit gracefully when EOF seen
  if (fp != stdin)
    fclose(fp);
}

void 
do_picture(FILE *fp)
{
  flyback_flag = 0;
  int c;
  while ((c = getc(fp)) == ' ')
    ;
  if (c == '<') 
    /* first nonblank character after .PS is '<' */
    {
      string filename;
      while ((c = getc(fp)) == ' ')
	;
      while (c != EOF && c != ' ' && c != '\n') 
	{
	  filename += char(c);
	  c = getc(fp);
	}
      if (c == ' ') 
	{
	  do 
	    {
	      c = getc(fp);
	    } while (c != EOF && c != '\n');
	}
      if (c == '\n') 
	current_lineno++;
      if (filename.length() == 0)
	error("missing filename after `<'");
      else 
	{
	  filename += '\0';
	  const char *old_filename = current_filename;
	  int old_lineno = current_lineno;
	  // filenames must be permanent
	  do_file (strsave(filename.contents())); // recursive call
	  current_filename = old_filename;
	  current_lineno = old_lineno;
	}
      out->set_location (current_filename, current_lineno);
    }
  else 
    /* first nonblank character after .PS is not '<' */
    {
      out->set_location (current_filename, current_lineno);

      /* get the starting line */
      string start_line;
      while (c != EOF) 
	{
	  if (c == '\n') 
	    {
	      current_lineno++;
	      break;
	    }
	  start_line += c;
	  c = getc(fp);
	}
      if (c == EOF)
	return;
      start_line += '\0';

      /* parse starting line for height and width */
      double wid, ht;
      switch (sscanf(&start_line[0], "%lf %lf", &wid, &ht)) 
	{
	case 1:
	  ht = 0.0;
	  break;
	case 2:
	  break;
	default:
	  ht = wid = 0.0;
	  break;
	}
      out->set_desired_width_height (wid, ht);
      out->set_args (start_line.contents());

      // do the parse
      lex_init (new top_input(fp));
      if (yyparse()) 
	{
	  had_parse_error = 1;
	  lex_error ("giving up on this picture");
	}
      parse_cleanup();
      lex_cleanup();

      // skip the rest of the .PF/.PE line
      while ((c = getc(fp)) != EOF && c != '\n')
	;
      if (c == '\n')
	current_lineno++;
      out->set_location (current_filename, current_lineno);
    }
}
