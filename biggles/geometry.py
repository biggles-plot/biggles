#
# $Id: geometry.py,v 1.25 2007/04/19 15:51:46 mrnolta Exp $
#
# Copyright (C) 2000-2001 Mike Nolta <mike@nolta.net>
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

import math, numpy

# pt_* functions --------------------------------------------------------------

def pt_add( u, v ):
    return u[0] + v[0], u[1] + v[1]

def pt_sub( u, v ):
    return u[0] - v[0], u[1] - v[1]

def pt_mul( a, u ):
    return a * u[0], a * u[1]

def pt_rot( u, angle ):
    c, s = math.cos(angle), math.sin(angle)
    return c*u[0] - s*u[1], s*u[0] + c*u[1]

def pt_len( u ):
    return math.hypot( u[0], u[1] )

def pt_angle( u ):
    return math.atan2( u[1], u[0] )

def pt_unit( u ):
    r = pt_len(u)
    return u[0]/r, u[1]/r

def pt_min( a, b ):
    if a is None: return b
    if b is None: return a

    # deal with python 3 change w.r.t. None, which
    # can no longer be considered ordered.  In python2
    # it always compared less
    if a[0] is None:
        min1=b[0]
    elif b[0] is None:
        min1=a[0]
    else:
        min1=min(a[0], b[0])

    if a[1] is None:
        min2=b[1]
    elif b[1] is None:
        min2=a[1]
    else:
        min2=min(a[1], b[1])

    return min1, min2

def pt_max( a, b ):
    if a is None: return b
    if b is None: return a

    # deal with python 3 change w.r.t. None, which
    # can no longer be considered ordered.  In python2
    # it always compared less
    if a[0] is not None and b[0] is not None:
        max1 = max( a[0], b[0] )
    else:
        if a[0] is None:
            max1=b[0]
        else:
            max1=a[0]

    if a[1] is not None and b[1] is not None:
        max2 = max( a[1], b[1] )
    else:
        if a[1] is None:
            max2=b[1]
        else:
            max2=a[1]

    return max1,max2

# BoundingBox -----------------------------------------------------------------

class BoundingBox(object):

    def __init__( self, *args ):
        if len(args) > 0:
            self.p0 = reduce( pt_min, args )
            self.p1 = reduce( pt_max, args )
        else:
            self.p0 = None
            self.p1 = None

    def __str__( self ):
        return "(%s,%s)" % (str(self.p0), str(self.p1))

    def copy( self ):
        return BoundingBox( self.p0, self.p1 )

    def is_null( self ):
        return self.p0 is None or self.p1 is None

    def width( self ):
        if self.is_null():
            return None
        else:
            return abs( self.p0[0] - self.p1[0] )

    def height( self ):
        if self.is_null():
            return None
        else:
            return abs( self.p0[1] - self.p1[1] )

    def diagonal( self ):
        if self.is_null():
            return None
        else:
            return math.hypot( self.width(), self.height() )

    def aspect_ratio( self ):
        if self.is_null():
            return None
        else:
            return self.height()/self.width()

    def xrange( self ):
        if self.is_null():
            return None
        else:
            return self.p0[0], self.p1[0]

    def yrange( self ):
        if self.is_null():
            return None
        else:
            return self.p0[1], self.p1[1]

    def lowerleft( self ):
        if self.is_null():
            return None
        else:
            return self.p0

    def upperleft( self ):
        if self.is_null():
            return None
        else:
            return self.p0[0], self.p1[1]

    def upperright( self ):
        if self.is_null():
            return None
        else:
            return self.p1

    def lowerright( self ):
        if self.is_null():
            return None
        else:
            return self.p1[0], self.p0[1]

    def center( self ):
        x = self.xrange()
        y = self.yrange()
        return (x[0]+x[1])/2., (y[0]+y[1])/2.

    def union( self, other ):
        self.p0 = pt_min( self.p0, other.p0 )
        self.p1 = pt_max( self.p1, other.p1 )

    def deform( self, dt, db, dl, dr ):
        self.p0 = pt_sub( self.p0, (dl,db) )
        self.p1 = pt_add( self.p1, (dr,dt) )

    def shift( self, dp ):
        self.p0 = pt_add( self.p0, dp )
        self.p1 = pt_add( self.p1, dp )

    def expand( self, factor ):
        dp = pt_mul( factor/2., (self.width(), self.height()) )
        self.p0 = pt_sub( self.p0, dp )
        self.p1 = pt_add( self.p1, dp )

    def rotate( self, angle, p ):
        a = pt_add(pt_rot(pt_sub( self.lowerleft(), p), angle), p)
        b = pt_add(pt_rot(pt_sub(self.lowerright(), p), angle), p)
        c = pt_add(pt_rot(pt_sub( self.upperleft(), p), angle), p)
        d = pt_add(pt_rot(pt_sub(self.upperright(), p), angle), p)
        self.p0 = pt_min( a, pt_min( b, pt_min( c, d ) ) )
        self.p1 = pt_max( a, pt_max( b, pt_max( c, d ) ) )

    def make_aspect_ratio( self, ratio ):
        if ratio < self.aspect_ratio():
            dh = self.height() - ratio * self.width()
            self.p0 = self.p0[0], self.p0[1] + dh/2
            self.p1 = self.p1[0], self.p1[1] - dh/2
        else:
            dw = self.width() - self.height() / ratio
            self.p0 = self.p0[0] + dw/2, self.p0[1]
            self.p1 = self.p1[0] - dw/2, self.p1[1]

    def contains( self, q ):
        if self.p0[0] <= q[0] and \
           q[0] <= self.p1[0] and \
           self.p0[1] <= q[1] and \
           q[1] <= self.p1[1]:
            return 1
        else:
            return 0

# AffineTransform -------------------------------------------------------------

def _matrix_multipy( A, B ):
    C00 = A[0][0] * B[0][0] + A[0][1] * B[1][0]
    C01 = A[0][0] * B[0][1] + A[0][1] * B[1][1]
    C10 = A[1][0] * B[0][0] + A[1][1] * B[1][0]
    C11 = A[1][0] * B[0][1] + A[1][1] * B[1][1]
    return (C00, C01), (C10, C11)

class AffineTransform(object):

    def __init__( self ):
        self.t = 0., 0.
        self.m = (1., 0.), (0., 1.)

    def __call__( self, x, y ):
        p = self.t[0] + self.m[0][0] * x + self.m[0][1] * y
        q = self.t[1] + self.m[1][0] * x + self.m[1][1] * y
        return p, q

    def call_vec( self, x, y ):
        x_ = numpy.asarray( x )
        y_ = numpy.asarray( y )
        p = self.t[0] + self.m[0][0] * x_ + self.m[0][1] * y_
        q = self.t[1] + self.m[1][0] * x_ + self.m[1][1] * y_
        return p, q

    def compose( self, other ):
        self.t = self( other.t[0], other.t[1] )
        self.m = _matrix_multiply( self.m, other.m )

class RectilinearMap( AffineTransform ):

    def __init__( self, src, dest ):
        super(RectilinearMap,self).__init__()
        #AffineTransform.__init__( self )
        sx = dest.width() / src.width()
        sy = dest.height() / src.height()
        p, q = dest.lowerleft(), src.lowerleft()
        tx = p[0] - sx * q[0]
        ty = p[1] - sy * q[1]
        self.t = tx, ty
        self.m = ( sx, 0. ), ( 0., sy )
