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

// include system headers: stdio, string, math (+ M_PI, M_SQRT2), stdlib, errno
#include "sys-defines.h"

// include all libgroff headers in ./include.
#include "cset.h"
#include "errarg.h"
#include "error.h"
#include "lib.h"
#include "position.h"
#include "ptable.h"
#include "stringclass.h"
#include "text.h"


// declare input and file_input classes
class input 
{
public:
  // ctor, dtor
  input ();
  virtual ~input ();
  // public functions (all virtual)
  virtual int get (void) = 0;
  virtual int peek (void) = 0;
  virtual int get_location (const char **filenamep, int *linenop);
  // friend classes
  friend class input_stack;
  friend class copy_rest_thru_input;
private:
  input *next;
};

class file_input : public input 
{
public:
  // ctor, dtor
  file_input (FILE *, const char *);
  ~file_input ();
  // public functions
  int get (void);
  int peek (void);
  int get_location(const char **filenamep, int *linenop);
private:
  FILE *fp;
  const char *filename;
  int lineno;
  string line;
  const char *ptr;
  int read_line (void);
};

// External functions

// interface to lexer in lex.cc
extern int yylex (void);
extern void copy_file_thru (const char *filename, const char *body, const char *until);
extern void copy_rest_thru (const char *body, const char *until);
extern void do_copy (const char *filename);
extern void do_for (char *var, double from, double to, int by_is_multiplicative, double by, char *body);
extern void do_lookahead (void);
extern void lex_cleanup (void);
extern void lex_error (const char *message, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
extern void lex_init (input *top);
extern void lex_warning (const char *message, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
extern void push_body (const char *s);
extern void yyerror (const char *s);

// interface to parser in gram.cc
extern int yyparse();
extern void parse_init (void);
extern void parse_cleanup (void);
extern int delim_flag;		// read-only variable

// Command-line flags in main.cc, used by lexer, parser, or driver
extern int command_char;
extern int compatible_flag;
extern int safer_flag;
extern int no_centering_flag;
