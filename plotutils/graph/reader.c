/* This file is part of the GNU plotutils package.  Copyright (C) 1989,
   1990, 1991, 1995, 1996, 1997, 1998, 1999, 2000, 2005, 2008, Free
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

/* This file is the point-reader half of GNU graph.  Included here are
   routines that will read data points from an input stream.  The input
   file may be in ascii format (a sequence of floating-point numbers,
   separated by whitespace), or in binary format (e.g., a sequence of
   doubles).  Gnuplot table format is also supported.

   A `point' is a structure.  Each point structure contains the following
   fields:

      x and y coordinates of the point
      a `have_x_errorbar' flag (true or false)
      a `have_y_errorbar' flag (true or false)
      xmin and xmax (meaningful only if have_x_errorbar is set)
      ymin and ymax (meaningful only if have_y_errorbar is set)
      a `pendown' flag

      a symbol type (a small integer, interpreted as a marker type)
      a symbol size (a fraction of the size of the plotting area)
      a symbol font name (relevant only for symbol types >= 32, see plotter.c)
      a linemode (a small integer)
      a linewidth (a fraction of the size of the display device)
      a polyline fill-fraction (in the interval [0,1], <0 means no fill)
      a use_color flag (true or false)

   An array of points defines a polyline, or a sequence of polylines.
   pendown=true means that a polyline is being drawn; pendown=false means
   that a polyline has just ended, and that the point (x,y), which begins a
   new polyline, should be moved to without drawing a line segment.  By
   convention, the final seven fields, and have_?_errorbar, are the same
   for each point in a polyline.  We use the term `dataset' to refer to the
   sequence of points in an input file that gives rise to a single
   polyline.

   If the input stream is in ascii format, two \n's in succession serves as
   a separator between datasets.  If, instead, the input stream is in
   binary format (e.g., a sequence of doubles), then a single infinite
   quantity, such as DBL_MAX, serves as a dataset separator.  If the input
   stream is in gnuplot `table' format, then two \n's in succession serves
   as a separator.  But there are always two \n's before EOF in gnuplot
   format; this is different from ascii format.

   This file exports four functions (point-reader methods, basically).
   They are declared in extern.h.  They are:

   	new_reader ()
	read_file ()
	alter_reader_parameters ()
	delete_reader ()

   Before any points are read, the point-reader must first be created and
   initialized by a call to the constructor new_reader().  It takes as
   arguments certain reader parameters: e.g., the initial linemode and
   symbol type, the input stream, and the format of the input stream.
   read_file() reads datasets in succession from the input stream, until
   EOF is reached.  After each dataset, besides breaking the polyline, it
   increments the linemode by unity (provided the reader's `autobump' flag
   is set).  After finishing with an input stream, and before switching to
   another input stream as a source of points, alter_reader_parameters()
   should be called.  It allows parameters of the reader that may differ
   from stream to stream to be changed.

   Directives in the input stream, specifying a change of linemode / symbol
   type, are supported.  Any such directive automatically terminates a
   dataset and begins a new one.  This is in agreement with the convention
   that every point in a polyline has the same linemode and the same
   plotting symbol.  During the reading of an ascii-format input stream, a
   string of the form "#m=%d,S=%d" will be interpreted as a directive to
   change to a specified linemode / symbol type.  Here the two %d's are the
   new linemode and symbol type, respectively.  There is currently no way
   of changing to a specific linemode / symbol type during the reading of a
   input stream that is in binary format.

   The function `read_and_plot_file' is also exported.  It is the same as
   `read_file', but it uses the plot_point() method of a Multigrapher (see
   plotter.c) to plot each point as it is read. */

#include "sys-defines.h"
#include "libcommon.h"
#include "extern.h"

/* New (larger) length of a Point array, as function of the old; used when
   reallocating due to exhaustion of storage. */
#define NEW_POINT_ARRAY_LENGTH(old_len) \
((old_len)*sizeof(Point) < 10000000 ? 2 * (old_len) : (old_len) + 10000000/sizeof(Point))

