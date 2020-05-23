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

// The `plot_output' class, subclassed from the `common_output' class.
// In this class we invoke GNU libplot operations to draw objects.

// If the `precision_dashing' flag is set, we draw some types of object
// (arcs, polygons, circles, rounded boxes) in a special way.  The object
// boundary is drawn as a sequence of line segments (if it's to be
// "dashed") or a sequence of filled circles (if it's to be "dotted").
// This is done by invoking e.g. the dashed_arc, dotted_arc, dashed_circle,
// dotted_circle, and rounded_box operations in the `common_output'
// superclass.

// This is the only reason why we subclass from `common_output', rather
// than directly from `output'.

#include "pic.h"
#include "output.h"
#include "common.h"
#include "plot.h"		// libplot header file

// Plotter parameter array, set from command line in main.cc
extern plPlotterParams *plotter_params;

// size of graphics display in `virtual inches'
#define DISPLAY_SIZE_IN_INCHES 8.0

#define POINTS_PER_INCH 72.0

// color name array in libplot; undocumented but accessible to programmers

typedef struct
{
  const char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} Colornameinfo;

extern const Colornameinfo _colornames[];

// our libplot driver

class plot_output : public common_output
{
public:
  // ctor, dtor
  plot_output();
  ~plot_output();
  // basic interface
  void start_picture (double sc, const position &ll, const position &ur);
  void finish_picture (void);
  // draw objects
  void arc (const position &start, const position &cent, const position &end,
	    const line_type &lt);
  void circle (const position &cent, double rad, const line_type &lt, 
	       double fill);
  void ellipse (const position &cent, const distance &dim,
		const line_type &lt, double fill);
  void line (const position &start, const position *v, int n,
	     const line_type &lt);
  void polygon (const position *v, int n,
		const line_type &lt, double fill);
  void spline (const position &start, const position *v, int n,
	       const line_type &lt);
  void text (const position &center, text_piece *v, int n, double angle);
  void rounded_box (const position &cent, const distance &dim,
		    double rad, const line_type &lt, double fill);
  // attribute-querying function
  int supports_filled_polygons (void);
private:
  // parameters
  plPlotter *plotter;		// pointer to opaque libplot Plotter object
  double default_plotter_line_thickness; // line thickness in virtual points
  int pen_red, pen_green, pen_blue;	 // 48-bit pen color
  // dynamic variables, keep track of Plotter drawing state
  int plotter_line_type; // one of line_type::solid etc.
  int plotter_fill_fraction; // libplot fill fraction
  double plotter_line_thickness; // in virtual points
  bool plotter_visible_pen;	// default is `yes'
  bool plotter_path_in_progress; // need to break?
  // internal functions, modify Plotter drawing state
  void set_line_type_and_thickness (const line_type &lt);
  void set_fill (double fill);
  void set_pen_visibility (bool visible);
  // invoked by common_output dotting methods
  void dot (const position &pos, const line_type &lt);
};

output *
make_plot_output()
{
  return new plot_output;
}

plot_output::plot_output()
{
  if ((plotter = pl_newpl_r (output_format, NULL, stdout, stderr,
			     plotter_params)) == NULL)
    {
      fprintf (stderr, "%s: error: could not open plot device\n", 
	       program_name);
      exit (EXIT_FAILURE);
    }
}

plot_output::~plot_output()
{
  pl_deletepl_r (plotter);
}

