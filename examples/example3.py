#!/usr/bin/env python

import sys
sys.path.insert(1,'..')

import biggles
import math, numpy 

x = numpy.arange( 0, 3*math.pi, math.pi/10 )
y = numpy.sin(x)

p = biggles.FramedPlot()
p.title = "Title"
p.xlabel = "X axis"
p.ylabel = "Y axis"

p.add( biggles.Histogram(y) )
p.add( biggles.PlotLabel(.5, .5, "Histogram", color=0xcc0000) )

t1 = biggles.Table( 1, 2 )
t1[0,0] = p
t1[0,1] = p

t2 = biggles.Table( 2, 1 )
t2[0,0] = t1
t2[1,0] = p

#t2.write_img( 400, 400, "example3.png" )
#t2.write_eps( "example3.eps" )
t2.show()
