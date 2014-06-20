#!/usr/bin/env python
import biggles
import string

m = []
for line in open('continents.dat','r'):
    if line[0] == '\n':
        continue
    line = line.strip()
    row = map( float, line.split() )
    m.append(row)

p = biggles.HammerAitoffPlot()
p.ribs_l = 2

for i in range(len(m)/2):
    l = m[2*i]
    b = m[2*i+1]
    p.add( biggles.Curve(l, b) )

#p.write_img( 400, 400, "example7.png" )
#p.write_eps( "example7.eps" )
p.show()
