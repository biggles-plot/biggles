#
# $Id: contour.py,v 1.23 2008/01/29 04:08:31 mrnolta Exp $
#
# Copyright (C) 2001 Mike Nolta <mike@nolta.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#

from . import biggles
from .biggles import \
        _series, _message, _range, \
        _LineComponent,  _PathObject, _PlotComponent, BigglesError
from .geometry import *
from . import _biggles

import numpy

def _span( a, b, n ):
    return a + float(b - a)*numpy.arange( 0, n, 1, numpy.Float )/(n-1)

def _pop2( x, i, j ):
    if i < j:
        b = x.pop( j )
        a = x.pop( i )
    elif i > j:
        a = x.pop( i )
        b = x.pop( j )
    return a, b

def _unzip( line ):
    x = []
    y = []
    for x0,y0 in line:
        x.append( x0 )
        y.append( y0 )
    return x, y

class Contour( _LineComponent ):
    """
    Object representing a contour
    TODO document args
    """
    def __init__( self, x, y, z, z0, **kw ):
        _LineComponent.__init__( self )
        self.kw_init( kw )
        self.x = x
        self.y = y
        self.z = z
        self.z0 = z0

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def _get_contours( self ):
        segs = _biggles.contour_segments( \
                self.x, self.y, self.z, self.z0 )
        open = [[ segs[0][0], segs[0][1] ]]
        closed = []
        for a,b in segs[1:]:
            xxx = []
            for i in range(len(open)):
                begin = open[i][0]
                end = open[i][-1]
                if a == begin:
                    xxx.append( (i,0,b) )
                elif a == end:
                    xxx.append( (i,1,b) )
                if b == begin:
                    xxx.append( (i,0,a) )
                elif b == end:
                    xxx.append( (i,1,a) )
            if len(xxx) == 0:
                open.append( [a,b] )
            elif len(xxx) == 1:
                i,end,pt = xxx[0]
                if end == 0:
                    open[i].insert( 0, pt )
                else:
                    open[i].append( pt )
            elif len(xxx) == 2:
                i0,end0,pt0 = xxx[0]
                i1,end1,pt1 = xxx[1]
                if i0 == i1:
                    # closed
                    l0 = open.pop( i0 )
                    l0.append( l0[0] )
                    closed.append( l0 )
                else:
                    l0, l1 = _pop2( open, i0, i1 )
                    m = None
                    if end0==1 and end1==0:
                        m = l0 + l1
                    elif  end0==0 and end1==1:
                        m = l1 + l0
                    elif end0==0 and end1==0:
                        l0.reverse()
                        m = l0 + l1
                    elif end0==1 and end1==1:
                        l1.reverse()
                        m = l0 + l1
                    if m is not None:
                        open.append( m )
                    else:
                        _message( "contour: m is None" )
            elif len(xxx) > 2:
                _message( "contour: len(xxx) > 2" )
        return open + closed

    def make( self, context ):
        lines = self._get_contours()
        for line in lines:
            x, y = _unzip( line )
            u, v = context.geom.call_vec( x, y )
            self.add( _PathObject(u, v) )

def _func_color_black( i, n, z0, z_min, z_max ):
    return 0x000000

def _func_linetype_dotneg( i, n, z0, z_min, z_max ):
    if z0 < 0:
        return "dotted"
    return "solid"

def _func_linewidth_placeholder( i, n, z0, z_min, z_max ):
    return 1

class Contours( _PlotComponent ):
    """
    Create a 2-d contour plot object

    parameters
    ----------
    z: 2d array
            The image or density grid.
    x: array, optional
            x values along the column direction
    y: array, optional
            y values along the row direction
    zrange: optional
            ?? what is this?

    **keywords
            Style and other keywords for the Contours.

            See the configuration options for Contours for details (TODO copy
            into here)
    """

    _named_func_color = {
            "black"                 : _func_color_black,
    }

    _named_func_linetype = {
            "dotted-negative"       : _func_linetype_dotneg,
    }

    _named_func_linewidth = {
            "placeholder"           : _func_linewidth_placeholder,
    }

    def __init__( self, z, x=None, y=None, zrange=None, **kw ):
        super(Contours,self).__init__()
        #apply( self.conf_setattr, ("Contours",), kw )
        self.conf_setattr( "Contours" )
        self.kw_init( kw )
        self.z = z
        self.x = x
        self.y = y
        self.zrange = zrange

    def _get_coords( self ):
        dim = self.z.shape
        x = self.x
        if x is None:
            x = range(dim[0])
        y = self.y
        if y is None:
            y = range(dim[1])
        return x, y, self.z

    def limits( self ):
        x, y, z = self._get_coords()
        return BoundingBox( (min(x),min(y)), (max(x),max(y)) )

    def make( self, context ):
        self.clear()

        x, y, z = self._get_coords()
        limits = self.limits()
        xr = limits.xrange()
        yr = limits.yrange()

        zr = self.zrange
        if zr is None:
            zr = _range( z )

        levels = self.levels
        if type(levels) == type(0):
            levels = _series( 1, self.levels, \
                    float(zr[1]-zr[0])/(self.levels+1), zr[0] )

        colorfunc = self.func_color
        if type(colorfunc) == type(""):
            colorfunc = self._named_func_color[colorfunc]

        linefunc = self.func_linetype
        if type(linefunc) == type(""):
            linefunc = self._named_func_linetype[linefunc]

        widthfunc = self.func_linewidth
        if type(widthfunc) == type(""):
            widthfunc = self._named_func_linewidth[widthfunc]

        nlevels = len(levels)
        for i in range(nlevels):
            kw = {}
            z0 = levels[i]
            args = i, nlevels, z0, zr[0], zr[1]
            if colorfunc is not None:
                color = colorfunc( *args )
                #color = apply( colorfunc, args )
                if color is not None:
                    kw["color"] = color
            if linefunc is not None:
                linetype = linefunc( *args )
                #linetype = apply( linefunc, args )
                if linetype is not None:
                    kw["linetype"] = linetype
            if widthfunc is not None:
                linewidth = widthfunc( *args )
                #linewidth = apply( widthfunc, args )
                if linewidth is not None:
                    kw["linewidth"] = linewidth
            c = Contour(x, y, z, z0, **kw )
            #c = apply( Contour, (x, y, z, z0), kw )
            self.add( c )

    def make_key( self, bbox ):
        xr = bbox.xrange()
        y = bbox.center()[1]
        p = xr[0], y
        q = xr[1], y
        return _LineObject(p,q, **self.kw_style )
        #return apply( _LineObject, (p,q), self.kw_style )
