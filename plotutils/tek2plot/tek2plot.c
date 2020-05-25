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

/* This file is the main routine for GNU tek2plot.  It reads a stream of
   Tektronix commands and draws graphics in real time by calling the
   appropriate routines in GNU libplot.  Written by Robert S. Maier
   <rsm@math.arizona.edu>.  Based on earlier work by Rich Murphey and by
   Edward Moy <moy@parc.xerox.com>.

   The table-driven parser is based on the one written by Ed at Berkeley in
   the mid-'80s.  The parsing tables in Tektable.c are essentially the same
   as the ones he designed for his `tek2ps' utility and for the Tektronix
   emulator included in the X10 and X11 versions of xterm(1). */

/* The basic reference on the features of the Tektronix 4014 with extended
   graphics module (EGM), which is what we emulate, is the 4014 Service
   Manual (Tektronix Part #070-1648-00, dated 8/74; there is also a User
   Manual [Part #070-1647-00]).  
   
   The code below emulates the non-interactive features of a Tektronix 4014
   with EGM [Extended Graphics Module], though not the interactive ones
   such as GIN mode or status inquiry.  It also doesn't support
   write-through mode or beam defocusing.  It does support the ANSI color
   extensions (ISO-6429) recognized by the MS-DOS Kermit v2.31 Tektronix
   emulator.  It also recognizes, and ignores, the VT340-style control
   sequences ESC [ ?38h (switch to Tektronix mode), ESC [ ?38l (switch to
   native mode), and ESC ^C (switch to native mode), which are used by some
   Tektronix emulators. */

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"
#include "fontlist.h"
#include "plot.h"
#include "Tekparse.h"

const char *progname = "tek2plot"; /* name of this program */
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " [FILE]...\n\
With no FILE, or when FILE is -, read standard input.\n";

/* Default font, ideally monospaced.  Each character in the font should
   ideally have a width equal to CHAR_WIDTH em, i.e. a width equal to
   CHAR_WIDTH times the font size. */
#define CHAR_WIDTH 0.6		/* valid for Courier family, at least */
#define DEFAULT_PS_FONT_NAME "Courier"
#define DEFAULT_PCL_FONT_NAME "Courier"
#define DEFAULT_HERSHEY_FONT_NAME "HersheySerif" /* not monospaced */

/* Coordinates in Tek file should be in range [0..4095]x[0..3119].  So to
   center points within a virtual graphics display of size
   [0..4095]x[0..4095], we add 488 to each y coordinate. */
#define TEK_WIDTH 4096
#define YOFFSET 488

/* Font size of the libplot marker symbol used to represent a `point', in
   Tek units.  We represent a Tektronix point by marker symbol #1, i.e. a
   dot (filled circle), which by convention has diameter 3/32 times the
   font size.  [See libplot/g_mark.c.]  So to get a diameter of 1 Tek unit,
   we choose 10 here. */
#define DOT_SIZE 10

/* parse tables in Tektable.c */
extern int Talptable[];
extern int Tbestable[];
extern int Tbyptable[];
extern int Tesctable[];
extern int Tipltable[];
extern int Tplttable[];
extern int Tpttable[];
extern int Tspttable[];

/* maximum size ANSI escape sequence we can handle */
#define BUFFER_SIZE 128

/* metrics for the four Tektronix fonts */
struct Tek_Char
{
  int hsize;	/* in Tek units */
  int vsize;	/* in Tek units */
  int charsperline;
  int nlines;
};

static const struct Tek_Char TekChar[4] = 
{ 
  {56, 88, 74, 35},	/* large */
  {51, 82, 81, 38},	/* #2 */
  {34, 53, 121, 58},	/* #3 */
  {31, 48, 133, 64},	/* small */
};

#define TEXT_BUFFER_SIZE 256	/* must be able to handle a full line */

/* Tektronix line types */
const char *linemodes[8] =
{
  "solid", "dotted", "dotdashed", "shortdashed", "longdashed",
  "solid", "solid", "solid"	/* final three treated as solid */
};

#define	TEKHOME		((TekChar[fontsize].nlines - 1)\
			 * TekChar[fontsize].vsize)
#define	MARGIN1		0	/* i.e. left edge */
#define	MARGIN2		1	/* i.e. half-way across page */

enum { PENDOWN, PENUP };
enum { NORTH = 04, SOUTH = 010, EAST = 01, WEST = 02 };

#define PRINTABLE_ASCII(c) ((c >= 0x20) && (c <= 0x7E))
#define	BEL		07

/* masks for coordinate-reading DFA */
#define ONE_BIT (0x1)
#define TWO_BITS (0x3)
#define FOUR_BITS (0x0f)
#define FIVE_BITS (0x1f)
#define TEN_BITS (0x3ff)

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "Op:F:W:T:";

struct option long_options[] = 
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* hidden */
  /* Other frequently used options */
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "emulate-color",	ARG_REQUIRED,	NULL, 'e' << 8 },
  { "font-name",	ARG_REQUIRED,	NULL, 'F' },
  { "line-width",	ARG_REQUIRED,	NULL, 'W' },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "max-line-length",	ARG_REQUIRED,	NULL, 'M' << 8 },
  { "page-number",	ARG_REQUIRED,	NULL, 'p' },
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "position-chars",	ARG_NONE,	NULL, 'S' << 8 },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  { "use-tek-fonts",	ARG_NONE,	NULL, 't' << 8 },
  /* Options relevant only to raw tek2plot (refers to metafile output) */
  { "portable-output",	ARG_NONE,	NULL, 'O' },
  /* Documentation options */
  { "help-fonts",	ARG_NONE,	NULL, 'f' << 8 },
  { "list-fonts",	ARG_NONE,	NULL, 'l' << 8 },
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL, 0}
};

