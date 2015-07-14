/*
 * $Id: libplot.c,v 1.12 2010/04/09 20:54:17 mrnolta Exp $ 
 *
 * Copyright (C) 2001 Mike Nolta <mike@nolta.net>
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

#define BGL_DArray1(v,i)\
	(*(double *)PyArray_GETPTR1(v,i))

#define BGL_DArray2(v,i,j)\
	(*(double *)PyArray_GETPTR2(v,i,j))

#define BGL_DArray3(v,i,j,k)\
	(*(double *)PyArray_GETPTR3(v,i,j,k))



#define BGL_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define BGL_MAX(a,b) (((a) > (b)) ? (a) : (b))

/*****************************************************************************
 *  clipping code
 */

#define BGL_TOP	0x1
#define BGL_BOTTOM	0x2
#define BGL_RIGHT	0x4
#define BGL_LEFT	0x8


/*

   This is the python object wrapping a plPlotter

*/
struct PyLibPlot {
    PyObject_HEAD

    char type[25];
    FILE* fptr; // for writing to files
    plPlotter *pl;
};

/*
   Deal with all the python3 vs python2 string issues

   PyBytes has been backported to newer python2 as a fake alias, but doesn't
   exist for older versions.

*/
static int set_param_from_key_value(plPlotterParams *params,
                                     PyObject *key,
                                     PyObject *value)
{
    int status=0;
	char *skey=NULL, *svalue=NULL;

#if PY_MAJOR_VERSION >= 3

    PyObject *tmp1, *tmp2;

    tmp1 = PyObject_CallMethod(key,"encode",NULL);
    if (!tmp1) {
        goto bail;
    }
    skey = PyBytes_AsString(tmp1);

    tmp2 = PyObject_CallMethod(value,"encode",NULL);
    if (!tmp2) {
        goto bail;
    }
    svalue = PyBytes_AsString(tmp2);

#else

    skey = PyString_AsString( key );
    if (!skey) {
        goto bail;
    }
    svalue = PyString_AsString( value );
    if (!svalue) {
        goto bail;
    }

#endif

    pl_setplparam( params, skey, svalue );

#if PY_MAJOR_VERSION >= 3
    Py_XDECREF(tmp1);
    Py_XDECREF(tmp2);
#endif

    status=1;

bail:
    return status;
}

static int extract_params(PyObject* params_dict, plPlotterParams *params)
{
    int status=0;
	PyObject *key=NULL, *value=NULL;
    Py_ssize_t pos=0;

	if ( PyDict_Check(params_dict) ) {
		while ( PyDict_Next( params_dict, &pos, &key, &value ) ) {
            if (!set_param_from_key_value(params, key, value)) {
                goto bail;
            }
		}
        status=1;
	} else if ( params_dict != Py_None ) {
		PyErr_SetString( PyExc_TypeError, "params are not a dict" );
	}

bail:
    return status;
}

static int
PyLibPlot_init(struct PyLibPlot* self, PyObject *args, PyObject *kwds)
{
    int status=0;
	PyObject *params_dict=NULL;
	char *type=NULL;
    char *filename=NULL;
	plPlotterParams *params;

    self->fptr=NULL;

	if ( !PyArg_ParseTuple( args, "sOs", &type, &params_dict, &filename) ) {
		return -1;
    }

    // for the repr
    snprintf(self->type, sizeof(self->type), "%s", type);

	params = pl_newplparams();

    if (!extract_params(params_dict, params)) {
        // exception set inside
        goto bail;
    }

    if (0 != strcmp(filename, "")) {
        // a filename was passed
        self->fptr = fopen(filename,"w");
        if (self->fptr==NULL) {
            fprintf(stderr,"error opening %s\n", filename);
            PyErr_Format( PyExc_TypeError, "could not open file: %s", filename);
            goto bail;
        }
    }

	self->pl = pl_newpl_r( type, NULL, self->fptr, NULL, params );
	if (!self->pl) {
		PyErr_SetString(PyExc_RuntimeError, "could not create plotter");
        goto bail;
	}

    status=1;

bail:

	pl_deleteplparams( params );

    if (!status) {
        return -1;
    } else {
        return 0;
    }

}

static void
PyLibPlot_dealloc(struct PyLibPlot* self)
{
    if (self->pl) {
        pl_deletepl_r(self->pl);
    }

    // must be closed *after* deleting the plPlotter
    if (self->fptr != NULL) {
        fclose(self->fptr);
    }

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif

}

