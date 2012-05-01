/*
 * $Id: libplot.c,v 1.12 2010/04/09 20:54:17 mrnolta Exp $ 
 *
 * Copyright (C) 2001 Mike Nolta <mrnolta@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 */

#include <Python.h>
#include <math.h>
#include <plot.h>

#include <numpy/arrayobject.h>

typedef int bool_t;
#define TRUE 1
#define FALSE 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PyArray_1D(v,i)\
	(*((double *)((v)->data+(i)*(v)->strides[0])))

#define PyArray_2D(m,i,j)\
	(*((double *)((m)->data+(i)*(m)->strides[0]+(j)*(m)->strides[1])))

#define PyArray_3D(m,i,j,k)\
	(*((double *) ( (m)->data + \
			(i)*(m)->strides[0] + \
			(j)*(m)->strides[1] + \
			(k)*(m)->strides[2])) )

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*****************************************************************************
 *  clipping code
 */

#define TOP	0x1
#define BOTTOM	0x2
#define RIGHT	0x4
#define LEFT	0x8

static unsigned char
outcode( double x, double y,
	double xmin, double xmax, double ymin, double ymax )
{
	unsigned char code = 0x0;

	if ( x < xmin ) code |= LEFT;
	if ( x > xmax ) code |= RIGHT;
	if ( y < ymin ) code |= BOTTOM;
	if ( y > ymax ) code |= TOP;

	/*
	printf( "%d (%g,%g,%g) (%g,%g,%g)\n",
		code, xmin, x, xmax, ymin, y, ymax );
	*/

	return code;
}

static bool_t
cohen_sutherland( double xmin, double xmax, double ymin, double ymax,
	double x0, double y0, double x1, double y1,
	double *xc0, double *yc0, double *xc1, double *yc1 )
{
	unsigned out, out0, out1;
	bool_t accept, done;
	double x = 0., y = 0.;

	out0 = outcode( x0, y0, xmin, xmax, ymin, ymax );
	out1 = outcode( x1, y1, xmin, xmax, ymin, ymax );

	accept = FALSE;
	done = FALSE;

	do
	{
		if ( (out0 == 0) && (out1 == 0) )
		{
			/* trivially inside */
			accept = TRUE;
			done = TRUE;
		}
		else if ( (out0 & out1) != 0 )
		{
			/* trivially outside */
			done = TRUE;
		}
		else
		{
			out = ( out0 != 0 ) ? out0 : out1 ;

			if ( (out & TOP) != 0 )
			{
				x = x0 + (x1 - x0)*(ymax - y0)/(y1 - y0);
				y = ymax;
			}
			else if ( (out & BOTTOM) != 0 )
			{
				x = x0 + (x1 - x0)*(ymin - y0)/(y1 - y0);
				y = ymin;
			}
			else if ( (out & RIGHT) != 0 )
			{
				y = y0 + (y1 - y0)*(xmax - x0)/(x1 - x0);
				x = xmax;
			}
			else if ( (out & LEFT) != 0 )
			{
				y = y0 + (y1 - y0)*(xmin - x0)/(x1 - x0);
				x = xmin;
			}

			if ( out == out0 )
			{
				x0 = x;
				y0 = y;
				out0 = outcode( x, y, xmin, xmax, ymin, ymax ); 
			}
			else
			{
				x1 = x;
				y1 = y;
				out1 = outcode( x, y, xmin, xmax, ymin, ymax );
			}
		}
	}
	while ( done != TRUE );

	if ( accept == TRUE )
	{
		*xc0 = x0;
		*yc0 = y0;
		*xc1 = x1;
		*yc1 = y1;

		/* printf( "(%g %g) (%g %g)\n", x0, y0, x1, y1 ); */
	}

	return accept;
}

static void
clipped_pl_fline_r( plPlotter *pl,
	double xmin, double xmax, double ymin, double ymax,
	double x0, double y0, double x1, double y1 )
{
	double xc0, yc0, xc1, yc1;
	bool_t accept;

	accept = cohen_sutherland( xmin, xmax, ymin, ymax,
		x0, y0, x1, y1, &xc0, &yc0, &xc1, &yc1 );

	if ( accept == TRUE )
		pl_fline_r( pl, xc0, yc0, xc1, yc1 );
}

/******************************************************************************
 *
 */

