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

/* Lookup table listing relative chordal deviation, i.e. the factor
   (chordal deviation)/(chord length), for binary subdivisions of the arc
   sizes we support.

   The symbolic names for these arc sizes, e.g. QUARTER_ARC and
   THREE_QUARTER_ARC, are defined in extern.h.  The constant
   MAX_ARC_SUBDIVISIONS, which can in principle be device dependent, is
   also defined there.  It should be no greater than
   TABULATED_ARC_SUBDIVISIONS, the length of each entry in the lookup
   table.

   The formula

       relative_deviation = (1 - cos (theta/2)) / (2 * sin (theta/2)),

   where theta = subtended angle, was used to compute the numbers in the
   lookup tables.  The formula is justified as follows.  Let

	   s = chordal deviation / radius (so that 0 <= s <= 2)
	   h = half chord length / radius.

   Then by elementary trigonometry, s = 1 - cos(theta/2) and 
   h = sin(theta/2), where theta = angle subtended by the chord.  Since
   the relative chordal deviation is s/2h, this justifies the formula.
   Incidentally, h = sqrt(s * (2-s))), irrespective of theta.

   There is an alternative derivation of the numbers in these tables,
   tail-recursive and elegant, which uses only sqrt().  Let s' and h' be
   the values of s and h when the arc is bisected, i.e. theta is divided by
   two.  Using half-angle formulae it is easy to check that

	   s' = 1 - sqrt (1 - s / 2)
	   h' = h / 2 (1 - s')

   (The first of these formulae is well-known; see the article of Ken
   Turkowski <turk@apple.com> in Graphics Gems V, where he approximates the
   right-hand side by s/4.)  So the pair (s',h') may be computed from the
   pair (s,h).  By updating (s,h) with every bisection, one can generate a
   table of successive values of the quotient s/2h, i.e. the relative
   chordal deviation.
*/

/* Maximum number of times a circular or elliptic arc is recursively
   subdivided, when it is being approximated by an inscribed polyline.  The
   polyline will contain no more than 2**MAX_ARC_SUBDIVISIONS line
   segments.  MAX_ARC_SUBDIVISIONS must be no larger than
   TABULATED_ARC_SUBDIVISIONS below (the size of the tables in g_arc.h). */
#define MAX_ARC_SUBDIVISIONS 5 	/* to avoid buffer overflow on HP7550[A|B] */

#define TABULATED_ARC_SUBDIVISIONS 15	/* length of each table entry */

/* Types of circular/elliptic arc.  These index into the doubly indexed
   table of `relative chordal deviations' below. */
#define NUM_ARC_TYPES 3

#define QUARTER_ARC 0
#define HALF_ARC 1
#define THREE_QUARTER_ARC 2
#define USER_DEFINED_ARC -1	/* does not index into table */

static const double _chord_table[NUM_ARC_TYPES][TABULATED_ARC_SUBDIVISIONS] =
{
  {	/* Quarter Arc */
    0.20710678,			/* for arc subtending 90 degrees */
    0.099456184,		/* for arc subtending 45 degrees */
    0.049245702,		/* for arc subtending 22.5 degrees */
    0.024563425,		/* for arc subtending 11.25 degrees */
    0.012274311,		/* etc. */
    0.0061362312,
    0.0030680001,
    0.0015339856,
    0.000766991,
    0.00038349527,
    0.00019174761,
    9.58738e-05,
    4.79369e-05,
    2.396845e-05,
    1.1984225e-05
  },
  {	/* Half Arc */
    0.5,			/* for arc subtending 180 degrees */
    0.20710678,			/* for arc subtending 90 degrees */
    0.099456184,		/* for arc subtending 45 degrees */
    0.049245702,		/* for arc subtending 22.5 degrees */
    0.024563425,		/* for arc subtending 11.25 degrees */
    0.012274311,		/* etc. */
    0.0061362312,
    0.0030680001,
    0.0015339856,
    0.000766991,
    0.00038349527,
    0.00019174761,
    9.58738e-05,
    4.79369e-05,
    2.396845e-05
  },
  {	/* Three Quarter Arc */
    1.2071068,			/* for arc subtending 270 degrees */
    0.33408932,			/* for arc subtending 135 degrees */
    0.15167334,			/* for arc subtending 67.5 degrees */
    0.074167994,		/* for arc subtending 33.75 degrees */
    0.036882216,		/* etc. */
    0.01841609,
    0.0092049244,
    0.0046020723,
    0.0023009874,
    0.0011504876,
    0.00057524305,
    0.00028762143,
    0.0001438107,
    7.190535e-05,
    3.5952675e-05
  }
};