static PyObject *
PyLibPlot_repr(struct PyLibPlot* self) {
    char repr[255];

    sprintf(repr,"plPlotter for output to '%s'", self->type);

#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)repr);
#else
    return PyString_FromString((const char*)repr);
#endif
}



static unsigned char
outcode( double x, double y,
	double xmin, double xmax, double ymin, double ymax )
{
	unsigned char code = 0x0;

	if ( x < xmin ) code |= BGL_LEFT;
	if ( x > xmax ) code |= BGL_RIGHT;
	if ( y < ymin ) code |= BGL_BOTTOM;
	if ( y > ymax ) code |= BGL_TOP;

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

			if ( (out & BGL_TOP) != 0 )
			{
				x = x0 + (x1 - x0)*(ymax - y0)/(y1 - y0);
				y = ymax;
			}
			else if ( (out & BGL_BOTTOM) != 0 )
			{
				x = x0 + (x1 - x0)*(ymin - y0)/(y1 - y0);
				y = ymin;
			}
			else if ( (out & BGL_RIGHT) != 0 )
			{
				y = y0 + (y1 - y0)*(xmax - x0)/(x1 - x0);
				x = xmax;
			}
			else if ( (out & BGL_LEFT) != 0 )
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


/******************************************************************************
 */

#define BGL_PL_FUNC(NAME,FUNCTION)   \
static PyObject *                    \
NAME (struct PyLibPlot *self)        \
{                                    \
	FUNCTION(self->pl);              \
    Py_RETURN_NONE;                  \
}

BGL_PL_FUNC( clear, pl_erase_r )
BGL_PL_FUNC( end_page, pl_closepl_r )
BGL_PL_FUNC( flush, pl_flushpl_r )
BGL_PL_FUNC( gsave, pl_savestate_r )
BGL_PL_FUNC( grestore, pl_restorestate_r )
//BGL_PL_FUNC( begin_page, pl_openpl_r )


static PyObject* begin_page(struct PyLibPlot *self)
{
    if (pl_openpl_r(self->pl) < 0) {
        fprintf(stderr,"Couldn't open device: '%s'.  If device is X, this is an irrecoverable error\n", self->type );
        PyErr_Format( PyExc_IOError, "Couldn't open device: '%s'", self->type );
        return NULL;
    }

    Py_RETURN_NONE;
}

/******************************************************************************
 */

#define BGL_PL_FUNC_I(NAME,FUNCTION)                 \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	int i0;                                          \
                                                     \
	if ( !PyArg_ParseTuple( args, "i", &i0 ) )       \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl, i0);                          \
    Py_RETURN_NONE;                                  \
}

BGL_PL_FUNC_I( set_fill_level, pl_filltype_r )

/******************************************************************************
 */

#define BGL_PL_FUNC_D(NAME,FUNCTION)                 \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0;                                       \
                                                     \
	if ( !PyArg_ParseTuple( args, "d", &d0 ) )       \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl, d0);                          \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DD(NAME,FUNCTION)                \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0, d1;                                   \
                                                     \
	if ( !PyArg_ParseTuple( args, "dd", &d0, &d1 ) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl, d0, d1);                      \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DDD(NAME,FUNCTION)               \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0, d1, d2;                               \
                                                     \
	if ( !PyArg_ParseTuple( args, "ddd", &d0,&d1,&d2) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl, d0, d1, d2);                  \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DDDD(NAME,FUNCTION)              \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0,d1,d2,d3;                              \
                                                     \
	if ( !PyArg_ParseTuple( args, "dddd", &d0,&d1,&d2,&d3) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl,d0,d1,d2,d3);                  \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DDDDD(NAME,FUNCTION)             \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0,d1,d2,d3,d4;                           \
                                                     \
	if ( !PyArg_ParseTuple( args, "ddddd", &d0,&d1,&d2,&d3,&d4) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl,d0,d1,d2,d3,d4);               \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DDDDDD(NAME,FUNCTION)            \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0,d1,d2,d3,d4,d5;                        \
                                                     \
	if ( !PyArg_ParseTuple( args, "dddddd", &d0,&d1,&d2,&d3,&d4,&d5) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl,d0,d1,d2,d3,d4,d5);            \
    Py_RETURN_NONE;                                  \
}

