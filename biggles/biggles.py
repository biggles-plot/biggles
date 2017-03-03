#
# $Id: biggles.py,v 1.235 2008/11/28 00:38:20 mrnolta Exp $
#
# Copyright (C) 2000-2008 Mike Nolta <mike@nolta.net>
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

from __future__ import print_function
import copy, math, os
import numpy
import tempfile

from . import config, _biggles
from geometry import *

from .libplot import renderer

# miscellaneous ---------------------------------------------------------------

_true, _false = 1, 0

def X11_is_running():
    from subprocess import Popen, PIPE

    #p = Popen(["xset", "-q"], stdout=PIPE, stderr=PIPE)
    p = Popen(["xdpyinfo"], stdout=PIPE, stderr=PIPE)
    p.communicate()
    return p.returncode == 0

def _range(x):
    x=numpy.array(x, copy=False)
    return x.min(), x.max()

def _floor(x):
    return long(math.floor(x))

def _ceil(x):
    return long(math.ceil(x))

def _tsil(x):
    l = list(x)
    l.reverse()
    return l

def _message( s ):
    print( "biggles: %s" % s )

def _series( m, n, a=1, b=0 ):
    return map( lambda x,y=a,z=b: x*y+z, range(m, n+1) )

def _first_not_none( *args ):
    for arg in args:
        if arg is not None:
            return arg
    return None

class _Alias(object):

    def __init__( self, *args ):
        self.__dict__["objs"] = args[:]

    def __call__( self, *args, **kw ):
        for obj in self.objs:
            obj( *args, **kw )

    def __getattr__( self, name ):
        objs = []
        for obj in self.objs:
            objs.append( getattr(obj, name) )
        return _Alias( *objs )

    def __setattr__( self, name, value ):
        for obj in self.objs:
            setattr( obj, name, value )

    def __setitem__( self, key, value ):
        for obj in self.objs:
            obj[key] = value

# BigglesError ----------------------------------------------------------------

class BigglesError( Exception ):
    pass

# relative size ---------------------------------------------------------------

def _size_relative( relsize, bbox ):
    w = bbox.width()
    h = bbox.height()
    yardstick = math.sqrt(8) * w * h / (w + h)
    return (relsize/100.) * yardstick

def _fontsize_relative( relsize, bbox, device ):
    devsize = _size_relative( relsize, bbox )
    devsize_min = _size_relative(
            config.value('default','fontsize_min'), device.bbox )
    return max( devsize, devsize_min )

# _PlotContext -----------------------------------------------------------------

class _PlotGeometry(object):

    _logfunc = math.log10
    _logfunc_vec = numpy.log10

    def __init__( self, src, dest, xlog=0, ylog=0 ):
        self.src_bbox = src
        self.dest_bbox = dest
        self.xlog = xlog
        self.ylog = ylog
        a, b = src.lowerleft()
        c, d = src.upperright()
        if xlog:
            a = self._logfunc(a)
            c = self._logfunc(c)
        if ylog:
            b = self._logfunc(b)
            d = self._logfunc(d)
        fsrc = BoundingBox( (a,b), (c,d) )
        self.aff = RectilinearMap( fsrc, dest )

    def __call__( self, x, y ):
        u, v = x, y
        if self.xlog: u = self._logfunc(x)
        if self.ylog: v = self._logfunc(y)
        return self.aff( u, v )

    def call_vec( self, x, y ):
        u = numpy.asarray( x )
        v = numpy.asarray( y )
        if self.xlog: u = self._logfunc_vec(u)
        if self.ylog: v = self._logfunc_vec(v)
        return self.aff.call_vec( u, v )

    def geodesic( self, x, y, div=1 ):
        return [(x, y)]

class _PlotContext(object):

    def __init__( self, device, dev, data, xlog=0, ylog=0 ):
        self.draw = device
        self.dev_bbox = dev
        self.data_bbox = data
        self.xlog = xlog
        self.ylog = ylog
        self.geom = _PlotGeometry( data, dev, xlog=xlog, ylog=ylog )
        self.plot_geom = _PlotGeometry( BoundingBox((0,0),(1,1)), dev )

    def do_clip( self ):
        xr = self.dev_bbox.xrange()
        yr = self.dev_bbox.yrange()
        self.draw.set( "cliprect", (xr[0], xr[1], yr[0], yr[1]) )

# _StyleKeywords --------------------------------------------------------------

def _kw_func_relative_fontsize( context, key, value ):
    device_size = _fontsize_relative( value, context.dev_bbox, context.draw )
    context.draw.set( key, device_size )

def _kw_func_relative_size( context, key, value ):
    device_size = _size_relative( value, context.dev_bbox )
    context.draw.set( key, device_size )

def _kw_func_relative_width( context, key, value ):
    device_width = _size_relative( value/10., context.dev_bbox )
    context.draw.set( key, device_width )

class _StyleKeywords(object):

    kw_style = None
    kw_defaults = {}
    kw_rename = {}
    kw_func = {
            'fontsize' : _kw_func_relative_fontsize,
            'linewidth' : _kw_func_relative_width,
            'symbolsize' : _kw_func_relative_size,
    }

    def kw_init( self, kw=None ):
        self.kw_style = {}
        self.kw_style.update( self.kw_defaults )
        if kw is not None:
            for key, value in kw.items():
                self.kw_set( key, value )

    def kw_set( self, key, value ):
        if self.kw_style is None:
            self.kw_init()
        key = self.kw_rename.get( key, key )
        self.kw_style[key] = value

    def style( self, **kw ):
        for key,val in kw.items():
            self.kw_set( key, val )

    def kw_get( self, key, notfound=None ):
        if self.kw_style is not None:
            return self.kw_style.get( key, notfound )
        else:
            return None

    def kw_predraw( self, context ):
        context.draw.save_state()
        if self.kw_style is not None:
            for key, value in self.kw_style.items():
                if self.kw_func.has_key(key):
                    method = self.kw_func[key]
                    method(context,key,value)
                else:
                    context.draw.set( key, value )

    def kw_postdraw( self, context ):
        context.draw.restore_state()

# _ConfAttributes -----------------------------------------------------------
# this can set global configuration!
class _ConfAttributes(object):
    """
    A class for setting configuration.

    If the section is in the global config, global configuration
    is set!
    """
    def conf_setattr( self, section, **kw ):
        """
        Set configuration paramaters

        parameters
        ----------
        section: string
                The section in global config.  If not in the
                global config, this is ignored

        **keywords:
                The keyword/value pairs to set.
                        self.key=val
        """
        import copy
        sec = config.options( section )
        if sec is not None:
            for key,val in sec.items():
                x = key.split( "." )
                obj = self
                for y in x[:-1]:
                    obj = getattr( obj, y )
                setattr( obj, x[-1], copy.copy(val) )
        for key,val in kw.items():
            setattr( self, key, copy.copy(val) )

# _DeviceObject ---------------------------------------------------------------

class _DeviceObject( _StyleKeywords ):

    def bbox( self, context ):
        return BoundingBox()

    def draw( self, context ):
        raise BigglesError

    def render( self, context ):
        self.kw_predraw( context )
        self.draw( context )
        self.kw_postdraw( context )

class _SymbolObject( _DeviceObject ):

    kw_rename = {
            'type' : 'symboltype',
            'size' : 'symbolsize',
    }

    def __init__( self, pos, **kw ):
        self.kw_init( kw )
        self.pos = pos

    def bbox( self, context ):
        self.kw_predraw( context )
        symbolsize = context.draw.get( 'symbolsize' )
        self.kw_postdraw( context )

        dp = symbolsize/2, symbolsize/2
        p = pt_sub( self.pos, dp )
        q = pt_add( self.pos, dp )
        return BoundingBox( p, q )

    def draw( self, context ):
        context.draw.symbol( self.pos )

class _TextObject( _DeviceObject ):

    kw_defaults = {
            'textangle'     : 0,
            'texthalign'    : 'center',
            'textvalign'    : 'center',
    }

    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
            'angle'         : 'textangle',
            'halign'        : 'texthalign',
            'valign'        : 'textvalign',
    }

    def __init__( self, pos, str, **kw ):
        self.kw_init( kw )
        self.pos = pos
        self.str = str

    __halign_offset = { 'right':(-1,0), 'center':(-.5,.5), 'left':(0,1) }
    __valign_offset = { 'top':(-1,0), 'center':(-.5,.5), 'bottom':(0,1) }

    def bbox( self, context ):
        self.kw_predraw( context )
        angle = context.draw.get( 'textangle' ) * math.pi/180.
        halign = context.draw.get( 'texthalign' )
        valign = context.draw.get( 'textvalign' )
        width = context.draw.textwidth( self.str )
        height = context.draw.textheight( self.str )
        self.kw_postdraw( context )

        hvec = pt_mul( width, _TextObject.__halign_offset[halign] )
        vvec = pt_mul( height, _TextObject.__valign_offset[valign] )

        p = self.pos[0] + hvec[0], self.pos[1] + vvec[0]
        q = self.pos[0] + hvec[1], self.pos[1] + vvec[1]

        bb = BoundingBox( p, q )
        bb.rotate( angle, self.pos )
        return bb

    def draw( self, context ):
        #bb = self.bbox( context )
        #context.draw.rect( bb.lowerleft(), bb.upperright() )
        context.draw.text( self.pos, self.str )

class _LabelsObject( _DeviceObject ):

    kw_defaults = {
            'textangle'     : 0,
            'texthalign'    : 'center',
            'textvalign'    : 'center',
    }

    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
            'angle'         : 'textangle',
            'halign'        : 'texthalign',
            'valign'        : 'textvalign',
    }

    def __init__( self, points, labels, **kw ):
        self.kw_init( kw )
        self.points = points
        self.labels = labels

    __halign_offset = { 'right':(-1,0), 'center':(-.5,.5), 'left':(0,1) }
    __valign_offset = { 'top':(-1,0), 'center':(-.5,.5), 'bottom':(0,1) }

    def bbox( self, context ):
        bb = BoundingBox()
        self.kw_predraw( context )

        angle = context.draw.get( 'textangle' ) * math.pi/180.
        halign = context.draw.get( 'texthalign' )
        valign = context.draw.get( 'textvalign' )

        height = context.draw.textheight( self.labels[0] )
        ho = _LabelsObject.__halign_offset[halign]
        vo = _LabelsObject.__valign_offset[valign]

        for i in range(len(self.labels)):
            pos = self.points[i]
            width = context.draw.textwidth( self.labels[i] )

            p = pos[0] + width * ho[0], pos[1] + height * vo[0]
            q = pos[0] + width * ho[1], pos[1] + height * vo[1]

            bb_label = BoundingBox( p, q )
            if angle != 0:
                bb_label.rotate( angle, pos )
            bb.union( bb_label )

        self.kw_postdraw( context )
        return bb

    def draw( self, context ):
        for i in range(len(self.labels)):
            context.draw.text( self.points[i], self.labels[i] )

class _LineTextObject( _TextObject ):

    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
    }

    def __init__( self, p, q, str, offset, **kw ):
        self.kw_init( kw )
        self.str = str

        midpoint = pt_mul( 0.5, pt_add(p, q) )
        direction = pt_unit( pt_sub(q, p) )
        angle = pt_angle( direction )
        direction = pt_rot( direction, math.pi/2 )
        self.pos = pt_add( midpoint, pt_mul(offset, direction) )

        self.kw_set( 'textangle', angle * 180./math.pi )
        self.kw_set( 'texthalign', 'center' )
        if offset > 0:
            self.kw_set( 'textvalign', 'bottom' )
        else:
            self.kw_set( 'textvalign', 'top' )