struct ReaderStruct		/* point reader datatype */
{
/* parameters which are constant over the lifetime of a Reader, and which
   affect the computation of the returned point structures */
  bool transpose_axes;		/* x <-> y ? */
  int log_axis;			/* x,y axes are logarithmic? (portmanteau) */
/* Reader parameters that are constant for the duration of each file  */
  FILE *input;			/* input stream */
  data_type format_type;	/* stream format (T_ASCII, T_DOUBLE, etc.) */
  bool auto_abscissa;		/* auto-generate x values?  */
  double delta_x;		/* increment for x value, if auto-generated */
  double initial_abscissa;	/* initial value for x, if auto-generated */
  bool auto_bump;		/* bump linemode when starting next polyline?*/
/* Reader parameters that are constant for the duration of each dataset */
  int symbol;			/* symbol type */
  double symbol_size;		/* symbol size (in `box coordinates') */
  const char *symbol_font_name;	/* font used for marker types >= 32 */
  int linemode;			/* linemode */
  double line_width;		/* line width (as frac. of display size) */
  double fill_fraction;		/* in interval [0,1], <0 means no filling */
  bool use_color;		/* color/monochrome interp. of linemode */
/* state variables, updated during Reader operation */
  bool need_break;		/* draw next point with pen up ? */
  double abscissa;		/* x value, if auto-generated */
};

/* Internal status codes: return values for read_dataset() and
   read_and_plot_dataset(), and also for the lower-level functions
   read_point() and read_point_{ascii,ascii_errorbar,binary,gnuplot}().

   IN_PROGRESS means the dataset being read is still in progress.
   ENDED_BY_EOF means it terminated with an EOF, and
   ENDED_BY_DATASET_TERMINATOR means it terminated with an explicit
   end-of-dataset marker.  An end-of-dataset marker is two newlines in
   succession for an ascii stream, and a DBL_MAX for a stream of doubles.
   ENDED_BY_MODE_CHANGE signals that a `set linemode / symbol type'
   directive was seen.  By convention, we interpret such a directive as
   ending a dataset.  */

typedef enum { IN_PROGRESS, ENDED_BY_EOF, ENDED_BY_DATASET_TERMINATOR, ENDED_BY_MODE_CHANGE } dataset_status_t;

/* forward references */
static bool skip_some_whitespace (FILE *stream);
static dataset_status_t read_and_plot_dataset (Reader *reader, Multigrapher *multigrapher);
static dataset_status_t read_dataset (Reader *reader, Point **p, int *length, int *no_of_points);
static dataset_status_t read_point (Reader *reader, Point *point);
static dataset_status_t read_point_ascii (Reader *reader, Point *point);
static dataset_status_t read_point_ascii_errorbar (Reader *reader, Point *point);
static dataset_status_t read_point_binary (Reader *reader, Point *point);
static dataset_status_t read_point_gnuplot (Reader *reader, Point *point);
static void reset_reader (Reader *reader);
static void skip_all_whitespace (FILE *stream);

/* ARGS: format_type = double, or ascii, etc.
   	 symbol_size = symbol size for markers
	 symbol_font_name = name for markers >= 32
	 line_width = fraction of display size
	 fill_fraction = number in range [0,1], <0 means unfilled (transparent)*/
Reader *
new_reader (FILE *input, data_type format_type, bool auto_abscissa, double delta_x, double abscissa, bool transpose_axes, int log_axis, bool auto_bump, int symbol, double symbol_size, const char *symbol_font_name, int linemode, double line_width, double fill_fraction, bool use_color)

{
  Reader *reader;

  reader = (Reader *)xmalloc (sizeof (Reader));

  reader->need_break = true;	/* next point will have pen up */
  reader->input = input;
  reader->format_type = format_type;
  reader->auto_abscissa = auto_abscissa;
  reader->delta_x = delta_x;
  reader->initial_abscissa = abscissa;
  reader->abscissa = reader->initial_abscissa;
  reader->transpose_axes = transpose_axes;
  reader->log_axis = log_axis;
  reader->auto_bump = auto_bump;
  reader->symbol = symbol;
  reader->symbol_size = symbol_size;
  reader->symbol_font_name = symbol_font_name;
  reader->linemode = linemode;
  reader->line_width = line_width;
  reader->fill_fraction = fill_fraction;
  reader->use_color = use_color;

  return reader;
}

void
delete_reader (Reader *reader)
{
  free (reader);
  return;
}