#define BGL_PL_FUNC_DDDDDDDD(NAME,FUNCTION)          \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	double d0,d1,d2,d3,d4,d5,d6,d7;                  \
                                                     \
	if ( !PyArg_ParseTuple( args, "dddddddd", &d0,&d1,&d2,&d3,&d4,&d5,&d6,&d7) ) \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl,d0,d1,d2,d3,d4,d5,d6,d7);      \
    Py_RETURN_NONE;                                  \
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

#define BGL_PL_FUNC_S(NAME,FUNCTION)                 \
static PyObject *                                    \
NAME (struct PyLibPlot *self, PyObject *args)        \
{                                                    \
	char* s0;                                        \
                                                     \
	if ( !PyArg_ParseTuple( args, "s", &s0 ) )       \
		return NULL;                                 \
                                                     \
	FUNCTION(self->pl, (const char*) s0);            \
    Py_RETURN_NONE;                                  \
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

#define BGL_PL_FUNC_COLOR(NAME,FUNCTION)                       \
static PyObject *                                              \
NAME(struct PyLibPlot *self, PyObject *args)                   \
{                                                              \
	double d0, d1, d2;                                         \
	int r, g, b;                                               \
                                                               \
	if ( !PyArg_ParseTuple( args, "ddd", &d0, &d1, &d2 ) )     \
		return NULL;                                           \
                                                               \
	r = (int) floor( d0*65535 );                               \
	g = (int) floor( d1*65535 );                               \
	b = (int) floor( d2*65535 );                               \
                                                               \
	FUNCTION(self->pl, r, g, b );                              \
    Py_RETURN_NONE;                                            \
}

BGL_PL_FUNC_COLOR( set_color_bg, pl_bgcolor_r )
BGL_PL_FUNC_COLOR( set_color_fg, pl_color_r )
BGL_PL_FUNC_COLOR( set_color_fill, pl_fillcolor_r )
BGL_PL_FUNC_COLOR( set_color_pen, pl_pencolor_r )

/******************************************************************************
 */

static PyObject *
string(struct PyLibPlot *self, PyObject *args)
{
	int i0, i1;
	char *s0;

	if ( !PyArg_ParseTuple( args, "iis", &i0, &i1, &s0 ) )
		return NULL;

	pl_alabel_r(self->pl, i0, i1, s0 );
    Py_RETURN_NONE;
}

static PyObject *
get_string_width(struct PyLibPlot *self, PyObject *args)
{
	char *s0;
	double width;

	if ( !PyArg_ParseTuple( args, "s", &s0 ) )
		return NULL;

	width = pl_flabelwidth_r(self->pl, s0 );
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
symbols(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ox, *oy;
	PyObject *x, *y;
	double d0;
	int i0;
    npy_intp i, n;

	if ( !PyArg_ParseTuple( args, "OOid", &ox, &oy, &i0, &d0 ) )
		return NULL;

	x = PyArray_ContiguousFromAny( ox, NPY_DOUBLE, 1, 1 );
	y = PyArray_ContiguousFromAny( oy, NPY_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = BGL_MIN( PyArray_SIZE(x), PyArray_SIZE(y) );

	_symbol_begin( self->pl, i0, d0 );

	for ( i = 0; i < n; i++ )
		_symbol_draw( self->pl, BGL_DArray1(x,i), BGL_DArray1(y,i), i0, d0 );

	_symbol_end( self->pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
    Py_RETURN_NONE;
}

static PyObject *
clipped_symbols(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ox, *oy;
	PyObject *x, *y;
	double xmin, xmax, ymin, ymax;
	double d0;
	int i0;
    npy_intp i, n;
	double px, py;

	if ( !PyArg_ParseTuple( args, "OOiddddd", &ox, &oy,
			&i0, &d0, &xmin, &xmax, &ymin, &ymax ) )
		return NULL;

	x = PyArray_ContiguousFromAny( ox, NPY_DOUBLE, 1, 1 );
	y = PyArray_ContiguousFromAny( oy, NPY_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = BGL_MIN( PyArray_SIZE(x), PyArray_SIZE(y) );

	_symbol_begin( self->pl, i0, d0 );

	for ( i = 0; i < n; i++ )
	{
		px = BGL_DArray1(x,i);
		py = BGL_DArray1(y,i);

		if ( px >= xmin && px <= xmax &&
		     py >= ymin && py <= ymax )
			_symbol_draw( self->pl, px, py, i0, d0 );
	}

	_symbol_end( self->pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
    Py_RETURN_NONE;
}

static PyObject *
clipped_colored_symbols(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ox, *oy, *oc;
	PyObject *x, *y, *c;
	double xmin, xmax, ymin, ymax;
	double d0;
	int i0;
    npy_intp i, n;
	double px, py;
	int r, g, b;

	if ( !PyArg_ParseTuple( args, "OOOiddddd", &ox, &oy, &oc,
			&i0, &d0, &xmin, &xmax, &ymin, &ymax ) )
		return NULL;

	x = PyArray_ContiguousFromAny( ox, NPY_DOUBLE, 1, 1 );
	y = PyArray_ContiguousFromAny( oy, NPY_DOUBLE, 1, 1 );
	c = PyArray_ContiguousFromAny( oc, NPY_DOUBLE, 2, 2 );

	if ( x == NULL || y == NULL || c == NULL )
		goto quit;

	n = BGL_MIN( PyArray_SIZE(x), PyArray_SIZE(y) );
	n = BGL_MIN( n, PyArray_SIZE(c) );
	
	_symbol_begin( self->pl, i0, d0 );

	for ( i = 0; i < n; i++ )
	{
		px = BGL_DArray1(x,i);
		py = BGL_DArray1(y,i);

		if ( px >= xmin && px <= xmax &&
		     py >= ymin && py <= ymax ) {
			r = (int) floor( BGL_DArray2(c,i,0)*65535 );
			g = (int) floor( BGL_DArray2(c,i,1)*65535 );
			b = (int) floor( BGL_DArray2(c,i,2)*65535 );
			pl_fillcolor_r( self->pl, r, g, b );
			pl_pencolor_r(  self->pl, r, g, b );
		  
			_symbol_draw( self->pl, px, py, i0, d0 );
		}
	}

	_symbol_end( self->pl, i0, d0 );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
	Py_XDECREF(c);
    Py_RETURN_NONE;
}



static PyObject *
curve(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ox, *oy;
	PyObject *x, *y;
	npy_intp i, n;

	if ( !PyArg_ParseTuple( args, "OO", &ox, &oy ) )
		return NULL;

	x = PyArray_ContiguousFromAny( ox, NPY_DOUBLE, 1, 1 );
	y = PyArray_ContiguousFromAny( oy, NPY_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = BGL_MIN( PyArray_SIZE(x), PyArray_SIZE(y) );
	if ( n <= 0 )
		goto quit;

	pl_fmove_r( self->pl, BGL_DArray1(x,0), BGL_DArray1(y,0) );
	for ( i = 1; i < n; i++ )
		pl_fcont_r( self->pl, BGL_DArray1(x,i), BGL_DArray1(y,i) );
	pl_endpath_r( self->pl );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
    Py_RETURN_NONE;
}

static PyObject *
clipped_curve(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ox, *oy;
	PyObject *x, *y;
	double xmin, xmax, ymin, ymax;
	npy_intp i, n;

	if ( !PyArg_ParseTuple( args, "OOdddd", &ox, &oy,
			&xmin, &xmax, &ymin, &ymax ) )
		return NULL;

	x = PyArray_ContiguousFromAny( ox, NPY_DOUBLE, 1, 1 );
	y = PyArray_ContiguousFromAny( oy, NPY_DOUBLE, 1, 1 );

	if ( x == NULL || y == NULL )
		goto quit;

	n = BGL_MIN( PyArray_SIZE(x), PyArray_SIZE(y) );
	if ( n <= 0 )
		goto quit;

	for ( i = 0; i < n-1; i++ )
	{
		clipped_pl_fline_r( self->pl,
			xmin, xmax, ymin, ymax,
			BGL_DArray1(x,i), BGL_DArray1(y,i),
			BGL_DArray1(x,i+1), BGL_DArray1(y,i+1) );
	}
	pl_endpath_r( self->pl );

quit:
	Py_XDECREF(x);
	Py_XDECREF(y);
    Py_RETURN_NONE;
}


/*
 * Draw a density plot --
 *   Given a grid of intensity values, plot uniform squares tiling
 *   the region (xmin, ymin) to (xmax, ymax).
 *
 */

static PyObject *
density_plot(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ogrid;
	PyObject *grid;
	double xmin, xmax, ymin, ymax;
	
	double px, py, dx, dy;
	npy_intp xi, yi, xn, yn;
	int    r, g, b;

	if ( !PyArg_ParseTuple( args, "Odddd", &ogrid,
				&xmin, &xmax, &ymin, &ymax ) )
		return NULL;

	grid = PyArray_ContiguousFromAny( ogrid, NPY_DOUBLE, 2, 2 );

	if ( grid == NULL )
		goto quit;

	if ( PyArray_NDIM(grid) != 2) {
		printf("Expect a NxM array for densgrid");
		goto quit;
	}
	
	xn = PyArray_DIM(grid, 0);
	yn = PyArray_DIM(grid, 1);
	dx = (xmax - xmin) / xn;
	dy = (ymax - ymin) / yn;
	
	for   ( xi=0, px=xmin; xi < xn; xi++, px+=dx ) {
	  for ( yi=0, py=ymin; yi < yn; yi++, py+=dy ) {
	    r=g=b = (int) floor( BGL_DArray2(grid,xi,yi)*65535 );
	    pl_filltype_r ( self->pl, 1.0 );
	    pl_fillcolor_r( self->pl, r, g, b );
	    pl_pencolor_r ( self->pl, r, g, b );

	    pl_fbox_r( self->pl, px, py, px+dx, py+dy );
	  }
	}

quit:
	Py_XDECREF(grid);
    Py_RETURN_NONE;
}

static PyObject *
color_density_plot(struct PyLibPlot *self, PyObject *args)
{
	PyObject *ogrid;
	PyObject *grid;
	double xmin, xmax, ymin, ymax;
	
	double px, py, dx, dy;
	npy_intp xi, yi, xn, yn;
	int    r, g, b;

	if ( !PyArg_ParseTuple( args, "Odddd", &ogrid,
				&xmin, &xmax, &ymin, &ymax ) )
		return NULL;

	grid = PyArray_ContiguousFromAny( ogrid, NPY_DOUBLE, 3, 3 );

	if ( grid == NULL )
		goto quit;

	if ( PyArray_NDIM(grid) != 3) {
		printf("Expect a NxMx3 array for color densgrid");
		goto quit;
	}
	
	xn = PyArray_DIM(grid, 0);
	yn = PyArray_DIM(grid, 1);
	dx = (xmax - xmin) / xn;
	dy = (ymax - ymin) / yn;
	
	for   ( xi=0, px=xmin; xi < xn; xi++, px+=dx ) {
	  for ( yi=0, py=ymin; yi < yn; yi++, py+=dy ) {
	    r = (int) floor( BGL_DArray3(grid,xi,yi,0)*65535 );
	    g = (int) floor( BGL_DArray3(grid,xi,yi,1)*65535 );
	    b = (int) floor( BGL_DArray3(grid,xi,yi,2)*65535 );
	    pl_filltype_r ( self->pl, 1.0 );
	    pl_fillcolor_r( self->pl, r, g, b );
	    pl_pencolor_r ( self->pl, r, g, b );

	    pl_fbox_r( self->pl, px, py, px+dx, py+dy );
	  }
	}

quit:
	Py_XDECREF(grid);
    Py_RETURN_NONE;
}

/*****************************************************************************
 *  module init
 */

static PyMethodDef PyLibPlot_methods[] = {

	// ()
	{ "clear", (PyCFunction)clear, METH_NOARGS ,""},
	{ "end_page", (PyCFunction)end_page, METH_NOARGS ,""},
	{ "flush", (PyCFunction)flush, METH_NOARGS ,""},
	{ "gsave", (PyCFunction)gsave, METH_NOARGS ,""},
	{ "grestore", (PyCFunction)grestore, METH_NOARGS ,""},
	{ "begin_page", (PyCFunction)begin_page, METH_NOARGS, "open page on device" },

	// (i)
	{ "set_fill_level", (PyCFunction)set_fill_level, METH_VARARGS ,""},

	// (d)
	{ "set_font_size", (PyCFunction)set_font_size, METH_VARARGS ,""},
	{ "set_line_size", (PyCFunction)set_line_size, METH_VARARGS ,""},
	{ "set_string_angle", (PyCFunction)set_string_angle, METH_VARARGS ,""},

	// (dd)
	{ "move", (PyCFunction)move, METH_VARARGS ,""},
	{ "lineto", (PyCFunction)lineto, METH_VARARGS ,""},
	{ "linetorel", (PyCFunction)linetorel, METH_VARARGS ,""},

	// (ddd)
	{ "circle", (PyCFunction)circle, METH_VARARGS ,""},

	// (dddd)
	{ "line", (PyCFunction)line, METH_VARARGS ,""},
	{ "rect", (PyCFunction)rect, METH_VARARGS ,""},
	{ "space", (PyCFunction)space, METH_VARARGS ,""},

	// (ddddd)
	{ "ellipse", (PyCFunction)ellipse, METH_VARARGS ,""},

	// (dddddd)
	{ "arc", (PyCFunction)arc, METH_VARARGS ,""},

	// (dddddddd)
	{ "clipped_line", (PyCFunction)clipped_line, METH_VARARGS ,""},

	// (s)
	{ "set_colorname_bg", (PyCFunction)set_colorname_bg, METH_VARARGS ,""},
	{ "set_colorname_fg", (PyCFunction)set_colorname_fg, METH_VARARGS ,""},
	{ "set_colorname_fill", (PyCFunction)set_colorname_fill, METH_VARARGS ,""},
	{ "set_colorname_pen", (PyCFunction)set_colorname_pen, METH_VARARGS ,""},
	{ "set_fill_type", (PyCFunction)set_fill_type, METH_VARARGS ,""},
	{ "set_font_type", (PyCFunction)set_font_type, METH_VARARGS ,""},
	{ "set_join_type", (PyCFunction)set_join_type, METH_VARARGS ,""},
	{ "set_line_type", (PyCFunction)set_line_type, METH_VARARGS ,""},

	// color
	{ "set_color_bg", (PyCFunction)set_color_bg, METH_VARARGS ,""},
	{ "set_color_fg", (PyCFunction)set_color_fg, METH_VARARGS ,""},
	{ "set_color_fill", (PyCFunction)set_color_fill, METH_VARARGS ,""},
	{ "set_color_pen", (PyCFunction)set_color_pen, METH_VARARGS ,""},

	{ "string", (PyCFunction)string, METH_VARARGS ,""},
	{ "get_string_width", (PyCFunction)get_string_width, METH_VARARGS ,""},

	{ "symbols", (PyCFunction)symbols, METH_VARARGS ,""},
	{ "clipped_symbols", (PyCFunction)clipped_symbols, METH_VARARGS ,""},
	{ "clipped_colored_symbols", (PyCFunction)clipped_colored_symbols, METH_VARARGS ,""},

	{ "curve", (PyCFunction)curve, METH_VARARGS ,""},
	{ "clipped_curve", (PyCFunction)clipped_curve, METH_VARARGS ,""},

	{ "density_plot",	(PyCFunction)density_plot,		METH_VARARGS ,""},
	{ "color_density_plot", (PyCFunction)color_density_plot,	METH_VARARGS ,""},

    {NULL}  /* Sentinel */
};


static PyTypeObject PyLibPlotType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_libplot_pywrap.Plotter",             /*tp_name*/
    sizeof(struct PyLibPlot), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyLibPlot_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyLibPlot_repr,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A class to work with libplot plPlotter.",
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyLibPlot_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyLibPlot_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};

/* module functions, not methods of the type */
static PyMethodDef libplot_methods[] = {
    {NULL}  /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_libplot_pywrap",      /* m_name */
        "Defines the PyLibPlot class and some methods",  /* m_doc */
        -1,                  /* m_size */
        libplot_methods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit__libplot_pywrap(void) 
#else
init_libplot_pywrap(void) 
#endif
{
    PyObject* m;

    PyLibPlotType.tp_new = PyType_GenericNew;

#if PY_MAJOR_VERSION >= 3
    if (PyType_Ready(&PyLibPlotType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&moduledef);
    if (m==NULL) {
        return NULL;
    }

#else
    if (PyType_Ready(&PyLibPlotType) < 0) {
        return;
    }
    m = Py_InitModule3("_libplot_pywrap", libplot_methods, 
            "This module defines a class to work with a plPlotter.\n");
    if (m==NULL) {
        return;
    }
#endif

    Py_INCREF(&PyLibPlotType);
    PyModule_AddObject(m, "Plotter", (PyObject *)&PyLibPlotType);

    import_array();
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