class _LineObject( _DeviceObject ):

    kw_rename = {
            'width'         : 'linewidth',
            'type'          : 'linetype',
    }

    def __init__( self, p, q, **kw ):
        self.kw_init( kw )
        self.p = p
        self.q = q

    def bbox( self, context ):
        return BoundingBox( self.p, self.q )

    def draw( self, context ):
        context.draw.line( self.p, self.q )

class _PolygonObject( _DeviceObject ):

    kw_rename = {
            'width'         : 'linewidth',
            'type'          : 'linetype',
    }

    def __init__( self, points, **kw ):
        self.kw_init( kw )
        self.points = points

    def bbox( self, context ):
        return BoundingBox( *self.points )

    def draw( self, context ):
        context.draw.polygon( self.points )

class _PathObject( _DeviceObject ):

    kw_rename = {
            'width'         : 'linewidth',
            'type'          : 'linetype',
    }

    def __init__( self, x, y, **kw ):
        self.kw_init( kw )
        self.x = x
        self.y = y

    def bbox( self, context ):
        xmin, xmax = _range( self.x )
        ymin, ymax = _range( self.y )
        return BoundingBox( (xmin,ymin), (xmax,ymax) )

    def draw( self, context ):
        context.draw.curve( self.x, self.y )

class _SymbolsObject( _DeviceObject ):

    kw_rename = {
            'type' : 'symboltype',
            'size' : 'symbolsize',
    }

    def __init__( self, x, y, **kw ):
        self.kw_init( kw )
        self.x = x
        self.y = y

    def bbox( self, context ):
        xmin, xmax = _range( self.x )
        ymin, ymax = _range( self.y )
        return BoundingBox( (xmin,ymin), (xmax,ymax) )

    def draw( self, context ):
        context.draw.symbols( self.x, self.y )

class _ColoredSymbolsObject( _DeviceObject ):

    kw_rename = {
            'type' : 'symboltype',
            'size' : 'symbolsize',
    }

    def __init__( self, x, y, c, **kw ):
        self.kw_init( kw )
        self.x = x
        self.y = y
        self.c = c

    def bbox( self, context ):
        xmin, xmax = _range( self.x )
        ymin, ymax = _range( self.y )
        return BoundingBox( (xmin,ymin), (xmax,ymax) )

    def draw( self, context ):
        context.draw.colored_symbols( self.x, self.y, self.c )

class _DensityObject( _DeviceObject ):

    kw_rename = {
    }

    def __init__( self, densgrid, extent,  **kw ):
        """
        extent is of the form ((xmin,ymin), (xmax,ymax))
        """
        self.kw_init( kw )
        self.densgrid = densgrid
        self.extent   = extent

    def bbox( self, context ):
        return BoundingBox( *self.extent )

    def draw( self, context ):
        #from numpy import rank
        #if rank(self.densgrid) == 3:
        if len(self.densgrid.shape) == 3:
            context.draw.color_density_plot( self.densgrid, self.extent )
        else:
            context.draw.density_plot( self.densgrid, self.extent )

class _EllipseObject( _DeviceObject ):

    def __init__( self, p, rx, ry, angle=0., **kw ):
        self.kw_init( kw )
        self.p = p
        self.rx = rx
        self.ry = ry
        self.angle = angle

    def bbox( self, context ):
        r = self.rx, self.ry
        p = pt_add( self.p, r )
        q = pt_sub( self.p, r )
        bb = BoundingBox( p, q )
        bb.rotate( self.p, self.angle )
        return bb

    def draw( self, context ):
        context.draw.ellipse( self.p, self.rx, self.ry, self.angle )

class _CombObject( _DeviceObject ):

    def __init__( self, points, dp, **kw ):
        self.kw_init( kw )
        self.points = points
        self.dp = dp

    def bbox( self, context ):
        return BoundingBox( *self.points )

    def draw( self, context ):
        for p in self.points:
            context.draw.move( p )
            context.draw.linetorel( self.dp )

class _BoxObject( _DeviceObject ):

    def __init__( self, p, q, **kw ):
        self.kw_init( kw )
        self.p = p
        self.q = q

    def bbox( self, context ):
        return BoundingBox( self.p, self.q )

    def draw( self, context ):
        context.draw.rect( self.p, self.q )

class _ArcObject( _DeviceObject ):

    def __init__( self, pc, p0, p1, **kw ):
        self.kw_init( kw )
        self.pc = pc
        self.p0 = p0
        self.p1 = p1

    def bbox( self, context ):
        return BoundingBox( self.pc, self.p0, self.p1 )

    def draw( self, context ):
        context.draw.arc( self.pc, self.p0, self.p1 )

# _PlotComponent --------------------------------------------------------------

class _PlotComponent( _StyleKeywords, _ConfAttributes ):

    def __init__( self ):
        self.clear()

    def add( self, *args ):
        for obj in args:
            self.device_objects.append( obj )

    def limits( self ):
        return BoundingBox()

    def clear( self ):
        self.device_objects = []

    def make( self, context ):
        raise BigglesError

    def make_key( self, bbox ):
        pass

    def bbox( self, context ):
        self.clear()
        self.make( context )
        bb = BoundingBox()
        for obj in self.device_objects:
            bb.union( obj.bbox(context) )
        return bb

    def render( self, context ):
        self.clear()
        self.make( context )
        self.kw_predraw( context )
        for obj in self.device_objects:
            obj.render( context )
        self.kw_postdraw( context )

# _LabelComponent -------------------------------------------------------------

class _LabelComponent( _PlotComponent ):

    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
            'angle'         : 'textangle',
            'halign'        : 'texthalign',
            'valign'        : 'textvalign',
    }

    def __init__( self, x, y, str, **kw ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "_LabelComponent" )
        self.kw_init( kw )
        self.pos = x, y
        self.str = str

    def limits( self ):
        return BoundingBox()

class DataLabel( _LabelComponent ):

    def make( self, context ):
        pos = context.geom( *self.pos )
        t = _TextObject(pos, self.str, **self.kw_style )
        self.add( t )

class PlotLabel( _LabelComponent ):

    def make( self, context ):
        pos = context.plot_geom( *self.pos )
        t = _TextObject(pos, self.str, **self.kw_style )
        self.add( t )

# _LabelsComponent ------------------------------------------------------------

class _LabelsComponent( _PlotComponent ):

    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
            'angle'         : 'textangle',
            'halign'        : 'texthalign',
            'valign'        : 'textvalign',
    }

    def __init__( self ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "_LabelsComponent" )

class Labels( _LabelsComponent ):

    def __init__( self, x, y, labels, **kw ):
        _LabelsComponent.__init__( self )
        self.conf_setattr( "Labels" )
        self.kw_init( kw )
        self.x = x
        self.y = y
        self.labels = labels

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        x, y = context.geom.call_vec( self.x, self.y )
        l = _LabelsObject(zip(x,y), self.labels, **self.kw_style )
        self.add( l )

# _LineComponent --------------------------------------------------------------

class _LineComponent( _PlotComponent ):

    def __init__( self ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "_LineComponent" )

    kw_rename = {
            'color' : 'linecolor',
            'width' : 'linewidth',
            'type' : 'linetype',
    }

    def make_key( self, bbox ):
        xr = bbox.xrange()
        y = bbox.center()[1]
        p = xr[0], y
        q = xr[1], y
        return _LineObject( p,q, **self.kw_style )

class Curve( _LineComponent ):
    """
    An object representing a curve.  Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
            The "x" values of each point, to be connected by lines.
    y: array or sequence
            The "y" values of each point, to be connected by lines..

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """


    def __init__( self, x, y, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "Curve" )
        self.kw_init( kw )
        self.x = x
        self.y = y

    def limits( self ):
        p0 = min(self.x), min(self.y)
        p1 = max(self.x), max(self.y)
        return BoundingBox( p0, p1 )

    def make( self, context ):
        segs = context.geom.geodesic( self.x, self.y )
        for seg in segs:
            x, y = context.geom.call_vec( seg[0], seg[1] )
            self.add( _PathObject(x, y) )


class DataLine( _LineComponent ):

    def __init__( self, p, q, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "DataLine" )
        self.kw_init( kw )
        self.p = p
        self.q = q

    def limits( self ):
        return BoundingBox( self.p, self.q )

    def make( self, context ):
        a = context.geom( *self.p )
        b = context.geom( *self.q )
        self.add( _LineObject(a, b) )

class Geodesic( _LineComponent ):

    def __init__( self, p, q, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "Geodesic" )
        self.kw_init( kw )
        self.p = p
        self.q = q

    def limits( self ):
        return BoundingBox( self.p, self.q )

    def make( self, context ):
        l = self.p[0], self.q[0]
        b = self.p[1], self.q[1]
        segs = context.geom.geodesic( l, b, self.divisions )
        for seg in segs:
            x, y = context.geom.call_vec( seg[0], seg[1] )
            self.add( _PathObject(x, y) )

class Histogram( _LineComponent ):
    """
    A histogram plot object.  Can be added to plot containers.

    parameters
    ----------
    values: array or sequence
            The "y" values for the histogram.
    x0: keyword, optional
            The "x" position of the first value in values.
    binsize: keyword, optional
            The binsize for the histogram. the plotted
            x values will start at x0 with spacing binsize.
    smooth: bool
        If set to True, draw a smooth curve rather than a
        bar graph.  Default is False.

    Style and other keywords for the histogram.
    These can also be set as attributes

    [line]color, [line]type, [line]width:
        style for line

    drop_to_zero: bool
        If true, draw down to zero always. Default True.  Useful
        to set False for log plots.
    """
    def __init__( self, values, x0=0, binsize=1, smooth=False, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "Histogram" )
        self.kw_init( kw )
        self.values = values
        self.x0 = x0
        self.binsize = binsize
        self.smooth=smooth

    def limits( self ):
        nval = len( self.values )
        if self.drop_to_zero:
            p = self.x0, min( 0, min(self.values) )
        else:
            p = self.x0, min(self.values)
        q = self.x0 + nval*self.binsize, max(self.values)
        return BoundingBox( p, q )

    def make( self, context ):
        if self.smooth:
            x,y=self._make_smooth()
        else:
            x,y=self._make_bar()

        u, v = context.geom.call_vec( x, y )
        self.add( _PathObject(u, v) )

    def _make_bar(self):
        """
        make a bar graph
        """
        nval = len( self.values )
        x = []
        y = []
        if self.drop_to_zero:
            x.append( self.x0 )
            y.append( 0 )
        for i in range(0,nval):
            xi = self.x0 + i * self.binsize
            yi = self.values[i]
            x.extend( [xi, xi + self.binsize] )
            y.extend( [yi, yi] )
        if self.drop_to_zero:
            x.append( self.x0 + nval*self.binsize )
            y.append( 0 )
        return x,y

    def _make_smooth(self):
        """
        Plot a smooth curve
        """
        nval = len( self.values )

        x = numpy.arange(
            self.x0 + 0.5*self.binsize,
            self.x0 + nval*self.binsize,
            self.binsize,
            dtype='f8'
        )
        y=self.values

        return x,y