void 
plot_output::start_picture(double sc, const position &ll,
			   const position &ur)
{
  double xcen, ycen, xmin, xmax, ymin, ymax;
  double scale;

  // open Plotter; record Plotter drawing state defaults
  pl_openpl_r (plotter);
  plotter_line_type = line_type::solid;
  plotter_fill_fraction = 0;	// i.e. unfilled
  plotter_visible_pen = true;
  plotter_path_in_progress = false;

  // Compute scale factor via compute_scale() method of output
  // class; see object.cc.  .PS line may contain desired width/height
  // in virtual inches; if so, scale to it.  If .PS line doesn't contain
  // desired width/height, scale according to the global `scale' variable
  // (normally set at top of pic file.  But on no account violate
  // the bounds maxpswid/maxpsht.
  scale = compute_scale(sc, ll, ur);

  /* Initialize map from user space to device space, by specifying
     rectangle in user space that will be mapped to graphics display in
     device space.  Possibly choose rectangle so that plot will be
     centered on the display. */

  if (no_centering_flag)
    {
      xmin = 0.0;
      ymin = 0.0;
    }
  else				// center
    {
      xcen = 0.5 * (ll.x + ur.x);
      ycen = 0.5 * (ll.y + ur.y);
      xmin = xcen - 0.5 * DISPLAY_SIZE_IN_INCHES * scale;
      ymin = ycen - 0.5 * DISPLAY_SIZE_IN_INCHES * scale;
    }
  xmax = xmin + DISPLAY_SIZE_IN_INCHES * scale;
  ymax = ymin + DISPLAY_SIZE_IN_INCHES * scale;

  pl_fspace_r (plotter, xmin, ymin, xmax, ymax);

  // clear Plotter of objects; initialize font name
  pl_erase_r (plotter);
  if (font_name)
    pl_fontname_r (plotter, font_name);
  
  // set pen/fill color (will modify later only by invoking pl_filltype_r)
  if (pen_color_name)
    pl_colorname_r (plotter, pen_color_name);

  // initialize font size and line thickness from values that can be set on
  // the command line (latter is dynamic, can be altered in pic file)

  font_size *= scale;
  line_width *= scale;

  if (font_size >= 0.0)
    // `font size', as set on command line, is in terms of display width,
    // but libplot, according to our scaling, uses virtual inches; so we
    // convert
    pl_ffontsize_r (plotter, DISPLAY_SIZE_IN_INCHES * font_size);
  else
    // use Plotter default; no need to issue a fontsize() instruction
    {
    }

  if (line_width >= 0.0)
    {
      // `line_width', as set on command line, is in terms of display
      // width, but libplot, according to our scaling, uses virtual inches;
      // pic2plot, uses virtual points both internally and in pic scripts
      pl_flinewidth_r (plotter, DISPLAY_SIZE_IN_INCHES * line_width);
      default_plotter_line_thickness 
	= DISPLAY_SIZE_IN_INCHES * POINTS_PER_INCH * line_width;
    }
  else
    // use Plotter default, represented internally by pic2plot as -1;
    // no need to issue a linewidth() instruction
    default_plotter_line_thickness = -1.0;

  /* store initial line thickness as a default, for later use */
  plotter_line_thickness = default_plotter_line_thickness;
}

void 
plot_output::finish_picture()
{
  pl_closepl_r (plotter);
}

//////////////////////////////////////////////////////////////////////
// SET PLOTTER DRAWING ATTRIBUTES
//////////////////////////////////////////////////////////////////////

// Manipulate fill color (idempotent, so may not actually do anything,
// i.e. may not break the path in progress, if any).

void
plot_output::set_fill (double fill)
{
  int fill_fraction;

  if (fill < 0.0)
    fill_fraction = 0;		// unfilled
  else
    {
      if (fill > 1.0)
	fill = 1.0;
      /* fill=0.0 is white, fraction=0xffff; 
	 fill=1.0 is solid color, fraction = 1 */
      fill_fraction = 0xffff - IROUND(0xfffe * fill);
    }

  if (fill_fraction != plotter_fill_fraction)
    {
      // manipulate fill color by setting the fill fraction
      pl_filltype_r (plotter, fill_fraction);

      plotter_fill_fraction = fill_fraction;
      plotter_path_in_progress = false;
    }
}

