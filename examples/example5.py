#!/usr/bin/env python

import sys
sys.path.insert(1,'..')

import biggles
import math, numpy 

x = numpy.arange( 0, 3*math.pi, math.pi/10 )
y = numpy.sin(x)

a = biggles.FramedArray( 2, 2, title='title' )
a.aspect_ratio = 0.75
a.xlabel = "x label"
a.ylabel = "y label"
a.uniform_limits = 1
a.cellspacing = 1.

a.add( biggles.LineY(0, type='dot') )

a[0,0].add( biggles.Curve(x, .25*y) )
a[0,1].add( biggles.Curve(x, .50*y) )
a[1,0].add( biggles.Curve(x, .75*y) )
a[1,1].add( biggles.Curve(x, y) )

#a.write_img( 400, 400, "example5.png" )
#a.write_eps( "example5.eps" )
a.show()
