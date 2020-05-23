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

/* This file contains low-level functions used by CGMPlotters.  E.g.,
   _cgm_emit_command_header and _cgm_emit_command_terminator, which begin
   and end a CGM command.  A CGM output file, in either the binary or clear
   text encoding, is simply a sequence of CGM commands.

   Commands are usually written to the CGMPlotter's output buffer, in which
   the current page of graphics (i.e. "picture", in CGM jargon) is stored.
   An output buffer (a plOutbuf) is manipulated by the routines in
   g_outbuf.c.  It includes an array of char, which can grow.

   This file also contains _cgm_emit_integer, _cgm_emit_unsigned_integer,
   _cgm_emit_point, _cgm_emit_points, _cgm_emit_index, _cgm_emit_enum,
   _cgm_emit_color_component, and _cgm_emit_string, etc., routines, which
   write the parameters of the command (i.e., its `data') to the output
   buffer.  The caller invokes zero or more of these routines between a
   _cgm_emit_command_header .. _cgm_emit_command_terminator pair.

   There is support for specifying a non-default output buffer, i.e., one
   not associated with the CGMPlotter in the usual way.  That is useful for
   preparing the output file's header and trailer, and per-page headers.
   See c_defplot.c.

   If the binary CGM encoding is used, CGM's data partitioning scheme is
   used.  As a command and its arguments are emitted, variables that play a
   role in implementing the data partitioning scheme are updated via
   pointers.  These include the number of data bytes written, and the total
   number of bytes written as part of the command.  The caller should
   initialize these variables to zero at the beginning of the CGM command.

   There is support for turning off data partitioning.  _cgm_emit_integer()
   and the other commands for emitting command parameters support a
   `no_partitioning' flag argument.  This is useful because some CGM
   commands take a `structured data record' argument.  An SDR is
   essentially a string [a sequence of octets], which may be emitted by
   calling _cgm_emit_string(), like an ordinary string.  However, an SDR
   must first be formed by calling a sequence of zero or more such commands
   as _cgm_emit_integer() etc., with output to a plOutbuf (with data
   partitioning turned off, if the binary encoding is used). */

/* Note: in the binary encoding is used, we go to extremes to make the
   written-out CGM file portable.  E.g., we hand-craft a big-endian
   2's-complement representation (the CGM standard) for each integer or
   unsigned integer, and write each octet individually to the output buffer
   as an unsigned char or char.  We don't assume the system represents
   integers using 2's complement.  We do assume that casting an unsigned
   char to a char doesn't change the bit pattern.

   The number of octets used in the CGM representation of an integer or
   unsigned integer, CGM_BINARY_BYTES_PER_INTEGER, is set in extern.h.  It
   should NOT be greater than the number of octets used in the system
   representation of an unsigned int; see comment below.  On nearly all
   systems that GNU supports, this maximum value for
   CGM_BINARY_BYTES_PER_INTEGER is 4 (it is never 2).

   Many CGM files use CGM_BINARY_BYTES_PER_INTEGER = 2.  In some old,
   noncompliant CGM parsers this value is hard-coded, even though it
   shouldn't be.  So use higher values (e.g., 3 and 4) with caution.  The
   "CGM Handbook" says the use of 3, rather than 2 or 4, is very rare.  */

#include "sys-defines.h"
#include "extern.h"

/* In the binary encoding, if the data section, i.e., the list of
   parameters for the command, contains more than 30 bytes, it is written
   in partitioned format.  This is the maximum number of data bytes we
   place in each block of the partition.  Could be as large as 32767, but
   we keep it small to avoid a buffer overrun (see comment in g_outbuf.c).  */
#define CGM_BINARY_DATA_BYTES_PER_PARTITION 3000

/* How to recognize the beginning of a new block of the partition
   (*data_byte_count is the running count of emitted data bytes,
   initialized by the caller to zero, and updated throughout the CGM
   command). */
#define CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count) \
(((data_len) > 30) && ((*(data_byte_count)) % CGM_BINARY_DATA_BYTES_PER_PARTITION == 0))