/* alter_reader_parameters() would be called just before reading datapoints
   from the second stream, the third stream,...  It breaks the polyline
   under construction, if any.  It also sets the stream and stream format,
   resets the abscissa (if auto-abscissa is in effect), and updates the
   linemode, symbol type, etc., if requested.  (In GNU graph, we use the
   last feature to permit command-line specification of linemode/symbol
   type on a per-file basis.)  */

/* ARGS: note that the final new_* args make up a mask */
void 
alter_reader_parameters (Reader *reader, FILE *input, data_type format_type, bool auto_abscissa, double delta_x, double abscissa, int symbol, double symbol_size, const char *symbol_font_name, int linemode, double line_width, double fill_fraction, bool use_color, bool new_symbol, bool new_symbol_size, bool new_symbol_font_name, bool new_linemode, bool new_line_width, bool new_fill_fraction, bool new_use_color)
{
  reader->need_break = true;	/* force break in polyline */
  reader->input = input;
  reader->format_type = format_type;
  reader->auto_abscissa = auto_abscissa;
  reader->delta_x = delta_x;
  reader->initial_abscissa = abscissa;
  reader->abscissa = reader->initial_abscissa;
  /* test bits in mask to determine which polyline attributes need updating */
  if (new_symbol)
    reader->symbol = symbol;
  if (new_symbol_size)
    reader->symbol_size = symbol_size;
  if (new_symbol_font_name)
    reader->symbol_font_name = symbol_font_name;
  if (new_linemode)
    reader->linemode = linemode;
  if (new_line_width)
    reader->line_width = line_width;
  if (new_fill_fraction)
    reader->fill_fraction = fill_fraction;
  if (new_use_color)
    reader->use_color = use_color;

  return;
}

/* read_point() calls read_point_ascii(), read_point_ascii_errorbar(),
   read_point_binary(), or read_point_gnuplot() to do the actual reading.
   It returns a status code (either IN_PROGRESS or ENDED_*, describing how
   the dataset in progress ended, if it did). */

static dataset_status_t
read_point (Reader *reader, Point *point)
{
  dataset_status_t status;

  /* following fields are constant throughout each polyline */
  point->symbol = reader->symbol;
  point->symbol_size = reader->symbol_size;
  point->symbol_font_name = reader->symbol_font_name;
  point->linemode = reader->linemode;
  point->line_width = reader->line_width;
  point->fill_fraction = reader->fill_fraction;
  point->use_color = reader->use_color;
  point->have_x_errorbar = false; /* not supported yet */
  point->have_y_errorbar = (reader->format_type == T_ASCII_ERRORBAR ? true : false);
  
 head:

  switch (reader->format_type)
    {
    case T_ASCII:
    default:
      status = read_point_ascii (reader, point);
      break;
    case T_SINGLE:
    case T_DOUBLE:
    case T_INTEGER:
      status = read_point_binary (reader, point);
      break;
    case T_ASCII_ERRORBAR:
      status = read_point_ascii_errorbar (reader, point);
      break;
    case T_GNUPLOT:		/* gnuplot `table' format */
      status = read_point_gnuplot (reader, point);
      break;
    }

  if (status == IN_PROGRESS)
    /* got a point; if not, we just pass back the return code */
    {
      bool bad_point = false;

      /* If we have log axes, the values we work with ALL refer to the log10
	 values of the data.  A nonpositive value generates a warning, and a
	 break in the polyline. */
      if (reader->log_axis & X_AXIS)
	{
	  if (point->x > 0.0)
	    point->x = log10 (point->x);
	  else
	    bad_point = true;
	  if (point->have_x_errorbar)
	    {
	      if (point->xmin > 0.0)
		point->xmin = log10 (point->xmin);
	      else
		bad_point = true;
	      if (point->xmax > 0.0)
		point->xmax = log10 (point->xmax);
	      else
		bad_point = true;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: the inappropriate point (%g,%g) is dropped, as this is a log plot\n",
		       progname, point->x, point->y);
	      reader->need_break = true;
	      goto head;		/* on to next point */
	    }
	}
      if (reader->log_axis & Y_AXIS)
	{
	  if (point->y > 0.0)
	    point->y = log10 (point->y);
	  else
	    bad_point = true;

	  if (point->have_y_errorbar)
	    {
	      if (point->ymin > 0.0)
		point->ymin = log10 (point->ymin);
	      else
		bad_point = true;
	      if (point->ymax > 0.0)
		point->ymax = log10 (point->ymax);
	      else
		bad_point = true;
	    }
	  
	  if (bad_point)
	    {
	      fprintf (stderr, "%s: the inappropriate point (%g,%g) is dropped, as this is a log plot\n",
		       progname, point->x, point->y);
	      reader->need_break = true;
	      goto head;		/* on to next point */
	    }
	}
      
      if (reader->transpose_axes)
	{
	  double tmp;
	  bool tmp_bool;
	  
	  tmp = point->x;
	  point->x = point->y;
	  point->y = tmp;
	  tmp = point->xmin;
	  point->xmin = point->ymin;
	  point->ymin = tmp;
	  tmp = point->xmax;
	  point->xmax = point->ymax;
	  point->ymax = tmp;
	  tmp_bool = point->have_x_errorbar;
	  point->have_x_errorbar = point->have_y_errorbar;
	  point->have_y_errorbar = tmp_bool;
	}
      
      /* we have a point, but we may need to break the polyline before it */
      if (reader->need_break)
	point->pendown = false;
      else
	point->pendown = true;
      
      /* reset break-polyline flag */
      reader->need_break = false;
    }

  return status;
}