class LineX( _LineComponent ):

    def __init__( self, x, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "LineX" )
        self.kw_init( kw )
        self.x = x

    def limits( self ):
        return BoundingBox( (self.x,None), (self.x,None) )

    def make( self, context ):
        yrange = context.data_bbox.yrange()
        p = self.x, yrange[0]
        q = self.x, yrange[1]
        a = context.geom( *p )
        b = context.geom( *q )
        self.add( _LineObject(a, b) )

class LineY( _LineComponent ):

    def __init__( self, y, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "LineY" )
        self.kw_init( kw )
        self.y = y

    def limits( self ):
        return BoundingBox( (None,self.y), (None,self.y) )

    def make( self, context ):
        xrange = context.data_bbox.xrange()
        p = xrange[0], self.y
        q = xrange[1], self.y
        a = context.geom( *p )
        b = context.geom( *q )
        self.add( _LineObject(a, b) )

class PlotLine( _LineComponent ):

    def __init__( self, p, q, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "PlotLine" )
        self.kw_init( kw )
        self.p = p
        self.q = q

    def make( self, context ):
        a = context.plot_geom( *self.p )
        b = context.plot_geom( *self.q )
        self.add( _LineObject(a, b) )

class Slope( _LineComponent ):

    def __init__( self, slope, intercept=None, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "Slope" )
        self.kw_init( kw )
        self.slope = slope
        self.intercept = intercept
        if intercept is None:
            self.intercept = (0.,0.)

    def _x( self, y ):
        x0, y0 = self.intercept
        return x0 + float(y - y0) / self.slope

    def _y( self, x ):
        x0, y0 = self.intercept
        return y0 + (x - x0) * self.slope

    def make( self, context ):
        xrange = context.data_bbox.xrange()
        yrange = context.data_bbox.yrange()
        if self.slope == 0:
            l = [ ( xrange[0], self.intercept[1] ),\
                  ( xrange[1], self.intercept[1] ) ]
        else:
            l = [ ( xrange[0], self._y(xrange[0]) ),\
                  ( xrange[1], self._y(xrange[1]) ),\
                  ( self._x(yrange[0]), yrange[0] ),\
                  ( self._x(yrange[1]), yrange[1] ) ]
        m = filter( context.data_bbox.contains, l )
        m.sort()
        if len(m) > 1:
            a = context.geom( *m[0] )
            b = context.geom( *m[-1] )
            self.add( _LineObject(a, b) )

class DataBox( _LineComponent ):
    """
    Get a curve representing a box.  Can be added to plot containers.

    parameters
    ----------
    corner1: 2-element sequence
        First corner of the box [x1,y1].
    corner2: 2-element sequence
        Second corner of the box [x2,y2].

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """
    def __init__( self, p, q, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "DataBox" )
        self.kw_init( kw )
        self.p = p
        self.q = q

    def limits( self ):
        return BoundingBox( self.p, self.q )

    def make( self, context ):
        a = context.geom( *self.p )
        b = context.geom( *self.q )
        self.add( _BoxObject(a, b) )

class PlotBox( _LineComponent ):

    def __init__( self, p, q, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "PlotBox" )
        self.kw_init( kw )
        self.p = p
        self.q = q

    def make( self, context ):
        a = context.plot_geom( *self.p )
        b = context.plot_geom( *self.q )
        self.add( _BoxObject(a, b) )

class PlotArc( _LineComponent ):

    def __init__( self, pc, r, a0, a1, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "PlotArc" )
        self.kw_init( kw )
        self.pc = pc
        self.p0 = pc[0] + r*math.cos(a0), pc[1] + r*math.sin(a0)
        self.p1 = pc[0] + r*math.cos(a1), pc[1] + r*math.sin(a1)

    def make( self, context ):
        pc = context.plot_geom( *self.pc )
        p0 = context.plot_geom( *self.p0 )
        p1 = context.plot_geom( *self.p1 )
        self.add( _ArcObject(pc, p0, p1) )

class DataArc( _LineComponent ):

    def __init__( self, pc, r, a0, a1, **kw ):
        _LineComponent.__init__( self )
        self.conf_setattr( "DataArc" )
        self.kw_init( kw )
        self.pc = pc
        self.p0 = pc[0] + r*math.cos(a0), pc[1] + r*math.sin(a0)
        self.p1 = pc[0] + r*math.cos(a1), pc[1] + r*math.sin(a1)

    def limits( self ):
        return BoundingBox( self.pc, self.p0, self.p1 )

    def make( self, context ):
        pc = context.geom( *self.pc )
        p0 = context.geom( *self.p0 )
        p1 = context.geom( *self.p1 )
        self.add( _ArcObject(pc, p0, p1) )

# _SymbolDataComponent --------------------------------------------------------

class _SymbolDataComponent( _PlotComponent ):

    kw_rename = {
            'type' : 'symboltype',
            'size' : 'symbolsize',
    }

    def make_key( self, bbox ):
        pos = bbox.center()
        return _SymbolObject( pos, **self.kw_style)

class Points( _SymbolDataComponent ):
    """
    A set of points.  Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
            The "x" values of each point.
    y: array or sequence
            The "y" values of each point.

    **keywords
            Style and other keywords for the Points.

            See the configuration options for Points for details (TODO copy
            into here)
    """

    kw_defaults = {
            'symboltype' : config.value('Points','symboltype'),
            'symbolsize' : config.value('Points','symbolsize'),
    }

    def __init__( self, x, y, **kw ):
        _SymbolDataComponent.__init__( self )
        self.conf_setattr( "Points" )
        self.kw_init( kw )
        self.x = x
        self.y = y

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        x, y = context.geom.call_vec( self.x, self.y )
        self.add( _SymbolsObject(x, y) )

def Point( x, y, **kw ):
    """
    Create a Points object for a single point.

    The parameters are the same as for the Points class,
    except x and y are scalars.
    """
    return Points( [x],[y], **kw )

class ColoredPoints( _SymbolDataComponent ):
    """
    A set of colored points

    parameters
    ----------
    x: array or sequence
            The "x" values of each point.
    y: array or sequence
            The "y" values of each point.
    c: array or sequence
            Colors for each point.

    **keywords
            Style and other keywords for the Points.

            See the configuration options for Points for details (TODO copy
            into here)
    """

    kw_defaults = {
            'symboltype' : config.value('Points','symboltype'),
            'symbolsize' : config.value('Points','symbolsize'),
    }

    def __init__( self, x, y, c=None, **kw ):
        _SymbolDataComponent.__init__( self )
        self.conf_setattr( "Points" )
        self.kw_init( kw )
        self.x = x
        self.y = y
        self.c = c

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        x, y = context.geom.call_vec( self.x, self.y )
        self.add( _ColoredSymbolsObject(x, y, self.c) )

def ColoredPoint( x, y, **kw ):
    """
    Create a ColoredPoints object for a single point.

    The parameters are the same as for the ColoredPoints class,
    except x and y are scalars.
    """

    return ColoredPoints( [x],[y], **kw )

# _DensityComponent -----------------------------------------------------------

class Density( _PlotComponent ):
    """
    Create a 2-d image plot object, or density map

    parameters
    ----------
    densgrid/image:
            The image or density grid.
    extent:
            extent = ( (xmin,ymin), (xmax,ymax) )

    **keywords
            Style and other keywords for the Density.

            See the configuration options for Density for details (TODO copy
            into here)

    TODO: mabye alias this to Image?
    """
    kw_defaults = {
            'foo' : config.value('Points','symbolsize'),
    }

    def __init__( self, densgrid, extent, **kw ):
        """
        extent has the form ((xmin,ymin), (xmax,ymax))
        """
        _PlotComponent.__init__( self )
        self.conf_setattr( "Density" )
        self.kw_init( kw )
        self.densgrid = densgrid
        self.extent   = extent

    def limits( self ):
        return BoundingBox( *self.extent )

    def make( self, context ):
        (x0,y0),(x1,y1) = self.extent
        (x0,x1),(y0,y1) = context.geom.call_vec((x0,x1),(y0,y1))
        self.add( _DensityObject(self.densgrid, ((x0,y0),(x1,y1))) )

# _FillComponent --------------------------------------------------------------

class _FillComponent( _PlotComponent ):

    kw_defaults = {
            'color' : config.value('_FillComponent','fillcolor'),
            'filltype' : config.value('_FillComponent','filltype'),
    }

    def make_key( self, bbox ):
        p = bbox.lowerleft()
        q = bbox.upperright()
        return _BoxObject( p,q, **self.kw_style )

class FillAbove( _FillComponent ):

    def __init__( self, x, y, **kw ):
        _FillComponent.__init__( self )
        self.conf_setattr( "FillAbove" )
        self.kw_init( kw )
        self.x = x
        self.y = y

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        coords = map( context.geom, self.x, self.y )
        max_y = context.data_bbox.yrange()[1]
        coords.append( context.geom(self.x[-1], max_y) )
        coords.append( context.geom(self.x[0], max_y) )
        self.add( _PolygonObject(coords) )

class FillBelow( _FillComponent ):

    def __init__( self, x, y, **kw ):
        _FillComponent.__init__( self )
        self.conf_setattr( "FillBelow" )
        self.kw_init( kw )
        self.x = x
        self.y = y

    def limits( self ):
        p = min(self.x), min(self.y)
        q = max(self.x), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        coords = map( context.geom, self.x, self.y )
        min_y = context.data_bbox.yrange()[0]
        coords.append( context.geom(self.x[-1], min_y) )
        coords.append( context.geom(self.x[0], min_y) )
        self.add( _PolygonObject(coords) )

class FillBetween( _FillComponent ):

    def __init__( self, x1, y1, x2, y2, **kw ):
        _FillComponent.__init__( self )
        self.conf_setattr( "FillBetween" )
        self.kw_init( kw )
        self.x1, self.y1 = x1, y1
        self.x2, self.y2 = x2, y2

    def limits( self ):
        min_x = min( min(self.x1), min(self.x2) )
        max_x = max( max(self.x1), max(self.x2) )
        min_y = min( min(self.y1), min(self.y2) )
        max_y = max( max(self.y1), max(self.y2) )
        return BoundingBox( (min_x,min_y), (max_x,max_y) )

    def make( self, context ):
        x = list(self.x1) + _tsil(self.x2)
        y = list(self.y1) + _tsil(self.y2)
        coords = map( context.geom, x, y )
        self.add( _PolygonObject(coords) )

# ErrorBars -------------------------------------------------------------------

class _ErrorBar( _PlotComponent ):

    kw_rename = {
            'color' : 'linecolor',
            'width' : 'linewidth',
            'type' : 'linetype',
    }

    def __init__( self ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "_ErrorBar" )

class ErrorBarsX( _ErrorBar ):
    """
    An object representing Error bars in the X direction.

    Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
        The "x" values of each point.
    y: array or sequence
        The "y" values of each point.
    low: array or sequence
        The left end of the error bar
    high: array or sequence
        The right end of the error bar

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """


    def __init__( self, y, lo, hi, **kw ):
        _ErrorBar.__init__( self )
        self.conf_setattr( "ErrorBarsX" )
        self.kw_init( kw )
        self.y = y
        self.lo = lo
        self.hi = hi

    def limits( self ):
        p = min( min(self.lo), min(self.hi) ), min(self.y)
        q = max( max(self.lo), max(self.hi) ), max(self.y)
        return BoundingBox( p, q )

    def make( self, context ):
        l = _size_relative( self.barsize, context.dev_bbox )
        for i in range(len(self.y)):
            p = context.geom( self.lo[i], self.y[i] )
            q = context.geom( self.hi[i], self.y[i] )
            l0 = _LineObject( p, q )
            l1 = _LineObject( (p[0],p[1]-l), (p[0],p[1]+l) )
            l2 = _LineObject( (q[0],q[1]-l), (q[0],q[1]+l) )
            self.add( l0, l1, l2 )