/* null-terminated list of options, such as obsolete-but-still-maintained
   options or undocumented options, which we don't show to the user */
const int hidden_options[] = { (int)('T' << 8), 0 };

typedef struct
{
  int red;
  int green;
  int blue;
} Color;

/* ANSI (ISO-6429) color extensions.  Scheme is essentially:
	0 = normal, 1 = bright
        foreground color (30-37) = 30 + colors
                where colors are   1=red, 2=green, 4=blue
	background color is similar, with `40' replacing `30'. */

const Color ansi_color[16] =
{
  {0x0000, 0x0000, 0x0000},		/* black, \033[0;30m */
  {0x8b8b, 0x0000, 0x0000},		/* red4 \033[0;31m */
  {0x0000, 0x8b8b, 0x0000},		/* green4 \033[0;32m */
  {0x8b8b, 0x8b8b, 0x0000},		/* yellow4 \033[0;33m */
  {0x0000, 0x0000, 0x8b8b},		/* blue4 \033[0;34m */
  {0x8b8b, 0x0000, 0x8b8b},		/* magenta4 \033[0;35m */
  {0x0000, 0x8b8b, 0x8b8b},		/* cyan4, \033[0;36m */
  {0x8b8b, 0x8b8b, 0x8b8b},		/* gray55 \033[0;37m */
  {0x4d4d, 0x4d4d, 0x4d4d},		/* gray30 \033[1;30m */
  {0xffff, 0x0000, 0x0000},		/* red \033[1;31m */
  {0x0000, 0xffff, 0x0000},		/* green \033[1;32m */
  {0xffff, 0xffff, 0x0000},		/* yellow \033[1;33m */
  {0x0000, 0x0000, 0xffff},		/* blue \033[1;34m */
  {0xffff, 0x0000, 0xffff},		/* magenta \033[1;35m */
  {0x0000, 0xffff, 0xffff},		/* cyan \033[1;36m */
  {0xffff, 0xffff, 0xffff}		/* white \033[1;37m */
};

/* global variables set on command line, used in Tek parsing routine */
bool position_indiv_chars = false; /* user may set this */
bool single_page_is_requested = false; /* set if user uses -p option */
bool use_tek_fonts = false;	/* fonts tekfont0..tekfont3 available? */
bool force_hershey_default = false; /* default font sh'd be Hershey? [kludge]*/
char *font_name = NULL;		/* initial font name, can be spec'd by user */
char *pen_color = NULL;		/* initial pen color, can be spec'd by user */
double line_width = -1.0;	/* initial line width, <0 means default */
int requested_page = 0;		/* user sets this via -p option */

/* variables used in parser */
bool plotter_open = false;
bool plotter_opened = false;
int cur_X = 0, cur_Y = 0;	/* graphics cursor position in Tek coors */
int current_page = 0;		/* page count */