static dataset_status_t
read_point_ascii (Reader *reader, Point *point)
{
  int items_read, lookahead;
  bool two_newlines;
  FILE *input = reader->input;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_some_whitespace (input);
  if (two_newlines)
    return ENDED_BY_DATASET_TERMINATOR;
  if (feof (input))
    return ENDED_BY_EOF;

  /* process linemode / symbol type directive */
  lookahead = getc (input);
  ungetc (lookahead, input);
  if (lookahead == (int)'#')
    {
      int new_symbol, new_linemode;
      int items_read;
      
      items_read = fscanf (input, 
			   "# m = %d, S = %d", &new_linemode, &new_symbol);
      if (items_read == 2)	/* insist on matching both */
	{
	  reader->linemode = new_linemode;
	  reader->symbol = new_symbol;
	  return ENDED_BY_MODE_CHANGE;
	}
      else			/* unknown comment line, ignore it */
	{
	  char c;
	  
	  do 
	    {
	      items_read = fread (&c, sizeof (c), 1, input);
	      if (items_read <= 0)
		return ENDED_BY_EOF;
	    }
	  while (c != '\n');
	  ungetc ((int)'\n', input); /* push back \n at the end of # line */
	  goto head;
	}
    }

  /* read coordinate(s) */
  if (reader->auto_abscissa)
    {
      point->x = reader->abscissa;
      reader->abscissa += reader->delta_x;
    }
  else
    {
      items_read = fscanf (input, "%lf", &(point->x));
      if (items_read != 1)
	return ENDED_BY_EOF; /* presumably */
    }

  items_read = fscanf (input, "%lf", &(point->y));
  if (items_read == 1)
    return IN_PROGRESS;	/* got a pair of floats */
  else 
    {
      if (!reader->auto_abscissa)
	fprintf (stderr, "%s: an input file terminated prematurely\n", progname);
      return ENDED_BY_EOF;	/* couldn't get y coor, effectively EOF */
    }
}

