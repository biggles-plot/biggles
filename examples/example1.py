#!/usr/bin/env python

import sys
sys.path.insert(1,'..')

import biggles
import numpy, math

x = numpy.arange( 0, 3*math.pi, math.pi/30 )
c = numpy.cos(x)
s = numpy.sin(x)

p = biggles.FramedPlot()
p.title = "title"
p.xlabel = r"$x$"
p.ylabel = r"$\Theta$"

p.add( biggles.FillBetween(x, c, x, s) )
p.add( biggles.Curve(x, c, color="red") )
p.add( biggles.Curve(x, s, color="blue") )

#p.write_img( 400, 400, "example1.png" )
#p.write_eps( "example1.eps" )
p.show()
