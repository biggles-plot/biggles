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

/* This file is the driving routine for the GNU `plotfont' program.  It
   includes code to plot all characters in a specified font. */

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"
#include "fontlist.h"
#include "plot.h"

const char *progname = "plotfont"; /* name of this program */
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = " FONT...\n";

enum radix { DECIMAL, OCTAL, HEXADECIMAL };

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "12oxOj:J:F:T:";

struct option long_options[] = 
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* hidden */
  /* Other frequently used options */
  { "box",		ARG_NONE,	NULL, 'b' << 8 },
  { "octal",		ARG_NONE,	NULL, 'o'},
  { "hexadecimal",	ARG_NONE,	NULL, 'x'},
  { "jis-page",		ARG_REQUIRED,	NULL, 'J'},
  { "jis-row",		ARG_REQUIRED,	NULL, 'j'},
  { "lower-half",	ARG_NONE,	NULL, '1'},
  { "upper-half",	ARG_NONE,	NULL, '2'},
  { "font-name",	ARG_REQUIRED,	NULL, 'F' }, /* hidden */
  /* Long options with no equivalent short option alias */
  { "numbering-font-name", ARG_REQUIRED, NULL, 'N' << 8 },
  { "title-font-name",	ARG_REQUIRED,	NULL, 'Z' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8 },
  { "emulate-color",	ARG_REQUIRED,	NULL, 'e' << 8 },  
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8 },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8 },
  /* Options relevant only to raw plotfont (refers to metafile output) */
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
const int hidden_options[] = { (int)'J', (int)'F', (int)('T' << 8), 0 };

/* forward references */
bool do_font (plPlotter *plotter, const char *name, bool upper_half, char *pen_color_name, char *numbering_font_name, char *title_font_name, bool bearings, enum radix base, int jis_page, bool do_jis_page);
void write_three_bytes (int charnum, char *numbuf, int radix);
void write_two_bytes (int charnum, char *numbuf, int radix);


