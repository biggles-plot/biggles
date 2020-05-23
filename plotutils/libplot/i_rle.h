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

/* This header file (i_rle.h) defines the external interface to the module
   i_rle.c, which does run-length encoding on a sequence of integers
   ("pixel values"), and writes the resulting encoded sequence to an output
   stream.  The encoded sequence should be GIF-compatible, even though the
   compression technique is not LZW.

   The module encapsulates the miGIF compression routines, originally
   written by der Mouse and ivo.  Their copyright notice appears in
   i_rle.c. */

/* an `int' should be able to hold 2**GIFBITS distinct values, together
   with -1 */
#define GIFBITS 12

/* the RLE output structure */
typedef struct
{
  int rl_pixel;
  int rl_basecode;
  int rl_count;
  int rl_table_pixel;
  int rl_table_max;
  bool just_cleared;
  int out_bits;
  int out_bits_init;
  int out_count;
  int out_bump;
  int out_bump_init;
  int out_clear;
  int out_clear_init;
  int max_ocodes;
  int code_clear;
  int code_eof;
  unsigned int obuf;
  int obits;
  FILE *ofile;
#ifdef LIBPLOTTER
  ostream *outstream;
#endif
  unsigned char oblock[256];
  int oblen;
} rle_out;

/* create, initialize, and return a new RLE output structure */
#ifdef LIBPLOTTER
extern rle_out *_rle_init (FILE *fp, ostream *out, int bit_depth);
#else
extern rle_out *_rle_init (FILE *fp, int bit_depth);
#endif
/* write a single integer (pixel) to the structure */
extern void _rle_do_pixel (rle_out *rle, int c);
/* wind things up and deallocate the RLE output structure */
extern void _rle_terminate (rle_out *rle);