class ErrorBarsY( _ErrorBar ):
    """
    An object representing Error bars in the Y direction.

    Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
        The "x" values of each point.
    y: array or sequence
        The "y" values of each point.
    low: array or sequence
        The lower end of the error bar
    high: array or sequence
        The upper end of the error bar

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """


    def __init__( self, x, lo, hi, **kw ):
        _ErrorBar.__init__( self )
        self.conf_setattr( "ErrorBarsY" )
        self.kw_init( kw )
        self.x = x
        self.lo = lo
        self.hi = hi

    def limits( self ):
        p = min(self.x), min( min(self.lo), min(self.hi) )
        q = max(self.x), max( max(self.lo), max(self.hi) )
        return BoundingBox( p, q )

    def make( self, context ):
        l = _size_relative( self.barsize, context.dev_bbox )
        for i in range(len(self.x)):
            p = context.geom( self.x[i], self.lo[i] )
            q = context.geom( self.x[i], self.hi[i] )
            l0 = _LineObject( p, q )
            l1 = _LineObject( (p[0]-l,p[1]), (p[0]+l,p[1]) )
            l2 = _LineObject( (q[0]-l,q[1]), (q[0]+l,q[1]) )
            self.add( l0, l1, l2 )

def SymmetricErrorBarsX( x, y, err, **kw ):
    """
    get an object representing Symmetric Error bars in the X direction.

    Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
        The "x" values of each point.
    y: array or sequence
        The "y" values of each point.
    err: array or sequence
        The size of the error bars.

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """


    import operator
    xlo = map( operator.sub, x, err )
    xhi = map( operator.add, x, err )
    return ErrorBarsX( y, xlo, xhi, **kw )

def SymmetricErrorBarsY( x, y, err, **kw ):
    """
    get an object representing Symmetric Error bars in the Y direction.

    Can be added to plot containers.

    parameters
    ----------
    x: array or sequence
        The "x" values of each point.
    y: array or sequence
        The "y" values of each point.
    err: array or sequence
        The size of the error bars.

    **keywords
            Style and other keywords for the Curve.

            See the configuration options for Curve for details (TODO copy
            into here)
    """


    import operator
    ylo = map( operator.sub, y, err )
    yhi = map( operator.add, y, err )
    return ErrorBarsY( x, ylo, yhi, **kw )

# Limits ----------------------------------------------------------------------

class _ErrorLimit( _PlotComponent ):

    kw_rename = {
            'color' : 'linecolor',
            'width' : 'linewidth',
            'type' : 'linetype',
    }

    def __init__( self ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "_ErrorLimit" )

class UpperLimits( _ErrorLimit ):

    def __init__( self, x, ulimit, **kw ):
        _ErrorLimit.__init__( self )
        self.conf_setattr( "UpperLimits" )
        self.kw_init( kw )
        self.x = x
        self.ulimit = ulimit

    def limits( self ):
        p = min(self.x), min(self.ulimit)
        q = max(self.x), max(self.ulimit)
        return BoundingBox( p, q )

    def make( self, context ):
        l = _size_relative( self.size, context.dev_bbox )
        for i in range(len(self.x)):
            p = context.geom( self.x[i], self.ulimit[i] )
            l1 = _LineObject( (p[0]-l,p[1]), (p[0]+l,p[1]) )
            l2 = _LineObject( (p[0],p[1]-2*l), (p[0],p[1]) )
            l3 = _LineObject( (p[0],p[1]-2*l), (p[0]+l,p[1]-l) )
            l4 = _LineObject( (p[0],p[1]-2*l), (p[0]-l,p[1]-l) )
            self.add( l1, l2, l3, l4 )

class LowerLimits( _ErrorLimit ):

    def __init__( self, x, llimit, **kw ):
        _ErrorLimit.__init__( self )
        self.conf_setattr( "UpperLimits" )
        self.kw_init( kw )
        self.x = x
        self.llimit = llimit

    def limits( self ):
        p = min(self.x), min(self.llimit)
        q = max(self.x), max(self.llimit)
        return BoundingBox( p, q )

    def make( self, context ):
        l = _size_relative( self.size, context.dev_bbox )
        for i in range(len(self.x)):
            p = context.geom( self.x[i], self.llimit[i] )
            l1 = _LineObject( (p[0]-l,p[1]), (p[0]+l,p[1]) )
            l2 = _LineObject( (p[0],p[1]+2*l), (p[0],p[1]) )
            l3 = _LineObject( (p[0],p[1]+2*l), (p[0]+l,p[1]+l) )
            l4 = _LineObject( (p[0],p[1]+2*l), (p[0]-l,p[1]+l) )
            self.add( l1, l2, l3, l4 )

# Ellipses --------------------------------------------------------------------

class Ellipses( _PlotComponent ):
    """
    Get curves representing a ellipses.  Can be added to plot containers.

    parameters
    ----------
    x: sequence
        x center of ellipses
    y: sequence
        y center of ellipses
    rx: sequence
        size in the x direction of ellipses
    ry: sequence
        size in the y direction of ellipses
    angle: sequence
        angle for ellipses

    **keywords
            Style and other keywords for the curves.

            See the configuration options for curve for details (TODO copy
            into here)
    """


    kw_rename = {
            'color' : 'linecolor',
            'width' : 'linewidth',
            'type' : 'linetype',
    }

    def __init__( self, x, y, rx, ry, angle=None, **kw ):
        _PlotComponent.__init__( self )
        self.kw_init( kw )
        self.x = x
        self.y = y
        self.rx = rx
        self.ry = ry
        self.angle = angle

    def limits( self ):
        # XXX:kludge
        minx, maxx = self.x[0], self.x[0]
        miny, maxy = self.y[0], self.y[0]
        for i in range(len(self.x)):
            r = max( self.rx[i], self.ry[i] )
            minx = min( minx, self.x[i]-r )
            miny = min( miny, self.y[i]-r )
            maxx = max( maxx, self.x[i]+r )
            maxy = max( maxy, self.y[i]+r )
        return BoundingBox( (minx,miny), (maxx,maxy) )

    def make( self, context ):
        for i in range(len(self.x)):
            p = context.geom( self.x[i], self.y[i] )
            r = context.geom( \
                    self.x[i] + self.rx[i], \
                    self.y[i] + self.ry[i] )
            rx, ry = pt_sub( r, p )

            if self.angle is not None:
                e = _EllipseObject( p, rx, ry, self.angle[i] )
            else:
                e = _EllipseObject( p, rx, ry )
            self.add( e )

def Ellipse( x, y, rx, ry, angle=None, **kw ):
    """
    Get a curve representing an ellipse.  Can be added to plot containers.

    parameters
    ----------
    x: scalar
        x center of ellipse
    y: scalar
        y center of ellipse
    rx: scalar
        size in the x direction
    ry: scalar
        size in the y direction
    angle: scalar
        angle for ellipse

    **keywords
            Style and other keywords for the curve.

            See the configuration options for curve for details (TODO copy
            into here)
    """


    if angle is None:
        args = ([x],[y],[rx],[ry])
    else:
        args = ([x],[y],[rx],[ry],[angle])
    return Ellipses( *args, **kw )

def Circles( x, y, r, **kw ):
    """
    Get curves representing circles.  Can be added to plot containers.

    parameters
    ----------
    x: sequence
        x center of circles
    y: sequence
        y center of circles
    r: sequence
        radii of circles

    **keywords
            Style and other keywords for the curve.

            See the configuration options for curve for details (TODO copy
            into here)
    """

    return Ellipses( x,y,r,r, **kw )

def Circle( x, y, r, **kw ):
    """
    Get a curve representing a circle.  Can be added to plot containers.

    parameters
    ----------
    x: scalar
        x center of circle
    y: scalar
        y center of circle
    r: scalar
        radius of circle

    **keywords
            Style and other keywords for the curve.

            See the configuration options for curve for details (TODO copy
            into here)
    """

    return Circles( [x],[y],[r], **kw )

# _PlotKey --------------------------------------------------------------------

class PlotKey( _PlotComponent ):
    """
    An object representing a Plot Key.  Can be added to plot objects.

    parameters
    ----------
    x: scalar
        x location of the key
    y: scalar
        y location of the key
    components: sequence
        A sequence of the components to be labeled. They should have label
        attributes.  E.g. Points, Curves, etc.

    **keywords
            Style and other keywords.

            See the configuration options for curve for details (TODO copy
            into here)
    """


    kw_rename = {
            'face'          : 'fontface',
            'size'          : 'fontsize',
            'angle'         : 'textangle',
            'halign'        : 'texthalign',
            'valign'        : 'textvalign',
    }

    def __init__( self, x, y, components=None, **kw ):
        _PlotComponent.__init__( self )
        self.conf_setattr( "PlotKey" )
        self.kw_init( kw )
        self.x = x
        self.y = y

        if components is None:
            components=[]
        self.components = components

    def add(self, *args):
        self.components += args

    def make( self, context ):
        key_pos = context.plot_geom( self.x, self.y )
        key_width = _size_relative( self.key_width, context.dev_bbox )
        key_height = _size_relative( self.key_height, context.dev_bbox )
        key_hsep = _size_relative( self.key_hsep, context.dev_bbox )
        key_vsep = _size_relative( self.key_vsep, context.dev_bbox )

        halign = self.kw_get( 'texthalign' )
        if halign == 'left':
            text_pos = pt_add( (key_width/2+key_hsep,0), key_pos )
        else:
            text_pos = pt_add( (-key_width/2-key_hsep,0), key_pos )
        bbox = BoundingBox( (-key_width/2,-key_height/2),
                (key_width/2,key_height/2) )
        bbox.shift( key_pos )
        dp = 0, -(key_vsep + key_height)

        for comp in self.components:
            try:
                obj,str = comp
            except:
                obj = comp
                str = getattr( comp, "label", "" )
            t = _TextObject( text_pos,str, **self.kw_style )
            #self.add( t, obj.make_key(bbox) )
            # add means something different in the parent
            super(PlotKey,self).add( t, obj.make_key(bbox) )
            text_pos = pt_add( text_pos, dp )
            bbox.shift( dp )

# XXX:deprecated
def OldKey( x, y, labels, align='left', **kw ):
    kw['texthalign'] = align
    return PlotKey(x,y,labels, **kw )

# _HalfAxis -------------------------------------------------------------------

def _magform( x ):
    "Given x, returns (a,b), where x = a*10^b [a >= 1., b integral]."
    if x == 0:
        return 0., 0
    a, b = math.modf(math.log10(abs(x)))
    a, b = math.pow(10,a), int(b)
    if a < 1.:
        a, b = a * 10, b - 1
    if x < 0.:
        a = -a
    return a, b

def _format_ticklabel( x, range=0. ):
    if x == 0:
        return "0"
    a, b = _magform( x )
    if abs(b) > 4:
        if a == 1.:
            return r"$10^{%d}$" % b
        elif a == -1.:
            return r"-$10^{%d}$" % b
        else:
            return r"$%g\times 10^{%d}$" % (a,b)
    if range < 1e-6:
        a, b = _magform( range )
        return "%.*f" % (abs(b),x)
    return "%g" % x

