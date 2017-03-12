#!/usr/bin/env python

import biggles
import math
import numpy

x = numpy.arange(0, 3 * math.pi, math.pi / 10)
y = numpy.sin(x)

a = biggles.FramedArray(2, 2, title='title')
a.aspect_ratio = 0.75
a.xlabel = "x label"
a.ylabel = "y label"
a.uniform_limits = 1
a.cellspacing = 1.

a += biggles.LineY(0, type='dot')
a += [biggles.LineY(-1, type='dashed'), biggles.LineY(1, type='dashed')]

a[0, 0].add(biggles.Curve(x, .25 * y))
a[0, 1].add(biggles.Curve(x, .50 * y))
a[1, 0].add(biggles.Curve(x, .75 * y))
a[1, 1].add(biggles.Curve(x, y))

a.write("example5.png", dpi=55)
a.write("example5.eps")
a.write("example5.pdf")
a.show()