// Set line type (solid/dashed/dotted) and thickness.  May not invoke a
// libplot operation if neither needs to be changed, so may not break the
// path in progress (if any).
void
plot_output::set_line_type_and_thickness (const line_type &lt)
{
  switch (lt.type)
    {
    case line_type::solid:
    default:
      if (plotter_line_type != line_type::solid)
	{
	  pl_linemod_r (plotter, "solid");
	  plotter_line_type = line_type::solid;
	  plotter_path_in_progress = false;
	}
      break;
    case line_type::dotted:
      if (plotter_line_type != line_type::dotted)
	{
	  double dashbuf[2];

	  pl_linemod_r (plotter, "dotted");
	  dashbuf[0] = 0.25 * lt.dash_width;
	  dashbuf[1] = 0.75 * lt.dash_width;
	  pl_flinedash_r (plotter, 2, dashbuf, 0.0);
	  plotter_line_type = line_type::dotted;
	  plotter_path_in_progress = false;
	}
      break;
    case line_type::dashed:
      if (plotter_line_type != line_type::dashed)
	{
	  double dashbuf[2];

	  pl_linemod_r (plotter, "shortdashed");
	  dashbuf[0] = dashbuf[1] = lt.dash_width;
	  pl_flinedash_r (plotter, 2, dashbuf, 0.0);
	  plotter_line_type = line_type::dashed;
	  plotter_path_in_progress = false;
	}
      break;
    }
  if (lt.thickness != plotter_line_thickness
      &&
      !(lt.thickness < 0.0 && plotter_line_thickness < 0.0))
    // need to change (recall negative thickness means `default')
    {
      if (lt.thickness < 0)
	pl_flinewidth_r (plotter, 
			 default_plotter_line_thickness / POINTS_PER_INCH);
      else
	pl_flinewidth_r (plotter, lt.thickness / POINTS_PER_INCH);
      plotter_line_thickness = lt.thickness;
      plotter_path_in_progress = false;
    }
}

// Set pen visibility (true/false).  This is needed for precision dashing
// around the boundary of any closed object; provided that it is filled, at
// least.  When first drawing the closed object itself, pen visibility
// needs to be set to `false'.
void
plot_output::set_pen_visibility (bool visible)
{
  if (visible != plotter_visible_pen)
    {
      if (visible)
	pl_pentype_r (plotter, 1);
      else
	pl_pentype_r (plotter, 0);

      plotter_visible_pen = visible;
    }
}

//////////////////////////////////////////////////////////////////////
// TEXT
//////////////////////////////////////////////////////////////////////

// Draw a text object.
void
plot_output::text(const position &center, text_piece *v, int n, double angle)
{
  int horizontal_adj, vertical_adj;
  double line_spacing;

  // convert from fraction of width of display, to virtual inches
  // also multiply by 1.2 (cf. 10pt with 12pt leading)
  line_spacing = 1.2 * (DISPLAY_SIZE_IN_INCHES * font_size);

  if (n > 0)
    {
      pl_ftextangle_r (plotter, 180 * angle / M_PI);
      plotter_path_in_progress = false;

      set_pen_visibility (true); // libplot may need this
    }

  for (int i = 0; i < n; i++)
    {
      pl_fmove_r (plotter, 
		  center.x - (0.5*(n-1) - i) * line_spacing * sin(angle), 
		  center.y + (0.5*(n-1) - i) * line_spacing * cos(angle));
      plotter_path_in_progress = false;

      switch ((int)(v[i].adj.h))
	{
	case (int)CENTER_ADJUST:
	default:
	  horizontal_adj = 'c';
	  break;
	case (int)LEFT_ADJUST:
	  horizontal_adj = 'l';
	  break;
	case (int)RIGHT_ADJUST:
	  horizontal_adj = 'r';
	  break;
	}
      switch ((int)(v[i].adj.v))
	{
	case (int)NONE_ADJUST:
	default:
	  vertical_adj = 'c';
	  break;
	case (int)ABOVE_ADJUST:
	  vertical_adj = 'b';
	  break;
	case (int)BELOW_ADJUST:
	  vertical_adj = 't';
	  break;
	}
      pl_alabel_r (plotter, horizontal_adj, vertical_adj, v[i].text);
      plotter_path_in_progress = false;
    }
}