def _ticklist_linear( lo, hi, sep, origin=0. ):
    r = []
    a = _ceil(float(lo - origin)/float(sep))
    b = _floor(float(hi - origin)/float(sep))
    #for i in range( a, b+1 ):
    #       r.append( origin + i * sep )
    r0 = origin + a*sep
    for i in range( b-a+1 ):
        r.append( r0 + i*sep )
    return r

def _pow10(x):
    return math.pow(10,x)

def _log10(x):
    return math.log10(x)

def _ticks_default_linear( lim ):
    a, b = _magform( (lim[1] - lim[0])/5. )
    if a < (1 + 2)/2.:
        x = 1
    elif a < (2 + 5)/2.:
        x = 2
    elif a < (5 + 10)/2.:
        x = 5
    else:
        x = 10

    major_div = x * math.pow(10, b)
    return _ticklist_linear( lim[0], lim[1], major_div )

def _ticks_default_log( lim ):
    log_lim = _log10(lim[0]), _log10(lim[1])
    nlo = _ceil( math.log10(lim[0]) )
    nhi = _floor( math.log10(lim[1]) )
    nn = nhi - nlo +1

    if nn >= 10:
        return map( _pow10, _ticks_default_linear(log_lim) )
    elif nn >= 2:
        return map( _pow10, range(nlo, nhi+1) )
    else:
        return _ticks_default_linear( lim )

def _ticks_num_linear( lim, num ):
    ticks = []
    a = lim[0]
    b = (lim[1] - lim[0])/float(num-1)
    for i in range(num):
        ticks.append( a + i*b )
    return ticks

def _ticks_num_log( lim, num ):
    ticks = []
    a = math.log10(lim[0])
    b = (math.log10(lim[1]) - a)/float(num - 1)
    for i in range(num):
        ticks.append( a + i*b )
    return map( _pow10, ticks )

def _subticks_linear( lim, ticks, num=None ):
    major_div = (ticks[-1] - ticks[0])/float(len(ticks) - 1)
    if num is None:
        _num = 4
        a, b = _magform( major_div )
        if 1. < a < (2 + 5)/2.:
            _num = 3
    else:
        _num = num
    minor_div = major_div/float(_num+1)
    return _ticklist_linear( lim[0], lim[1], minor_div, ticks[0] )

def _subticks_log( lim, ticks, num=None ):
    log_lim = _log10(lim[0]), _log10(lim[1])
    nlo = _ceil( math.log10(lim[0]) )
    nhi = _floor( math.log10(lim[1]) )
    nn = nhi - nlo +1

    if nn >= 10:
        return map( _pow10, _subticks_linear(log_lim, map(_log10,ticks), num) )
    elif nn >= 2:
        minor_ticks = []
        for i in range(nlo-1,nhi+1):
            for j in range(1,10):
                z = j * _pow10(i)
                if lim[0] <= z and z <= lim[1]:
                    minor_ticks.append(z)
        return minor_ticks
    else:
        return _subticks_linear( lim, ticks, num )

class _Group(object):

    def __init__( self, objs ):
        self.objs = objs[:]

    def bbox( self, context ):
        bb = BoundingBox()
        for obj in self.objs:
            bb.union( obj.bbox(context) )
        return bb

class _HalfAxis( _PlotComponent ):

    func_ticks_default      = _ticks_default_linear, _ticks_default_log
    func_ticks_num          = _ticks_num_linear, _ticks_num_log

    func_subticks_default   = _subticks_linear, _subticks_log
    func_subticks_num       = _subticks_linear, _subticks_log

    _attr_map = {
            'labeloffset'           : 'label_offset',
            'major_ticklabels'      : 'ticklabels',
            'major_ticks'           : 'ticks',
            'minor_ticks'           : 'subticks',
    }

    def __init__( self, **kw ):
        _PlotComponent.__init__( self )
        self.kw_init( kw )
        self.conf_setattr( "_HalfAxis" )

    def __getattr__( self, name ):
        return self.__dict__[ self._attr_map.get(name,name) ]

    def __setattr__( self, name, value ):
        self.__dict__[ self._attr_map.get(name,name) ] = value

    def _ticks( self, context ):
        log = self._log( context )
        _range = self._range( context )
        if self.ticks is None:
            return self.func_ticks_default[log]( _range )
        elif type(self.ticks) == type(0):
            return self.func_ticks_num[log]( _range, self.ticks )
        else:
            return self.ticks

    def _subticks( self, context, ticks ):
        log = self._log( context )
        _range = self._range( context )
        if self.subticks is None:
            return self.func_subticks_default[log]( _range, ticks )
        elif type(self.subticks) == type(0):
            return self.func_subticks_num[log]( \
                    _range, ticks, self.subticks )
        else:
            return self.subticks

    def _ticklabels( self, context, ticks ):
        if self.ticklabels is None:
            range = [ max(ticks) - min(ticks) ] * len(ticks)
            return map( _format_ticklabel, ticks, range )
        else:
            return self.ticklabels

    def _make_ticklabels( self, context, pos, labels ):
        if labels is None or not len(labels) > 0:
            return

        dir = self.ticklabels_dir
        offset = _size_relative( self.ticklabels_offset, \
                context.dev_bbox )
        if self.draw_ticks and self.tickdir > 0:
            offset = offset + _size_relative( \
                    self.ticks_size, context.dev_bbox )
        labelpos = []
        for i in range(len(labels)):
            labelpos.append( \
                    self._pos(context, pos[i], dir*offset) )

        halign, valign = self._align()

        style = { "halign" : halign, "valign" : valign }
        style.update( self.ticklabels_style )

        l = _LabelsObject( labelpos, labels, **style )
        self.add( l )

    def _make_spine( self, context ):
        a, b = self._range( context )
        p = self._pos( context, a )
        q = self._pos( context, b )
        self.add( _LineObject(p, q, **self.spine_style) )

    def _make_ticks( self, context, ticks, size, style ):
        if ticks is None or not len(ticks) > 0:
            return

        dir = self.tickdir * self.ticklabels_dir
        ticklen = self._dpos( dir *\
                _size_relative(size, context.dev_bbox) )
        tickpos = []
        for tick in ticks:
            tickpos.append( self._pos(context, tick) )

        self.add( _CombObject(tickpos, ticklen, **style) )

    def make( self, context ):
        if self.draw_nothing:
            return

        ticks = self._ticks( context )
        subticks = self._subticks( context, ticks )
        ticklabels = self._ticklabels( context, ticks )

        implicit_draw_subticks = self.draw_subticks is None and \
                self.draw_ticks

        implicit_draw_ticklabels = self.draw_ticklabels is None and \
                (self.range is not None or self.ticklabels is not None)

        if self.draw_grid:
            self._make_grid( context, ticks )

        if self.draw_axis:
            if self.draw_subticks or implicit_draw_subticks:
                self._make_ticks( context, subticks, \
                        self.subticks_size, \
                        self.subticks_style )

            if self.draw_ticks:
                self._make_ticks( context, ticks, \
                        self.ticks_size, self.ticks_style )

            if self.draw_spine:
                self._make_spine( context )

        if self.draw_ticklabels or implicit_draw_ticklabels:
            self._make_ticklabels( context, ticks, ticklabels )

        ## has to be made last
        if self.label is not None:
            tgroup = _Group(self.device_objects)
            tlab = _BoxLabel(tgroup,
                             self.label,
                             self._side(),
                             self.label_offset,
                             **self.label_style)
            self.add( tlab )


class _HalfAxisX( _HalfAxis ):

    def _pos( self, context, a, db=0. ):
        p = context.geom( a, self._intercept(context) )
        return p[0], p[1] + db

    def _dpos( self, d ):
        return 0., d

    def _align( self ):
        if self.ticklabels_dir < 0:
            return 'center', 'top'
        else:
            return 'center', 'bottom'

    def _intercept( self, context ):
        if self.intercept is not None:
            return self.intercept
        limits = context.data_bbox
        if self.ticklabels_dir < 0:
            return limits.yrange()[0]
        else:
            return limits.yrange()[1]

    def _log( self, context ):
        if self.log is None:
            return context.xlog
        return self.log

    def _side( self ):
        if self.ticklabels_dir < 0:
            return 'bottom'
        else:
            return 'top'

    def _range( self, context ):
        if self.range is not None:
            a,b = self.range
            if a is None or b is None:
                c,d = context.data_bbox.xrange()
                if a is None: a = c
                if b is None: b = d
                return a,b
            else:
                return self.range
        return context.data_bbox.xrange()

    def _make_grid( self, context, ticks ):
        if ticks is None:
            return
        for tick in ticks:
            self.add( LineX(tick, **self.grid_style) )

class _HalfAxisY( _HalfAxis ):

    def _pos( self, context, a, db=0. ):
        p = context.geom( self._intercept(context), a )
        return p[0] + db, p[1]

    def _dpos( self, d ):
        return d, 0.

    def _align( self ):
        if self.ticklabels_dir > 0:
            return 'left', 'center'
        else:
            return 'right', 'center'

    def _intercept( self, context ):
        if self.intercept is not None:
            return self.intercept
        limits = context.data_bbox
        if self.ticklabels_dir > 0:
            return limits.xrange()[1]
        else:
            return limits.xrange()[0]

    def _log( self, context ):
        if self.log is None:
            return context.ylog
        return self.log

    def _side( self ):
        if self.ticklabels_dir > 0:
            return 'right'
        else:
            return 'left'

    def _range( self, context ):
        if self.range is not None:
            a,b = self.range
            if a is None or b is None:
                c,d = context.data_bbox.yrange()
                if a is None: a = c
                if b is None: b = d
                return a,b
            else:
                return self.range
        return context.data_bbox.yrange()

    def _make_grid( self, context, ticks ):
        if ticks is None:
            return
        for tick in ticks:
            self.add( LineY(tick, **self.grid_style) )

# _BoxLabel -------------------------------------------------------------------

class _BoxLabel( _PlotComponent ):

    kw_rename = {
            'face' : 'fontface',
            'size' : 'fontsize',
    }

    def __init__( self, obj, str, side, offset, **kw ):
        _PlotComponent.__init__( self )
        self.kw_init( kw )
        self.obj = obj
        self.str = str
        self.side = side
        self.offset = offset

    def make( self, context ):
        bb = self.obj.bbox( context )
        offset = _size_relative( self.offset, context.dev_bbox )
        if self.side == 'top':
            p = bb.upperleft()
            q = bb.upperright()
        elif self.side == 'bottom':
            p = bb.lowerleft()
            q = bb.lowerright()
            offset = -offset
        elif self.side == 'left':
            p = bb.lowerleft()
            q = bb.upperleft()
        elif self.side == 'right':
            p = bb.upperright()
            q = bb.lowerright()

        lt = _LineTextObject(p, q, self.str, offset,
                             **self.kw_style )
        self.add( lt )

# _PlotComposite --------------------------------------------------------------

class _PlotComposite( _StyleKeywords ):

    def __init__( self, **kw ):
        self.kw_init( kw )
        self.components = []
        self.dont_clip = 0

    def add( self, *args ):
        for obj in args:
            self.components.append( obj )

    def clear( self ):
        self.components = []

    def empty( self ):
        return len(self.components) == 0

    def limits( self ):
        bb = BoundingBox()
        for obj in self.components:
            bb.union( obj.limits() )
        return bb

    def make( self, context ):
        pass

    def bbox( self, context ):
        self.make( context )
        bb = BoundingBox()
        for obj in self.components:
            bb.union( obj.bbox(context) )
        return bb

    def render( self, context ):
        self.make( context )
        self.kw_predraw( context )
        if not self.dont_clip:
            context.do_clip()
        for obj in self.components:
            obj.render( context )
        self.kw_postdraw( context )

