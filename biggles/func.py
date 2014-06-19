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
    A wrapper to perform a quick scatter plot with biggles.
    
    For anything more complex, it is better to use the object oriented
    interface.

    parameters
    ----------
    x, y: sequences or arrays
        The x,y data to plot
    yerr: sequence or array
        Optional error bars in the y direction
    xerr: sequence or array
        Optional error bars in the x direction
    xrange: 2-element sequence
        Optional range for x axis
    yrange: 2-element sequence
        Optional range for y axis
    symboltype: string
        Type for symbols
    symbolcolor: string
        Color for symbols
    linetype: string
        Type for lines.   Default 'solid'
    linecolor: string
        Color for lines
    xlabel: string
        Label for x axis.  Tex sequences are allowed, e.g.
        r'$\sigma$'
    ylabel: string
        Label for y axis.
    title: string
        Label for top of plot
    visible: bool
        If True, show plot on the screen.  Default True
    plt: biggles plot object
        If sent, add symbols or lines to this object.

    **keywords:
        Other keywords for the FramedPlot instance (or
        whatever is passed though plt=), the Points, 
        and Curve instances.

    returned value
    ---------------
    The biggles plot object.
    """
    if plt is None:
        plt = biggles.FramedPlot(**kw)
    else:
        for key,value in kw.items():
            if hasattr(plt,key):
                setattr(plt,key,value)

    #deal with log values
    xlog = kw.get('xlog',False)
    ylog = kw.get('ylog',False)

    xmin = -numpy.inf
    if xlog:
        xmin = 0.0

    ymin = -numpy.inf
    if ylog:
        ymin = 0.0

    w,=numpy.where( (xin > xmin) & (yin > ymin) )
    if len(w) == 0:
        raise ValueError("no points in range for plot")
    if xlog and ylog:
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