static dataset_status_t
read_point_ascii_errorbar (Reader *reader, Point *point)
{
  int items_read, lookahead;
  bool two_newlines;
  double error_size;
  FILE *input = reader->input;

 head:

  /* skip whitespace, up to but not including 2nd newline if any */
  two_newlines = skip_some_whitespace (input);
  if (two_newlines)
    return ENDED_BY_DATASET_TERMINATOR;
  if (feof (input))
    return ENDED_BY_EOF;

  /* process linemode / symbol type directive */
  lookahead = getc (input);
  ungetc (lookahead, input);
  if (lookahead == (int)'#')
    {
      int new_symbol, new_linemode;
      int items_read;
      
      items_read = fscanf (input, 
			   "# m = %d, S = %d", &new_linemode, &new_symbol);
      if (items_read == 2)	/* insist on matching both */
	{
	  reader->linemode = new_linemode;
	  reader->symbol = new_symbol;
	  return ENDED_BY_MODE_CHANGE;
	}
      else			/* unknown comment line, ignore it */
	{
	  char c;
	  
	  do 
	    {
	      items_read = fread (&c, sizeof (c), 1, input);
	      if (items_read <= 0)
		return ENDED_BY_EOF;
	    }
	  while (c != '\n');
	  ungetc ((int)'\n', input); /* push back \n at the end of # line */
	  goto head;
	}
    }

  /* read coordinate(s) */
  if (reader->auto_abscissa)
    {
      point->x = reader->abscissa;
      reader->abscissa += reader->delta_x;
    }
  else
    {
      items_read = fscanf (input, "%lf", &(point->x));
      if (items_read != 1)
	return ENDED_BY_EOF; /* presumably */
    }

  items_read = fscanf (input, "%lf", &(point->y));
  if (items_read != 1)
    {
      if (!reader->auto_abscissa)
	fprintf (stderr, "%s: an input file (in errorbar format) terminated prematurely\n", progname);
      return ENDED_BY_EOF;	/* couldn't get y coor, effectively EOF */
    }

  items_read = fscanf (input, "%lf", &error_size);
  if (items_read != 1)
    {
      fprintf (stderr, "%s: an input file (in errorbar format) terminated prematurely\n", progname);
      return ENDED_BY_EOF;	/* couldn't get y coor, effectively EOF */
    }

  point->ymin = point->y - error_size;
  point->ymax = point->y + error_size;

  /* don't support reading of x errorbars yet */
  point->xmin = 0.0;
  point->xmax = 0.0;

  return IN_PROGRESS;
}

static dataset_status_t
read_point_binary (Reader *reader, Point *point)
{
  int items_read;
  data_type format_type = reader->format_type;
  FILE *input = reader->input;
  
  /* read coordinate(s) */
  if (reader->auto_abscissa)
    {
      point->x = reader->abscissa;
      reader->abscissa += reader->delta_x;
    }
  else
    {
      switch (format_type)
	{
	case T_DOUBLE:
	default:
	  items_read = 
	    fread ((void *) &(point->x), sizeof (double), 1, input);
	  break;
	case T_SINGLE:
	  {
	    float fx;
	    
	    items_read = 
	      fread ((void *) &fx, sizeof (fx), 1, input);
	    point->x = fx;
	  }
	  break;
	case T_INTEGER:
	  {
	    int ix;
	    
	    items_read = 
	      fread ((void *) &ix, sizeof (ix), 1, input);
	    point->x = ix;
	  }
	  break;
	}
      if (items_read <= 0)
	return ENDED_BY_EOF; /* presumably */
    }

  if ((format_type == T_DOUBLE && point->x == DBL_MAX)
      || (format_type == T_SINGLE && point->x == (double)FLT_MAX)
      || (format_type == T_INTEGER && point->x == (double)INT_MAX))
    return ENDED_BY_DATASET_TERMINATOR;

  switch (format_type)
    {
    case T_DOUBLE:
    default:
      items_read = 
	fread ((void *) &(point->y), sizeof (double), 1, input);
      break;
    case T_SINGLE:
      {
	float fy;
	
	items_read = 
	  fread ((void *) &fy, sizeof (fy), 1, input);
	point->y = fy;
      }
      break;
    case T_INTEGER:
      {
	int iy;
	
	items_read = 
	  fread ((void *) &iy, sizeof (iy), 1, input);
	point->y = iy;
      }
      break;
    }

  if (items_read != 1)		/* didn't get a pair of floats */
    {
      if (!reader->auto_abscissa)
	fprintf (stderr, "%s: an input file (in binary format) terminated prematurely\n", progname);
      return ENDED_BY_EOF;	/* effectively */
    }
  else if (point->x != point->x || point->y != point->y)
    {
      fprintf (stderr, "%s: a NaN (not-a-number) was encountered in a binary input file\n",
	       progname);
      return ENDED_BY_EOF;	/* effectively */
    }
  else
    return IN_PROGRESS;	/* got a pair of floats */
}

/* Read a point from a file in gnuplot `table' format.  There are two kinds
   of table format we can read: the old style (from early gnuplot 3.5 and
   before) and a more modern style (from later gnuplot 3.5, circa 1997). */