int
main (int argc, char *argv[])
{
  plPlotter *plotter;
  plPlotterParams *plotter_params;
  bool bearings = false;	/* show sidebearings on characters? */
  bool do_jis_page = false;	/* show page of HersheyEUC in JIS encoding? */
  bool do_list_fonts = false;	/* show a list of fonts? */
  bool show_fonts = false;	/* supply help on fonts? */
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  bool upper_half = false;	/* upper half of font, not lower? */
  char *output_format = (char *)"meta"; /* default libplot output format */
  char *numbering_font_name = NULL; /* numbering font name, NULL -> default */
  char *option_font_name = NULL; /* allows user to use -F */
  char *pen_color = NULL;	/* initial pen color, can be spec'd by user */
  char *title_font_name = NULL;	/* title font name, NULL -> current font */
  enum radix base = DECIMAL;
  int errcnt = 0;		/* errors encountered */
  int jis_page = 33;		/* page of HersheyEUC in JIS encoding */
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
	  pl_setplparam (plotter_params, "META_PORTABLE", (char *)"yes");
	  break;
	case '1':		/* Lower half */
	  upper_half = false;
	  break;
	case '2':		/* Upper half */
	  upper_half = true;
	  break;
	case 'o':		/* Octal */
	  base = OCTAL;
	  break;
	case 'x':		/* Hexadecimal */
	  base = HEXADECIMAL;
	  break;
	case 'F':		/* set the initial font */
	  option_font_name = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (option_font_name, optarg);
	  break;
	case 'e' << 8:		/* Emulate color via grayscale */
	  pl_setplparam (plotter_params, "EMULATE_COLOR", (char *)optarg);
	  break;
	case 'N' << 8:		/* Numbering Font name, ARG REQUIRED */
	  numbering_font_name = xstrdup (optarg);
	  break;
	case 'Z' << 8:		/* Title Font name, ARG REQUIRED */
	  title_font_name = xstrdup (optarg);
	  break;
	case 'C' << 8:		/* set the initial pen color */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'q' << 8:		/* set the initial background color */
	  pl_setplparam (plotter_params, "BG_COLOR", (void *)optarg);
	  break;
	case 'B' << 8:		/* Bitmap size */
	  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)optarg);
	  break;
	case 'P' << 8:		/* Page size */
	  pl_setplparam (plotter_params, "PAGESIZE", (void *)optarg);
	  break;
	case 'r' << 8:		/* Rotation angle */
	  pl_setplparam (plotter_params, "ROTATION", (void *)optarg);
	  break;
	case 'b' << 8:		/* Bearings requested */
	  bearings = true;
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
	case 'J':		/* JIS page */
	  if (sscanf (optarg, "%d", &jis_page) <= 0
	      || (jis_page < 33) || (jis_page > 126))
	    {
	      fprintf (stderr,
		       "%s: the JIS page number is bad (it should be in the range 33..126)\n",
		       progname);
	      errcnt++;
	    }
	  else
	    do_jis_page = true;
	  break;
	case 'j':		/* JIS row */
	  if (sscanf (optarg, "%d", &jis_page) <= 0
	      || (jis_page < 1) || (jis_page > 94))
	    {
	      fprintf (stderr,
		       "%s: the JIS row number is bad (it should be in the range 1..94)\n",
		       progname);
	      errcnt++;
	    }
	  else
	    {
	      jis_page += 32;
	      do_jis_page = true;
	    }
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

  if (option_font_name == NULL && optind >= argc)
    {
      fprintf (stderr, "%s: no font or fonts are specified\n", progname);
      return EXIT_FAILURE;
    }

  if (do_jis_page)
    {
      if ((!((option_font_name == NULL && optind == argc - 1)
	    || (option_font_name && optind >= argc)))
	  || (option_font_name && strcasecmp (option_font_name, "HersheyEUC") != 0)
	  || (!option_font_name && strcasecmp (argv[optind], "HersheyEUC") != 0))
	{
	  fprintf (stderr, "%s: a JIS page can only be specified for the HersheyEUC font\n", progname);
	  return EXIT_FAILURE;
	}	  
    }

  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr, 
			     plotter_params)) == NULL)
    {
      fprintf (stderr, "%s: error: the plot device could not be created\n", progname);
      return EXIT_FAILURE;
    }

  if (option_font_name)
    /* user specifed a font with -F */
    {
      if (do_font (plotter, option_font_name, upper_half, pen_color, numbering_font_name, title_font_name, bearings, base, jis_page, do_jis_page) == false)
	return EXIT_FAILURE;
    }
  
  if (optind < argc)
    /* 1 or more fonts named explicitly on command line */
    {
      for (; optind < argc; optind++)
	{
	  char *font_name;

	  font_name = argv[optind];
	  if (do_font (plotter, font_name, upper_half, pen_color, numbering_font_name, title_font_name, bearings, base, jis_page, do_jis_page) == false)
	    return EXIT_FAILURE;
	}
    }

  /* clean up */
  retval = EXIT_SUCCESS;
  if (pl_deletepl_r (plotter) < 0)
    {
      fprintf (stderr, "%s: error: the plot device could not be deleted\n", progname);
      retval = EXIT_FAILURE;
    }
  pl_deleteplparams (plotter_params);

  return retval;
}

#define NUM_ROWS 12

#define SIZE 1.0
#define MAX_TITLE_LENGTH 0.9

#define HSPACING 0.1	/* must have 8*HSPACING < SIZE */
#define VSPACING 0.06	/* must have 12*VSPACING < SIZE-LINE_VOFFSET */

#define LINE_HOFFSET (0.5 * ((SIZE) - 8 * (HSPACING)))
#define LINE_VOFFSET 0.15	/* top line down from top of display by this */
#define CHAR_HOFFSET ((LINE_HOFFSET) + 0.5 * (HSPACING))
#define CHAR_VOFFSET ((LINE_VOFFSET) + 0.5 * (VSPACING))

#define TOP (SIZE - (LINE_VOFFSET))
#define BOTTOM ((SIZE - (LINE_VOFFSET)) - 12 * (VSPACING))
#define LEFT (LINE_HOFFSET)
#define RIGHT (LINE_HOFFSET + 8 * HSPACING)

#define FONT_SIZE 0.04
#define TITLE_FONT_SIZE 0.045
#define NUMBERING_FONT_SIZE 0.015

/* shifts of numbers in grid cells leftward and upward */
#define N_X_SHIFT 0.015
#define N_Y_SHIFT 0.05

