/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

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

/* This header file is #include'd by g_pagetype.c.  It is a database rather
   than a true header file: it lists known page sizes and dimensions.

   Fields are:
   (1) name, (2) alternative name, (3) name used by Fig, 
   (4) whether metric, as opposed to imperial, units are associated
       with this page size.
   (5) width in inches,
   (6) height in inches,
   (7) the size of the viewport (a square) we place on the page.  

       (The viewport is positioned at the center of the page except when
       producing HP-GL[/2] output.  For HP-GL[/2], as opposed to HP-GL/2
       embedded in PCL5, we have no definite information about the location
       of the origin of the device coordinate system [we don't even know
       whether the device is plotting in portrait or landscape mode,
       though it's probably landscape mode].)

       Our convention: both in the pure-HP-GL[/2] case and in all other
       cases, the viewport is chosen to have a fixed size that's no larger
       than the smaller dimension of the plotting area used, for the
       specified page type, by AutoCAD.  AutoCAD presumably does a good job
       of selecting a plotting area (a rectangle) that will fit on a
       landscape-mode HP-GL[/2] page.

       In the HP-GL[/2] case we make no attempt at centering the viewport,
       though at least one of the two coordinates shouldn't be centered too
       badly.  The user can manually adjust the location of the viewport.
       
   (8,9) the origin of the device coordinate system, when HP-GL/2 is
       embedded in PCL5; relative to the lower left corner of page.  
       Source: the "PCL 5 Comparison Guide", from Hewlett-Packard.

       Note that when a PCL5 "dual context" portrait-mode device is
       switched to HP-GL/2 mode, the HP-GL/2 plotting is also in portrait
       mode.  I.e., on a PCL5 device doing HP-GL/2 emulation it's possible
       to set the portrait/landscape mode programmatically, unlike a pure
       HP-GL/2 device.

   (10) the plot length (of importance mostly for roll plotters, when
        pure HP-GL/2, rather than HP-GL/2 embedded in PCL5, is output).
	I've come up with values that I hope are appropriate.

   Explanatory comments:

   In general, the origin for the HP-GL[/2] coordinate system is not a
   corner of the printed page.  (This is an old convention, dating back to
   early pen plotter days.)  It is the lower left corner of the `hard-clip
   region', a proper subrectangle of the printed page.  The size and
   orientation of the hard-clip region differ from device to device.
   Fields #8, #9 below give the location of the origin in a PCL device
   supporting HP-GL/2.  Fields #8, #9 could equally well be defined as the
   x margin and y margin between the HP-GL/2 hard-clip region and the
   boundary of the page.  The HP-GL/2 hard-clip region is a proper
   subrectangle of the rectangular area of the page that is imageable from
   within PCL.

   In a pure HP-GL/2 device, there is no easy way to determine the origin.
   (In fact in early HP-GL plotters, unlike HP-GL/2 plotters, the lower
   left corner of the hard-clip region wasn't even the same as the default
   location of the so-called HP-GL `scaling point' P1.) */

#define PL_NUM_PAGESIZES 13

static const plPageData _pagedata[PL_NUM_PAGESIZES] =
{
  /* ANSI A, 8.5in x 11.0in */
  /* AutoCAD plotting area is 8.0x10.5;
     we choose viewport size 8.0in */
  { "a", "letter", "Letter", false, 
    8.5, 11.0, 8.0, 75.0/300, 150.0/300, 10.5 },

  /* ANSI B, 11.0in x 17.0in */
  /* AutoCAD plotting area is 10.0x16.0;
     we choose viewport size 10.0in */
  { "b", "tabloid", "B", false,
    11.0, 17.0, 10.0, 75.0/300, 150.0/300, 16.0 },

  /* ANSI C, 17.0in x 22.0in */
  /* AutoCAD plotting area is 16.0x21.0;
     we choose viewport size 16.0in */
  { "c", NULL, "C", false,
    17.0, 22.0, 16.0, 75.0/300, 150.0/300, 21.0 },

  /* ANSI D, 22.0in x 34.0in */
  /* AutoCAD plotting area is 21.0x33.0;
     we choose viewport size 20.0in */
  { "d", NULL, "D", false,
    22.0, 34.0, 20.0, 75.0/300, 150.0/300, 33.0 },

  /* ANSI E, 34.0in x 44.0in */
  /* AutoCAD plotting area is 33.0x43.0;
     we choose viewport size 32.0in */
  { "e", NULL, "E", false,
    34.0, 44.0, 32.0, 75.0/300, 150.0/300, 43.0 },

  /* legal, 8.5in x 14in; not an ANSI size */
  /* AutoCAD plotting area unknown; 
     we choose viewport size = 8.0in */
  { "legal", NULL, "Legal", false,
    8.5, 14.0, 8.0, 75.0/300, 150.0/300, 16.0 },

  /* ledger, 17in x 11in (rotated ANSI B); not an ANSI size ? */
  /* we use viewport size = 10.0in */
  { "ledger", NULL, "Ledger", false,
    17.0, 11.0, 10.0, 150.0/300, 75.0/300, 10.0 },

  /* ISO A4, 21.0cm x 29.7 cm = 8.27 x 11.69 */
  /* AutoCAD plotting area is 7.8x11.2; 
     we choose viewport size 7.8in=19.81cm [N.B. 7.87in = 20cm] */
  { "a4", NULL, "A4", true,
    8.27, 11.69, 7.8, 71.0/300, 150.0/300, 11.2 },

  /* ISO A3, 29.7cm x 42.0 cm  = 11.69 x 16.54 */
  /* AutoCAD plotting area is 10.7x15.6; 
     we choose viewport size 10.7in=27.18cm [N.B. 10.62in = 27cm] */
  { "a3", NULL, "A3", true,
    11.69, 16.54, 10.7, 71.0/300, 150.0/300, 15.6 },

  /* ISO A2, 42.0cm x 59.4 cm = 16.54 x 23.39 */
  /* AutoCAD plotting area is 15.6x22.4; 
     we choose viewport size 15.6in=39.62cm [N.B. 15.75in = 40cm] */
  { "a2", NULL, "A2", true,
    16.54, 23.39, 15.6, 71.0/300, 150.0/300, 22.4 },

  /* ISO A1, 59.4cm x 84.1 cm = 23.39 x 33.11 */
  /* AutoCAD plotting area is 22.4x32.2; 
     we choose viewport size 22.4in=56.90cm [N.B. 21.26in = 54cm] */
  { "a1", NULL, "A1", true,
    23.39, 33.11, 22.4, 71.0/300, 150.0/300, 32.2 },

  /* ISO A0, 84.1cm x 118.9 cm = 33.11 x 46.81 */
  /* AutoCAD plotting area is 32.2x45.9; 
     we choose viewport size 32.2in=81.79cm [N.B. 31.50in = 80cm] */
  { "a0", NULL, "A0", true,
    33.11, 46.81, 32.2, 71.0/300, 150.0/300, 45.9 },

  /* JIS B5, 18.2cm x 25.7 cm = 7.17 x 10.12 */
  /* AutoCAD plotting area is 6.67x9.62(?); 
     we choose viewport size 6.67in=16.94cm [N.B. 6.30in = 16cm] */
  { "b5", NULL, "B5", true,
    7.17, 10.12, 6.67, 71.0/300, 150.0/300, 9.62 }
};
