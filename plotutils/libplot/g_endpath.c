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

/* This file contains the endpath() method, which is a GNU extension to
   libplot.  A path object may be constructed incrementally, by repeated
   invocation of such operations as cont(), arc(), etc.  The construction
   may be terminated, and the path object finalized, by an explict
   invocation of endpath().  If endpath() is invoked when no path is under
   construction, it has no effect. */

/* endpath() is a wrapper around the internal paint_path() method, which
   any Plotter can define as it chooses.  Any path is stored as a plPath
   structure in the drawing state.  This may contain a list of segments
   (line segments, curve segments etc.; which are allowed is
   Plotter-dependent) or simply a Plotter-specific drawing primitive such
   as a circle, ellipse, or rectangle.  paint_path() should be able to
   handle anything that is appropriate for the given type of Plotter. */

/* This file also contains the endsubpath() and closepath() methods. */

#include "sys-defines.h"
#include "extern.h"

int
_API_endpath (S___(Plotter *_plotter))
{
  int i;

  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "endpath: invalid operation");
      return -1;
    }

  /* end simple path under construction (if any), and move it to the array
     of stored simple paths */
  _API_endsubpath (S___(_plotter));

  if (_plotter->drawstate->num_paths == 0)
    /* no stored simple paths; so nothing to do, and we're out of here */
    return 0;
  
  /* at this point, compound path is available as an array of simple paths,
     of length at least 1, in _plotter->drawstate->paths[] */

  /* Two cases: either the line mode is `disconnected', or it isn't (the
     normal case). */

  if (!_plotter->drawstate->points_are_connected)
    /* Special case: "disconnected" linemode.  If we have a pen, path will
       be drawn as a sequence of filled circles, one per juncture point.
       Path will not be filled (our convention). */
    {
      if (_plotter->drawstate->pen_type != 0)
	/* have a pen, so we can draw something */
	{
	  plPath **saved_paths;
	  int saved_num_paths;
	  double radius = 0.5 * _plotter->drawstate->line_width;
	  int i;
	  
	  /* Switch to a temporary paths buffer.  Needed because the
	     fcircle() method calls endpath(), which would otherwise mess
	     up the real paths buffer. */
	  saved_paths = _plotter->drawstate->paths;
	  saved_num_paths = _plotter->drawstate->num_paths;
	  _plotter->drawstate->paths = (plPath **)NULL;
	  _plotter->drawstate->num_paths = 0;
	  
	  /* save graphics state */
	  _API_savestate (S___(_plotter));

	  /* set attributes appropriate for drawing filled edgeless
	     circles, in the current pen (rather than filling) color */
	  _API_filltype (R___(_plotter) 1);
	  _API_fillcolor (R___(_plotter)
			       _plotter->drawstate->fgcolor.red, 
			       _plotter->drawstate->fgcolor.green, 
			       _plotter->drawstate->fgcolor.blue);
	  _API_pentype (R___(_plotter) 0); /* edgeless */
	  _API_linemod (R___(_plotter) "solid"); /* necessary; see below*/
	  
	  /* loop over saved simple paths */
	  for (i = 0; i < saved_num_paths; i++)
	    {
	      plPath *path;
	      bool closed;
	      int j;

	      path = saved_paths[i];

	      /* sanity check: if linemode is disconnected, we should never
		 have created any simple path other than a segment list;
		 also, should have at least two juncture points */
	      if (path->type != PATH_SEGMENT_LIST || path->num_segments < 2)
		continue;

	      /* check for closure */
	      if ((path->num_segments >= 3)
		  && (path->segments[path->num_segments - 1].p.x == 
		      path->segments[0].p.x)
		  && (path->segments[path->num_segments - 1].p.y == 
		      path->segments[0].p.y))
		closed = true;
	      else
		closed = false;		/* 2-point ones should be open */
	  
	      /* draw each point as a filled circle, diameter = line width */
	      for (j = 0; j < path->num_segments - (closed ? 1 : 0); j++)
		_API_fcircle (R___(_plotter)
			      path->segments[j].p.x, 
			      path->segments[j].p.y, 
			      radius);
	      if (closed)
		/* restore graphics cursor */
		_plotter->drawstate->pos = path->segments[0].p;
	    }
	  
	  /* Restore graphics state.  This will first do a recursive
	     endpath() and hence reset the newly populated paths buffer.
	     That won't result in infinite recursion: since the line type
	     was set to "solid" above, the `points_are_connected' element
	     is now `false', and this code won't be invoked again. */
	  _API_restorestate (S___(_plotter));

	  /* switch back to original paths buffer */
	  _plotter->drawstate->paths = saved_paths;
	  _plotter->drawstate->num_paths = saved_num_paths;
	}
    }

  else
    /* normal case: line mode isn't disconnected, so no contortions needed */
    {
      if (_plotter->drawstate->num_paths == 1)
	/* compound path is just a single simple path, so paint it by
	   calling the Plotter-specific paint_path() method (the painting
	   may involve both filling and/or edging) */
	{
	  _plotter->drawstate->path = _plotter->drawstate->paths[0];
	  _plotter->paint_path (S___(_plotter));
	  _plotter->drawstate->path = (plPath *)NULL;
	}
      else
	/* compound path comprises more than one simple path */
	{
	  /* first, attempt to use Plotter-specific support for painting
	     compound paths (not many Plotters have this) */

	  if (_plotter->paint_paths (S___(_plotter)) == false)
	    /* Plotter either has no such support, or was unable to paint
	       this particular compound path; so we paint it in a clever,
	       device-independent way.  For filling, we merge the simple
	       paths into a single path, and invoke paint_path() on the
	       result.  For edging, we stroke each of the simple paths
	       individually. */
	    {
	      int fill_type, pen_type;
	      
	      fill_type = _plotter->drawstate->fill_type;      
	      pen_type = _plotter->drawstate->pen_type;
	      
	      if (fill_type && _plotter->data->have_solid_fill)
		/* fill the compound path, by merging its simple paths into
		   a single simple path, and then invoking paint_path() on
		   the result */
		{
		  plPath **merged_paths;
		  _plotter->drawstate->fill_type = fill_type;
		  _plotter->drawstate->pen_type = 0; /* unedged */
		  
		  merged_paths = _merge_paths ((const plPath **)_plotter->drawstate->paths,
					       _plotter->drawstate->num_paths);
		  for (i = 0; i < _plotter->drawstate->num_paths; i++)
		    {
		      
		      if (merged_paths[i] == (plPath *)NULL)
			continue;
		      
		      _plotter->drawstate->path = merged_paths[i];
		      _plotter->paint_path (S___(_plotter));
		      if (merged_paths[i] != _plotter->drawstate->paths[i])
			_delete_plPath (merged_paths[i]);
		    }
		  _plotter->drawstate->path = (plPath *)NULL;
		}
	      
	      if (pen_type)
		/* edge the compound path, i.e., edge each of its simple
                   paths */
		{
		  _plotter->drawstate->pen_type = pen_type;
		  _plotter->drawstate->fill_type = 0; /* unfilled */
		  for (i = 0; i < _plotter->drawstate->num_paths; i++)
		    {
		      _plotter->drawstate->path = _plotter->drawstate->paths[i];
		      _plotter->paint_path (S___(_plotter));
		    }
		  _plotter->drawstate->path = (plPath *)NULL;
		}
	      
	      /* restore filling/edging attributes */
	      _plotter->drawstate->fill_type = fill_type;
	      _plotter->drawstate->pen_type = pen_type;
	    }
	}
    }
  
  /* compound path is now painted, so remove it from paths buffer */
  for (i = 0; i < _plotter->drawstate->num_paths; i++)
    _delete_plPath (_plotter->drawstate->paths[i]);
  free (_plotter->drawstate->paths);
  _plotter->drawstate->paths = (plPath **)NULL;
  _plotter->drawstate->num_paths = 0;

  return 0;
}

int
_API_endsubpath (S___(Plotter *_plotter))
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "endsubpath: invalid operation");
      return -1;
    }

  if (_plotter->drawstate->path)
    /* have a simple path under construction, so move it to list of stored
       simple paths */
    {
      if (_plotter->drawstate->num_paths == 0)
	_plotter->drawstate->paths = 
	  (plPath **)_pl_xmalloc(sizeof (plPath *));
      else
	_plotter->drawstate->paths = 
	  (plPath **)_pl_xrealloc(_plotter->drawstate->paths,
				    (_plotter->drawstate->num_paths + 1) 
				    * sizeof (plPath *));
      _plotter->drawstate->paths[_plotter->drawstate->num_paths++] =
	_plotter->drawstate->path;
      _plotter->drawstate->path = (plPath *)NULL;
    }

  return 0;
}

int
_API_closepath (S___(Plotter *_plotter))
{
  if (!_plotter->data->open)
    {
      _plotter->error (R___(_plotter) 
		       "closepath: invalid operation");
      return -1;
    }

  /* NOT YET IMPLEMENTED */

  return 0;
}


