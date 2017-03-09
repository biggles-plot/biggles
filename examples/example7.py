#!/usr/bin/env python
import biggles
import string

m = []
for line in open('continents.dat','r'):
    if line[0] == '\n':
        continue
    line = line.strip()
    # use list() for python 3
    row = list(map( float, line.split() ))
    m.append(row)

p = biggles.HammerAitoffPlot()
p.ribs_l = 2

p += [biggles.Curve(m[2*i], m[2*i+1]) for i in range(len(m)//2)]

p.write("example7.png", dpi=55)
p.write("example7.eps")
p.write("example7.pdf")
p.show()
