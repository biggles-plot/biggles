// The output class: almost pure virtual (a protocol class), but
// see some definitions in object.cc.

struct line_type 
{
  enum { invisible, solid, dotted, dashed } type;
  double dash_width;		// or inter-dot spacing, for dotted lines
  double thickness;		// line thickness in points
  // ctor 
  line_type();
};

class output 
{
public:
  // ctor, dtor 
  output();
  virtual ~output();
  // interface: implemented in toto in each concrete output class 
  virtual void start_picture (double sc, const position &ll, const position &ur) = 0;
  virtual void finish_picture (void) = 0;
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
  virtual void rounded_box (const position &cent, const distance &dim,
			    double rad, const line_type &lt, double fill) = 0;
  // no-ops, can optionally be overridden 
  virtual void command (const char *s, const char *filename, int lineno);
  virtual void set_location (const char *filename, int lineno);
  // returns 0 (false), can optionally be overridden 
  virtual int supports_filled_polygons (void);
  // no-ops; can optionally be overridden 
  virtual void begin_block (const position &ll, const position &ur);
  virtual void end_block (void);
  // not overridable; related to scaling 
  void set_desired_width_height (double wid, double ht);
  void set_args (const char *);
protected:
  char *args;
  double desired_height;	// zero if no height specified
  double desired_width;		// zero if no depth specified
  double compute_scale (double sc, const position &ll, const position &ur);
};

// A global; we have only one of these.  Its member function are what do
// the output of objects of various kinds (they're invoked by the
// objects' `print' operations).  Defined in main.cc.
extern output *out;

#define TROFF_SUPPORT 0
#define TEX_SUPPORT 0
#define PLOT_SUPPORT 1

#ifdef TROFF_SUPPORT
output *make_troff_output (void);
#endif

#ifdef TEX_SUPPORT
output *make_tex_output (void);
output *make_tpic_output (void);
#endif

#ifdef PLOT_SUPPORT
output *make_plot_output (void);
extern char *output_format;

extern char *font_name;
extern char *pen_color_name;
extern double font_size;
extern double line_width;
extern int precision_dashing;
#endif