static PyObject *
new( PyObject *self, PyObject *args )
{
	PyObject *o, *odict, *ofile, *key, *value;
	Py_ssize_t pos;
	char *type, *skey, *svalue;
	FILE *outfile;
	plPlotterParams *params;
	plPlotter *pl;

	if ( !PyArg_ParseTuple( args, "sOO", &type, &odict, &ofile ) )
		return NULL;

	params = pl_newplparams();

	if ( PyDict_Check(odict) )
	{
		pos = 0;
		while ( PyDict_Next( odict, &pos, &key, &value ) )
		{
			skey = PyString_AsString( key );
			svalue = PyString_AsString( value );
			pl_setplparam( params, skey, svalue );
		}
	}
	else if ( odict != Py_None )
	{
		PyErr_SetString( PyExc_TypeError, "not a dict" );
		return NULL;
	}

	outfile = NULL;

	if ( PyFile_Check(ofile) )
	{
		outfile = PyFile_AsFile( ofile );
	}
	else if ( ofile != Py_None )
	{
		PyErr_SetString( PyExc_TypeError, "not a file" );
		return NULL;
	}

	pl = pl_newpl_r( type, NULL, outfile, NULL, params );

	pl_deleteplparams( params );

	o = PyCObject_FromVoidPtr( (void *) pl, NULL );
	return Py_BuildValue( "O", o );
}

/******************************************************************************
 */

#define BGL_PL_FUNC(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "O", &o ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

BGL_PL_FUNC( clear, pl_erase_r )
BGL_PL_FUNC( end_page, pl_closepl_r )
BGL_PL_FUNC( delete, pl_deletepl_r )
BGL_PL_FUNC( flush, pl_flushpl_r )
BGL_PL_FUNC( gsave, pl_savestate_r )
BGL_PL_FUNC( grestore, pl_restorestate_r )
BGL_PL_FUNC( begin_page, pl_openpl_r )

/******************************************************************************
 */

#define BGL_PL_FUNC_I(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	int i0;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Oi", &o, &i0 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, i0 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

BGL_PL_FUNC_I( set_fill_level, pl_filltype_r )

/******************************************************************************
 */

#define BGL_PL_FUNC_D(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Od", &o, &d0 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Odd", &o, &d0, &d1 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DDD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Oddd", &o, &d0, &d1, &d2 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1, d2 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DDDD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2, d3;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Odddd", &o, &d0, &d1, &d2, &d3 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1, d2, d3 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DDDDD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2, d3, d4;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Oddddd", &o, &d0, &d1, &d2, &d3, &d4 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1, d2, d3, d4 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DDDDDD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2, d3, d4, d5;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Odddddd",\
			&o, &d0, &d1, &d2, &d3, &d4, &d5 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1, d2, d3, d4, d5 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

#define BGL_PL_FUNC_DDDDDDDD(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2, d3, d4, d5, d6, d7;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Odddddddd",\
			&o, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, d0, d1, d2, d3, d4, d5, d6, d7 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

BGL_PL_FUNC_D( set_font_size, pl_ffontsize_r );
BGL_PL_FUNC_D( set_line_size, pl_flinewidth_r );
BGL_PL_FUNC_D( set_string_angle, pl_ftextangle_r );

BGL_PL_FUNC_DD( move, pl_fmove_r );
BGL_PL_FUNC_DD( lineto, pl_fcont_r );
BGL_PL_FUNC_DD( linetorel, pl_fcontrel_r );

BGL_PL_FUNC_DDD( circle, pl_fcircle_r );

BGL_PL_FUNC_DDDD( line, pl_fline_r )
BGL_PL_FUNC_DDDD( rect, pl_fbox_r )
BGL_PL_FUNC_DDDD( space, pl_fspace_r )

BGL_PL_FUNC_DDDDD( ellipse, pl_fellipse_r )

BGL_PL_FUNC_DDDDDD( arc, pl_farc_r )

BGL_PL_FUNC_DDDDDDDD( clipped_line, clipped_pl_fline_r )

/******************************************************************************
 */

#define BGL_PL_FUNC_S(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	char *s0;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Os", &o, &s0 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	FUNCTION ( (plPlotter *) vptr, s0 );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