/* forward references */
bool getpoint (int *xcoor, int *ycoor, FILE *stream, int *badstatus, int *margin);
bool read_plot (plPlotter *plotter, FILE *in_stream);
int read_byte (FILE *stream, int *badstatus);
void begin_page (plPlotter *plotter);
void end_page (plPlotter *plotter);
void set_font_size (plPlotter *plotter, int new_fontsize);
void unread_byte (int byte, FILE *in_stream, int *badstatus);

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
  double local_line_width;	/* temporary storage */
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

	  /* Kludge: if HP-GL[/2] output is requested, be sure to use a
	     Hershey font as the default font, even though the Plotter
	     nominally supports PS fonts.  Reason: nominal != real. */

	  if (strcasecmp (output_format, "hpgl") == 0)
	    force_hershey_default = true;
	  else
	    force_hershey_default = false;
	  break;
	case 'F':		/* set the initial font */
	  font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (font_name, optarg);
	  break;
	case 'p':		/* page number */
	  if (sscanf (optarg, "%d", &local_page_number) <= 0
	      || local_page_number < 0)
	    {
	      fprintf (stderr,
		       "%s: error: the page number `%s' is bad (it should be a nonnegative integer)\n",
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
	  if (sscanf (optarg, "%lf", &local_line_width) <= 0)
	    {
	      fprintf (stderr,
		       "%s: error: the line thickness `%s' is bad (it should be a number)\n",
		       progname, optarg);
	      errcnt++;
	      break;
	    }
	  if (local_line_width < 0.0)
	    fprintf (stderr, "%s: the request for a negative line thickness `%f' is disregarded\n",
		     progname, local_line_width);
	  else
	    line_width = local_line_width;
	  break;
	case 'O':		/* Portable version of metafile output */
	  pl_setplparam (plotter_params, "META_PORTABLE", (void *)"yes");
	  break;

	  /*---------------- Long options below here ----------------*/
	case 'e' << 8:		/* Emulate color via grayscale */
	  pl_setplparam (plotter_params, "EMULATE_COLOR", (void *)optarg);
	  break;
	case 'q' << 8:		/* Set the initial background color */
	  pl_setplparam (plotter_params, "BG_COLOR", (void *)optarg);
	  break;
	case 'B' << 8:		/* Bitmap size */
	  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)optarg);
	  break;
	case 'C' << 8:		/* Set the initial pen color */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'M' << 8:		/* Max line length */
	  pl_setplparam (plotter_params, "MAX_LINE_LENGTH", (void *)optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  pl_setplparam (plotter_params, "PAGESIZE", (void *)optarg);
	  break;
	case 'S' << 8:        /* Position chars in text strings individually */
	  position_indiv_chars = true;
	  break;
	case 'r' << 8:		/* Rotation angle */
	  pl_setplparam (plotter_params, "ROTATION", (void *)optarg);
	  break;
	case 't' << 8:		/* Use Tektronix fonts (must be installed) */
	  if (strcmp (output_format, "X") == 0)
	    use_tek_fonts = true;
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
	case 'V' << 8:		/* Version */
	  show_version = true;
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

  /* turn off special interpretation of `erase' in GIF Plotters */
  pl_setplparam (plotter_params, "GIF_ANIMATION", (void *)"no");

  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr,
			     plotter_params)) == NULL)
    {
      fprintf (stderr, "%s: error: the plot device could not be created\n", progname);
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
		  fprintf (stderr, "%s: this file is ignored.\n", progname);
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
		  continue;	/* back to top of for loop */
	    }

	  if (data_file != stdin) /* Don't close stdin */
	    if (fclose (data_file) < 0)
	      {
		fprintf (stderr, 
			 "%s: error: the input file `%s' could not be closed\n",
			 progname, argv[optind]);
		return EXIT_FAILURE; /* exit immediately */
	      }
	}
    } /* endfor */
  else
    /* no files/streams spec'd on the command line, just read stdin */
    {
      if (read_plot (plotter, stdin) == false)
	{
	  fprintf (stderr, "%s: the input could not be parsed\n", progname);
	  retval = EXIT_FAILURE;
	}
    }

  /* if nothing was emitted ... */
  if (plotter_opened == false)
    {
      if (single_page_is_requested == false)
	/* output a blank page */
	{
	  begin_page (plotter);
	  end_page (plotter);
	}
      else
	{
	  if (requested_page >= current_page)
	    {
	      fprintf (stderr, "%s: the requested page does not exist\n", progname);
	      retval = EXIT_FAILURE;
	    }
	  else
	    /* page must have been seen, but was empty; output a blank page */
	    {
	      begin_page (plotter);
	      end_page (plotter);
	    }
	}
    }

  if (pl_deletepl_r (plotter) < 0)
    {
      fprintf (stderr, "%s: error: the plot device could not be deleted\n", progname);
      retval = EXIT_FAILURE;
    }
  pl_deleteplparams (plotter_params);

  return retval;
}

void
unread_byte (int c, FILE *in_stream, int *badstatus)
{
  if (*badstatus == 0)
    {
      if (ungetc (c, in_stream) == EOF)	/* means error, not EOF */
	*badstatus = 2;		/* treat as EOF anyway */
    }
}

int 
read_byte (FILE *in_stream, int *badstatus)
{
  int i;

  if (*badstatus == 1)		/* status = parse error */
    return 0;
  i = getc (in_stream);
  if (i == EOF)
    {
      *badstatus = 2;		/* status = eof */
      return 0;
    }
  return (i & 0x7f);		/* high bit ignored */
}


/* getpoint() reads a point (x,y) from the input stream, in Tektronix
   format, and returns it.  A point is a pair of coordinates in the range
   0..4095.  Reading a point will normally require reading anywhere between
   1 and 5 bytes from the input stream.  This function contains internal
   state: several static variables.

   Return value indicates whether a point is successfully read.  Failure to
   return a point may occur on account of a parsing problem or because of
   eof.  In either of these two cases an error code is returned through
   `badstatus', signalling that parsing of the input stream should
   terminate.

   A point may also fail to be returned if the first byte that is read from
   the input stream is not a byte in the 0x20..0xff range.  Normally, no
   byte in the range 0x00..0x1f range may be part of a point.  So if such a
   byte is seen, it is pushed back on the stream and point-reading is
   aborted (no error code is returned through badstatus).  Note that if the
   very first byte that is read is in this range, this function may return
   having read, in all, 0 bytes.

   An exception to the last rule: if any of the bytes CR, LF, or NUL is
   seen during the reading of a point, it is discarded and reading
   continues.  So it is also possible that >5 bytes may be read in all.

   A possible side effect of calling getpoint(): if the MSB of the egm
   byte, which is one of the bytes that make up the point, is set, then the
   left-hand margin will be set to MARGIN2, i.e. to 2048.  */

