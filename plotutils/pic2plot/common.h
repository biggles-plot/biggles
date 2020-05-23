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

// The common_output class: subclassed from the output class, 
// providing support for dotting/dashing and for rounded boxes.
// The output class is defined in output.h.

class common_output : public output 
{
public:
  // basic interface, implemented in concrete classes rather than here
  virtual void start_picture (double sc, const position &ll, const position &ur) = 0;
  virtual void finish_picture () = 0;
  // draw objects (not implemented here)
  virtual void arc (const position &start, const position &cent, 
		    const position &end, const line_type &lt) = 0;
  virtual void circle (const position &cent, double rad, 
		       const line_type &lt, double fill) = 0;
  virtual void ellipse (const position &cent, const distance &dim,
			const line_type &lt, double fill) = 0;
  virtual void line (const position &start, const position *v, int n,
		     const line_type &lt) = 0;
  virtual void polygon (const position *v, int n,
			const line_type &lt, double fill) = 0;
  virtual void spline (const position &start, const position *v, int n,
		       const line_type &lt) = 0;
  virtual void text (const position &center, text_piece *v, int n,
		     double angle) = 0;
  // draw objects (implemented here)
  virtual void rounded_box (const position &cent, const distance &dim,
		    double rad, const line_type &lt, double fill);
protected:
  /* implemented in concrete classes (used for dotting lines by hand) */
  virtual void dot (const position &cent, const line_type &lt) = 0;
  /* implemented in terms of arc (); can be overridden (e.g. in tex_output) */
  virtual void solid_arc (const position &cent, double rad, double start_angle,
			  double end_angle, const line_type &lt);
  /* dashing and dotting `by hand' (used by troff_output, not tex_output) */
  void dashed_circle (const position &cent, double rad, const line_type &lt);
  void dotted_circle (const position &cent, double rad, const line_type &lt);

  void dashed_arc (const position &start, const position &cent,
		   const position &end, const line_type &lt);
  void dotted_arc (const position &start, const position &cent,
		   const position &end, const line_type &lt);

  void dashed_rounded_box (const position &cent, const distance &dim,
			   double rad, const line_type &lt);
  void dotted_rounded_box (const position &cent, const distance &dim,
			   double rad, const line_type &lt);

  void solid_rounded_box (const position &cent, const distance &dim,
			  double rad, const line_type &lt);
  void filled_rounded_box (const position &cent, const distance &dim,
			   double rad, double fill);
private:
  void dash_line (const position &start, const position &end,
		  const line_type &lt, double dash_width, double gap_width,
		  double *offsetp);
  void dash_arc (const position &cent, double rad,
		 double start_angle, double end_angle, const line_type &lt,
		 double dash_width, double gap_width, double *offsetp);
  void dot_line (const position &start, const position &end,
		 const line_type &lt, double gap_width, double *offsetp);
  void dot_arc (const position &cent, double rad,
		double start_angle, double end_angle, const line_type &lt,
		double gap_width, double *offsetp);
};

// not private because TeX driver uses this
int compute_arc_center (const position &start, const position &cent, const position &end, position *result);