# Frame -----------------------------------------------------------------------

class Frame( _PlotComposite ):

    def __init__( self, labelticks=(0,1,1,0), **kw ):
        super(Frame,self).__init__(**kw)
        self.dont_clip = 1

        self.x2 = _HalfAxisX()
        self.x2.draw_ticklabels = labelticks[0]
        self.x2.ticklabels_dir = 1
        self.x2.ticklabels_style.update(**kw)

        self.x1 = _HalfAxisX()
        self.x1.draw_ticklabels = labelticks[1]
        self.x1.ticklabels_dir = -1
        self.x1.ticklabels_style.update(**kw)

        self.y1 = _HalfAxisY()
        self.y1.draw_ticklabels = labelticks[2]
        self.y1.ticklabels_dir = -1
        self.y1.ticklabels_style.update(**kw)

        self.y2 = _HalfAxisY()
        self.y2.draw_ticklabels = labelticks[3]
        self.y2.ticklabels_dir = 1
        self.y2.ticklabels_style.update(**kw)

    def make( self, context ):
        self.clear()
        self.add( self.x1, self.x2, self.y1, self.y2 )

# _PlotContainer --------------------------------------------------------------

'''
def _open_output( filename ):
    if filename == '-':
        import sys
        return sys.stdout
    else:
        # b is for windows
        return open( filename, 'wb' )

def _close_output( file ):
    if 2 < file.fileno():
        file.close()
    else:
        file.flush()
'''

def _draw_text( device, p, str, **kw ):
    device.save_state()
    for key,val in kw.items():
        device.set( key, val )
    device.text( p, str )
    device.restore_state()

def win_temp_path():
    """
    Intended for Windows, returns a valid temp directory, or
    at least the current working directory.
    """
    import os
    if os.environ.has_key('TEMP'):
        if os.path.exists(os.environ['TEMP']):
            return(os.environ['TEMP'])
    else:
        possible_temp_paths = [ '\\temp', '\\winnt\\temp', \
                '\\windows\\temp', '\\tmp' ]
        for i in possible_temp_paths:
            if os.path.exists(i):
                return i
    return os.getcwd()

class _PlotContainer( _ConfAttributes ):
    """
    plot container base class

    parameters
    ----------
    **keywords: optional keywords
    """
    def __init__( self, **kw ):
        self.conf_setattr( "_PlotContainer", **kw )

    def empty( self ):
        pass

    def interior( self, device, exterior ):
        TOL = 0.005

        interior = exterior.copy()
        region_diagonal = exterior.diagonal()

        for i in range(10):
            bb = self.exterior( device, interior )

            dll = pt_sub( exterior.lowerleft(), bb.lowerleft() )
            dur = pt_sub( exterior.upperright(), bb.upperright() )

            sll = pt_len(dll) / region_diagonal
            sur = pt_len(dur) / region_diagonal

            if sll < TOL and sur < TOL:
                # XXX:fixme
                if self.aspect_ratio is not None:
                    interior.make_aspect_ratio(\
                            self.aspect_ratio )
                return interior

            scale = interior.diagonal() / bb.diagonal()
            dll = pt_mul( scale, dll )
            dur = pt_mul( scale, dur )

            interior = BoundingBox(
                    pt_add(interior.lowerleft(), dll),
                    pt_add(interior.upperright(), dur) )

        raise BigglesError

    def exterior( self, device, interior ):
        return interior.copy()

    def compose_interior( self, device, interior ):
        if self.title is not None:
            offset = _size_relative( self.title_offset, interior )
            exterior = self.exterior( device, interior )
            x = interior.center()[0]
            y = exterior.yrange()[1] + offset
            style = self.title_style.copy()
            style["fontsize"] = _fontsize_relative( \
                    self.title_style["fontsize"], interior, device )
            style["texthalign"] = "center"
            style["textvalign"] = "bottom"
            _draw_text(device, (x,y), self.title, **style )

    def compose( self, device, region ):
        if self.empty():
            raise BigglesError( "empty container" )
        exterior = region.copy()
        if self.title is not None:
            offset = _size_relative( self.title_offset, exterior )
            fontsize = _fontsize_relative( \
                    self.title_style["fontsize"], exterior, device )
            exterior.deform( -offset-fontsize, 0, 0, 0 )
        interior = self.interior( device, exterior )
        self.compose_interior( device, interior )

    def page_compose( self, device ):
        device.open()
        bb = BoundingBox( device.lowerleft, device.upperright )
        device.bbox = bb.copy()
        for key,val in config.options('default').items():
            device.set( key, val )
        bb.expand( -self.page_margin )
        self.compose( device, bb )
        device.close()

    def show( self, width=None, height=None ):
        import os
        if width is None:
            width = config.value('screen','width')
        if height is None:
            height = config.value('screen','height')
        if os.name == 'posix':
            self.show_x11( width, height )
        elif os.name == 'dos' or os.name == 'nt':
            self.show_win( width, height )
        else:
            _message( "show: system type '%s' not supported" \
                    % os.name )

    def show_x11( self, width, height ):
        """
        show the plot in an X11 window
        """
        from .libplot.renderer import ScreenRenderer

        # need to check each time.  An example is when
        # using screen, and reattaching a running session
        # without an existing display

        if not X11_is_running():
            raise RuntimeError("No X11 display found")

        persistent = config.interactive() and \
                config.bool('screen','persistent')

        with ScreenRenderer( persistent, width, height ) as device:
            self.page_compose( device )

    def show_win( self, width, height ):
        """
        Substitute for show() that will work on Windows.
        Generates temporary files somewhere that end with
        '_biggles.png'. These temporary files are not deleted,
        they must be manually cleaned up during normal
        temp directory maintenance.
        """
        #tf = os.path.join( win_temp_path(), 'biggles_graph.png' )
        tf = tempfile.mktemp('_biggles.png')
        self.write_img( width, height, tf )
        os.startfile( tf )

    '''
    def psprint( self, printcmd=None, **kw ):
        import os, copy
        from .libplot.renderer import PSRenderer

        if os.name != 'posix':
            _message( "psprint: system type '%s' not supported" \
                    % os.name )
        if printcmd is None:
            printcmd = config.value("printer","command")
        opt = copy.copy( config.options("postscript") )
        opt.update( kw )
        _message( 'printing plot with "%s"' % printcmd )

        printer = os.popen( printcmd, 'w' )

        with PSRenderer(printer, **opt ) as device:
            self.page_compose( device )

        printer.close()
    '''

    def write_eps( self, outfile, **kw ):
        """
        write the plot to postscript. Extra keywords can be
        sent
        """
        from .libplot.renderer import PSRenderer

        opt = copy.copy( config.options("postscript") )
        opt.update( kw )

        with PSRenderer(outfile, **opt ) as device:
            self.page_compose( device )

    def write_pdf( self, pdfname, **kw ):
        """
        write the plot to pdf. Extra keywords can be
        sent, they will be passed onto the eps writer
        """

        epsname = tempfile.mktemp(suffix='.eps')

        self.write_eps(epsname, **kw)

        if not os.path.exists(epsname):
            raise RuntimeError("failed to write "
                               "temporary eps file '%s'" % epsname)

        self._convert_eps_to_pdf(epsname, pdfname)

        try:
            os.remove(epsname)
        except:
            pass

    def _convert_eps_to_pdf(self, epsname, pdfname):
        """
        convert using ghostscript (gs)
        """

        cmd = """
        gs \
                -dEPSCrop \
                -dSAFER \
                -dBATCH \
                -dNOPAUSE \
                -sDEVICE=pdfwrite \
                -sOutputFile={pdf} {eps} > /dev/null
        """
        cmd=cmd.format(eps=epsname, pdf=pdfname)

        ret=os.system(cmd)

        if ret != 0:
            raise RuntimeError("failed to convert %s to %s" % (epsname,pdfname))

    def _show_eps(self, epsname):
        """
        test using ghostscript with display device x11alpha

        this would work in principle for an antialiased
        x11 viewer, but ghostscript doesn't seem to want
        to go into the background, it either wants to
        be interactive or exit
        """

        cmd = """
        gs \
                -dSAFER \
                -dBATCH \
                -sDEVICE=x11alpha \
		-r75 {eps}
        """

        cmd=cmd.format(eps=epsname)

        ret=os.system(cmd)

    def write(self, outfile, **kw):
        """
        write PDF, EPS, or image files with antialiasing

        parameters
        ----------
        outfile: string
            Output file name
        type: string, optional
            File type, e.g. pdf, eps, png, jpg
        dpi: int, optional
            Optional dpi for image output, default 100
        **kw: keywords
            Other keywords for the eps writer
        """

        type=kw.pop('type',None)
        if type is None:
            type = outfile[-3:].lower()
        else:
            type=type.lower()

        if type == 'eps':
            self.write_eps(outfile, **kw)
        elif type == 'pdf':
            self.write_pdf(outfile, **kw)
        else:
            self._write_img_from_eps(type, outfile, **kw)

    def _write_img_from_eps(self, type, outfile, **kw):

        dpi = kw.pop('dpi',100)

        epsname = tempfile.mktemp(suffix='.eps')

        self.write_eps(epsname, **kw)

        if not os.path.exists(epsname):
            raise RuntimeError("failed to write "
                               "temporary eps file '%s'" % epsname)

        self._convert_eps_to_img(type, outfile, epsname, dpi, **kw)

        try:
            os.remove(epsname)
        except:
            pass

    def _convert_eps_to_img(self, type, outfile, epsname, dpi, **kw):

        if type=='png':
            device='png16m'
        elif type=='jpg':
            device='jpeg'
        else:
            raise NotImplementedError("image type should be png or jpg")

        cmd="""
        gs -dTextAlphaBits={bits} \
                -dGraphicsAlphaBits={bits} \
                -dEPSCrop \
                -dSAFER \
                -dBATCH \
                -dNOPAUSE \
                -r{dpi} \
                -sDEVICE={device} \
                -sOutputFile={ofile} \
                {eps} > /dev/null
        """
        cmd = cmd.format(
            bits=4,
            device=device,
            ofile=outfile,
            dpi=dpi,
            eps=epsname,
        )

        ret=os.system(cmd)

        if ret != 0:
            raise RuntimeError("failed to convert %s to %s" % (epsname,pdfname))


    def write_img( self, *args, **kw ):
        """
        Deprecated method to write an image file.  Use write() instead

        can be called in one of two ways

        write_imge(type, width, height, outfile )
        write_imge(width, height, outfile )

        In the second case, the type is inferred from the extension,
        e.g. ".png"

        """
        from .libplot.renderer import ImageRenderer

        antialias=kw.get('noaa',False)

        if len(args) == 4:
            type,width,height,outfile = args
        elif len(args) == 3:
            width,height,outfile = args
            type = outfile[-3:].lower()

        with ImageRenderer( type, width, height, outfile ) as device:
            self.page_compose( device )

    save_as_eps = write_eps
    save_as_img = write_img

    '''
    def draw_piddle( self, canvastype=None, size=(500,500) ):
        """
        what is this?

        There is file device from which to import, so I'm going
        to comment this
        """
        from device.piddle import PiddleRenderer
        device = PiddleRenderer( canvastype, size )
        self.page_compose( device )
        canvas = device.canvas
        return canvas
    '''

    def write_back_png( self, *args ):
        """
        Saves PNG file in temporary file. Returns file contents.
        """
        if len(args) == 2:
            width,height = args
        type = 'png'
        file = tempfile.mktemp('_biggles.png')
        self.write_img(type, width, height, file)
        f = open(file, 'rb')
        output = f.read()
        f.close()
        os.remove(file)
        return output

