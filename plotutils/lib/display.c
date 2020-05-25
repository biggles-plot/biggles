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

/* This file contains the display_usage routine, which is used in
   user-level programs in the plotutils package.  It prints program options
   and long options in a reasonably nice format.
   
   This file also contains the display_version routine. */

#include "libcommon.h"
#include "getopt.h"

/* global array of long options, in program we're to be linked with */
extern struct option long_options[];

#define	ARG_NONE	0
#define	ARG_REQUIRED	1
#define	ARG_OPTIONAL	2

/* forward references */
bool elementp (int item, const int *list);
void display_usage (const char *progname, const int *omit_vals, const char *appendage, int info);
void display_version (const char *progname, const char *written, const char *copyright);

/* ARGS: list = null-terminated list of integers */
bool
elementp (int item, const int *list)
{
  int list_item;

  while ((list_item = *list++) != 0)
    {
      if (item == list_item)
	return true;
    }
  return false;
}

/* final arg of display_usage below is 0/1/2, meaning:

   0.  Print no info on output formats, because it is not relevant.
       (This is used by `spline' and `ode'.) 
   1.  Print info on output formats, specified by a `-T format' option,
       but not on the existence of a `--help-fonts' option, which returns 
       info specific to the choice of an output format.
       (This is used by `hersheydemo', which has a -T option but
       no user-specified fonts.)
   2.  Print info on output formats, specified by a `-T format' option,
       and also on the existence of a `--help-fonts' option, which returns 
       info on fonts that is specific to the choice of an output format.
       (This is used by `graph', `plot', 'tek2plot', `plotfont'.)
*/       

void
display_usage (const char *progname, const int *omit_vals, const char *appendage, int info)
{
  int i;
  int col = 0;
  
  fprintf (stdout, "Usage: %s", progname);
  col += (strlen (progname) + 7);
  for (i = 0; long_options[i].name; i++)
    {
      int option_len;
      
      if (elementp (long_options[i].val, omit_vals))
	continue;

      option_len = strlen (long_options[i].name);
      if (col >= 80 - (option_len + 16))
	{
	  fputs ("\n\t", stdout);
	  col = 8;
	}
      fprintf (stdout, " [--%s", long_options[i].name);
      col += (option_len + 4);
      if ((unsigned int)(long_options[i].val) < 256)
	{
	  fprintf (stdout, " | -%c", long_options[i].val);
	  col += 5;
	}
      if (long_options[i].has_arg == ARG_REQUIRED)
	{
	  fputs (" arg]", stdout);
	  col += 5;
	}
      else if (long_options[i].has_arg == ARG_OPTIONAL)
	{
	  fputs (" [arg(s)]]", stdout);
	  col += 10;
	}
      else
	{
	  fputs ("]", stdout);
	  col++;
	}
    }

  if (appendage != NULL)
    fputs (appendage, stdout);
  else
    fputs ("\n", stdout);

  if (info == 1)
    {
    fprintf (stdout, "\n\
To specify an output format, type `%s -T \"format\"',\n\
where \"format\" is one of:\n", progname);
    }
  else if (info == 2)
    {
    fprintf (stdout, "\n\
To list available fonts, type `%s -T \"format\" --help-fonts',\n\
where \"format\" is the output format, and is one of:\n", progname);
    }
  
  if (info == 1 || info == 2)
    {
#ifdef INCLUDE_PNG_SUPPORT
#ifndef X_DISPLAY_MISSING
    fprintf (stdout, "\
X, png, pnm, or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, or tek (vector formats).\n");
#else  /* X_DISPLAY_MISSING */
    fprintf (stdout, "\
png, pnm, or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, or tek (vector formats).\n");
#endif /* X_DISPLAY_MISSING */
#else  /* not INCLUDE_PNG_SUPPORT */
#ifndef X_DISPLAY_MISSING
    fprintf (stdout, "\
X, pnm, or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, or tek (vector formats).\n");
#else  /* X_DISPLAY_MISSING */
    fprintf (stdout, "\
pnm or gif (bitmap formats), or\n\
svg, ps, ai, cgm, fig, pcl, hpgl, regis, or tek (vector formats).\n");
#endif /* X_DISPLAY_MISSING */
#endif
    fprintf (stdout, "\
The default format is \"meta\", which is probably not what you want.\n");
    }
  
  if ((appendage != NULL) || info == 1 || info == 2)
    fputs ("\n", stdout);
  fprintf (stdout, "\
Report bugs to %s.\n", PACKAGE_BUGREPORT);
}

void
display_version (const char *progname, const char *written, const char *copyright)
{
  fprintf (stdout, "%s (%s) %s\n", 
	   progname, PACKAGE_NAME, PACKAGE_VERSION);
  fprintf (stdout, "%s\n",
	   copyright);
  fprintf (stdout, "%s",
	   "This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
  fprintf (stdout, "%s\n",
	   written);
}