BGL_PL_FUNC_S( set_colorname_bg, pl_bgcolorname_r );
BGL_PL_FUNC_S( set_colorname_fg, pl_colorname_r );
BGL_PL_FUNC_S( set_colorname_fill, pl_fillcolorname_r );
BGL_PL_FUNC_S( set_colorname_pen, pl_pencolorname_r );

BGL_PL_FUNC_S( set_fill_type, pl_fillmod_r );
BGL_PL_FUNC_S( set_font_type, pl_fontname_r );
BGL_PL_FUNC_S( set_join_type, pl_joinmod_r );
BGL_PL_FUNC_S( set_line_type, pl_linemod_r );

/******************************************************************************
 *  color functions
 */

#define BGL_PL_FUNC_COLOR(NAME,FUNCTION)\
static PyObject *\
NAME ( PyObject *self, PyObject *args )\
{\
	PyObject *o;\
	double d0, d1, d2;\
	int r, g, b;\
	void *vptr;\
\
	if ( !PyArg_ParseTuple( args, "Oddd", &o, &d0, &d1, &d2 ) )\
		return NULL;\
	vptr = (void *) PyCObject_AsVoidPtr( o );\
\
	r = (int) floor( d0*65535 );\
	g = (int) floor( d1*65535 );\
	b = (int) floor( d2*65535 );\
\
	FUNCTION ( (plPlotter *) vptr, r, g, b );\
	Py_INCREF( Py_None );\
	return Py_None;\
}

BGL_PL_FUNC_COLOR( set_color_bg, pl_bgcolor_r )
BGL_PL_FUNC_COLOR( set_color_fg, pl_color_r )
BGL_PL_FUNC_COLOR( set_color_fill, pl_fillcolor_r )
BGL_PL_FUNC_COLOR( set_color_pen, pl_pencolor_r )

/******************************************************************************
 */