def multipage( plots, filename, **kw ):
    """
    Write the plot objects to the file as postscript, one per page

    parameters
    ----------
    plots: list
        A list of plot objects, e.g. FramedPlot etc.
    filename: string
        The file for the postscript file.
    **kw:
        Extra keywords
    """
    from .libplot.renderer import PSRenderer

    opt = copy.copy( config.options("postscript") )
    opt.update( kw )

    with PSRenderer(filename, **opt ) as device:
        for plot in plots:
            plot.page_compose( device )

# -----------------------------------------------------------------------------

def _limits_axis( content_range, gutter, user_range, log ):

    if not log:
        r0, r1 = 0, 1
    else:
        r0, r1 = -1, 0

    if content_range is not None:
        a, b = content_range
        if a is not None:
            r0 = math.log10(a) if log else a
        if b is not None:
            r1 = math.log10(b) if log else b

    if gutter is not None:
        dx = 0.5 * gutter * (r1 - r0)
        r0 = r0 - dx
        r1 = r1 + dx

    if user_range is not None:
        a, b = user_range
        if a is not None:
            r0 = math.log10(a) if log else a
        if b is not None:
            r1 = math.log10(b) if log else b

    if r0 == r1:
        r0 = r0 - 1
        r1 = r1 + 1

    if log:
        r0 = math.pow(10.0, r0)
        r1 = math.pow(10.0, r1)

    return r0, r1

def _limits( content_bbox, gutter, xlog, ylog, xrange, yrange ):

    xr = _limits_axis( content_bbox.xrange(), gutter, xrange, xlog )
    yr = _limits_axis( content_bbox.yrange(), gutter, yrange, ylog )

    return BoundingBox( (xr[0],yr[0]), (xr[1],yr[1]) )

# Plot ------------------------------------------------------------------------

class Plot( _PlotContainer ):

    def __init__( self, **kw ):
        super(Plot,self).__init__()
        self.conf_setattr( "Plot", **kw )
        self.content = _PlotComposite()

    def __iadd__( self, other ):
        self.add( other )

    def empty( self ):
        return self.content.empty()

    def add( self, *args ):
        self.content.add( *args )

    def limits( self ):
        return _limits( self.content.limits(), self.gutter, \
                self.xlog, self.ylog, self.xrange, self.yrange )

    def compose_interior( self, device, region, limits=None ):
        if limits is None:
            limits = self.limits()
        context = _PlotContext( device, region, limits,
                xlog=self.xlog, ylog=self.ylog )
        self.content.render( context )

    def compose( self, device, region, limits=None ):
        interior = self.interior( device, region )
        self.compose_interior( device, interior, limits )

# FramedPlot ------------------------------------------------------------------

class FramedPlot( _PlotContainer ):
    """
    A framed plot

    parameters
    ----------
    xlabel: string
        Label for the lower x axis
    ylabel: string
        Label for the left y axis


    xrange: sequence
        range for the x axis, e.g. [0.2, 5.5]
    yrange: sequence
        range for the y axis, e.g. [8, 100]

    xlog: bool
        Set True to use a log axis
    ylog: bool
        Set True to use a log axis

    title: string
        Title for the plot, above the top x axis
    title_offset: number
        The distance between the title and the container's contents.
    title_style: dict
        Dict describing the style of the title.
    aspect_ratio: float
        height/width

    key: PlotKey
        A PlotKey instance to be managed by the plot.  When
        components are added to the plot, they are also added
        to this key if they have a label attribute.  Note the
        user can still add PlotKey instances to the plot
        as normal using .add(key)

    """
    def __init__( self, **kw ):
        super(FramedPlot,self).__init__()


        self.content1 = _PlotComposite()
        self.content2 = _PlotComposite()

        # the user can send a key, and things are added to it
        key = kw.pop('key',None)
        if key is not None:
            self._add_key(key)

        self.x1 = _HalfAxisX()
        self.x1.ticklabels_dir = -1
        self.y1 = _HalfAxisY()
        self.y1.ticklabels_dir = -1
        self.x2 = _HalfAxisX()
        self.x2.draw_ticklabels = None
        self.y2 = _HalfAxisY()
        self.y2.draw_ticklabels = None
        self.frame = _Alias( self.x1, self.x2, self.y1, self.y2 )
        self.frame1 = _Alias( self.x1, self.y1 )
        self.frame2 = _Alias( self.x2, self.y2 )
        self.x = _Alias( self.x1, self.x2 )
        self.y = _Alias( self.y1, self.y2 )
        self.conf_setattr( "FramedPlot", **kw )


    _attr_map = {
            "xlabel"        : ("x1", "label"),
            "ylabel"        : ("y1", "label"),
            "xlog"          : ("x1", "log"),
            "ylog"          : ("y1", "log"),
            "xrange"        : ("x1", "range"),
            "yrange"        : ("y1", "range"),
            "xtitle"        : ("x1", "label"),
            "ytitle"        : ("y1", "label"),
    }

    def __repr__( self ):
        return "<biggles.FramedPlot instance>"

    def __getattr__( self, name ):
        if self._attr_map.has_key( name ):
            xs = self._attr_map[ name ]
            obj = self
            for x in xs[:-1]:
                obj = getattr( obj, x )
            return getattr( obj, xs[-1] )
        else:
            if name in self.__dict__:
                return self.__dict__[name]
            else:
                raise AttributeError("attribute not found: '%s'" % name)

    def __setattr__( self, name, value ):
        if self._attr_map.has_key( name ):
            xs = self._attr_map[ name ]
            obj = self
            for x in xs[:-1]:
                obj = getattr( obj, x )
            setattr( obj, xs[-1], value )
        else:
            self.__dict__[name] = value

    def empty( self ):
        return self.content1.empty() and self.content2.empty()

    def add( self, *args ):
        """
        Add components to the plot

        parameters
        -----------
        *args: plot objects
            any number of plot objects

        usage
        -----
        plt=FramedPlot()
        plt.add( points )
        plt.add( more_points, curve )
        """
        self.content1.add( *args )

        if hasattr(self,'key'):
            for arg in args:
                if hasattr(arg, 'label'):
                    self.key.add(arg)

    def add2( self, *args ):
        self.content2.add( *args )

    def _add_key(self, key):
        """
        Add a plot key to by managed by the plot.  This is treated specially,
        in that objects are added to the key if they are added to the plot, if
        they have label attributes

        parameters
        ----------
        key: PlotKey
            An instance of a PlotKey
        """

        self.key = key

        if isinstance(key, PlotKey):
            self.content1.add(key)
        else:
            raise ValueError("expected PlotKey, got '%s'" % type(key))



    def _xy2log( self ):
        return _first_not_none(self.x2.log, self.x1.log), \
               _first_not_none(self.y2.log, self.y1.log)

    def _limits1( self ):
        return _limits( self.content1.limits(),
                self.gutter, self.x1.log, self.y1.log, \
                self.x1.range, self.y1.range )

    def _context1( self, device, region ):
        return _PlotContext( device, region, self._limits1(),
                xlog=self.x1.log, ylog=self.y1.log )

    def _limits2( self ):
        limits = self.content2.limits()
        if self.content2.empty():
            limits = self.content1.limits()
        xlog, ylog = self._xy2log()
        xrange = _first_not_none( self.x2.range, self.x1.range )
        yrange = _first_not_none( self.y2.range, self.y1.range )
        return _limits( limits, self.gutter, xlog, ylog, \
                xrange, yrange )

    def _context2( self, device, region ):
        xlog, ylog = self._xy2log()
        return _PlotContext( device, region, self._limits2(),
                xlog, ylog )

    def exterior( self, device, region ):
        bbox = BoundingBox()

        context1 = self._context1( device, region )
        bbox.union( self.x1.bbox(context1) )
        bbox.union( self.y1.bbox(context1) )

        context2 = self._context2( device, region )
        bbox.union( self.x2.bbox(context2) )
        bbox.union( self.y2.bbox(context2) )

        return bbox

    def compose_interior( self, device, region ):
        _PlotContainer.compose_interior( self, device, region )

        context1 = self._context1( device, region )
        context2 = self._context2( device, region )

        self.content1.render( context1 )
        self.content2.render( context2 )

        self.y2.render( context2 )
        self.x2.render( context2 )
        self.y1.render( context1 )
        self.x1.render( context1 )

class OldCustomFramedPlot( FramedPlot ):

    def __init__( self, **kw ):
        super(OldCustomFramedPlot,self).__init__(**kw)
        self.x = self.x1
        self.y = self.y1

# Table -----------------------------------------------------------------------

class _Grid(object):

    def __init__( self, nrows, ncols, bbox,
                            cellpadding=0, cellspacing=0,
                            row_fractions=None,
                            col_fractions=None):
        self.nrows = nrows
        self.ncols = ncols

        if row_fractions is None:
            self.row_fractions = numpy.ones(self.nrows)/self.nrows
        else:
            self.row_fractions = numpy.array(row_fractions[::-1])/numpy.sum(row_fractions)

        if len(self.row_fractions) != self.nrows:
            raise BigglesError( "row_fractions must have length nrows!" )

        if col_fractions is None:
            self.col_fractions = numpy.ones(self.ncols)/self.ncols
        else:
            self.col_fractions = numpy.array(col_fractions[::-1])/numpy.sum(col_fractions)

        if len(self.col_fractions) != self.ncols:
            raise BigglesError( "col_fractions must have length ncols!" )

        w, h = bbox.width(), bbox.height()
        cp = _size_relative( cellpadding, bbox )
        self.cp = cp
        cs = _size_relative( cellspacing, bbox )
        self.cs = cs

        self.origin = pt_add( bbox.lowerleft(), (cp,cp) )
        self.step_x = w*self.col_fractions + cs/ncols
        self.step_y = h*self.row_fractions + cs/nrows

    def cell( self, i, j ):
        ii = self.nrows-1 - i
        p = pt_add( self.origin, (numpy.sum(self.step_x[:j]),numpy.sum(self.step_y[:ii])) )
        cell_dimen = self.step_x[j] - self.cs - 2*self.cp, \
                self.step_y[ii] - self.cs - 2*self.cp
        q = pt_add( p, cell_dimen )
        return BoundingBox( p, q )

class Table( _PlotContainer ):
    """
    A table of plots

    parameters
    ----------
    nrows: int
            Number of rows of plots
    ncols: int
            Number of columns of plots

    keywords:
            See the configuration options for Table for details (TODO copy
            into here)
    """

    def __init__( self, rows, cols, **kw ):
        super(Table,self).__init__()
        self.conf_setattr( "Table", **kw )
        self.rows = rows
        self.cols = cols
        self.content = {}

    def __getitem__( self, key ):
        return self.content[key]

    def __setitem__( self, key, value ):
        self.content[key] = value

    def set( self, i, j, obj ):
        self.content[i,j] = obj

    def get( self, i, j ):
        return self.content.get( (i,j), None )

    def exterior( self, device, interior ):
        ext = interior.copy()

        if self.align_interiors:
            g = _Grid( self.rows, self.cols, interior, \
                       self.cellpadding, self.cellspacing)

            for key,obj in self.content.items():
                subregion = g.cell( *key )
                ext.union( obj.exterior(device, subregion) )
        return ext

    def compose_interior( self, device, interior ):
        _PlotContainer.compose_interior( self, device, interior )

        g = _Grid( self.rows, self.cols, interior, \
                self.cellpadding, self.cellspacing)

        for key,obj in self.content.items():
            subregion = g.cell( *key )
            if self.align_interiors:
                obj.compose_interior( device, subregion )
            else:
                obj.compose( device, subregion )