/* forward references */
static void cgm_emit_partition_control_word (plOutbuf *outbuf, int data_len, const int *data_byte_count, int *byte_count);
static void double_to_ieee_single_precision (double d, unsigned char output[4]);
static void int_to_cgm_int (int n, unsigned char *cgm_int, int octets_per_cgm_int);
static void unsigned_int_to_cgm_unsigned_int (unsigned int n, unsigned char *cgm_unsigned_int, int octets_per_cgm_unsigned_int);


/* Write the header of a CGM command.  

   In the clear text encoding, a string (the `op code') is written.

   In the binary encoding, a 2-byte word is written: it specifies the CGM
   element class and element ID, and `data_len': the number of data bytes
   that the caller will write, by subsequently calling the functions that
   emit command arguments.

   `data_len' includes CGM_BINARY_BYTES_PER_INTEGER bytes for an integer,
   and twice that for a point; two bytes for an index or enumerative, and
   four bytes for a real.  For a string, the number of data bytes can be
   computed from the CGM_BINARY_BYTES_PER_STRING() macro.  The caller
   should initialize *byte_count to zero, and also *data_byte_count (the
   latter is updated by the argument-emitting functions).  */

void
_cgm_emit_command_header (plOutbuf *outbuf, int cgm_encoding, int element_class, int id, int data_len, int *byte_count, const char *op_code)
{
  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      {
	int temp;
	
	if (data_len > 30)
	  data_len = 31;  /* set all 5 bits; will partition the data */

	temp = (element_class & 017) << 4; /* 4 bits, shifted up by 4 */
	temp |= (id >> 3) & 017; /* top 4 of 7 bits, shifted down by 3 */
	outbuf->point[0] = (char)(unsigned char)temp;
	temp = (id & 0177) << 5; /* lower 3 of 7 bits, shifted up by 5 */
	temp |= (data_len & 037); /* 5 bits, not shifted */
	outbuf->point[1] = (char)(unsigned char)temp;
	_update_buffer_by_added_bytes (outbuf, 2);
	(*byte_count) += 2;
      }
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, "%s", op_code);
      _update_buffer (outbuf);
      break;
    }
}

/* In the binary encoding, this is called automatically at the beginning of
   each data partition, if a partitioned parameter list is used.  It writes
   a 2-byte big-endian control word that specifies how many data bytes the
   partition will contain.  It may set a continuation flag in the control
   word, to indicate that another data partition will follow. */

static void
cgm_emit_partition_control_word (plOutbuf *outbuf, int data_len, const int *data_byte_count, int *byte_count)
{
  int bytes_remaining = data_len - (*data_byte_count);
  int bytes_in_partition;
  unsigned int control_word;

  if (bytes_remaining > CGM_BINARY_DATA_BYTES_PER_PARTITION)
    {
      bytes_in_partition = CGM_BINARY_DATA_BYTES_PER_PARTITION;
      control_word = 1 << 15;	/* set continuation flag */
    }
  else
    {
      bytes_in_partition = bytes_remaining;
      control_word = 0;
    }
  control_word |= (unsigned int)bytes_in_partition;

  /* write control word, big-endian */
  outbuf->point[0] = (char)(unsigned char)((control_word >> 8) & 0377);
  outbuf->point[1] = (char)(unsigned char)(control_word & 0377);
  _update_buffer_by_added_bytes (outbuf, 2);
  (*byte_count) += 2;
}

/* Encode a (signed) integer in binary CGM format.  This is a big-endian
   2's complement format, with k=8*octets_per_cgm_int bits per integer.
   The signed integer is clamped to the range -(2^(k-1) - 1) .. (2^(k-1)-1)
   and split into octets, with attention paid to the sign bit.

   We do not assume the system representation of integers is a 2's
   complement format.  We do assume that the system uses at least k octets
   per unsigned int.

   The octets are returned in an array of unsigned chars.  Since any of our
   output buffers contains an array of char, we'll be assuming that the bit
   pattern of chars and unsigned chars is the same, so that we can cast
   unsigned chars to chars with impunity. */

