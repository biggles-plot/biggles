/* This file is part of the GNU plotutils package.  
   Copyright (C) 1989--2009, Free Software Foundation, Inc.

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

/* This program, hersheydemo, is a demo program for the Hershey vector
   fonts, as implemented in GNU libplot.  It outputs a demo page, designed
   by Dr. Allen Hershey himself.  The page is taken from his 1972 article
   "A computer system for scientific typography", published in Computer
   Graphics and Image Processing (vol. 1, no. 4, pp. 373-385).

   The demo page may be output in any of the vector or bitmap formats that
   GNU libplot supports.  The output format is specified with the `-T'
   option.  For instance, do

   hersheydemo -TX 

   to pop up a window on an X Window System display, showing Dr. Hershey's
   demo page.  Do

   hersheydemo -Tps > output.ps

   to obtain a Postscript output file, and 

   hersheydemo -Tps | gv -

   to display the file immediately, using Ghostview.  Do

   hersheydemo -Tsvg > output.svg

   to obtain an SVG (Scalable Vector Graphics) file, or

   hersheydemo -Tsvg | display

   to display it immediately, using the `display' program that is part of
   the ImageMagick package.  Within a traditional `xterm' terminal emulator
   window (though not within any of the more recent terminal emulators),
   you can do

   hersheydemo -Ttek

   to display the demo page using the terminal window's Tektronix mode,
   which will pop up an auxiliary window.

   Additional formats are supported; do `hersheydemo --help' for a list.

   In most of the above output formats, you can use the `--pen-color
   option' to change then pen color; for instance, you can include
   `--pen-color blue' to draw with a blue pen instead of a black pen (the
   default).  Also, in many of the above output formats you can use the
   `--bg-color' option to specify the background color.  For instance, you
   can include `--bg-color yellow' to obtain a yellow background.
   
   In most output formats, you can use the `--rotation 90' option to
   rotate by 90 degrees, etc.
*/

#include "sys-defines.h"
#include "libcommon.h"
#include "getopt.h"
#include "plot.h"

#ifdef M_SQRT1_2
#undef M_SQRT1_2
#endif
#define M_SQRT1_2   (0.70710678118654752440) /* 1/sqrt(2) */

#define NUM_DEMO_WORDS 34
struct hershey_word 
{
  const char *word;
  const char *fontname;		/* Hershey font name, libplot-style */
  double m[6];	    /* PS-style transformation matrix */
  char just;	    /* horizontal justification (left/center/right) */
};

/* Dr. Hershey's glyph database includes reduced-size (`indexical') and
   small-size (`cartographic') glyphs.  Some of the strings on his demo
   page used them.  By default, libplot does not use the indexical or
   cartographic glyphs (though they can be retrieved from the glyph
   database, which is incorporated bodily into libplot, if necessary).
   Hence we scale by an appropriate factor when rendering the strings that
   were originally indexical or cartographic, as the full-sized glyphs in
   the database will be used to render them. */

#define CART (9.0/21.0)		/* `cartographic' size */
#define INDEXICAL (13.0/21.0)	/* `indexical' size */

/* bounding box */

#define LLX -3800.0
#define LLY -3450.0
#define URX 3800.0
#define URY 4150.0

#define BASE_FONTSIZE 220.0

