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

/* This file contains the internal _pl_m_emit_integer, _pl_m_emit_float,
   _pl_m_emit_op_code, and _pl_m_emit_string routines, which are used by
   MetaPlotters.  They take into account the desired format (binary
   metafile format or ascii [human-readable] metafile format.

   In the unnumbered versions of libplot prior to version 0.0 (which was
   released as part of plotutils-2.0, in 1/98), we assumed that in binary
   metafile format, two bytes sufficed to represent any integer.  This was
   the convention used in traditional plot(5) format, and can be traced to
   the PDP-11.  Unfortunately it confined us to the range -0x10000..0x7fff
   (assuming two's complement).  Actually, the parsing in our `plot'
   utility always treated the arguments to pencolor(), fillcolor(), and
   filltype() specially.  An argument of any of those functions was treated
   as an unsigned integer, so it could be in the range 0..0xffff.

   In version 0.0 of libplot, we switched in binary metafile format to
   representing integers as machine integers.  The parsing of metafiles by
   `plot' now takes this into account.  `plot' has command-line options for
   backward compatibility with plot(5) format.

   Our representation for floating-point numbers in binary metafiles is
   simply the machine representation for single-precision floating point.
   plot(5) format did not support floating point arguments, so there are no
   concerns over backward compatibility. */

#include "sys-defines.h"
#include "extern.h"

/* emit one unsigned character, passed as an int */
void
_pl_m_emit_op_code (R___(Plotter *_plotter) int c)
{
  if (_plotter->data->outfp)
    putc (c, _plotter->data->outfp);
#ifdef LIBPLOTTER
  else if (_plotter->data->outstream)
    _plotter->data->outstream->put ((unsigned char)c);
#endif
}

void
_pl_m_emit_integer (R___(Plotter *_plotter) int x)
{
  if (_plotter->data->outfp)
    {
      if (_plotter->meta_portable_output)
	fprintf (_plotter->data->outfp, " %d", x);
      else
	fwrite ((void *) &x, sizeof(int), 1, _plotter->data->outfp);
    }
#ifdef LIBPLOTTER
  else if (_plotter->data->outstream)
    {
      if (_plotter->meta_portable_output)
	(*(_plotter->data->outstream)) << ' ' << x;
      else
	_plotter->data->outstream->write((char *)&x, sizeof(int));
    }
#endif
}

void
_pl_m_emit_float (R___(Plotter *_plotter) double x)
{
  if (_plotter->data->outfp)
    {
      if (_plotter->meta_portable_output)
	{
	  /* treat equality with zero specially, since some printf's print
	     negative zero differently from positive zero, and that may
	     prevent regression tests from working properly */
	  fprintf (_plotter->data->outfp, x == 0.0 ? " 0" : " %g", x);
	}
      else
	{
	  float f;
	  
	  f = FROUND(x);
	  fwrite ((void *) &f, sizeof(float), 1, _plotter->data->outfp);
	}
    }
#ifdef LIBPLOTTER
  else if (_plotter->data->outstream)
    {
      if (_plotter->meta_portable_output)
	(*(_plotter->data->outstream)) << ' ' << x;
      else
	{
	  float f;
	  
	  f = FROUND(x);
	  _plotter->data->outstream->write((char *)&f, sizeof(float));
	}
    }
#endif
}

void
_pl_m_emit_string (R___(Plotter *_plotter) const char *s)
{
  bool has_newline;
  char *t = NULL;		/* keep compiler happy */
  char *nl;
  const char *u;
  
  /* null pointer handled specially */
  if (s == NULL)
    s = "(null)";
  
  if (strchr (s, '\n'))
    /* don't grok arg strings containing newlines; truncate at first
       newline if any */
    {
      has_newline = true;
      t = (char *)_pl_xmalloc (strlen (s) + 1);      
      strcpy (t, s);
      nl = strchr (t, '\n');
      *nl = '\0';
      u = t;
    }
  else
    {
      has_newline = false;
      u = s;
    }
      
  /* emit string, with appended newline if output format is binary (old
     plot(3) convention, which makes sense only if there can be at most one
     string among the command arguments, and it's positioned last) */
  if (_plotter->data->outfp)
    {
      fputs (u, _plotter->data->outfp);
      if (_plotter->meta_portable_output == false)
	putc ('\n', _plotter->data->outfp); 
    }
#ifdef LIBPLOTTER
  else if (_plotter->data->outstream)
    {
      (*(_plotter->data->outstream)) << u;
      if (_plotter->meta_portable_output == false)
	(*(_plotter->data->outstream)) << '\n';
    }
#endif

  if (has_newline)
    free (t);
}

/* End a directive that was begun by invoking _pl_m_emit_op_code() (q.v.).  In
   portable format, the terminator is a newline; in binary format, there is
   no terminator. */
void
_pl_m_emit_terminator (S___(Plotter *_plotter))
{
  if (_plotter->meta_portable_output)
    {
      if (_plotter->data->outfp)
	putc ('\n', _plotter->data->outfp);
#ifdef LIBPLOTTER
      else if (_plotter->data->outstream)
	(*(_plotter->data->outstream)) << '\n';
#endif
    }
}