bool
getpoint (int *xcoor, int *ycoor, FILE *in_stream, int *badstatus, int *margin)
{
  /* variables for the point-reading DFA, initialized */
  int status_one = 0, status_three = 0;	/* 0=none, 1=seen one, 2=finished */
  bool got_lo_y = false;
  bool got_hi_x = false, got_hi_y = false;
  int lo_x = 0, lo_y = 0, hi_x = 0, hi_y = 0;
  bool got_egm = false;
  int egm = 0;
  int temp_three = 0;

  /* following variables are saved from point to point */
  static int saved_lo_y = 0, saved_hi_x = 0, saved_hi_y = 0;
  static bool margin_reset = false;

  int byte_read, type;

  if (*badstatus)
    return false;

  for ( ; ; )
    {
      byte_read = read_byte (in_stream, badstatus);
      if (*badstatus)
	return false;

      /* Ignore high bit (bit 8); bit pattern of next two bits (bits 7/6)
	 determines what sort of coordinate byte we have.  1 = Hi_X or
	 Hi_Y, 2 = Lo_X, 3 = Lo_Y or EGM; 0 usually means abort point.
	 Coordinate bytes appear in order

	 	[Hi_Y] [EGM] [Lo_Y] [Hi_X] Lo_X.  

		  1      3     3       1    2

	 All save last are optional, except that if EGM or Hi_X is
	 transmitted, also need need a Lo_Y.  We remember old values of
	 Hi_Y, Lo_Y, Hi_X, although not EGM or Lo_X, in our DFA. */

      type = (byte_read>>5) & TWO_BITS; /* type of byte */
      byte_read &= FIVE_BITS;	/* mask off 5 relevant bits */
      
      switch (type)
	{
	case 0:			/* interruption of point-reading (parse error?) */
	  fprintf (stderr, 
		   "%s: an incomplete point in the input is ignored\n",
		   progname);
	  if (byte_read == '\n' || byte_read == '\r' || byte_read == '\0')
	    continue;		/* discard, on to next byte */
	  else
	    /* put unread byte back on stream; hope we can parse it later */
	    unread_byte (byte_read, in_stream, badstatus);
	    return false;
	case 1:		/* Hi_Y or Hi_X */
	  switch (status_one)
	    {
	    case 0:
	      if (status_three)
		{
		  hi_x = byte_read; /* 2nd = Hi_X */
		  got_hi_x = true;
		  if (status_three == 1) 
		    {
		      lo_y = temp_three; /* Lo_Y */
		      got_lo_y = true;
		    }
		  
		  status_one = 2; /* no more 1's */
		  status_three = 2; /* no more 3's */
		}
	      else
		{
		  hi_y = byte_read; /* 1st = Hi_Y */
		  got_hi_y = true;
		  status_one = 1;
		}
	      break;
	    case 1:
	      if (status_three == 0)
		{
		  fprintf (stderr, 
			   "%s: error: a point in the input has Hi_Y, Hi_X bytes with no Lo_Y between\n",
			   progname);
		  *badstatus = 1; /* parse error */
		  return false;
		}
	      if (status_three == 1) 
		{
		  
		  lo_y = temp_three; /* Lo_Y */
		  got_lo_y = true;
		}
	      hi_x = byte_read; /* 2nd = Hi_X */
	      got_hi_x = true;
	      status_one = 2; /* no more 1's */
	      status_three = 2; /* no more 3's */
	      break;
	    case 2:
	      fprintf (stderr, 
		       "%s: error: a point in the input contains too many Hi_Y/Hi_X bytes\n",
		       progname);
	      *badstatus = 1; /* parse error */
	      return false;
	    }
	  break;
	case 3:		/* EGM or Lo_Y */
	  switch (status_three)
	    {
	    case 0:
	      if (status_one == 2)
		{
		  fprintf (stderr, 
			   "%s: error: a point in the input has an EGM/Lo_Y byte after 2 Hi_X/Hi_Y bytes\n",
			   progname);
		  *badstatus = 1; /* parse error */
		  return false;
		}
	      else
		{
		  temp_three = byte_read;
		  status_three = 1;
		}
	      break;
	    case 1:
	      if (status_one == 2)
		{
		  fprintf (stderr, 
			   "%s: error: a point in the input has an EGM/Lo_Y byte after 2 Hi_X/Hi_Y bytes\n",
			   progname);
		  *badstatus = 1; /* parse error */
		  return false;
		}
	      egm = temp_three; /* 1st = EGM */
	      got_egm = true;
	      lo_y = byte_read; /* 2nd = Lo_Y */
	      got_lo_y = true;
	      status_three = 2;
	      break;
	    case 2:
	      fprintf (stderr, 
		       "%s: error: a point in the input has too many EGM/Lo_Y bytes\n",
		       progname);
	      *badstatus = 1; /* parse error */
	      return false;
	    }
	  break;
	case 2:		/* Lo_X, final byte */
	  {
	    int low_res_x, low_res_y;
	    int x, y;
	    
	    if (status_three == 1)
	      {
		lo_y = temp_three; /* Lo_Y */
		got_lo_y = true;
	      }
	    lo_x = byte_read; /* Lo_X */
	    
	    lo_y = got_lo_y ? lo_y : saved_lo_y;	      
	    hi_x = got_hi_x ? hi_x : saved_hi_x;
	    hi_y = got_hi_y ? hi_y : saved_hi_y;
	    
	    saved_lo_y = lo_y;
	    saved_hi_x = hi_x;
	    saved_hi_y = hi_y;	      
	    
	    /* On a genuine Tektronix 4014, the MSB of the 5-bit EGM
	       byte sets the margin to equal Margin2 (2048) */
	    if ((egm >> 4) & ONE_BIT)
	      {
		*margin = MARGIN2;
		if (margin_reset == false)
		  fprintf (stderr,
			   "%s: the left margin of the Tektronix was reset by the input\n",
			   progname);
		margin_reset = true;
	      }
	    
	    /* low_res is what we'd use on a pre-EGM Tektronix */
	    low_res_x = (hi_x << 5) | lo_x;
	    low_res_y = (hi_y << 5) | lo_y;
	    x = (low_res_x << 2) | (egm & TWO_BITS);
	    y = (low_res_y << 2) | ((egm >> 2) & TWO_BITS);
	    
	    *xcoor = x;
	    *ycoor = y;
	    
	    return true;	/* end of `case 2' in switch: success */
	  }
	} /* end of switch */ 

    } /* end of while loop */
  /* NOTREACHED */
}


