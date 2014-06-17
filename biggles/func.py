#
# $Id: func.py,v 1.21 2007/04/19 15:51:46 mrnolta Exp $
#
# Copyright (C) 2000-2001 Mike Nolta <mike@nolta.net>
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

import biggles, config
import string
import numpy
import itertools,operator,copy

def plot(xin, yin, visible=True, plt=None, **kw):
	"""
	Name:
	plot
	Purpose:
	A wrapper to perform a quick scatter plot with biggles.  For anything
	more complex, it is better to use the object oriented interface.
	
	Calling Sequence:
	plot(x, y, 
             xerr=None, 
             yerr=None,
             xrange=None,
             yrange=None,
             symboltype=None,
             symbolcolor=None,
             linecolor=None,
             linetype='Solid'
             xlabel=None, 
             ylabel=None, 
             title=None,
             visible=True
             plt=None)
	
	Return value is the used biggles plot object.
	
	For overplotting, send an existing biggles plot object in the plt= keyword
	
	"""
	if plt is None:
		plt = biggles.FramedPlot(**kw)
	else:
		for key,value in kw.items():
			if hasattr(plt,key):
				setattr(plt,key,value)

	#deal with log values
	xlog = kw.get('xlog',0)
	ylog = kw.get('ylog',0)

	xmin = -numpy.inf
	if xlog == 1:
		xmin = 0.0

	ymin = -numpy.inf
	if ylog == 1:
		ymin = 0.0

	w,=numpy.where( (xin > xmin) & (yin > ymin) )
	if len(w) == 0:
		raise ValueError("no points in range for plot")
	if xlog == 0 and ylog == 0:
		assert len(w) == len(xin) or len(w) == len(yin)

	# by default plot a line, but use a symbol if it is in the keywords
	if 'symboltype' not in kw.keys() or ('symboltype' in kw.keys() and 'linetype' in kw.keys()):
		for key, grps in itertools.groupby(enumerate(w), lambda (i,x):i-x):
			wgrp = map(operator.itemgetter(1), grps)
			plt.add(biggles.Curve(xin[wgrp], yin[wgrp], **kw))

	if 'symboltype' in kw.keys():
		kwsym = copy.copy(kw)
		if 'linecolor' in kwsym.keys():
			kwsym.pop('linecolor',None)
		if 'symbolcolor' in kwsym.keys():
			kwsym['color'] = kwsym['symbolcolor']
		plt.add(biggles.Points(xin[w], yin[w], **kwsym))

	#do error bars
	xerr = kw.get('xerr',None)
	yerr = kw.get('yerr',None)
	kwerr = copy.copy(kw)
	if 'errlinetype' in kw.keys():
		if 'errlinetype' in kw.keys():
			kwerr['linetype'] = kwerr['errlinetype']
		else:
			kwerr['linetype'] = 'solid'
	if 'errlinewidth' in kw.keys():
		kwerr['linewidth'] = kwerr['errlinewidth']
	if 'errlinecolor' in kw.keys():
		kwerr['linecolor'] = kwerr['errlinecolor']

	if xerr is not None:
		low = xin-xerr
		high = xin+xerr
		q, = numpy.where( (low[w] > xmin) & (high[w] > xmin) )
		if len(q) > 0:
			plt.add(biggles.ErrorBarsX(yin[w[q]], low[w[q]], high[w[q]], **kwerr))

	if yerr is not None:
		low = yin-yerr
		high = yin+yerr
		q, = numpy.where( (low[w] > ymin) & (high[w] > ymin) )
		if len(q) > 0:
			plt.add(biggles.ErrorBarsY(xin[w[q]], low[w[q]], high[w[q]], **kwerr))

	#get bounding box for the data as is, and then add in extra error
	# bars where the point falls of the edge
	bb = plt._limits1()
	bbxmin,bbxmax = bb.xrange()
	bbymin,bbymax = bb.yrange()

	if xerr is not None:
		low = xin-xerr
		high = xin+xerr
		ql, = numpy.where( (yin > ymin) & (high > bbxmin) & (low <= bbxmin) )
		if len(ql) > 0:
			low[ql[:]] = bbxmin
			plt.add(biggles.ErrorBarsX(yin[ql], low[ql], high[ql], **kwerr))

		low = xin-xerr
		high = xin+xerr
		qh, = numpy.where( (yin > ymin) & (high >= bbxmax) & (low < bbxmax) )
		if len(qh) > 0:
			high[qh[:]] = bbxmax
			plt.add(biggles.ErrorBarsX(yin[qh], low[qh], high[qh], **kwerr))

		if 'xrange' not in kw.keys() and (len(ql) > 0 or len(qh) > 0):
			plt.xrange = (bbxmin,bbxmax)

	if yerr is not None:
		low = yin-yerr
		high = yin+yerr
		ql, = numpy.where( (xin > xmin) & (high > bbymin) & (low <= bbymin) )
		if len(ql) > 0:
			low[ql[:]] = bbymin
			plt.add(biggles.ErrorBarsY(xin[ql], low[ql], high[ql], **kwerr))

		low = yin-yerr
		high = yin+yerr
		qh, = numpy.where( (xin > xmin) & (high >= bbymax) & (low < bbymax) )
		if len(qh) > 0:
			high[qh[:]] = bbymax
			plt.add(biggles.ErrorBarsY(xin[qh], low[qh], high[qh], **kwerr))

		if 'yrange' not in kw.keys() and (len(ql) > 0 or len(qh) > 0):
			plt.yrange = (bbymin,bbymax)

	if visible:
		plt.show()
	return plt

def read_column( num, filename, atox=float, \
		comment_char=None, return_numpy=None ):
	import string
	if comment_char is None:
		comment_char = config.value('read_column','comment_char')
	if return_numpy is None:
		return_numpy = config.value('read_column','return_numpy')
	x = []
	f = open( filename )
	lines = map( string.strip, f.readlines() )
	f.close()
	lines = filter( None, lines )	# get rid of ''
	for line in lines:
		if line[0] == comment_char[0]:
			continue
		column = string.split( line )
		x.append( column[num] )
	x = map( atox, x )
	if return_numpy:
		import numpy
		return numpy.array( x )
	return x

def read_rows( filename, atox=float, comment_char=None, return_numpy=None ):
	import string
	if comment_char is None:
		comment_char = config.value('read_rows','comment_char')
	if return_numpy is None:
		return_numpy = config.value('read_rows','return_numpy')
	if return_numpy:
		import numpy
	x = []
	f = open( filename )
	lines = map( string.strip, f.readlines() )
	f.close()
	lines = filter( None, lines )	# get rid of blank lines
	for line in lines:
		if line[0] == comment_char[0]:
			continue
		row = map( atox, string.split(line) )
		if return_numpy:
			row = numpy.array( row )
		x.append( row )
	return x

def read_matrix( filename, atox=float, comment_char=None, return_numpy=None ):
	if comment_char is None:
		comment_char = config.value('read_matrix','comment_char')
	if return_numpy is None:
		return_numpy = config.value('read_matrix','return_numpy')
	m = read_rows( filename, atox=atox, comment_char=comment_char, \
		return_numpy=0 )
	if return_numpy:
		import numpy
		return numpy.array( m )
	return m