static void
int_to_cgm_int (int n, unsigned char *cgm_int, int octets_per_cgm_int)
{
  int max_int, i;
  unsigned int u;
  bool negative = false;

  /* clamp integer; we assume here that the system uses at least
     octest_per_cgm_int octets per unsigned int, i.e. that the system
     precision is at least as great as the CGM precision */
  max_int = 0;
  for (i = 0; i < (8 * octets_per_cgm_int - 1); i++)
    max_int += (1 << i);

  if (n > max_int)
    n = max_int;
  else if (n < -max_int)
    n = -max_int;
  
  if (n < 0)
    {
      int temp;

      negative = true;
      temp = -(n + 1);
      u = (unsigned int)(max_int - temp); /* compute 2's complement */
    }
  else
    u = (unsigned int)n;
  
  for (i = 0; i < octets_per_cgm_int; i++)
    {
      unsigned char v;

      v = 0xff & (u >> (8 * ((octets_per_cgm_int - 1) - i)));
      if (i == 0 && negative)
	v |= 0x80;
      cgm_int[i] = v;
    }
}

/* similar to the preceding, but for unsigned ints rather than signed ints */

static void
unsigned_int_to_cgm_unsigned_int (unsigned int n, unsigned char *cgm_unsigned_int, int octets_per_cgm_unsigned_int)
{
  unsigned int max_unsigned_int;
  int i;

  /* clamp unsigned integer; we assume here that the system uses at least
     octets_per_cgm_unsigned_int octets per unsigned int, i.e. that the
     system precision is at least as great as the CGM precision */
  max_unsigned_int = 0;
  for (i = 0; i < (8 * octets_per_cgm_unsigned_int); i++)
    max_unsigned_int += (1 << i);

  if (n > max_unsigned_int)
    n = max_unsigned_int;
  
  for (i = 0; i < octets_per_cgm_unsigned_int; i++)
    {
      unsigned char v;

      v = 0xff & (n >> (8 * ((octets_per_cgm_unsigned_int - 1) - i)));
      cgm_unsigned_int[i] = v;
    }
}

/* Write a (signed) integer in CGM format.  In the binary encoding,
   CGM_BINARY_BYTES_PER_INTEGER bytes are written.  In CGM files the
   default value for that parameter (defined in extern.h) is 2, but it can
   be increased. */

void
_cgm_emit_integer (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cgm_int[CGM_BINARY_BYTES_PER_INTEGER];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      int_to_cgm_int (x, cgm_int, CGM_BINARY_BYTES_PER_INTEGER);
      for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %d", x);
      _update_buffer (outbuf);
      break;
    }
}

/* similar to the preceding, but writes an unsigned integer rather than a
   signed integer. */

void
_cgm_emit_unsigned_integer (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cgm_unsigned_int[CGM_BINARY_BYTES_PER_INTEGER];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      unsigned_int_to_cgm_unsigned_int (x, cgm_unsigned_int, CGM_BINARY_BYTES_PER_INTEGER);
      for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_unsigned_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %u", x);
      _update_buffer (outbuf);
      break;
    }
}

/* similar to the preceding, but writes an `8-bit' unsigned integer (an
   unsigned integer in the range 0.255) as a single byte.  */

void
_cgm_emit_unsigned_integer_8bit (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count)
{
  /* clamp to 0..255 */
  if (x > (unsigned int)255)
    x = (unsigned int)255;

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      if (no_partitioning == false
	  && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
      *(outbuf->point) = (char)(unsigned char)x;
      _update_buffer_by_added_bytes (outbuf, 1);
      (*data_byte_count)++;
      (*byte_count)++;
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %u", x);
      _update_buffer (outbuf);
      break;
    }
}

/* Write a point, i.e. a pair of (signed) integers, in CGM format.  In the
   binary encoding, 2 * CGM_BINARY_BYTES_PER_INTEGER bytes are written. */