const struct hershey_word demo_word[NUM_DEMO_WORDS] = 
{
  {"Invitation", "HersheyScript-Bold", 
     { 1., 0., 0., 1., -3125., 3980. }, 'l' },
  {"ECONOMY", "HersheySans",           
     { 1., 0., 0., 1., -3125., 3340. }, 'l'},
  {"CARTOGRAPHY", "HersheySans",       
     { CART, 0., 0., CART, -3125., 2700. }, 'l'},
  {"Gramma", "HersheySerifSymbol",    
     { 1., 0., 0., 1., -3125., 2060. }, 'l'},
  {"\347\322\301\306\311\313\301", "HersheyCyrillic",       
     { 1., 0., 0., 1., -3125., 1420. }, 'l'},

  {"COMMUNICATION", "HersheySans-Bold", 
     { 1., 0., 0., 1., 0., 3980. }, 'c'},
  {"VERSATILITY", "HersheySerif-Italic",
     { 1., 0., 0., 1., 0., 3340. }, 'c'},
  {"Standardization", "HersheySerif",   
     { 1., 0., 0., 1., 0., 2700. }, 'c'},
  {"Sumbolon", "HersheySerifSymbol",  
     { INDEXICAL, 0., 0., INDEXICAL, 0., 2060. }, 'c'},
  {"\363\354\357\366\356\357\363\364\370", "HersheyCyrillic",      
     { 1., 0., 0., 1., 0., 1420. }, 'c'},

  {"Publication", "HersheyScript-Bold", 
     { 1., 0., 0., 1., 3125., 3980. }, 'r'},
  {"Quality", "HersheyGothicEnglish",  
     { 1., 0., 0., 1., 3125., 3340. }, 'r'},
  {"TYPOGRAPHY", "HersheySans",         
     { CART, 0., 0., CART, 3125., 2700. }, 'r'},
  {"AriJmo\\s-", "HersheySerifSymbol",    
     { 1., 0., 0., 1., 3125., 2060. }, 'r'},
  {"\346\317\316\305\324\311\313\301", "HersheyCyrillic",       
     { 1., 0., 0., 1., 3125., 1420. }, 'r'},

  {"EXTENSION", "HersheySans", 
     { 17./7., 0., 0., 2./7., 0., 780. }, 'c'},
  {"CONDENSATION", "HersheySans", 
     { 5./7., 0., 0., 17./7., 0., -20. }, 'c'},
  {"Rotation", "HersheySans", 
     { M_SQRT1_2, M_SQRT1_2, -M_SQRT1_2, M_SQRT1_2, -2880., -20. }, 'l'},
  {"ROTATION", "HersheySans", 
     { M_SQRT1_2, -M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, 2880., -20. }, 'r'},

  {"Syllabary", "HersheySerif",           
     { 1., 0., 0., 1., -3125., -780. }, 'l'},
  {"Art", "HersheyGothicEnglish",        
     { 1., 0., 0., 1., -3125., -1420. }, 'l'},
  {"Meteorology", "HersheySerif-Italic",  
     { INDEXICAL, 0., 0., INDEXICAL, -3125., -2060.}, 'l'},
  {"CHEMISTRY", "HersheySerif",           
     { 1., 0., 0., 1., -3125., -2700.}, 'l'},
  {"Analysis", "HersheySerif-BoldItalic", 
     { 1., 0., 0., 1., -3125., -3340.}, 'l'},

  {"LEXIKON", "HersheySerifSymbol-Bold",      
     { 1., 0., 0., 1., 0., -780.}, 'c'},
  {"\\#J3d71\\#J463b", "HersheySerif",      
     { 1./.7, 0., 0., 1./.7, 0., -1420.}, 'c'},
  {"Wissenschaft", "HersheyGothicGerman",
     { 1., 0., 0., 1., 0., -2060.}, 'c'},
  {"Electronics", "HersheySerif-Italic",  
     { 1., 0., 0., 1., 0., -2700.}, 'c'},
  {"COMPUTATION", "HersheySerif-Bold",    
     { 1., 0., 0., 1., 0., -3340.}, 'c'},

  {"Alphabet", "HersheySerif",            
     { 1., 0., 0., 1., 3125., -780.}, 'r'},
  {"Music", "HersheyGothicItalian",      
     { 1., 0., 0., 1., 3125., -1420.}, 'r'},
  {"Astronomy", "HersheySerif",           
     { INDEXICAL, 0., 0., INDEXICAL, 3125., -2060.}, 'r'},
  {"MATHEMATICS", "HersheySerif",         
     { 1., 0., 0., 1., 3125., -2700.}, 'r'},
  {"Program", "HersheySerif-BoldItalic",  
     { 1., 0., 0., 1., 3125., -3340.}, 'r'},
};

const char *progname = "hersheydemo"; /* name of this program */
const char *written = "Written by Robert S. Maier.";
const char *copyright = "Copyright (C) 2009 Free Software Foundation, Inc.";

const char *usage_appendage = "\n";

char *bg_color = NULL;	     /* bg color */
char *pen_color = NULL;	     /* pen color */
char *bitmap_size = NULL;    /* e.g. 500x500 (for bitmap output formats) */
char *page_size = NULL;	     /* e.g. a4 (certain vector output formats) */

/* options */

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

const char *optstring = "T:";

