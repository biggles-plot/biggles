#!/usr/bin/env python
import biggles

p = biggles.FramedPlot()
p.title = "title"
p.xlabel = r"$x$"
p.ylabel = r"$\Theta$"
p.ylog = 1
p.xlog = 1

p.add(biggles.LineX(0.5))
p.add(biggles.LineY(0.5))

p.write("example11.png", dpi=55)
p.write("example11.eps")
p.write("example11.pdf")
p.show()