/* Parse a Tektronix stream and make appropriate libplot calls, paying
   attention to several global variables.  Will output at least one
   openpl()..closepl(). */
bool 
read_plot (plPlotter *plotter, FILE *in_stream)
{
  /* variables for DFA */
  int *Tparsestate = Talptable;	/* start in ALPHA mode */
  int *curstate = Talptable;	/* for temporary storage (for `bypass' mode) */
  int pen = PENUP;		/* pen up or pen down */
  int linetype = 0;		/* in range 0..7, 0 means "solid" */
  int fontsize = 0;		/* in range 0..3, 0 means large  */
  int margin = MARGIN1;		/* MARGIN1=left side, MARGIN2=halfway across */
  char text[TEXT_BUFFER_SIZE];	/* for storage of text strings */
  int badstatus = 0;		/* 0=OK, 1=parse error, 2=eof */

  while (!badstatus)		/* exit from loop only on parse error or eof */
    {
      int c;
      int x, y;

      c = read_byte (in_stream, &badstatus);
      if (badstatus)
	break;			/* parse error or eof; exit from while loop */

      switch (Tparsestate[c])
	{
	  /* Switch among 5 basic states: ALPHA, PLOT, POINT PLOT, 
	     SPECIAL POINT PLOT and INCREMENTAL PLOT. */

	case CASE_ALP_STATE:	/* Enter ALPHA mode */
	  Tparsestate = curstate = Talptable;
	  break;
	  
	case CASE_PLT_STATE:	/* Enter PLOT mode */
	  Tparsestate = curstate = Tplttable;
	  c = read_byte (in_stream, &badstatus);
	  /* do lookahead */
	  if (c == BEL)		
	    /* no initial dark vector */
	    pen = PENDOWN;
	  else 
	    {
	      pen = PENUP;
	      unread_byte (c, in_stream, &badstatus);
	    }
	  break;

	case CASE_PT_STATE:	/* Enter POINT PLOT mode */
	  Tparsestate = curstate = Tpttable;
	  break;
	  
	case CASE_SPT_STATE:	/* enter SPECIAL POINT PLOT mode */
	  Tparsestate = curstate = Tspttable;
	  break;
	  
	case CASE_IPL_STATE:	/* enter INCREMENTAL PLOT mode */
	  Tparsestate = curstate = Tipltable;
	  break;
	  
	  /*****************************************/

	  /* ALPHA mode commands */

	case CASE_PRINT:	/* printable character */
	  {
	    char *cp = text;
	    int x_here, y_here, n;
	    
	    x_here = cur_X, y_here = cur_Y;
	    
	    /* push back, so we can read string as a unit */
	    unread_byte (c, in_stream, &badstatus);
	    if (badstatus)
	      break;
	    
	    n = (position_indiv_chars ? 1 : TEXT_BUFFER_SIZE - 1);
	    y = cur_Y;
	    while (!badstatus && n-- > 0 && y == cur_Y) 
	      {
		c = read_byte (in_stream, &badstatus);
		if (badstatus)
		  {
		    break;	/* end label on eof or read error */
		  }
		
		if (!PRINTABLE_ASCII (c))
		  {
		    /* push back */
		    unread_byte (c, in_stream, &badstatus);
		    break;	/* end label on non-ascii character */
		  }
		*cp++ = c;
		
		/* following block is merely `cursor right' (cf. below) */
		{
		  const struct Tek_Char *t = &TekChar[fontsize];
		  int l;
		  
		  cur_X += t->hsize;
		  if (cur_X > TEK_WIDTH) 
		    {
		      l = cur_Y / t->vsize - 1;
		      if (l < 0)
			{
			  margin = !margin;
			  l = t->nlines - 1;
			}
		      cur_Y = l * t->vsize;
		      cur_X = (margin == MARGIN1 ? 0 : TEK_WIDTH / 2);
		    }
		}

	      } /* end of string-reading while loop */

	    *cp = '\0';		/* null-terminate string, and output it */
	    if (current_page == requested_page || !single_page_is_requested)
	      {
		if (plotter_open == false)
		  begin_page (plotter);
		if (position_indiv_chars)
		  /* string consists of a single char */
		  {
		    int halfwidth = TekChar[fontsize].hsize / 2;

		    pl_move_r (plotter, x_here + halfwidth, y_here + YOFFSET);
		    pl_alabel_r (plotter, 'c', 'b', text);
		  }
		else
		  {
		    pl_move_r (plotter, x_here, y_here + YOFFSET);
		    pl_alabel_r (plotter, 'l', 'b', text);
		  }
		pl_move_r (plotter, cur_X, cur_Y);
	      }

	  } /* end of CASE_PRINT */
	  break;

	  /* PLOT mode commands */

	case CASE_PLT_VEC:	/* PLT: vector */
	  /* push back, so we can read vector as a unit */
	  unread_byte (c, in_stream, &badstatus);
	  if (getpoint (&x, &y, in_stream, &badstatus, &margin)
	      && !badstatus)
	    /* N.B. getpoint returns w/o having read c only if c=0x00..0x1f,
	       so there's no chance of a infinite loop (see parsetable) */
	    {
	      if (current_page == requested_page || !single_page_is_requested)
		{
		  if (pen == PENDOWN)
		    {
		      if (plotter_open == false)
			begin_page (plotter);
		      pl_cont_r (plotter, x, y + YOFFSET);
		    }
		  else
		    {
		      /* N.B. Don't begin a new page just for a move() */ 
		      if (plotter_open == true)
			pl_move_r (plotter, x, y + YOFFSET);
		    }
		}
	      cur_X = x;
	      cur_Y = y;
	      pen = PENDOWN;
	    }
	  break;
	  
	  /* POINT PLOT mode commands */

	case CASE_PT_POINT:	/* PT: point */
	  /* push back, so we can read vector as a unit */
	  unread_byte (c, in_stream, &badstatus);
	  if (getpoint (&x, &y, in_stream, &badstatus, &margin)
	      && !badstatus)
	    /* N.B. getpoint returns w/o having read c only if c=0x00..0x1f,
	       so there's no chance of a infinite loop (see parsetable) */
	    {
	      if (current_page == requested_page || !single_page_is_requested)
		{
		  if (plotter_open == false)
		    begin_page (plotter);
		  pl_fmarker_r (plotter, 
				(double)x, (double)(y + YOFFSET), 
				M_DOT, (double)DOT_SIZE);
		}
	      cur_X = x;
	      cur_Y = y;
	    }
	  break;
	  
	  /* SPECIAL POINT PLOT mode commands */

	case CASE_SPT_POINT:	/* SPT: point */
	  /* don't push back c (ignore intensity byte) */
	  if (getpoint (&x, &y, in_stream, &badstatus, &margin)
	      && !badstatus)
	    /* N.B. getpoint returns w/o having read c only if c=0x00..0x1f,
	       so there's no chance of a infinite loop (see parsetable) */
	    {
	      /* assume intensity is > 0 */
	      if (current_page == requested_page || !single_page_is_requested)
		{
		  if (plotter_open == false)
		    begin_page (plotter);
		  pl_fmarker_r (plotter, 
				(double)x, (double)(y + YOFFSET), 
				M_DOT, (double)(DOT_SIZE));
		}
	      cur_X = x;
	      cur_Y = y;
	    }
	  break;
	  
	  /* INCREMENTAL PLOT mode commands */

	case CASE_PENUP:	/* IPL: penup */
	  pen = PENUP;
	  break;
	  
	case CASE_PENDOWN:	/* IPL: pendown */
	  pen = PENDOWN;
	  break;
	  
	case CASE_IPL_POINT:	/* IPL: point */
	  x = cur_X;
	  y = cur_Y;
	  if (c & NORTH)
	    y++;
	  else if (c & SOUTH)
	    y--;
	  if (c & EAST)
	    x++;
	  else if (c & WEST)
	    x--;
	  if (current_page == requested_page || !single_page_is_requested)
	    {
	      if (pen == PENDOWN)
		{
		  if (plotter_open == false)
		    begin_page (plotter);
		  pl_cont_r (plotter, x, y + YOFFSET);
		}
	      else
		{
		  /* N.B. Don't begin a new page just for a move() */
		  if (plotter_open == true)
		    pl_move_r (plotter, x, y + YOFFSET);
		}
	    }
	  cur_X = x;
	  cur_Y = y;
	  break;
	  
	  /****************************************/

	  /* These switch the parsetable temporarily to one of three
             pseudo-states, while stashing the current state. */

	case CASE_BES_STATE:	/* Byp: an escape char */
	  Tparsestate = Tbestable;
	  break;
	  
	case CASE_BYP_STATE:	/* set bypass condition */
	  Tparsestate = Tbyptable;
	  break;
	  
	case CASE_ESC_STATE:	/* ESC */
	  Tparsestate = Tesctable;
	  break;
	  
	  /*****************************************/

	  /* Cursor motion, useful mostly in ALPHA mode.  Some of these
	     restore the stashed state (if any) and some do not. */

	case CASE_CR:		/* CR */
	  cur_X = (margin == MARGIN1 ? 0 : TEK_WIDTH / 2);
	  Tparsestate = curstate = Talptable; /* switch to ALPHA mode */
	  break;
	  
	case CASE_BS:		/* BS, cursor left */
	  Tparsestate = curstate; /* clear bypass condition if any */
	  {
	    const struct Tek_Char *t;
	    int x, l;
	    
	    x = (cur_X -= (t = &TekChar[fontsize])->hsize);
	    if ((margin == MARGIN1 && x < 0)
		|| (margin == MARGIN2 && x < TEK_WIDTH / 2))
	      {
		if ((l = (cur_Y + (t->vsize - 1)) / t->vsize + 1) >=
		    t->nlines) 
		  {
		    margin = !margin;
		    l = 0;
		  }
		cur_Y = l * t->vsize;
		cur_X = (t->charsperline - 1) * t->hsize;
	      }
	  }
	  break;
	  
	case CASE_TAB:		/* HT */
	  Tparsestate = curstate; /* clear bypass condition if any */
	  /* FALL THROUGH */

	case CASE_SP:		/* SP, cursor right */
	  {
	    const struct Tek_Char *t = &TekChar[fontsize];
	    int l;

	    cur_X += t->hsize;
	    if (cur_X > TEK_WIDTH) 
	      {
		l = cur_Y / t->vsize - 1;
		if (l < 0) 
		  {
		    margin = !margin;
		    l = t->nlines - 1;
		  }
		cur_Y = l * t->vsize;
		cur_X = (margin == MARGIN1 ? 0 : TEK_WIDTH / 2);
	      }
	  }
	  break;
	  
	case CASE_LF:		/* LF, cursor down */
	  {
	    const struct Tek_Char *t;
	    int l;
	    
	    t = &TekChar[fontsize];
	    if ((l = cur_Y / t->vsize - 1) < 0) 
	      {
		l = t->nlines - 1;
		if ((margin = !margin) != MARGIN1) 
		  {
		    if (cur_X < TEK_WIDTH / 2)
		      cur_X += TEK_WIDTH / 2;
		  } 
		else if (cur_X >= TEK_WIDTH / 2)
		  cur_X -= TEK_WIDTH / 2;
	      }
	    cur_Y = l * t->vsize;
	  }
	  break;
	  
	case CASE_UP:		/* cursor up */
	  Tparsestate = curstate; /* clear bypass condition if any */
	  {
	    const struct Tek_Char *t;
	    int l;
	    
	    t = &TekChar[fontsize];
	    if ((l = (cur_Y + (t->vsize - 1)) / t->vsize + 1) >= t->nlines) 
	      {
		l = 0;
		if ((margin = !margin) != MARGIN1) 
		  {
		    if (cur_X < TEK_WIDTH / 2)
		      cur_X += TEK_WIDTH / 2;
		  } 
		else if (cur_X >= TEK_WIDTH / 2)
		  cur_X -= TEK_WIDTH / 2;
	      }
	    cur_Y = l * t->vsize;
	  }
	  break;

	  /****************************************/

	  /* Miscellaneous: functions we interpret as `next page',
	     `set font size', and `set line type'. */

	case CASE_PAGE:		/* page function, clears bypass cond. */
	  /* do closepl to flush out page (if nonempty) */
	  if (plotter_open == true)
	    end_page (plotter);

	  if (single_page_is_requested && current_page == requested_page)
	    {
	      badstatus = 2;	/* requested page is finished, so signal eof */
	      break;		/* exit from while loop */
	    }

	  /* now beginning parse of a new Tektronix page */
	  current_page++;

	  /* special case: if only a single page is requested, and line
	     mode and font size have changed due to commands on previous
	     Tek pages, output them */
	  if (single_page_is_requested && current_page == requested_page)
	    {
	      if (linetype != 0)
		{
		  if (plotter_open == false)
		    begin_page (plotter);
		  pl_linemod_r (plotter, linemodes[linetype]);
		}
	      if (fontsize != 0)
		{
		  if (plotter_open == false)
		    begin_page (plotter);
		  set_font_size (plotter, fontsize);
		}
	    }
	  cur_X = 0;
	  cur_Y = TEKHOME;		/* home pos. depends on fontsize */
	  break;
	  
	case CASE_CHAR_SIZE:	/* select font size */
	  fontsize = c & 03;
	  if (current_page == requested_page || !single_page_is_requested)
	    {
	      if (plotter_open == false)
		begin_page (plotter);
	      set_font_size (plotter, fontsize);
	    }
	  Tparsestate = curstate;
	  break;
	  
	case CASE_BEAM_VEC:	/* select beam and vector types */
	  /* disregard beam type */
	  c &= 07;
	  if (c != linetype)
	    if (current_page == requested_page || !single_page_is_requested)
	      {
		if (plotter_open == false)
		  begin_page (plotter);
		linetype = c;
		pl_linemod_r (plotter, linemodes[linetype]);
	      }
	  Tparsestate = curstate;
	  break;

	  /****************************************/

	  /* Things we ignore. */

	case CASE_OSC:		/* do osc escape */
	  /* ignore all bytes up to and including first non-ascii byte
	     (presumably BEL) */
	  do
	    c = read_byte (in_stream, &badstatus);
	  while (!badstatus && PRINTABLE_ASCII(c));
	  Tparsestate = curstate;
	  break;

	case CASE_ANSI:		/* parse an ANSI-style escape sequence */
	  {
	    char ansi[BUFFER_SIZE]; /* buffer for escape sequence */
	    char type = 0;	/* `type' (i.e. final byte) */
	    int i;		/* length of arg bytes, incl. separators */
	    
	    i = 0;
	    for ( ; ; )
	      {
		c = read_byte (in_stream, &badstatus);
		if (badstatus)
		  break;
		if ((c >= '0' && c <= '9') || c == ';'
		    || (i == 0 && c == '?'))
		  /* an arg byte, or a separator byte */
		  ansi[i++] = c;
		else
		  {
		    type = c;
		    if (!(PRINTABLE_ASCII(type)))
		      badstatus = 1; /* parse error */
		    break;
		  }
		if (i == BUFFER_SIZE)
		  {
		    fprintf (stderr, 
			     "%s: error: an overly long ANSI escape sequence was encountered\n",
			     progname);
		    badstatus = 1; /* parse error */
		    break;
		  }
	      }
	    Tparsestate = curstate;
	    if (badstatus)
	      break;
	    
	    if (i == 3 && (type == 'h' || type == 'l')
		&& (ansi[0] == '?' && ansi[1] == '3' && ansi[2] == '8'))
	      /* switch to Tek or VT100 mode, ignore */
	      break;

	    if (i == 4 && type == 'm' 
		&& (ansi[0] == '0' || ansi[0] == '1') 
		&& ansi[1] == ';' && ansi[2] == '3'
		&& ansi[3] >= '0' && ansi[3] <= '7')
	      /* set ANSI foreground color */
	      {
		int intensity, color_index;

		if (plotter_open == false)
		  begin_page (plotter);
		intensity = ansi[0] - '0';
		color_index = ansi[3] - '0';
		pl_pencolor_r (plotter, 
			       ansi_color[8 * intensity + color_index].red,
			       ansi_color[8 * intensity + color_index].green,
			       ansi_color[8 * intensity + color_index].blue);
		break;
	      }
	  }
	  break;

	case CASE_IGNORE:	/* Esc: totally ignore CR, ESC, LF, ~ */
	  break;
	  
	  /****************************************/

	  /* We interpret these as just restoring the stashed 
	     state (if any). */

	case CASE_REPORT:	/* report address */
	case CASE_VT_MODE:	/* special return to vt102 mode */
	case CASE_BEL:		/* BEL */
	case CASE_COPY:		/* make copy */
	case CASE_CURSTATE:
	  Tparsestate = curstate; /* clear bypass condition */
	  break;
	  
	case CASE_ASCII:	/* select ASCII char set */
	  /* ignore for now */
	  Tparsestate = curstate;
	  break;
	  
	case CASE_APL:		/* select APL char set */
	  /* ignore for now */
	  Tparsestate = curstate;
	  break;
	  
	case CASE_GIN:		/* do Tek GIN mode */
	  Tparsestate = Tbyptable; /* Bypass mode */
	  break;

	}
    }

  /* end parsing of this Tektronix stream */

  if (plotter_open == true)	/* close plotter, i.e. end page if any */
    end_page (plotter);

  current_page++;		/* bump page count for next file if any */

  return (badstatus == 2 ? true : false); /* OK to end parse at EOF */
}


