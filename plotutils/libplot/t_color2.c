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

/* The 16 standard colors (ANSI, ISO 6429) supported by the Tektronix mode
   of MS-DOS Kermit.  (Reportedly `color xterm' supports them too, but
   maybe not in Tektronix emulation mode?)

   It includes the 8 standard colors (vertices of the RGB cube).  It also
   includes an intermediate brightness for each of the 6 colors other than
   black and white, and two intermediate brightnesses for white (i.e., two
   shades of gray intermediate between black and white).  The two shades of
   gray are the only colors in the interior of the cube. */

#include "sys-defines.h"
#include "extern.h"

const plColor _pl_t_kermit_stdcolors[TEK_NUM_ANSI_SYS_COLORS] = 
{
  {0x00, 0x00, 0x00},		/* black */
  {0x8b, 0x00, 0x00},		/* red4 */
  {0x00, 0x8b, 0x00},		/* green4 */
  {0x8b, 0x8b, 0x00},		/* yellow4 */
  {0x00, 0x00, 0x8b},		/* blue4 */
  {0x8b, 0x00, 0x8b},		/* magenta4 */
  {0x00, 0x8b, 0x8b},		/* cyan4 */
  {0x8b, 0x8b, 0x8b},		/* gray55 */
  {0x4d, 0x4d, 0x4d},		/* gray30 */
  {0xff, 0x00, 0x00},		/* red */
  {0x00, 0xff, 0x00},		/* green */
  {0xff, 0xff, 0x00},		/* yellow */
  {0x00, 0x00, 0xff},		/* blue */
  {0xff, 0x00, 0xff},		/* magenta */
  {0x00, 0xff, 0xff},		/* cyan */
  {0xff, 0xff, 0xff}		/* white */
};

/* Ordering of these two lists of ANSI escape sequences must match the
   above. */

const char * const _pl_t_kermit_fgcolor_escapes[TEK_NUM_ANSI_SYS_COLORS] = 
{
  "\033[0;30m",		/* black */
  "\033[0;31m",		/* red4 */
  "\033[0;32m",		/* green4 */
  "\033[0;33m",		/* yellow4 */
  "\033[0;34m",		/* blue4 */
  "\033[0;35m",		/* magenta4 */
  "\033[0;36m",		/* cyan4 */
  "\033[0;37m",		/* gray55 */
  "\033[1;30m",		/* gray30 */
  "\033[1;31m",		/* red */
  "\033[1;32m",		/* green */
  "\033[1;33m",		/* yellow */
  "\033[1;34m",		/* blue */
  "\033[1;35m",		/* magenta */
  "\033[1;36m",		/* cyan */
  "\033[1;37m"		/* white */
};

const char * const _pl_t_kermit_bgcolor_escapes[TEK_NUM_ANSI_SYS_COLORS] = 
{
  "\033[0;40m",		/* black */
  "\033[0;41m",		/* red4 */
  "\033[0;42m",		/* green4 */
  "\033[0;43m",		/* yellow4 */
  "\033[0;44m",		/* blue4 */
  "\033[0;45m",		/* magenta4 */
  "\033[0;46m",		/* cyan4 */
  "\033[0;47m",		/* gray55 */
  "\033[1;40m",		/* gray30 */
  "\033[1;41m",		/* red */
  "\033[1;42m",		/* green */
  "\033[1;43m",		/* yellow */
  "\033[1;44m",		/* blue */
  "\033[1;45m",		/* magenta */
  "\033[1;46m",		/* cyan */
  "\033[1;47m"		/* white */
};
