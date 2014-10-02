#
# $Id: hammer.py,v 1.25 2001/10/29 07:52:53 mrnolta Exp $
#
# Copyright (C) 2000-2001 :
#
#   Walter Brisken <walterfb@users.sourceforge.net>
#   Mike Nolta <mike@nolta.net>
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

#
# Hammer-Aitoff coordinates are an equal area projection of the
# sphere into the plane. The spherical coordinates l and b are
# used where l runs from -pi to pi and b from -pi/2 to pi/2.
# The equator is b=0, and b = +/-pi/2 are the north/south poles.
#
from __future__ import print_function
import math
from biggles import \
        _series, _PlotComposite, _PlotGeometry, _PlotContainer, Geodesic, Curve
from geometry import *
from . import _biggles

class _HammerAitoffGeometry(object):

    def __init__( self, dest, l0=0., b0=0, rot=0. ):
        self.src_bbox = BoundingBox( (-1.,-.5), (1.,.5) )
        self.dest_bbox = dest
        self.aff = RectilinearMap( self.src_bbox, dest )
        self.l0 = l0
        self.b0 = b0
        self.rot = rot

    def __call__( self, l_, b_ ):
        xh, yh = _biggles.hammer_call( \
                l_, b_, self.l0, self.b0, self.rot )
        return self.aff( xh, yh )

    def call_vec( self, l_, b_ ):
        xh, yh = _biggles.hammer_call_vec( \
                l_, b_, self.l0, self.b0, self.rot )
        return self.aff.call_vec( xh, yh )

    def geodesic( self, l_, b_, div=2 ):
        l, b = _biggles.hammer_geodesic_fill( l_, b_, div )
        segs = []
        i0 = 0
        for i in range(1,len(l)):
            if _biggles.hammer_connect( \
                            l[i-1], b[i-1], l[i], b[i], \
                            self.l0, self.b0, self.rot ):
                segs.append( (l[i0:i],b[i0:i]) )
                i0 = i
        segs.append( (l[i0:],b[i0:]) )
        return segs

class _HammerAitoffContext(object):

    def __init__( self, device, dev, l0=0., b0=0., rot=0. ):
        self.draw = device
        self.dev_bbox = dev
        self.geom = _HammerAitoffGeometry( dev, l0, b0, rot )
        self.plot_geom = _PlotGeometry( BoundingBox((0,0),(1,1)), dev )

    def do_clip( self ):
        pass

class HammerAitoffPlot( _PlotContainer ):

    _attr_deprecated = {
            "num_l_ribs"    : "ribs_l",
            "num_b_ribs"    : "ribs_b",
    }

    def __init__( self, l0=0., b0=0, rot=0., **kw ):
        super(HammerAitoffPlot,self).__init__()
        #apply( _PlotContainer.__init__, (self,) )
        self.conf_setattr( "HammerAitoffPlot", **kw )
        #apply( self.conf_setattr, ("HammerAitoffPlot",), kw )
        self.content = _PlotComposite()
        self.l0 = l0
        self.b0 = b0
        self.rot = rot

    _attr_deprecated = {
            "num_l_ribs"    : "ribs_l",
            "num_b_ribs"    : "ribs_b",
    }

    def __setattr__( self, name, value ):
        self.__dict__[ self._attr_deprecated.get(name,name) ] = value

    def __iadd__( self, other ):
        self.add( other )

    def add( self, *args ):
        self.content.add( *args )
        #apply( self.content.add, args )

    def _draw_background( self, context ):
        pc = _PlotComposite()

        nl = self.ribs_l
        b = _series( -90//2, 90//2, 2*math.pi/180. )
        for l0 in _series( -nl, nl, math.pi/nl ):
            c = Curve( [l0]*len(b), b, **self.ribs_style )
            #c = apply( Curve, ([l0]*len(b), b), self.ribs_style )
            pc.add( c )

        nb = self.ribs_b
        l = _series( -180//2, 180//2, 2*math.pi/180. )
        for b0 in _series( -nb, nb, 0.5*math.pi/(nb+1) ):
            c = Curve( l, [b0]*len(l), **self.ribs_style )
            #c = apply( Curve, (l, [b0]*len(l)), self.ribs_style )
            pc.add( c )

        pc.render( context )

    def exterior( self, device, interior ):
        bb = interior.copy()
        context = _HammerAitoffContext( device, interior,
                self.l0, self.b0, self.rot )
        bb.union( self.content.bbox(context) )
        return bb

    def compose_interior( self, device, interior ):
        _PlotContainer.compose_interior( self, device, interior )
        context = _HammerAitoffContext( device, interior,
                self.l0, self.b0, self.rot )
        self._draw_background( context )
        self.content.render( context )