# FramedArray -----------------------------------------------------------------
#
# Hideous, but it works...
#

def _frame_draw( obj, device, region, limits, labelticks=(0,1,1,0), fontsizefac=1.0 ):
    fontsize = config.value('default','fontsize')*fontsizefac
    frame = Frame( labelticks=labelticks, fontsize=fontsize )
    context = _PlotContext( device, region, limits,
            xlog=obj.xlog, ylog=obj.ylog )
    frame.render( context )

def _frame_bbox( obj, device, region, limits, labelticks=(0,1,1,0) ):
    frame = Frame( labelticks=labelticks )
    context = _PlotContext( device, region, limits,
            xlog=obj.xlog, ylog=obj.ylog )
    return frame.bbox( context )

def _range_union( a, b ):
    if a is None: return b
    if b is None: return a
    return min(a[0],b[0]), max(a[1],b[1])

class _FAFramedPlot(FramedPlot):
    """
    for use in a FramedArray
    """
    def limits(self):
        return self._limits1()

    def _limits1(self, limits=None):
        if limits is None:
            limits=super(_FAFramedPlot,self)._limits1()
        return limits

    def _limits2(self, limits=None):
        if limits is None:
            limits=super(_FAFramedPlot,self)._limits2()
        return limits

    def _context1( self, device, region, limits=None):
        return _PlotContext( device, region, self._limits1(limits=limits),
                xlog=self.x1.log, ylog=self.y1.log )

    def _context2( self, device, region, limits=None):

        if limits is None:
            limits = self._limits2()

        xlog, ylog = self._xy2log()
        return _PlotContext( device, region, limits,
                xlog, ylog )



    def compose_interior(self, device, region, limits=None):
        _PlotContainer.compose_interior( self, device, region )

        context1 = self._context1( device, region, limits=limits )
        context2 = self._context2( device, region, limits=limits )

        self.content1.render( context1 )
        self.content2.render( context2 )

        self.y2.render( context2 )
        self.x2.render( context2 )
        self.y1.render( context1 )
        self.x1.render( context1 )


class FramedArray( _PlotContainer ):
    """
    A framed array of plots

    parameters
    ----------
    nrows: int
            Number of rows of plots
    ncols: int
            Number of columns of plots

    uniform_limits: bool
        If True, the axislimits are set to be the same
        for all plots
    cellspacing: float
        Spacing between cells

    xlog, ylog: bool
        Set xlog and ylog for the entire frame.  These can also
        be set for individual plots
    xrange, yrange: bool
        Set xrange and yrange for the entire frame.  These can also
        be set for individual plots

    row_fractions: sequence
        The fraction of the frame taken up by each row of plots
    col_fractions: sequence
        The fraction of the frame taken up by each column of plots

    """
    def __init__( self, nrows, ncols, **kw ):
        super(FramedArray,self).__init__()
        self.nrows = nrows
        self.ncols = ncols
        self.row_fractions = None
        self.col_fractions = None
        self.content = {}
        for i in range(nrows):
            for j in range(ncols):
                plt = _FAFramedPlot()

                if i==self.nrows-1:
                    plt.x1.draw_ticklabels=1
                else:
                    plt.x1.draw_ticklabels=0

                if j==0:
                    plt.y1.draw_ticklabels=1
                else:
                    plt.y1.draw_ticklabels=0

                self.content[i,j] = plt

                self[i,j].visible=True

        self.conf_setattr( "FramedArray", **kw )

    _attr_distribute = [
            'gutter',
            'xlog',
            'ylog',
            'xrange',
            'yrange',
    ]

    _attr_deprecated = {
            'labeloffset'   : 'label_offset',
            'labelsize'     : 'label_size',
    }

    def __setattr__( self, name, value ):
        if name in self._attr_distribute:
            for obj in self.content.values():
                setattr( obj, name, value )
        else:
            _name = self._attr_deprecated.get( name, name )
            self.__dict__[_name] = value

    def __getitem__( self, key ):
        return self.content[key]

    def _limits( self, i, j ):
        if self.uniform_limits:
            return self._limits_uniform()
        else:
            return self._limits_nonuniform( i, j )

    def _limits_uniform( self ):
        limits = BoundingBox()
        for obj in self.content.values():
            limits.union( obj.limits() )
        return limits

    def _limits_nonuniform( self, i, j ):
        lx = None
        for k in range(self.nrows):
            if self[k,j].visible:
                l = self.content[k,j].limits()
                lx = _range_union( l.xrange(), lx )

        ly = None
        for k in range(self.ncols):
            if self[i,k].visible:
                l = self.content[i,k].limits()
                ly = _range_union( l.yrange(), ly )

        return BoundingBox( (lx[0],ly[0]), (lx[1],ly[1]) )

    def _grid( self, interior ):
        return _Grid( self.nrows, self.ncols, interior,
                cellspacing=self.cellspacing,
                row_fractions=self.row_fractions,
                col_fractions=self.col_fractions)

    def _frames_bbox( self, device, interior ):
        bb = BoundingBox()
        g = self._grid( interior )
        corners = [(0,0),(self.nrows-1,self.ncols-1)]

        for key in corners:
            obj = self.content[key]
            subregion = g.cell( *key )
            limits = self._limits( *key )
            axislabels = [0,0,0,0]
            if key[0] == self.nrows-1:
                axislabels[1] = 1
            if key[1] == 0:
                axislabels[2] = 1
            bb.union( _frame_bbox(obj, device, subregion, \
                            limits, axislabels) )

        return bb

    def exterior( self, device, interior ):
        bb = self._frames_bbox( device, interior )

        labeloffset = _size_relative( self.label_offset, interior )
        labelsize = _fontsize_relative( \
                self.label_size, interior, device )
        margin = labeloffset + labelsize

        if self.xlabel is not None:
            bb.deform( 0, margin, 0, 0 )
        if self.ylabel is not None:
            bb.deform( 0, 0, margin, 0 )

        return bb

    def _frames_draw( self, device, interior ):
        g = self._grid( interior )

        for key,obj in self.content.items():
            if self[key].visible:
                subregion = g.cell( *key )
                limits = self._limits( *key )
                axislabels = [0,0,0,0]
                #if key[0] == self.nrows-1:
                #    axislabels[1] = 1
                #if key[1] == 0:
                #    axislabels[2] = 1
                fontsizefac = 1.0
                if self.row_fractions is not None or self.col_fractions is not None:
                    if self.row_fractions is not None:
                        rnorm = self.row_fractions/numpy.sum(self.row_fractions)
                    else:
                        rnorm = numpy.ones(self.nrows)/self.nrows
                    if self.col_fractions is not None:
                        cnorm = self.col_fractions/numpy.sum(self.col_fractions)
                    else:
                        cnorm = numpy.ones(self.ncols)/self.ncols
                    gs =  _Grid( self.nrows, self.ncols, interior,
                                 cellspacing=self.cellspacing)
                    subregions = gs.cell( *key )
                    fontsizefac = subregions.width()*subregions.height()/(subregions.width() + subregions.height())
                    fontsizefac /= subregion.width()*subregion.height()/(subregion.width() + subregion.height())
                _frame_draw( obj, device, subregion, \
                             limits, axislabels, fontsizefac=fontsizefac )

    def _data_draw( self, device, interior ):
        g = self._grid( interior )

        for key,obj in self.content.items():
            if self[key].visible:
                subregion = g.cell( *key )
                limits = self._limits( *key )
                obj.compose_interior( device, subregion, limits )

    def _labels_draw( self, device, interior ):
        bb = self._frames_bbox( device, interior )

        labeloffset = _size_relative( self.label_offset, interior )
        labelsize = _fontsize_relative( \
                self.label_size, interior, device )

        device.save_state()
        device.set( 'fontsize', labelsize )
        device.set( 'texthalign', 'center' )
        if self.xlabel is not None:
            x = interior.center()[0]
            y = bb.yrange()[0] - labeloffset
            device.set( 'textvalign', 'top' )
            device.text( (x,y), self.xlabel )
        if self.ylabel is not None:
            x = bb.xrange()[0] - labeloffset
            y = interior.center()[1]
            device.set( 'textangle', 90. )
            device.set( 'textvalign', 'bottom' )
            device.text( (x,y), self.ylabel )
        device.restore_state()

    def add( self, *args ):
        for obj in self.content.values():
            obj.add( *args )

    def compose_interior( self, device, interior ):
        _PlotContainer.compose_interior( self, device, interior )
        self._data_draw( device, interior )
        self._frames_draw( device, interior )
        self._labels_draw( device, interior )

# Text ------------------------------------------------------------------------

class Text( _PlotContainer ):

    def __init__( self, text, **kw ):
        super(Text,self).__init__()
        self.conf_setattr( "Text", **kw )
        self.lines = text.split( "\n" )
        if self.lines[0] == '':
            del self.lines[0]
        if self.lines[-1] == '':
            del self.lines[-1]

    def compose( self, device, region ):
        device.save_state()
        context = _PlotContext( device, region, region )
        fontsize = _fontsize_relative(
                self.fontsize, context.dev_bbox, device )
        device.set( 'fontsize', fontsize )

        block_w = 0
        for line in self.lines:
            block_w = max( block_w, device.textwidth(line) )
        dy = fontsize * self.lineheight
        block_h = fontsize + dy * (len(self.lines)-1)

        x0, y0 = region.center()
        y0 = y0 + block_h/2.

        if self.halign == 'left':
            x0 = x0 - (region.width() - block_w)/2.
        elif self.halign == 'right':
            x0 = x0 + (region.width() - block_w)/2.

        if self.valign == 'top':
            y0 = y0 + (region.height() - block_h)/2.
        elif self.valign == 'bottom':
            y0 = y0 - (region.height() - block_h)/2.

        if self.justify == 'left':
            x0 = x0 - block_w/2
        elif self.justify == 'right':
            x0 = x0 + block_w/2

        ## render
        block = _PlotComposite()

        y = y0
        for line in self.lines:
            block.add( DataLabel(x0, y, line, \
                    halign=self.justify, valign='top', \
                    fontface=self.fontface, \
                    fontsize=self.fontsize) )
            y = y - dy

        block.render( context )
        device.restore_state()

# Inset -----------------------------------------------------------------------

class _Inset(object):

    def __init__( self, p, q, plot ):
        self.plot_limits = BoundingBox( p, q )
        self.plot = plot

    def render( self, context ):
        region = self.bbox( context )
        self.plot.compose_interior( context.draw, region )

class DataInset( _Inset ):

    def bbox( self, context ):
        p = context.geom( *self.plot_limits.lowerleft() )
        q = context.geom( *self.plot_limits.upperright() )
        return BoundingBox( p, q )

    def limits( self ):
        return self.plot_limits.copy()

class PlotInset( _Inset ):

    def bbox( self, context ):
        p = context.plot_geom( *self.plot_limits.lowerleft() )
        q = context.plot_geom( *self.plot_limits.upperright() )
        return BoundingBox( p, q )

    def limits( self ):
        return BoundingBox()