//////////////////////////////////////////////////////////////////////
// OPEN PIC OBJECTS
//////////////////////////////////////////////////////////////////////

// Draw a polyline ("open" in pic's sense, i.e., unfilled, may be part of a
// continuing path).
void
plot_output::line(const position &start, const position *v, int n,
		  const line_type &lt)
{
  if (n == 0)
    return;
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled, pic convention
  set_pen_visibility (true);

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_line_type_and_thickness (lt);
      pl_fline_r (plotter, start.x, start.y, v[0].x, v[0].y);
      for (int i = 1; i < n; i++)
	pl_fcont_r (plotter, v[i].x, v[i].y);
      plotter_path_in_progress = true;
    }
  else
    {
      switch (lt.type) 
	{
	case line_type::dashed:
	  {
	    // edge polyline, with dashes
	    line_type slt = lt;
	    slt.type = line_type::solid;
	    set_line_type_and_thickness (slt);
	    position from_point = start, to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		if (dist <= lt.dash_width*2.0)
		  pl_fline_r (plotter, 
			      from_point.x, from_point.y, to_point.x, to_point.y);
		else 
		  {
		    // round number of dashes to integer, along each segment
		    int ndashes = int((dist - lt.dash_width)/(lt.dash_width*2.0) + .5);
		    distance dash_vec = vec*(lt.dash_width/dist);
		    double dash_gap = (dist - lt.dash_width)/ndashes;
		    distance dash_gap_vec = vec*(dash_gap/dist);
		    for (int j = 0; j <= ndashes; j++) 
		      {
			position s(from_point + dash_gap_vec*j);
			pl_fline_r (plotter, 
				    s.x, s.y, s.x + dash_vec.x, s.y + dash_vec.y);
		      }
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	    pl_endpath_r (plotter);
	    plotter_path_in_progress = false;
	  }
	  break;
	case line_type::dotted:
	  {
	    // edge polyline, with dots
	    position from_point = start, to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		// round dot spacings to integer, along line segment
		int ndots = IROUND(dist/lt.dash_width);
		if (ndots == 0)
		  dot (from_point, lt);
		else 
		  {
		    vec /= double(ndots);
		    for (int j = 0; j <= ndots; j++)
		      dot (from_point + vec*j, lt);
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	  }
	  break;
	default:
	  break;
	}
    }
}

// Draw a spline ("open" in pic's sense, i.e. unfilled, may be part 
// of a continuing path).
void
plot_output::spline(const position &start, const position *v, int n,
		    const line_type &lt)
{
  if (n == 0)
    return;
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled, pic convention
  set_pen_visibility (true);
  set_line_type_and_thickness (lt);

  if (n == 1)
    pl_fline_r (plotter, start.x, start.y, v[0].x, v[0].y);    
  else if (n == 2)
    pl_fbezier2_r (plotter, 
		   start.x, start.y, v[0].x, v[0].y, v[1].x, v[1].y);
  else
    {
      pl_fbezier2_r (plotter, 
		     start.x, start.y, 
		     v[0].x, v[0].y,
		     0.5 * (v[0].x + v[1].x), 0.5 * (v[0].y + v[1].y));
      for (int i = 0; i < n - 3; i++)
	pl_fbezier2_r (plotter,
		       0.5 * (v[i].x + v[i+1].x), 0.5 * (v[i].y + v[i+1].y),
		       v[i+1].x, v[i+1].y,
		       0.5 * (v[i+1].x + v[i+2].x), 0.5 * (v[i+1].y + v[i+2].y));
      pl_fbezier2_r (plotter, 
		     0.5 * (v[n-3].x + v[n-2].x), 0.5 * (v[n-3].y + v[n-2].y),
		     v[n-2].x, v[n-2].y,
		     v[n-1].x, v[n-1].y);
    }
  plotter_path_in_progress = true;
}

// Draw an arc object ("open" in pic's sense, i.e., unfilled, may
// be part of a continuing path).
void
plot_output::arc (const position &start, const position &cent,
		  const position &end, const line_type &lt)
     // in libplot, arcs don't subtend >= 180 degrees, but that's OK
     // because they don't subtend >=180 degrees in pic either
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, end.x, end.y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (-1.0);		// unfilled (pic convention)
  set_pen_visibility (true);

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_line_type_and_thickness (lt);
      pl_farc_r (plotter, cent.x, cent.y, start.x, start.y, end.x, end.y);
      plotter_path_in_progress = true;
    }
  else
    {
      line_type slt;

      slt = lt;
      slt.type = line_type::solid;
      set_line_type_and_thickness (slt);
      switch (lt.type) 
	{
	case line_type::dashed:
	  // edge arc, with dashes
	  if (plotter_path_in_progress)
	    pl_endpath_r (plotter);
	  dashed_arc (start, cent, end, lt);
	  pl_endpath_r (plotter);
	  plotter_path_in_progress = false;
	  break;
	case line_type::dotted:
	  // edge arc, with dots
	  dotted_arc (start, cent, end, lt);
	  plotter_path_in_progress = false;
	  break;
	default:
	  break;
	}
    }
}