struct option long_options[] = 
{
  /* The most important option ("--display-type" is an obsolete variant) */
  { "output-format",	ARG_REQUIRED,	NULL, 'T'},
  { "display-type",	ARG_REQUIRED,	NULL, 'T' << 8 }, /* obsolete */
  /* Long options with (mostly) no equivalent short option alias */
  { "bg-color",		ARG_REQUIRED,	NULL, 'q' << 8 },
  { "pen-color",	ARG_REQUIRED,	NULL, 'C' << 8 },
  { "rotation",		ARG_REQUIRED,	NULL, 'r' << 8},
  { "bitmap-size",	ARG_REQUIRED,	NULL, 'B' << 8},
  { "page-size",	ARG_REQUIRED,	NULL, 'P' << 8},
  /* Documentation options */
  { "version",		ARG_NONE,	NULL, 'V' << 8 },
  { "help",		ARG_NONE,	NULL, 'h' << 8 },
  { NULL,		0,		NULL,  0}
};

/* null-terminated list of options, such as obsolete-but-still-maintained
   options or undocumented options, which we don't show to the user */
const int hidden_options[] = {(int)('T' << 8), 0 };

int
main (int argc, char *argv[])
{
  plPlotter *plotter;
  plPlotterParams *plotter_params;
  bool show_usage = false;	/* show usage message? */
  bool show_version = false;	/* show version message? */
  char *output_format = (char *)"meta"; /* default libplot output format */
  int errcnt = 0;		/* errors encountered */
  int opt_index;		/* long option index */
  int option;			/* option character */
  int i;

  /* set Plotter parameters */
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

	case 'C' << 8:		/* set the pen color, ARG REQUIRED */
	  pen_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (pen_color, optarg);
	  break;
	case 'q' << 8:		/* set the background color, ARG REQUIRED */
	  bg_color = (char *)xmalloc (strlen (optarg) + 1);
	  strcpy (bg_color, optarg);
	  break;
	case 'r' << 8:		/* Plot rotation angle, ARG REQUIRED	*/
	  pl_setplparam (plotter_params, "ROTATION", (void *)optarg);
	  break;

	case 'B' << 8:		/* Bitmap size, ARG REQUIRED */
	  pl_setplparam (plotter_params, "BITMAPSIZE", (void *)optarg);
	  break;
	case 'P' << 8:		/* Page size, ARG REQUIRED */
	  pl_setplparam (plotter_params, "PAGESIZE", (void *)optarg);
	  break;

	case 'V' << 8:		/* Version */
	  show_version = true;
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
  if (show_usage)
    {
      display_usage (progname, hidden_options, usage_appendage, 1);
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

  /* open the plot device, set up user coordinate system */
  pl_openpl_r (plotter);
  pl_erase_r (plotter);
  pl_fspace_r (plotter, LLX, LLY, URX, URY);

  if (pen_color)
    pl_pencolorname_r (plotter, pen_color);

  /* loop through words, displaying each; and each time, saving and
     restoring the graphics state (which includes the current
     transformation matrix) */
  for (i = 0; i < NUM_DEMO_WORDS; i++)
    {
      pl_savestate_r (plotter);
      pl_fontname_r (plotter, demo_word[i].fontname);
      /* insert a PS-style transformation matrix, including both
	 repositioning and scaling, into the graphics pipeline */
      pl_fconcat_r (plotter,
		    demo_word[i].m[0], demo_word[i].m[1], 
		    demo_word[i].m[2], demo_word[i].m[3],
		    demo_word[i].m[4], demo_word[i].m[5]);
      /* all words have the same font size in user coordinates
	 (though not necessarily in device coordinates) */
      pl_ffontsize_r (plotter, BASE_FONTSIZE);
      /* all words have the same location in user coordinates
	 (though not, obviously, in device coordinates) */
      pl_fmove_r (plotter, 0.0, 0.0);
      /* each word is drawn as an `alabel' (adjusted label), i.e., text
	 string with specified horizontal justification and with vertical
	 justification that is always `c', i.e., which is vertically
	 centered */
      pl_alabel_r (plotter, demo_word[i].just, 'c', demo_word[i].word);
      pl_restorestate_r (plotter);
    }

  /* close and delete Plotter */
  pl_closepl_r (plotter);
  if (pl_deletepl_r (plotter) < 0)
    {
      fprintf (stderr, "Couldn't delete Plotter\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
