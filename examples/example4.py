#!/usr/bin/env python

import sys
sys.path.insert(1,'..')

import biggles
import math, numpy 

x = numpy.arange( 0, 2*math.pi, math.pi/20 )
s = numpy.sin(x)
c = numpy.cos(x)

inset = biggles.FramedPlot()
inset.title = "inset"
inset.frame.draw_ticks = 0

inset.add( biggles.Curve(x, s, type="dashed") )

p = biggles.FramedPlot()
p.aspect_ratio = 1.
p.frame.tickdir = +1
p.frame.draw_spine = 0

p.add( biggles.SymmetricErrorBarsY(x, s, [0.2]*len(x)) )
p.add( biggles.Points(x, s, color="red") )
p.add( biggles.Inset((.6,.6), (.95,.95), inset) )

#p.write_img( 400, 400, "example4.png" )
#p.write_eps( "example4.eps" )
p.show()
