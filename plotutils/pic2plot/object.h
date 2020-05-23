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

struct place;

enum object_type 
{
  OTHER_OBJECT,
  BOX_OBJECT,
  CIRCLE_OBJECT,
  ELLIPSE_OBJECT,
  ARC_OBJECT,
  SPLINE_OBJECT,
  LINE_OBJECT,
  ARROW_OBJECT,
  MOVE_OBJECT,
  TEXT_OBJECT,
  BLOCK_OBJECT,
  MARK_OBJECT
};

class bounding_box;

class object 
{
public:
  // ctor, dtor
  object();
  virtual ~object();
  // doubly linked list
  object *prev;
  object *next;
  // public functions (all virtual)
  // 1. functions returning positions and dimensions
  virtual position origin();
  virtual double width();
  virtual double radius();
  virtual double height();
  virtual position north();
  virtual position south();
  virtual position east();
  virtual position west();
  virtual position north_east();
  virtual position north_west();
  virtual position south_east();
  virtual position south_west();
  virtual position start();
  virtual position end();
  virtual position center();
  virtual place *find_label(const char *); // lookup location by string
  // 2. other functions
  virtual void move_by(const position &);
  virtual int blank();
  virtual void update_bounding_box(bounding_box *);
  virtual object_type type() = 0;
  virtual void print();
  virtual void print_text();
};

typedef position (object::*corner)();

struct place 
{
  object *obj;
  double x, y;
};

class string_list;

class path 
{
public:
  // ctors, dtor
  path(corner = 0);
  path(char *, corner = 0);
  ~path();
  // public functions
  void append(corner);
  void append(char *);
  void set_ypath(path *);
  int follow(const place &, place *) const;
private:
  corner crn;
  string_list *label_list;
  path *ypath;
};

class object_list 
{
public:
  // ctor
  object_list();
  // public functions
  void append(object *);
  void wrap_up_block(object_list *);
  // public data
  object *head;
  object *tail;
};

declare_ptable(place)

// these go counterclockwise
enum direction 
{
  RIGHT_DIRECTION,
  UP_DIRECTION,
  LEFT_DIRECTION,
  DOWN_DIRECTION
  };

struct graphics_state 
{
  double x, y;
  direction dir;
};

struct saved_state : public graphics_state 
{
  saved_state *prev;
  PTABLE(place) *tbl;
};

class text_item 
{
public:
  // ctor, dtor
  text_item(char *, const char *, int);
  ~text_item();
  // public data
  text_item *next;
  char *text;
  adjustment adj;
  const char *filename;
  int lineno;
};

const unsigned long IS_DOTTED = 01;
const unsigned long IS_DASHED = 02;
const unsigned long IS_CLOCKWISE = 04;
const unsigned long IS_INVISIBLE = 020;
const unsigned long HAS_LEFT_ARROW_HEAD = 040;
const unsigned long HAS_RIGHT_ARROW_HEAD = 0100;
const unsigned long HAS_SEGMENT = 0200;
const unsigned long IS_SAME = 0400;
const unsigned long HAS_FROM = 01000;
const unsigned long HAS_AT = 02000;
const unsigned long HAS_WITH = 04000;
const unsigned long HAS_HEIGHT = 010000;
const unsigned long HAS_WIDTH = 020000;
const unsigned long HAS_RADIUS = 040000;
const unsigned long HAS_TO = 0100000;
const unsigned long IS_CHOPPED = 0200000;
const unsigned long IS_DEFAULT_CHOPPED = 0400000;
const unsigned long HAS_THICKNESS = 01000000;
const unsigned long IS_FILLED = 02000000;
const unsigned long IS_DEFAULT_FILLED = 04000000;
const unsigned long IS_ALIGNED = 010000000;

class segment 
{
public:
  // ctor
  segment(const position &, int, segment *);
  // public data
  int is_absolute;
  position pos;
  segment *next;
};

class rectangle_object;
class graphic_object;
class linear_object;

class object_spec 
{
public:
  // ctor, dtor
  object_spec(object_type);
  ~object_spec();
  // public functions (mostly for creating objects)
  object *make_object(position *, direction *);
  graphic_object *make_box(position *, direction *);
  graphic_object *make_block(position *, direction *);
  graphic_object *make_text(position *, direction *);
  graphic_object *make_ellipse(position *, direction *);
  graphic_object *make_circle(position *, direction *);
  linear_object *make_line(position *, direction *);
  linear_object *make_arc(position *, direction *);
  graphic_object *make_linear(position *, direction *);
  graphic_object *make_move(position *, direction *);
  int position_rectangle(rectangle_object *p, position *curpos, direction *dirp);
  // public data (mostly, object attributes)
  unsigned long flags;
  object_type type;
  object_list oblist;
  PTABLE(place) *tbl;
  double dash_width;
  position from;
  position to;
  position at;
  position by;
  path *with;
  text_item *text;
  double height;
  double radius;
  double width;
  double segment_width;
  double segment_height;
  double start_chop;
  double end_chop;
  double thickness;
  double fill;
  direction dir;
  segment *segment_list;
  position segment_pos;
  int segment_is_absolute;
};

object *make_object(object_spec *, position *, direction *);

object *make_mark_object();
object *make_command_object(char *, const char *, int);

// interface to parser in gram.cc 
extern void define_variable (const char *name, double val);
extern int lookup_variable (const char *name, double *val);

// function in object.cc
extern void print_picture (object *obj);

