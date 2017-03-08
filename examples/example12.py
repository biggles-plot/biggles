#!/usr/bin/env python
import biggles

p = biggles.FramedPlot()
p.title = "triangle"
p.xlabel = r"$x$"
p.ylabel = r"$y$"

p.add(biggles.Polygon([0, 1, 0.5], [0, 0, 1]))

p.write("example12.png", dpi=55)
p.write("example12.eps")
p.write("example12.pdf")
p.show()