//////////////////////////////////////////////////////////////////////
// CLOSED PIC OBJECTS
// (some drawn differently if we do `precision dashing')
//////////////////////////////////////////////////////////////////////

// Draw a polyline object ("closed" in pic's sense).
void
plot_output::polygon(const position *v, int n,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, v[n-1].x, v[n-1].y);
      plotter_path_in_progress = false;
      return;
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      set_pen_visibility (true);
      set_line_type_and_thickness (lt);

      if (n == 4 
	  && v[0].x == v[1].x && v[2].x == v[3].x
	  && v[0].y == v[3].y && v[1].y == v[2].y)
	{
	  pl_fbox_r (plotter, v[3].x, v[3].y, v[1].x, v[1].y);
	  plotter_path_in_progress = false;
	}
      else
	{
	  pl_fmove_r (plotter, v[n-1].x, v[n-1].y);
	  for (int i = 0; i < n; i++)
	    pl_fcont_r (plotter, v[i].x, v[i].y);
	  pl_endpath_r (plotter);
	  plotter_path_in_progress = false;
	}
    }
  else
    // precision dashing (or dotting)
    {
      line_type slt;

      if (fill >= 0.0)
	// fill polygon, but don't edge it
	{
	  set_fill (fill);
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;
	  set_line_type_and_thickness (slt);
	  set_pen_visibility (false); // edge will not be drawn

	  pl_fmove_r (plotter, v[n-1].x, v[n-1].y);
	  for (int i = 0; i < n; i++)
	    pl_fcont_r (plotter, v[i].x, v[i].y);
	  pl_endpath_r (plotter);
	  plotter_path_in_progress = false;
	}

      // draw polygon boundary (unfilled) 

      set_fill (-1.0);
      set_pen_visibility (true);

      switch (lt.type) 
	{
	case line_type::dashed:
	  {
	    // edge polygon, with dashes
	    slt = lt;
	    slt.type = line_type::solid;
	    set_line_type_and_thickness (slt);
	    position from_point = v[n-1], to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		if (dist <= lt.dash_width*2.0)
		  pl_fline_r (plotter, 
			      from_point.x, from_point.y, to_point.x, to_point.y);
		else 
		  {
		    // round number of dashes to integer, along each segment
		    int ndashes = int((dist - lt.dash_width)/(lt.dash_width*2.0) + .5);
		    distance dash_vec = vec*(lt.dash_width/dist);
		    double dash_gap = (dist - lt.dash_width)/ndashes;
		    distance dash_gap_vec = vec*(dash_gap/dist);
		    for (int j = 0; j <= ndashes; j++) 
		      {
			position s(from_point + dash_gap_vec*j);
			pl_fline_r (plotter, 
				    s.x, s.y, s.x + dash_vec.x, s.y + dash_vec.y);
		      }
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	    pl_endpath_r (plotter);
	    plotter_path_in_progress = false;
	  }
	  break;
	case line_type::dotted:
	  {
	    // edge polygon, with dots
	    position from_point = v[n-1], to_point = v[0];
	    for (int i = 0; i < n; i++)
	      {
		distance vec(to_point - from_point);
		double dist = hypot(vec);
		// round dot spacings to integer, along line segment
		int ndots = IROUND(dist/lt.dash_width);
		if (ndots == 0)
		  dot (from_point, lt);
		else 
		  {
		    vec /= double(ndots);
		    for (int j = 0; j <= ndots; j++)
		      dot (from_point + vec*j, lt);
		  }
		from_point = v[i];
		to_point = v[i+1];
	      }
	  }
	  break;
	default:		// shouldn't happen
	  break;
	}
    }
}

