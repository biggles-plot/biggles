#!/usr/bin/env python

import sys
sys.path.insert(1,'..')

import biggles
import string

err_msg = """
This example needs the following data file:

	http://biggles.sourceforge.net/data/continents
"""
		
try:
	m = biggles.read_rows( "continents" )
except IOError:
	print err_msg
	sys.exit(-1)

p = biggles.HammerAitoffPlot()
p.ribs_l = 2

for i in range(len(m)/2):
	l = m[2*i]
	b = m[2*i+1]
	p.add( biggles.Curve(l, b) )

#p.write_img( 400, 400, "example7.png" )
#p.write_eps( "example7.eps" )
p.show()