static dataset_status_t
read_point_gnuplot (Reader *reader, Point *point)
{
  int lookahead, items_read;
  char directive, c;
  bool two_newlines;
  double x, y;
  FILE *input = reader->input;
  
 head:
  
  /* skip whitespace, up to but not including 2nd newline */
  two_newlines = skip_some_whitespace (input);
  if (two_newlines)
    /* end of dataset */
    {
      skip_all_whitespace (input);
      if (feof (input))
	return ENDED_BY_EOF;	/* no dataset follows */
      else
	return ENDED_BY_DATASET_TERMINATOR; /* dataset presumably follows */
    }

  lookahead = getc (input);
  ungetc (lookahead, input);
  switch (lookahead)
    {
    case 'C':			/* old-style `Curve' line, discard it */
    case '#':			/* modern-style comment line, discard it */
      do 
	{
	  items_read = fread (&c, sizeof (c), 1, input);
	  if (items_read <= 0)
	    return ENDED_BY_EOF; /* effectively */
	}
      while (c != '\n');
      ungetc ((int)'\n', input); /* push back \n at the end of line */
      goto head;

    case 'i':		    /* old-style directive-first line (in-range) */
    case 'o':		    /* old-style directive-first line (out-of-range) */
      /* read coordinates */
      items_read = fscanf (input, 
			   "%c x=%lf y=%lf", 
			   &directive, &x, &y);
      if (items_read == 3)	/* must match all */
	{
	  point->x = x;
	  point->y = y;
	  return IN_PROGRESS; /* got a pair of floats */
	}
      else
	{
	  fprintf (stderr, 
		   "%s: an input file in gnuplot format could not be parsed\n", 
		   progname);
	  return ENDED_BY_EOF; /* effectively */
	}

    case 'u':			/* old-style directive-first line */
      /* `undefined', next point begins new polyline (same line mode) */
      do 
	{
	  items_read = fread (&c, sizeof (c), 1, input);
	  if (items_read <= 0)
	    {
	      fprintf (stderr, 
		       "%s: an input file in gnuplot format could not be parsed\n", 
		       progname);
	    return ENDED_BY_EOF; /* effectively */
	    }
	}
      while (c != '\n');
      /* break the polyline here in a soft way (i.e. don't bump line mode) */
      reader->need_break = true;	
      goto head;

    default:			/* parse as a new-style directive-last line */
      items_read = fscanf (input, 
			   "%lf %lf %c", 
			   &x, &y, &directive);
      if (items_read == 3 
	  && (directive == 'i' || directive == 'o' || directive == 'u'))
	{
	  if (directive == 'u')
	    {
	      /* drop point; break the polyline here in a soft way
                 (i.e. don't bump line mode) */
	      reader->need_break = true;	
	      goto head;
	    }
	  else
	    {
	      point->x = x;
	      point->y = y;
	      return IN_PROGRESS;
	    }
	}
      else
	{
	  fprintf (stderr, 
		   "%s: an input file in gnuplot format could not be parsed\n", 
		   progname);
	  return ENDED_BY_EOF; /* effectively */
	}
    }
}


/* read_dataset() reads an entire dataset (a sequence of points) from an
   input file, and stores the resulting array of points in a block that has
   been allocated on the heap.  The length of the block in which the points
   are stored, and the number of points, are passed back.  */

/* ARGS: length = buffer length in bytes, should begin >0 */
static dataset_status_t
read_dataset (Reader *reader, Point **p_addr, int *length, int *no_of_points)
{
  Point *p = *p_addr;
  dataset_status_t status;

  for ( ; ; )
    {
      /*
       * Grow the buffer if needed
       */
      if (*no_of_points >= *length)
	{
	  int old_length, new_length;
	  
	  old_length = *length;
	  new_length = NEW_POINT_ARRAY_LENGTH(old_length);
	  p = (Point *)xrealloc (p, new_length * sizeof (Point));
	  *length = new_length;
	}

      status = read_point (reader, &(p[*no_of_points]));
      if (status != IN_PROGRESS)
	/* we didn't get a point, i.e. dataset ended */
	break;

      (*no_of_points)++;
    }

  *p_addr = p;			/* update beginning of array if needed */

  return status;
}

/* read_file() reads all datasets from an input file, and stores the
   resulting array of points in a block that has been allocated on the
   heap.  The length of the block in which the data points are stored, and
   the number of points, are passed back.  */