bool
do_font (plPlotter *plotter, const char *name, bool upper_half, char *pen_color_name, char *numbering_font_name, char *title_font_name, bool bearings, enum radix base, int jis_page, bool do_jis_page)
{
  char buf[16];
  char numbuf[16];
  char suffixbuf[16];
  char *titlebuf;
  const char *suffix;
  double title_width;
  int i, j, bottom_octet, top_octet;

  if (do_jis_page)
    {
      switch (base)
	{
	case DECIMAL:
	default:
	  sprintf (suffixbuf, " (row %d)", jis_page - 32);
	  break;
	case OCTAL:
	  sprintf (suffixbuf, " (row 0%o)", jis_page - 32);
	  break;
	case HEXADECIMAL:
	  sprintf (suffixbuf, " (row 0x%X)", jis_page - 32);
	  break;
	}
      suffix = suffixbuf;
    }
  else
    suffix = upper_half ? " (upper half)" : " (lower half)";
  titlebuf = (char *)xmalloc (strlen (name) + strlen (suffix) + 1);
  strcpy (titlebuf, name);
  strcat (titlebuf, suffix);

  if (pl_openpl_r (plotter) < 0)
    {
      fprintf (stderr, "%s: error: the plot device could not be opened\n", progname);
      return false;
    }

  pl_fspace_r (plotter, 0.0, 0.0, (double)SIZE, (double)SIZE);
  pl_erase_r (plotter);
  if (pen_color_name)
    pl_pencolorname_r (plotter, pen_color_name);
  
  pl_fmove_r (plotter, 0.5 * SIZE, 0.5 * (SIZE + TOP));
  if (title_font_name)
    pl_fontname_r (plotter, title_font_name);
  else
    pl_fontname_r (plotter, name);
  pl_ffontsize_r (plotter, (double)(TITLE_FONT_SIZE));

  title_width = pl_flabelwidth_r (plotter, titlebuf);
  if (title_width > MAX_TITLE_LENGTH)
    /* squeeze title to fit */
    pl_ffontsize_r (plotter, 
		    (double)(TITLE_FONT_SIZE) * (MAX_TITLE_LENGTH / title_width));

  /* print title */
  pl_alabel_r (plotter, 'c', 'c', titlebuf);

  if (do_jis_page)
    bottom_octet = 4;
  else			/* ordinary map */
    {
      if (upper_half)
	bottom_octet = 20;
      else
	bottom_octet = 4;
    }
  top_octet = bottom_octet + NUM_ROWS - 1;

  /* draw grid */

  pl_linemod_r (plotter, "solid");
  pl_fbox_r (plotter, LEFT, BOTTOM, RIGHT, TOP);
  for (i = 1; i <= 7; i++)
    /* boustrophedon */
    {
      if (i % 2)
	pl_fline_r (plotter, 
		    LINE_HOFFSET + i * HSPACING, BOTTOM, 
		    LINE_HOFFSET + i * HSPACING, TOP);
      else
	pl_fline_r (plotter, 
		    LINE_HOFFSET + i * HSPACING, TOP,
		    LINE_HOFFSET + i * HSPACING, BOTTOM);
    }      
  for (j = 1; j <= 11; j++)
    /* boustrophedon */
    {
      if (j % 2)
	pl_fline_r (plotter, 
		    RIGHT, TOP - j * VSPACING,
		    LEFT, TOP - j * VSPACING);
      else
	pl_fline_r (plotter,  
		    LEFT, TOP - j * VSPACING,
		    RIGHT, TOP - j * VSPACING);
    }

  /* number grid cells */

  if (numbering_font_name)
    pl_fontname_r (plotter, numbering_font_name);
  else				/* select default font */
    pl_fontname_r (plotter, "");
  pl_ffontsize_r (plotter, (double)(NUMBERING_FONT_SIZE));
  if (bearings)
    pl_linemod_r (plotter, "dotted");
  for (i = bottom_octet; i <= top_octet; i++)
    for (j = 0; j < 8; j++)
      {
	int row, column, charnum;

	row = i - bottom_octet;	/* row, 0..11 */
	column = j;		/* column, 0..7 */

	charnum = (unsigned char)(8 * i + j);
	if (charnum == 127)	/* 0xff not a legitimate glyph */
	  continue;
	if (do_jis_page && charnum == 32)
	  continue;		/* 0x20 not legitimate for JIS */

	switch (base)
	  {
	  case HEXADECIMAL:
	    write_two_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 16);
	    break;
	  case DECIMAL:
	  default:
	    write_three_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 10);
	    break;
	  case OCTAL:
	    write_three_bytes (charnum - (do_jis_page ? 32 : 0), numbuf, 8);
	    break;
	  }
	
	pl_fmove_r (plotter,
		    (double)(LINE_HOFFSET + HSPACING * (column + 1 - N_X_SHIFT)),
		    (double)(SIZE - (LINE_VOFFSET + VSPACING * (row + 1 - N_Y_SHIFT))));
	pl_alabel_r (plotter, 'r', 'x', numbuf);
      }

  /* fill grid cells with characters */

  pl_fontname_r (plotter, name);
  pl_ffontsize_r (plotter, (double)(FONT_SIZE));
  for (i = bottom_octet; i <= top_octet; i++)
    for (j = 0; j < 8; j++)
      {
	int row, column, charnum;

	row = i - bottom_octet;	/* row, 0..11 */
	column = j;		/* column, 0..7 */
	
	charnum = (unsigned char)(8 * i + j);
	if (charnum == 127)	/* 0xff not a legitimate glyph */
	  continue;
	if (do_jis_page && charnum == 32)
	  continue;		/* 0x20 not legitimate for JIS */

	if (!do_jis_page)
	  {
	    buf[0] = charnum;
	    buf[1] = '\0';
	  }
	else			/* do JIS page */
	  {
	    /* two bytes, set high bits on both page and character */
	    buf[0] = jis_page + 0x80;
	    buf[1] = charnum + 0x80;
	    buf[2] = '\0';
	  }

	pl_fmove_r (plotter, 
		    (double)(LINE_HOFFSET + HSPACING * (column + 0.5)),
		    (double)(SIZE - (LINE_VOFFSET + VSPACING * (row + 0.5))));
	/* place glyph on page */
	pl_alabel_r (plotter, 'c', 'c', (char *)buf);
	if (bearings)
	  {
	    double halfwidth;

	    /* compute glyph width */
	    halfwidth = 0.5 * pl_flabelwidth_r (plotter, (char *)buf);
	    if (halfwidth == 0.0)
	      /* empty glyph, draw only one vertical dotted line */
	      pl_fline_r (plotter,
			  (double)(CHAR_HOFFSET + HSPACING * column),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			  (double)(CHAR_HOFFSET + HSPACING * column),
			  (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
	    else
	      /* draw vertical dotted lines to either side of glyph */
	      {
		pl_fline_r (plotter, 
			    (double)(CHAR_HOFFSET + HSPACING * column - halfwidth),
			    (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			    (double)(CHAR_HOFFSET + HSPACING * column - halfwidth),
			    (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
		pl_fline_r (plotter, 
			    (double)(CHAR_HOFFSET + HSPACING * column + halfwidth),
			    (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row - 0.5))),
			    (double)(CHAR_HOFFSET + HSPACING * column + halfwidth),
			    (double)(SIZE - (CHAR_VOFFSET + VSPACING * (row + 0.5))));
	      }
	  }
      }
  
  if (pl_closepl_r (plotter) < 0)
    {
      fprintf (stderr, "%s: error: the plot device could not be closed\n", progname);
      return false;
    }
  return true;
}

/* Write an integer that is < (radix)**4 as three ascii bytes with respect
   to the specified radix.  Initial zeroes are converted to spaces. */
void 
write_three_bytes (int charnum, char *numbuf, int radix)
{
  int i;

  numbuf[0] = charnum / (radix * radix);
  numbuf[1] = (charnum - (radix * radix) * (charnum / (radix * radix))) / radix;
  numbuf[2] = charnum % radix;
  numbuf[3] = '\0';

  for (i = 0 ; i <= 2 ; i++)
    {
      if (numbuf[i] >= 10)
	numbuf[i] += ('A' - 10);
      else 
	numbuf[i] += '0';
    }
  if (numbuf[0] == '0')
    {
      numbuf[0] = ' ';
      if (numbuf[1] == '0')
	numbuf[1] = ' ';
    }
}

/* Write an integer that is < (radix)**3 as two ascii bytes with respect to
   the specified radix.  Initial zeroes are converted are not changed. */
void 
write_two_bytes (int charnum, char *numbuf, int radix)
{
  int i;

  numbuf[0] = charnum / radix;
  numbuf[1] = charnum % radix;
  numbuf[2] = '\0';

  for (i = 0 ; i <= 1 ; i++)
    {
      if (numbuf[i] >= 10)
	numbuf[i] += ('A' - 10);
      else 
	numbuf[i] += '0';
    }
}