// Draw a circle object ("closed" in pic's sense).
void
plot_output::circle (const position &cent, double rad,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, cent.x, cent.y);
      plotter_path_in_progress = false;
      return;
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      set_pen_visibility (true);
      set_line_type_and_thickness (lt);

      pl_fcircle_r (plotter, cent.x, cent.y, rad);
      plotter_path_in_progress = false;
    }
  else
    // precision dashing (or dotting)
    {
      line_type slt;

      if (fill >= 0.0)
	// fill circle, but don't edge it
	{
	  set_fill (fill);
	  set_pen_visibility (false); // edge will not be drawn
	  slt = lt;
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;
	  set_line_type_and_thickness (slt);

	  pl_fcircle_r (plotter, cent.x, cent.y, rad);
	  plotter_path_in_progress = false;
	}

      // draw circle boundary (unfilled)
      set_fill (-1.0);
      set_pen_visibility (true);
      slt = lt;
      slt.type = line_type::solid;
      set_line_type_and_thickness (slt);
      switch (lt.type) 
	{
	case line_type::dashed:
	  // edge circle, with dashes
	  if (plotter_path_in_progress)
	    pl_endpath_r (plotter);
	  dashed_circle(cent, rad, lt);
	  pl_endpath_r (plotter);
	  plotter_path_in_progress = false;
	  break;
	case line_type::dotted:
	  // edge circle, with dots
	  dotted_circle (cent, rad, lt);
	  break;
	default:		// shouldn't happen
	  break;
	}
    }
}