/* ARGS: length = buffer length in bytes, should begin >0 */
void
read_file (Reader *reader, Point **p_addr, int *length, int *no_of_points)
{
  dataset_status_t status;

  do
    {
      status = read_dataset (reader, p_addr, length, no_of_points);

      /* After each dataset, reset reader: force break in polyline, bump
	 linemode (if auto-bump is in effect), and reset abscissa (if
	 auto-abscissa is in effect).  But if dataset ended with an
	 explicit set linemode / symbol style directive, don't bump the
	 linemode. */
      if (status == ENDED_BY_MODE_CHANGE)
	{
	  bool saved_auto_bump;

	  saved_auto_bump = reader->auto_bump;
	  reader->auto_bump = false;
	  reset_reader (reader);
	  reader->auto_bump = saved_auto_bump;
	}
      else
	reset_reader (reader);
    }
  while (status != ENDED_BY_EOF);
}

/* reset_reader() is called after each dataset.  A new polyline will be
   begun, the linemode will be bumped if auto-bumping is in effect, and the
   abscissa will be reset if auto-abscissa is in effect. */

static void
reset_reader (Reader *reader)
{
  reader->need_break = true;	/* force break in polyline */

  /* bump linemode if appropriate */
  if (reader->auto_bump)
    reader->linemode += ((reader->linemode > 0) ? 1 : -1);

  /* reset abscissa if auto-abscissa is in effect */
  if (reader->auto_abscissa)
    reader->abscissa = reader->initial_abscissa;

  return;
}


/* Skip whitespace in an ascii-format or gnuplot-format input file, up to
   but not including a second newline.  Return value indicates whether or
   not two newlines were in fact seen.  (Two newlines signals
   end-of-dataset.) */

static bool
skip_some_whitespace (FILE *stream)
{
  int lookahead;
  int nlcount = 0;
  
  do 
    {
      lookahead = getc (stream);
      if (lookahead == (int)'\n')
	  nlcount++;
    }
  while (lookahead != EOF 
	 && isspace((unsigned char)lookahead)
	 && nlcount < 2);

  if (lookahead == EOF)
    return false;
  
  ungetc (lookahead, stream);
  return (nlcount == 2 ? true : false);
}

/* Skip all whitespace; used for discarding whitespace in gnuplot-format
   input files.  Old-style gnuplot table format follows each dataset by two
   newlines, but new-style format uses three newlines. */

static void
skip_all_whitespace (FILE *stream)
{
  int lookahead;
  
  do 
    lookahead = getc (stream);
  while (lookahead != EOF 
	 && isspace((unsigned char)lookahead));

  if (lookahead == EOF)
    return;
  else
    ungetc (lookahead, stream);
}


/**********************************************************************/

/* read_and_plot_dataset() reads an entire dataset (a sequence of points)
   from an input file, and calls a Multigrapher's plot_point() method on
   each point as it is read.  So plotting is accomplished in real time (the
   points are not stored).  */

static dataset_status_t
read_and_plot_dataset (Reader *reader, Multigrapher *multigrapher)
{
  dataset_status_t status;

  for ( ; ; )
    {
      Point point;

      status = read_point (reader, &point);
      if (status != IN_PROGRESS)
	/* we didn't get a point, i.e. dataset ended */
	break;
      else
	plot_point (multigrapher, &point);
    }
  
  return status;
}

/* read_and_plot_file() reads a sequence of datasets from a stream, and
   calls a Multigrapher's plot_point() method on them as they are read.  So
   plotting is accomplished in real time (the points are not stored).  */

void
read_and_plot_file (Reader *reader, Multigrapher *multigrapher)
{
  dataset_status_t status;

  do
    {
      status = read_and_plot_dataset (reader, multigrapher);

      /* After each dataset, reset reader: force break in polyline, bump
	 linemode (if auto-bump is in effect), and reset abscissa (if
	 auto-abscissa is in effect).  If dataset ended with an explicit
	 set linemode / symbol style directive, don't bump the linemode. */
      if (status == ENDED_BY_MODE_CHANGE)
	{
	  bool saved_auto_bump;

	  saved_auto_bump = reader->auto_bump;
	  reader->auto_bump = false;
	  reset_reader (reader);
	  reader->auto_bump = saved_auto_bump;
	}
      else
	reset_reader (reader);

      /* after each dataset, flush the constructed polyline to the display
         device by invoking a special Multigrapher method; this ensures
         real-time performance */
      end_polyline_and_flush (multigrapher);
    }
  while (status != ENDED_BY_EOF);
}