static PyObject *
string( PyObject *self, PyObject *args )
{
	PyObject *o;
	int i0, i1;
	char *s0;
	void *vptr;

	if ( !PyArg_ParseTuple( args, "Oiis", &o, &i0, &i1, &s0 ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );

	pl_alabel_r( (plPlotter *) vptr, i0, i1, s0 );
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
get_string_width( PyObject *self, PyObject *args )
{
	PyObject *o;
	char *s0;
	void *vptr;
	double width;

	if ( !PyArg_ParseTuple( args, "Os", &o, &s0 ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );

	width = pl_flabelwidth_r( (plPlotter *) vptr, s0 );
	return Py_BuildValue( "d", width );
}

/******************************************************************************
 *  vector routines
 */

static void
_symbol_begin( plPlotter *pl, int type, double size )
{
	if ( type > 31 )
	{
		pl_savestate_r( pl );
		pl_ffontsize_r( pl, size );
	}
}

static void
_symbol_draw( plPlotter *pl, double x, double y, int type, double size )
{
	if ( type > 31 )
	{
		char type_str[2];
		type_str[0] = type;
		type_str[1] = '\0';
		pl_fmove_r( pl, x, y );
		pl_alabel_r( pl, 'c', 'c', type_str );
	}
	else
	{
		pl_fmarker_r( pl, x, y, type, size );
	}
}

static void
_symbol_end( plPlotter *pl, int type, double size )
{
	if ( type > 31 )
		pl_restorestate_r( pl );
}

static PyObject *
symbols( PyObject *self, PyObject *args )
{
	PyObject *o, *ox, *oy;
	PyArrayObject *x, *y;
	double d0;
	int i0, i, n;
	void *vptr;
	plPlotter *pl;

	if ( !PyArg_ParseTuple( args, "OOOid", &o, &ox, &oy, &i0, &d0 ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;

	x = (PyArrayObject *)
		PyArray_ContiguousFromObject( ox, PyArray_DOUBLE, 1, 1 );
	y = (PyArrayObject *)
		PyArray_ContiguousFromObject( oy, PyArray_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = MIN( x->dimensions[0], y->dimensions[0] );

	_symbol_begin( pl, i0, d0 );

	for ( i = 0; i < n; i++ )
		_symbol_draw( pl, PyArray_1D(x,i), PyArray_1D(y,i), i0, d0 );

	_symbol_end( pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
clipped_symbols( PyObject *self, PyObject *args )
{
	PyObject *o, *ox, *oy;
	PyArrayObject *x, *y;
	double xmin, xmax, ymin, ymax;
	double d0;
	int i0, i, n;
	void *vptr;
	plPlotter *pl;
	double px, py;

	if ( !PyArg_ParseTuple( args, "OOOiddddd", &o, &ox, &oy,
			&i0, &d0, &xmin, &xmax, &ymin, &ymax ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;

	x = (PyArrayObject *)
		PyArray_ContiguousFromObject( ox, PyArray_DOUBLE, 1, 1 );
	y = (PyArrayObject *)
		PyArray_ContiguousFromObject( oy, PyArray_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = MIN( x->dimensions[0], y->dimensions[0] );

	_symbol_begin( pl, i0, d0 );

	for ( i = 0; i < n; i++ )
	{
		px = PyArray_1D(x,i);
		py = PyArray_1D(y,i);

		if ( px >= xmin && px <= xmax &&
		     py >= ymin && py <= ymax )
			_symbol_draw( pl, px, py, i0, d0 );
	}

	_symbol_end( pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
clipped_colored_symbols( PyObject *self, PyObject *args )
{
	PyObject *o, *ox, *oy, *oc;
	PyArrayObject *x, *y, *c;
	double xmin, xmax, ymin, ymax;
	double d0;
	int i0, i, n;
	void *vptr;
	plPlotter *pl;
	double px, py;
	int r, g, b;

	if ( !PyArg_ParseTuple( args, "OOOOiddddd", &o, &ox, &oy, &oc,
			&i0, &d0, &xmin, &xmax, &ymin, &ymax ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;

	x = (PyArrayObject *)
		PyArray_ContiguousFromObject( ox, PyArray_DOUBLE, 1, 1 );
	y = (PyArrayObject *)
		PyArray_ContiguousFromObject( oy, PyArray_DOUBLE, 1, 1 );
	c = (PyArrayObject *)
		PyArray_ContiguousFromObject( oc, PyArray_DOUBLE, 2, 2 );

	if ( x == NULL || y == NULL || c == NULL )
		goto quit;

	n = MIN( x->dimensions[0], y->dimensions[0] );
	n = MIN( n, c->dimensions[0] );

	/* printf("c dims %dx%d first rgb %g,%g,%g\n",
	 *      c->dimensions[0], c->dimensions[1],
	 *      PyArray_2D(c,0,0),PyArray_2D(c,0,1),PyArray_2D(c,0,2) );
	 */
	
	_symbol_begin( pl, i0, d0 );

	for ( i = 0; i < n; i++ )
	{
		px = PyArray_1D(x,i);
		py = PyArray_1D(y,i);

		if ( px >= xmin && px <= xmax &&
		     py >= ymin && py <= ymax ) {
			r = (int) floor( PyArray_2D(c,i,0)*65535 );
			g = (int) floor( PyArray_2D(c,i,1)*65535 );
			b = (int) floor( PyArray_2D(c,i,2)*65535 );
			pl_fillcolor_r( (plPlotter *) vptr, r, g, b );
			pl_pencolor_r(  (plPlotter *) vptr, r, g, b );
		  
			_symbol_draw( pl, px, py, i0, d0 );
		}
	}

	_symbol_end( pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_XDECREF(c);
	Py_INCREF( Py_None );
	return Py_None;
}


/*
 * Draw a density plot --
 *   Given a grid of intensity values, plot uniform squares tiling
 *   the region (xmin, ymin) to (xmax, ymax).
 *
 */

static PyObject *
density_plot( PyObject *self, PyObject *args )
{
	PyObject *o, *ogrid;
	PyArrayObject *grid;
	double xmin, xmax, ymin, ymax;
	void *vptr;
	plPlotter *pl;
	
	double px, py, dx, dy;
	int    xi, yi, xn, yn;
	int    r, g, b;

	if ( !PyArg_ParseTuple( args, "OOdddd", &o, &ogrid,
				&xmin, &xmax, &ymin, &ymax ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;
	grid = (PyArrayObject *)
		PyArray_ContiguousFromObject( ogrid, PyArray_DOUBLE, 2, 2 );
	if ( grid == NULL )
		goto quit;

	xn = grid->dimensions[0];
	yn = grid->dimensions[1];
	dx = (xmax - xmin) / xn;
	dy = (ymax - ymin) / yn;

	/* printf("grid dims %dx%d from (%g,%g) to (%g,%g) first row %g,%g,%g,...\n",
	 *      grid->dimensions[0], grid->dimensions[1],
	 *      xmin, ymin, xmax, ymax,
	 *      PyArray_2D(grid,0,0),PyArray_2D(grid,0,1),PyArray_2D(grid,0,2) );
	 */
	
	for   ( xi=0, px=xmin; xi < xn; xi++, px+=dx ) {
	  for ( yi=0, py=ymin; yi < yn; yi++, py+=dy ) {
	    r=g=b = (int) floor( PyArray_2D(grid,xi,yi)*65535 );
	    pl_filltype_r ( (plPlotter *) vptr, 1.0 );
	    pl_fillcolor_r( (plPlotter *) vptr, r, g, b );
	    pl_pencolor_r ( (plPlotter *) vptr, r, g, b );

	    pl_fbox_r( pl, px, py, px+dx, py+dy );
	  }
	}

quit:
	Py_XDECREF(grid);
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
color_density_plot( PyObject *self, PyObject *args )
{
	PyObject *o, *ogrid;
	PyArrayObject *grid;
	double xmin, xmax, ymin, ymax;
	void *vptr;
	plPlotter *pl;
	
	double px, py, dx, dy;
	int    xi, yi, xn, yn;
	int    r, g, b;

	if ( !PyArg_ParseTuple( args, "OOdddd", &o, &ogrid,
				&xmin, &xmax, &ymin, &ymax ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;
	grid = (PyArrayObject *)
		PyArray_ContiguousFromObject( ogrid, PyArray_DOUBLE, 3, 3 );
	if ( grid == NULL )
		goto quit;
	if ( grid->dimensions[2] != 3) {
		printf("Expect a NxMx3 array for densgrid");
		goto quit;
	}
	
	xn = grid->dimensions[0];
	yn = grid->dimensions[1];
	dx = (xmax - xmin) / xn;
	dy = (ymax - ymin) / yn;

	/* printf("grid dims %dx%d from (%g,%g) to (%g,%g) first elt %g,%g,%g,...\n",
	 *      grid->dimensions[0], grid->dimensions[1],
	 *      xmin, ymin, xmax, ymax,
	 *      PyArray_3D(grid,0,0,0),PyArray_3D(grid,0,0,1),PyArray_3D(grid,0,0,2) );
	 */
	
	for   ( xi=0, px=xmin; xi < xn; xi++, px+=dx ) {
	  for ( yi=0, py=ymin; yi < yn; yi++, py+=dy ) {
	    r = (int) floor( PyArray_3D(grid,xi,yi,0)*65535 );
	    g = (int) floor( PyArray_3D(grid,xi,yi,1)*65535 );
	    b = (int) floor( PyArray_3D(grid,xi,yi,2)*65535 );
	    pl_filltype_r ( (plPlotter *) vptr, 1.0 );
	    pl_fillcolor_r( (plPlotter *) vptr, r, g, b );
	    pl_pencolor_r ( (plPlotter *) vptr, r, g, b );

	    pl_fbox_r( pl, px, py, px+dx, py+dy );
	  }
	}

quit:
	Py_XDECREF(grid);
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
curve( PyObject *self, PyObject *args )
{
	PyObject *o, *ox, *oy;
	PyArrayObject *x, *y;
	int i, n;
	void *vptr;
	plPlotter *pl;

	if ( !PyArg_ParseTuple( args, "OOO", &o, &ox, &oy ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;

	x = (PyArrayObject *)
		PyArray_ContiguousFromObject( ox, PyArray_DOUBLE, 1, 1 );
	y = (PyArrayObject *)
		PyArray_ContiguousFromObject( oy, PyArray_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = MIN( x->dimensions[0], y->dimensions[0] );
	if ( n <= 0 )
		goto quit;

	pl_fmove_r( pl, PyArray_1D(x,0), PyArray_1D(y,0) );
	for ( i = 1; i < n; i++ )
		pl_fcont_r( pl, PyArray_1D(x,i), PyArray_1D(y,i) );
	pl_endpath_r( pl );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_INCREF( Py_None );
	return Py_None;
}

static PyObject *
clipped_curve( PyObject *self, PyObject *args )
{
	PyObject *o, *ox, *oy;
	PyArrayObject *x, *y;
	double xmin, xmax, ymin, ymax;
	int i, n;
	void *vptr;
	plPlotter *pl;

	if ( !PyArg_ParseTuple( args, "OOOdddd", &o, &ox, &oy,
			&xmin, &xmax, &ymin, &ymax ) )
		return NULL;
	vptr = (void *) PyCObject_AsVoidPtr( o );
	pl = (plPlotter *) vptr;

	x = (PyArrayObject *)
		PyArray_ContiguousFromObject( ox, PyArray_DOUBLE, 1, 1 );
	y = (PyArrayObject *)
		PyArray_ContiguousFromObject( oy, PyArray_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = MIN( x->dimensions[0], y->dimensions[0] );
	if ( n <= 0 )
		goto quit;

	for ( i = 0; i < n-1; i++ )
	{
		clipped_pl_fline_r( pl,
			xmin, xmax, ymin, ymax,
			PyArray_1D(x,i), PyArray_1D(y,i),
			PyArray_1D(x,i+1), PyArray_1D(y,i+1) );
	}
	pl_endpath_r( pl );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_INCREF( Py_None );
	return Py_None;
}

/*****************************************************************************
 *  module init
 */

static PyMethodDef LibplotMethods[] = 
{
	{ "new", new, METH_VARARGS },

	/* () */
	{ "clear", clear, METH_VARARGS },
	{ "end_page", end_page, METH_VARARGS },
	{ "delete", delete, METH_VARARGS },
	{ "flush", flush, METH_VARARGS },
	{ "gsave", gsave, METH_VARARGS },
	{ "grestore", grestore, METH_VARARGS },
	{ "begin_page", begin_page, METH_VARARGS },

	/* (i) */
	{ "set_fill_level", set_fill_level, METH_VARARGS },

	/* (d) */
	{ "set_font_size", set_font_size, METH_VARARGS },
	{ "set_line_size", set_line_size, METH_VARARGS },
	{ "set_string_angle", set_string_angle, METH_VARARGS },

	/* (dd) */
	{ "move", move, METH_VARARGS },
	{ "lineto", lineto, METH_VARARGS },
	{ "linetorel", linetorel, METH_VARARGS },

	/* (ddd) */
	{ "circle", circle, METH_VARARGS },

	/* (dddd) */
	{ "line", line, METH_VARARGS },
	{ "rect", rect, METH_VARARGS },
	{ "space", space, METH_VARARGS },

	/* (ddddd) */
	{ "ellipse", ellipse, METH_VARARGS },

	/* (dddddd) */
	{ "arc", arc, METH_VARARGS },

	/* (dddddddd) */
	{ "clipped_line", clipped_line, METH_VARARGS },

	/* (s) */
	{ "set_colorname_bg", set_colorname_bg, METH_VARARGS },
	{ "set_colorname_fg", set_colorname_fg, METH_VARARGS },
	{ "set_colorname_fill", set_colorname_fill, METH_VARARGS },
	{ "set_colorname_pen", set_colorname_pen, METH_VARARGS },
	{ "set_fill_type", set_fill_type, METH_VARARGS },
	{ "set_font_type", set_font_type, METH_VARARGS },
	{ "set_join_type", set_join_type, METH_VARARGS },
	{ "set_line_type", set_line_type, METH_VARARGS },

	/* color */
	{ "set_color_bg", set_color_bg, METH_VARARGS },
	{ "set_color_fg", set_color_fg, METH_VARARGS },
	{ "set_color_fill", set_color_fill, METH_VARARGS },
	{ "set_color_pen", set_color_pen, METH_VARARGS },

	{ "string", string, METH_VARARGS },
	{ "get_string_width", get_string_width, METH_VARARGS },

	{ "symbols", symbols, METH_VARARGS },
	{ "curve", curve, METH_VARARGS },

	{ "clipped_symbols", clipped_symbols, METH_VARARGS },
	{ "clipped_curve", clipped_curve, METH_VARARGS },

	{ "clipped_colored_symbols", clipped_colored_symbols, METH_VARARGS },
	{ "density_plot",	density_plot,		METH_VARARGS },
	{ "color_density_plot", color_density_plot,	METH_VARARGS },

	{ NULL, NULL }
};

void
initlibplot( void )
{
	Py_InitModule( "libplot", LibplotMethods );
	import_array();
}