// Draw a rounded box object ("closed" in pic's sense).
void
plot_output::rounded_box(const position &cent, const distance &dim, double rad, const line_type &lt, double fill)
{
  static bool recursive = false;
  position tem, arc_start, arc_cent, arc_end;
  position line_start, line_end;

  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, cent.x, cent.y);      
      plotter_path_in_progress = false;
      return;
    }

  if (plotter_path_in_progress)
    {
      pl_endpath_r (plotter);
      plotter_path_in_progress = false;      
    }

  if (!precision_dashing || lt.type == line_type::solid)
    {
      set_fill (fill);
      if (!recursive)
	// _not_ invoked recursively on account of precision dashing
	{
	  set_pen_visibility (true);
	  set_line_type_and_thickness (lt);
	}

      tem = cent - dim/2.0;
      arc_start = tem + position(0.0, rad);
      arc_cent = tem + position(rad, rad);
      arc_end = tem + position(rad, 0.0);
      pl_farc_r (plotter, arc_cent.x, arc_cent.y, 
		 arc_start.x, arc_start.y, arc_end.x, arc_end.y);

      line_start = cent + position(-dim.x/2.0 + rad, -dim.y/2.0);
      line_end = cent + position(dim.x/2.0 - rad, -dim.y/2.0);
      pl_fline_r (plotter, arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + position(dim.x/2.0, -dim.y/2.0);
      arc_start = tem + position(-rad, 0.0);
      arc_cent = tem + position(-rad, rad);
      arc_end = tem + position(0.0, rad);
      pl_farc_r (plotter, arc_cent.x, arc_cent.y, 
		 line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(dim.x/2.0, -dim.y/2.0 + rad);
      line_end = cent + position(dim.x/2.0, dim.y/2.0 - rad);
      pl_fline_r (plotter, arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + dim/2.0;
      arc_start = tem + position(0.0, -rad);
      arc_cent = tem + position(-rad, -rad);
      arc_end = tem + position(-rad, 0.0);
      pl_farc_r (plotter, arc_cent.x, arc_cent.y, 
		 line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(dim.x/2.0 - rad, dim.y/2.0);
      line_end = cent + position(-dim.x/2.0 + rad, dim.y/2.0);
      pl_fline_r (plotter, arc_end.x, arc_end.y, line_end.x, line_end.y);

      tem = cent + position(-dim.x/2.0, dim.y/2.0);
      arc_start  = tem + position(rad, 0.0);
      arc_cent =  tem + position(rad, -rad);
      arc_end =  tem + position(0.0, -rad);
      pl_farc_r (plotter, arc_cent.x, arc_cent.y, 
		 line_end.x, line_end.y, arc_end.x, arc_end.y);

      line_start = cent + position(-dim.x/2.0, dim.y/2.0 - rad);
      line_end = cent + position(-dim.x/2.0, -dim.y/2.0 + rad);
      pl_fline_r (plotter, arc_end.x, arc_end.y, line_end.x, line_end.y);

      pl_endpath_r (plotter);
      plotter_path_in_progress = false;
    }
  else
    // precision dashing (or dotting)
    {
      if (fill >= 0.0)
	{
	  // fill rounded box (boundary solid, thickness 0) via recursive call
	  set_fill (fill);
	  set_pen_visibility (false); // edge will not be drawn
	  line_type slt = lt;
	  slt.type = line_type::solid;
	  slt.thickness = 0.0;

	  recursive = true;
	  rounded_box(cent, dim, rad, slt, fill);
	  recursive = false;

	  plotter_path_in_progress = false;
	}

      // draw rounded box boundary, unfilled
      set_pen_visibility (true);
      set_line_type_and_thickness (lt);	// only thickness is relevant
      common_output::rounded_box(cent, dim, rad, lt, -1.0); //-1 means unfilled
      if (plotter_path_in_progress)
	{
	  pl_endpath_r (plotter);
	  plotter_path_in_progress = false;      
	}
    }
}

// Draw an ellipse object ("closed" in pic's sense).
// No support for precision dashing, but there should be.
void
plot_output::ellipse(const position &cent, const distance &dim,
		     const line_type &lt, double fill)
{
  if (lt.type == line_type::invisible)
    {
      pl_fmove_r (plotter, cent.x, cent.y);
      plotter_path_in_progress = false;
      return;
    }

  set_fill (fill);
  set_pen_visibility (true);
  set_line_type_and_thickness (lt);

  pl_fellipse_r (plotter, cent.x, cent.y, 0.5 * dim.x, 0.5 * dim.y, 0.0);
  plotter_path_in_progress = false;
}

//////////////////////////////////////////////////////////////////////
// MISC.
//////////////////////////////////////////////////////////////////////

// Internal function, used for precision dotting; also invoked by
// precision dotting methods in the common_output superclass.
void
plot_output::dot (const position &cent, const line_type &lt)
// lt arg determines diameter of dot
{
  line_type slt;
  
  set_fill (1.0);
  set_pen_visibility (true);
  slt.type = line_type::solid;
  slt.thickness = 0.0;
  set_line_type_and_thickness (slt);

  pl_fcircle_r (plotter, cent.x, cent.y, 0.5 * lt.thickness / POINTS_PER_INCH);
  plotter_path_in_progress = false;
}

int
plot_output::supports_filled_polygons()
{
  return 1;
}
