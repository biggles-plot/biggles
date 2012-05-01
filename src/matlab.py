#
# $Id: matlab.py,v 1.5 2007/04/19 15:51:46 mrnolta Exp $
#
# Copyright (C) 2001 :
#
#   Jamie Mazer <mazer@socrates.berkeley.edu>
#   Mike Nolta <mrnolta@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#

#
#  matlab-like quick plot tools
#
#	import biggles.matlab
#
#	plot(x,y,style)
#	hist(n, nbins)
#	errorbar(x,y,e,style)
#	hold_on()
#	hold_off()
#	clf()
#	drawnow() **required**
#	subplot(nr,nc,n) or subplot(nr,nc,r,c) (note: r & c zerobased)
#
#  NOTE: PLOTS ARE NOT DRAWN UNTIL YOU CALL DRAWNOW()!!
#

import biggles
import math
import numpy
import string

class _MatlabEmulation:

	LINES	= [ "solid", "longdashed", "dotted" ]

	SYMBOLS	= [ "filled circle", "plus", "asterisk", "circle",
		"cross", "square", "triangle", "diamond", "star",
		"inverted triangle", "octagon", "filled square",
		"filled triangle", "filled diamond",
		"filled inverted triangle" ]

	COLORS	= [ "black", "red", "green", "blue", "magenta", "cyan" ]

	LAST	= None
	HOLD	= None
	TABLE	= None
	NROWS	= 1
	NCOLS	= 1
	R	= 0
	C	= 0

	def parse_style( self, s ):
		l,c,d = None, 'black', None
		if string.find(s, '--') >= 0:
			l = 'longdashed'
			s = string.replace(s, '--', '')
		elif string.find(s, '-') >= 0:
			l = 'solid'
			s = string.replace(s, '-', '')
		elif string.find(s, ':') >= 0:
			l = 'dotted'
			s = string.replace(s, ':', '')
	
		if 'y' in s: c='yellow'
		elif 'm' in s: c='magenta'
		elif 'c' in s: c='cyan'
		elif 'r' in s: c='red'
		elif 'g' in s: c='green'
		elif 'b' in s: c='blue'
		elif 'k' in s: c='black'
		elif 'w' in s: c='white'
	
		if '.' in s: d = 'dot'
		elif 'o' in s: d = 'filled circle'
		elif 'x' in s: d = 'cross'
		elif '+' in s: d = 'plus'
		elif '*' in s: d = 'asterisk'
		elif 's' in s: d = 'filled square'
		elif 'd' in s: d = 'filled diamond'
		elif '^' in s: d = 'triangle'
		elif 'v' in s: d = 'inverted triangle'
		elif '<' in s: d = 'filled triangle'
		elif '>' in s: d = 'filled inverted triangle'
		elif 'p' in s: d = 'octagon'
		elif 'h' in s: d = 'filled octagon'
	
		return l,c,d

	def get_table( self ):
		if self.TABLE is None:
			self.TABLE = biggles.Table( self.NROWS, self.NCOLS )
		return self.TABLE

	def get_plot( self, p=None ):
		if p is None:
			if self.HOLD:
				p = self.HOLD
			else:
				t = self.get_table()
				p = biggles.FramedPlot()
				t[ self.R, self.C ] = p
	
		try:
			nth = p.mplotnum + 1
		except KeyError:
			nth = 0
		p.mplotnum = nth

		return p

	def get_style( self, p, style=None ):
		n = p.mplotnum
		if style:
			return self.parse_style( style )
		else:
			return self.LINES[n], self.COLORS[n], self.SYMBOLS[n]

_matlab = _MatlabEmulation()

def subplot( nr, nc, r, c=None ):
	global _matlab
	if _matlab.NROWS != nr or _matlab.NCOLS != nc:
        	_matlab.TABLE = None
	_matlab.NROWS = nr
	_matlab.NCOLS = nc
	if c is None:
		_matlab.R = int(float(r-1)/float(nc))
		_matlab.C = int(0.5+math.fmod(float(r)-1.0, float(nc)))
	else:
		_matlab.R = r
		_matlab.C = c

def hold_on():
	global _matlab
	_matlab.HOLD = _matlab.LAST

def hold_off():
	global _matlab
	_matlab.HOLD = None

def clf():
	global _matlab
	_matlab.LAST = None
	_matlab.HOLD = None
	_matlab.TABLE = None
	_matlab.NROWS = 1
	_matlab.NCOLS = 1
	_matlab.R = 0
	_matlab.C = 0

def drawnow( width=None, height=None ):
	global _matlab
	if _matlab.TABLE:
		_matlab.TABLE.show( width=width, height=height )

def xlabel( s ):
	global _matlab
	if _matlab.LAST:
		_matlab.LAST.xlabel = s
        
def ylabel( s ):
	global _matlab
	if _matlab.LAST:
		_matlab.LAST.ylabel = s

def title( s ):
	global _matlab
	if _matlab.LAST:
		_matlab.LAST.title = s

def plot( x=None, y=None, style=None, show=None, p=None ):
	global _matlab
	p = _matlab.get_plot( p )
	lstyle, color, mstyle = _matlab.get_style( p, style )

	if y is None:
		y = x
		x = range(len(y))
        
	if mstyle is not None:
		p.add(biggles.Points(x, y, color=color, symboltype=mstyle))
	if lstyle is not None:
		p.add(biggles.Curve(x, y, color=color, linetype=lstyle))

	if show:
		p.show()
	_matlab.LAST = p
	return p

def errorbar( x, y, e, style='ko-', show=None, p=None ):
	global _matlab
	p = _matlab.get_plot( p )
	lstyle, color, mstyle = _matlab.get_style( p, style )

	p.add( biggles.SymmetricErrorBarsY(x, y, e, color=color) )

	if mstyle is not None:
		p.add( biggles.Points(x, y, color=color, type=mstyle) )
	if lstyle is not None:
		p.add( biggles.Curve(x, y, color=color, type=lstyle) )

	if show:
		p.show()
	_matlab.LAST = p
	return p

def hist( v, nbins=10, vmin=None, vmax=None, show=None, p=None ):
	global _matlab
	p = _matlab.get_plot( p )
        
	if vmin is None:
		vmin = float(min(v))
	if vmax is None:
		vmax = float(max(v))
	binwidth = (vmax - vmin) / float(nbins-1)
	x = numpy.arrayrange( vmin, vmax+binwidth, binwidth )
	y = numpy.zeros( x.shape, int )

	for i in range(len(v)):
		n = int(round((float(v[i]) - vmin) / binwidth, 0))
		try:
			y[n] = y[n] + 1
		except IndexError:
			pass

	xx = numpy.zeros( 2*len(x) + 3 )
	yy = numpy.zeros( 2*len(y) + 3 )
	xx[0] = x[0]
	yy[0] = 0

	for i in range(0, len(x)):
		xx[1+2*i],xx[1+2*i+1] = x[i],x[i]+binwidth
		yy[1+2*i],yy[1+2*i+1] = y[i],y[i]

	xx[1+2*i+2] = x[i]+binwidth
	yy[1+2*i+2] = 0;
	xx[1+2*i+3] = x[0];
	yy[1+2*i+3] = 0;

	p.add( biggles.FillBelow(xx, yy), biggles.Curve(xx, yy) )
	p.yrange = 0, max(y)

	if show is not None:
		p.show()
	_matlab.LAST = p
	return p

