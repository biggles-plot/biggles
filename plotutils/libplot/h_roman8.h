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

/* This header file specifies HP's Roman-8 encoding (used by HP's vector
   fonts [`stick fonts'], including both fixed-width stick fonts and
   variable-width arc fonts).  In particular, it gives a mapping from the
   upper half of the ISO-Latin-1 character set to Roman-8. */

/* ISO-Latin-1 characters not included in Roman-8; we map each of them to
   040, i.e., to the space character. */
#define COPYRIGHT 040
#define NEGATION 040
#define REGISTERED 040
#define RAISEDONE 040
#define RAISEDTWO 040
#define RAISEDTHREE 040
#define CEDILLA 040
#define MULTIPLY 040
#define DIVIDES 040

static const unsigned char iso_to_roman8 [128] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* printable iso-latin-1 characters */
    040, 0270, 0277, 0257, 0272, 0274, (unsigned char)'|', 0275,
    0253, COPYRIGHT, 0371, 0373, NEGATION, (unsigned char)'-', REGISTERED, 0260,
    0263, 0376, RAISEDTWO, RAISEDTHREE, 0250, 0363, 0364, 0362, 
    CEDILLA, RAISEDONE, 0372, 0375, 0367, 0370, 0365, 0271,
    0241, 0340, 0242, 0341, 0330, 0320, 0323, 0264,
    0243, 0334, 0244, 0245, 0346, 0345, 0246, 0247,
    0343, 0266, 0350, 0347, 0337, 0351, 0332, MULTIPLY,
    0322, 0255, 0355, 0256, 0333, 0261, 0360, 0336,
    0310, 0304, 0300, 0342, 0314, 0324, 0327, 0265,
    0311, 0305, 0301, 0315, 0331, 0325, 0321, 0335,
    0344, 0267, 0312, 0306, 0302, 0352, 0316, DIVIDES,
    0326, 0313, 0307, 0303, 0317, 0262, 0361, 0357,
};