void
_cgm_emit_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int y, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cgm_int[CGM_BINARY_BYTES_PER_INTEGER];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      int_to_cgm_int (x, cgm_int, CGM_BINARY_BYTES_PER_INTEGER);
      for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      int_to_cgm_int (y, cgm_int, CGM_BINARY_BYTES_PER_INTEGER);
      for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " (%d, %d)", x, y);
      _update_buffer (outbuf);
      break;
    }
}

/* Write a list of points, i.e. a list of pairs of (signed) integers, in
   CGM format.  */

void
_cgm_emit_points (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const int *x, const int *y, int npoints, int data_len, int *data_byte_count, int *byte_count)
{
  int i, j;
  unsigned char cgm_int[CGM_BINARY_BYTES_PER_INTEGER];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      for (j = 0; j < npoints; j++)
	{
	  int_to_cgm_int (x[j], cgm_int, CGM_BINARY_BYTES_PER_INTEGER);
	  for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	    {
	      if (no_partitioning == false
		  && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
		cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	      
	      *(outbuf->point) = (char)(cgm_int[i]);
	      _update_buffer_by_added_bytes (outbuf, 1);
	      (*data_byte_count)++;
	      (*byte_count)++;
	    }
	  int_to_cgm_int (y[j], cgm_int, CGM_BINARY_BYTES_PER_INTEGER);
	  for (i = 0; i < CGM_BINARY_BYTES_PER_INTEGER; i++)
	    {
	      if (no_partitioning == false
		  && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
		cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	      
	      *(outbuf->point) = (char)(cgm_int[i]);
	      _update_buffer_by_added_bytes (outbuf, 1);
	      (*data_byte_count)++;
	      (*byte_count)++;
	    }
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;
      
    case CGM_ENCODING_CLEAR_TEXT:
      for (i = 0; i < npoints; i++)
	{
	  sprintf (outbuf->point, " (%d, %d)", x[i], y[i]);
	  _update_buffer (outbuf);
	}
      break;
    }
}

/* Write an `enumerative', in CGM format.  In the binary encoding, 2 bytes
   are written.  This is just like _cgm_emit_integer, except that the
   precision is fixed at 16 bits.  In the clear text encoding, a text
   string is written. */

void
_cgm_emit_enum (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count, const char *text_string)
{
  int i;
  unsigned char cgm_int[2];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      int_to_cgm_int (x, cgm_int, 2);
      for (i = 0; i < 2; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %s", text_string);
      _update_buffer (outbuf);
      break;
    }
}

/* Write an `index' in CGM format.  In the binary encoding, 2 bytes are
   written.  This is just like _cgm_emit_integer, except that we fix the
   precision at 16 bits (this could be changed, but according to the "CGM
   Handbook", using any other index precision is very rare).

   In c_defplot.c, we use this routine also for writing 2-byte integers or
   VDC integers (necessary before we reset the integer and VDC integer
   precisions). */

void
_cgm_emit_index (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, int x, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cgm_int[2];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      int_to_cgm_int (x, cgm_int, 2);
      for (i = 0; i < 2; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %d", x);
      _update_buffer (outbuf);
      break;
    }
}

/* Write a `color component' in CGM format.  In the binary encoding,
   CGM_BINARY_BYTES_PER_COLOR_COMPONENT bytes are written.  Valid values
   for that parameter (set in extern.h) are 1, 2, 3, 4, but our code in
   c_color.c supports only 1 or 2, i.e. 24-bit color or 48-bit color. */

void
_cgm_emit_color_component (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, unsigned int x, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cgm_unsigned_int[CGM_BINARY_BYTES_PER_COLOR_COMPONENT];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      unsigned_int_to_cgm_unsigned_int (x, cgm_unsigned_int,
					 CGM_BINARY_BYTES_PER_COLOR_COMPONENT);
      for (i = 0; i < CGM_BINARY_BYTES_PER_COLOR_COMPONENT; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_unsigned_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %u", x);
      _update_buffer (outbuf);
      break;
    }
}

/* Write a real quantity.  In the binary encoding, the default CGM
   fixed-point format is used.  That is 32 bits, with 16 bits for integer
   part [including sign bit] and 16 for added fraction in range [0,1);
   numbers from -32767.0 to 32768.0- may be represented.  In the clear text
   encoding, a conventional representation is used. */

void
_cgm_emit_real_fixed_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count)
{
  int x_floor;
  unsigned int x_frac;
  int i;
  unsigned char cgm_int[2], cgm_unsigned_int[2];

  /* clamp to range [-32767.0,32767.0] */
  if (x < -32767.0)
    x = -32767.0;
  else if (x > 32767.0)
    x = 32767.0;

  x_floor = (x >= 0.0 ? (int)x : -1 - ((int)(-x)));
  x_frac = (unsigned int)(65536 * (x - x_floor));

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      int_to_cgm_int (x_floor, cgm_int, 2);
      for (i = 0; i < 2; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      unsigned_int_to_cgm_unsigned_int (x_frac, cgm_unsigned_int, 2);
      for (i = 0; i < 2; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  
	  *(outbuf->point) = (char)(cgm_unsigned_int[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      if (x != 0.0)
	sprintf (outbuf->point, " %.8f", x);
      else
	sprintf (outbuf->point, " 0.0");
      _update_buffer (outbuf);
      break;
    }
}

/* Express a real number (a C `double') in IEEE single precision format:
   32 bits, including 1 sign bit, 8 exponent bits, and 23 mantissa bits,
   split into 4 octets, i.e. bytes, in big-endian order.

   The octets are returned in an array of unsigned chars.  Since any of our
   output buffers contains an array of char, we'll be assuming that the bit
   pattern of chars and unsigned chars is the same, so that we can cast
   unsigned chars to chars with impunity. */

static void
double_to_ieee_single_precision (double d, unsigned char output[4])
{
  double min_magnitude, max_magnitude, tmp_power, max_power;
  bool got_a_bit;
  int i, j;
  int sign_bit;
  int mantissa_bits[23];	/* leading `1' omitted */
  int exponent_bits[8];
  int biased_exponent = 0;	/* usually 1..254, meaning 1-127..254-127 */
  int bits[256];		/* as indices, 1..254 are meaningful */
  int output_bits[32];
  
  /* compute min, max magnitudes we'll produce */

  /* minimum = 2^(1-127) = 2^(-126).  This is the minimum non-subnormalized
     IEEE single-precision floating point number. */
  min_magnitude = 1.0;
  for (i = 0; i < 127-1; i++)
    min_magnitude /= 2;

  /* maximum = 2^(255-127) [1.0 - 2^(-24)] = 2^128 - 2^104
             = 1.11111111111111111111111 * 2^(254-127)
             = 1.11111111111111111111111 * 2^127         
    This is the maximum IEEE single-precision floating point number. */
  tmp_power = 1.0;
  max_magnitude = 0.0;
  for (i = 0; i <= 254-127; i++)
    {    
      if (i >= 104)
	max_magnitude += tmp_power;
      tmp_power *= 2;
    }
  
  /* replace NaN by maximum positive value */
  if (d != d)
    d = max_magnitude;
  
  /* extract sign bit */
  if (d < 0.0)
    {
      sign_bit = 1;
      d = -d;
    }
  else
    sign_bit = 0;

  /* if nonzero, clamp to allowed range */
  if (d != 0.0 && d < min_magnitude)
    d = min_magnitude;
  else if (d > max_magnitude)
    d = max_magnitude;
  
  /* compute max power of two that can occur in binary expansion,
     i.e. 2^(254-127) = 2^127 */
  max_power = 1.0;
  for (i = 0; i < 254-127; i++)
    max_power *= 2;

  /* compute bits array; location of first `1' will be biased exponent */
  for (i = 0; i < 256; i++)
    bits[i] = 0;
  got_a_bit = false;
  for (i = 254, tmp_power = max_power; i >= 1; i--, tmp_power /= 2)
    if (d >= tmp_power)
      {
	if (got_a_bit == false)
	  {
	    biased_exponent = i; /* will be in range 1..254, if set */
	    got_a_bit = true;
	  }
	bits[i] = 1;
	d -= tmp_power;
      }
  if (got_a_bit == false)
    /* d = 0.0, use bogus value for biased exponent */
    biased_exponent = 0;
  
  /* extract mantissa bits: in bits array, they start after first `1' */
  for (j = 0; j < 23; j++)
    mantissa_bits[j] = 0;
  if (got_a_bit == true)
    for (i = biased_exponent - 1, j = 0; i >= 1 && j < 23; i--, j++)
      mantissa_bits[j] = bits[i];
  
  /* extract exponent bits; exponent is in range 0..254 */
  for (j = 7; j >= 0; j--)
    {
      exponent_bits[j] = biased_exponent % 2;
      biased_exponent /= 2;
    }

  /* construct output array of 32 bits */
  output_bits[0] = sign_bit;
  for (j = 0; j < 8; j++)
    output_bits[j + 1] = exponent_bits[j];
  for (j = 0; j < 23; j++)
    output_bits[j + 9] = mantissa_bits[j];
  
  for (j = 0; j < 4; j++)
    output[j] = (unsigned char)0;
  for (j = 0; j < 32; j++)
    if (output_bits[j] == 1)
      output[j / 8] |= (1 << ((31 - j) % 8));
}

/* Write a real quantity.  Like the _cgm_emit_real_fixed_point, but in the
   binary encoding, rather than a fixed-point format, a floating-point
   format is used.  In particular, IEEE single-precision format, occupying
   32 bits; split into octets in big-endian order.

   A CGMPlotter calls this function only to write a mandatory `scaling
   factor' that is probably bogus.  See c_defplot.c. */

void
_cgm_emit_real_floating_point (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, double x, int data_len, int *data_byte_count, int *byte_count)
{
  int i;
  unsigned char cp[4];

  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      double_to_ieee_single_precision (x, cp);
      for (i = 0; i < 4; i++)
	{
	  if (no_partitioning == false
	      && CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	    cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	  *(outbuf->point) = (char)(cp[i]);
	  _update_buffer_by_added_bytes (outbuf, 1);
	  (*data_byte_count)++;
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      sprintf (outbuf->point, " %.8f", x);
      _update_buffer (outbuf);
      break;
    }
}

/* Write a string, in CGM format.  

   In the binary encoding, string encoding depends on string length.  (1)
   If length <= 254 bytes, the length is prepended to the string, as a
   single byte.  (2) If length >= 255 bytes, the encoding begins with a
   byte equal to 255.  Then there is a sixteen-bit word containing a length
   (up to 32767) and a continuation flag, followed by data bytes; the two
   of them constitute a `string partition', which may be repeated
   arbitrarily many times.  We use at most CGM_STRING_PARTITION_SIZE data
   bytes in a partition, rather than 32767, to avoid buffer overrun; see
   comment above.  The total byte length of the encoded string, if
   string_length=original length, equals
   CGM_BINARY_BYTES_PER_STRING(string_length).  This macro is defined in
   extern.h.

   In the clear text encoding, we surround the string by quotes, and escape
   any quote that it contains by doubling it.  We use single quotes unless
   the `use_double_quotes' flag is set. */

void
_cgm_emit_string (plOutbuf *outbuf, bool no_partitioning, int cgm_encoding, const char *s, int string_length, bool use_double_quotes, int data_len, int *data_byte_count, int *byte_count)
{
  int i, encoded_string_length;
  const char *sp = s;
  char *t, *tp, c;
	
  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      {
#if 0
	fprintf (stderr, "cgm_emit_string(), length=%d\n", string_length);
	for (i = 0; i < string_length; i++)
	  putc (s[i], stderr);
	putc ('\n', stderr);
#endif
	
	/* first, encode the string */

	encoded_string_length = CGM_BINARY_BYTES_PER_STRING(string_length);
	tp = t = (char *)_pl_xmalloc (encoded_string_length * sizeof(char));

	if (string_length <= 254)
	  {
	    /* begin with `count' byte, follow by original string */
	    *tp++ = (char)(unsigned char)string_length;
	    for (i = 0; i < string_length; i++)
	      *tp++ = *sp++;
	  }
	else
	  {
	    /* first byte is `255' */
	    *tp++ = (char)255;

	    /* copy data bytes, with string partition headers interpolated
	       as needed; `i' counts data bytes copied */
	    for (i = 0; i < string_length; i++, sp++)
	      {
		if (i % CGM_STRING_PARTITION_SIZE == 0)
		  /* write two-byte string partition header */
		  {
		    int bytes_remaining = string_length - i;
		    int string_header_word;

		    if (bytes_remaining <= CGM_STRING_PARTITION_SIZE)
		      string_header_word = bytes_remaining;
		    else
		      /* must continue; set continuation flag */
		      {
			string_header_word = (1 << 15);
			string_header_word |= CGM_STRING_PARTITION_SIZE;
		      }
		    /* write string partition header word, big-endian */
		    *tp++ = (char)((string_header_word >> 8) & 0377);
		    *tp++ = (char)(string_header_word & 0377);
		  }

		  /* copy byte */
		  *tp++ = *sp;
	      }
	  }

	/* copy encoded string to output buffer; it may require more than
	   one data partition */
	for (i = 0; i < encoded_string_length; i++)
	  {
	    if (no_partitioning == false
		&& CGM_BINARY_DATA_PARTITION_BEGINS(data_len, data_byte_count))
	      cgm_emit_partition_control_word (outbuf, data_len, data_byte_count, byte_count);
	    *(outbuf->point) = t[i];
	    _update_buffer_by_added_bytes (outbuf, 1);
	    (*data_byte_count)++;
	    (*byte_count)++;
	  }
	
	/* free encoded string */
	free (t);
      }
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      {
	/* allocate space for encoded string, including initial and final
           quotes, a space for readability, and a final NULL */
	encoded_string_length = 2 * string_length + 3;
	tp = t = (char *)_pl_xmalloc ((encoded_string_length + 1) * sizeof(char));

	/* begin with a space for readability, and a quote */
	*tp++ = ' ';
	*tp++ = (use_double_quotes ? '"' : '\'');
	while ((c = *sp++) != '\0')
	  {
	    /* escape all quotes by doubling them */
	    if (((use_double_quotes == true) && c == '"')
		|| ((use_double_quotes == false) && c == '\''))
	      *tp++ = c;
	    *tp++ = c;
	  }
	/* end with a quote */
	*tp++ = (use_double_quotes ? '"' : '\'');
	*tp++ = '\0';

	strcpy (outbuf->point, t);
	_update_buffer (outbuf);
	free (t);
      }
      break;
    }
}

/* Write the terminator of a CGM command.  In the binary encoding this
   writes a single null if and only if the number of bytes previously
   written (kept track of via the `byte_count' pointer) is odd; otherwise
   it does nothing.  In the clear text encoding it writes ";\n". */

void
_cgm_emit_command_terminator (plOutbuf *outbuf, int cgm_encoding, int *byte_count)
{
  switch (cgm_encoding)
    {
    case CGM_ENCODING_BINARY:
    default:
      if ((*byte_count) % 2 == 1)
	{
	  *(outbuf->point) = '\0';
	  _update_buffer_by_added_bytes (outbuf, 1);	  
	  (*byte_count)++;
	}
      break;

    case CGM_ENCODING_CHARACTER: /* not supported */
      break;

    case CGM_ENCODING_CLEAR_TEXT:
      strcpy (outbuf->point, ";\n");
      _update_buffer (outbuf);
      break;
    }
}