void
set_font_size (plPlotter *plotter, int new_fontsize)
{
  if (use_tek_fonts)
    /* switch among Tektronix fonts (may not be available on all X servers) */
    {
      switch (new_fontsize)
	{
	case 0:
	default:
	  pl_fontname_r (plotter, "tekfont0");
	  break;
	case 1:
	  pl_fontname_r (plotter, "tekfont1");
	  break;
	case 2:
	  pl_fontname_r (plotter, "tekfont2");
	  break;
	case 3:
	  pl_fontname_r (plotter, "tekfont3");
	  break;
	}
    }
  else
    /* we presumably are using a scalable font */
    pl_ffontsize_r (plotter,
		    (double)(TekChar[new_fontsize].hsize) / CHAR_WIDTH);
}

void
begin_page (plPlotter *plotter)
{
  if (pl_openpl_r (plotter) < 0)
    {
      fprintf (stderr, 
	       "%s: error: the plot device could not be opened\n", 
	       progname);
      exit (EXIT_FAILURE);
    }
  plotter_open = true;
  plotter_opened = true;

  /* set background color, set affine map from user frame to device frame */
  pl_erase_r (plotter);
  pl_space_r (plotter, 0, 0, TEK_WIDTH - 1, TEK_WIDTH - 1);

  /* improve smoothness of plotted curves */
  pl_joinmod_r (plotter, "round");
  /* may be necessary if zero-length lines are to display as points */
  pl_capmod_r (plotter, "round");

  /* optionally initialize pen color, font, fontsize, line width */
  if (pen_color)
    pl_pencolorname_r (plotter, pen_color);
  if (use_tek_fonts)
    pl_fontname_r (plotter, "tekfont0");
  else
    {
      if (font_name)
	pl_fontname_r (plotter, font_name);
      else
	{
	  if (!force_hershey_default)
	    /* normal case */
	    {
	      if (pl_havecap_r (plotter, "PS_FONTS") == 1)
		pl_fontname_r (plotter, DEFAULT_PS_FONT_NAME);
	      else if (pl_havecap_r (plotter, "PCL_FONTS") == 1)
		pl_fontname_r (plotter, DEFAULT_PCL_FONT_NAME);
	      else
		/* use Hershey font as a default */
		pl_fontname_r (plotter, DEFAULT_HERSHEY_FONT_NAME);
	    }
	  else
	    /* forced to use Hershey font as a default, even if other fonts
	       are available (this happens for `-T hpgl'; see above) */
	    pl_fontname_r (plotter, DEFAULT_HERSHEY_FONT_NAME);
	}
      /* `large' is default size */
      pl_ffontsize_r (plotter, (double)(TekChar[0].hsize) / CHAR_WIDTH);
    }
  if (line_width >= 0.0)
    pl_flinewidth_r (plotter, line_width * TEK_WIDTH);

  /* move to current position on page */
  pl_move_r (plotter, cur_X, cur_Y + YOFFSET);
}

void
end_page (plPlotter *plotter)
{
  if (pl_closepl_r (plotter) < 0)
    {
      fprintf (stderr, 
	       "%s: error: the plot device could not be closed\n",
	       progname);
      exit (EXIT_FAILURE);
    }
  plotter_open = false;
}
