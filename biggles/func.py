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
    visible: bool
        If True, show plot on the screen.  Default True
    plt: biggles plot object
        If sent, add symbols or lines to this object.

    **keywords:
        Keywords for the FramedPlot instance (or whatever is passed though
        plt=), the Points, and Curve instances.

    Example keywords...

    # plot range keywords
    xrange: 2-element sequence
        Optional range for x axis
    yrange: 2-element sequence
        Optional range for y axis

    # marker type keywords can be sent explicitly as symboltype=, linetype=
    # or shorthand as type=.  If type= is sent, the marker type is
    # determined by the name (e.g. 'filled circle' implies a symbol
    # while 'longdashed' implies a line.
    #
    # If not type is sent, the overall biggles default is used (currently
    # open diamond symbol)
    #
    # Also if both symboltype= and linetype= are sent, then both will be
    # plotted

    type: string, optional keyword
        The marker type.  If one of 
            ["solid","dotted","dotdashed","shortdashed", "longdashed",
            "dotdotdashed","dotdotdotdashed"]
        then a Curve is plotted, else symbols.

    symboltype: string
        Explicitly specify a Point is to be plotted, with the
        indicated type.
    linetype: string
        Explicitly specify a Curve is to be plotted, with the
        indicated type.

    [symbol|line]color: string
        Color to be used for the marker.  Either the short color=
        can be used or the more explicity symbolcolor= or linecolor=
        can be used.

    xlabel: string
        Label for x axis.  Tex sequences are allowed, e.g.
        r'$\sigma$'
    ylabel: string
        Label for y axis.
    title: string
        Label for top of plot

    returned value
    ---------------
    The biggles plot object.

    examples
    --------

    import biggles
    biggles.plot(x, y, yerr=yerr, type='filled circle')
    """

    # deal with log plots and get a subset of points if needed
    xpts, ypts, xrng, yrng = _get_range_and_subpts(xin, yin, kw)

    # so the type= shorthand can be sent
    _set_markers_from_shorthand(kw)

    plt=_get_plot_object(plt, kw)

    linetype=kw.get('linetype',None)
    symboltype=kw.get('symboltype',None)

    # note we default to symbols if no type is set.  Also if both types are
    # sent, we plot both
    if symboltype is not None or (symboltype is None and linetype is None):
        plt.add(biggles.Points(xpts, ypts, **kw))

    if linetype is not None:
        plt.add(biggles.Curve(xpts, ypts, **kw))

    _add_error_bars(plt, xpts, ypts, xrng, yrng, kw)

    if visible:
        plt.show()
    return plt

def _set_markers_from_shorthand(keys):
    """
    set long-form marker type in the case where the shorthand type= is sent

    parameters
    -----------
    keys:
        Keywords to determine the marker type.  If type=
        is sent, try to figure out the implied marker type
        and set the full name
    """

    # shorthand for either symbol or line type
    type = keys.get('type', None)

    # figure out what marker type we have
    if type is not None:
        # figure out the marker type from the marker name
        if type in ["solid","dotted","dotdashed","shortdashed",
                    "longdashed","dotdotdashed","dotdotdotdashed"]:
            keys['linetype']=type
        else:
            keys['symboltype']=type

def get_log_plot_range(x, err=None, input_range=None, get_good=False):
    """
    Get a plot range in the case of log axes
    """
    if input_range is not None:
        if len(input_range) < 2:
            raise ValueError("expected [xmin,xmax] for input range")
        if input_range[0] <= 0. or input_range[1] <= 0.:
            raise ValueError("cannot use plot range < 0 for log plots, got [%s,%s]" % tuple(input_range))
        if get_good:
            w,=numpy.where((x >= input_range[0]) & (x <= input_range[1]))
            return input_range, w
        else:
            return input_range

    w,=numpy.where(x > 0.)
    if w.size == 0:
        raise ValueError("No values are greater than zero in log plot")

    minval = min(x[w])
    if err is not None:
        w2, = numpy.where( (x[w] - err[w]) > 0 )
        if w2.size > 0:
            minval2 =  min(x[w[w2]] - err[w[w2]])
            minval = min(minval,minval2)

        maxval = max(x+err)
    else:
        maxval = max(x)

    minval *= 0.5
    maxval *= 2

    if get_good:
        return [minval,maxval], w
    else:
        return [minval,maxval]

def add_log_error_bars(plt, axis, x, y, err, prange, **keys):
    from .biggles import ErrorBarsX, ErrorBarsY
    if axis == 'x':
        low = x-err
        high = x+err
    else:
        low = y-err
        high = y+err

    w,=numpy.where(high > 0)
    if w.size > 0:
        high = high[w]

        # outside range to avoid seeing hat
        low = low[w].clip(0.5*prange[0], 2.0*max(max(high),prange[1]) )

        if axis == 'x':
            p=biggles.ErrorBarsX(y[w], low, high, **keys)
        else:
            p=biggles.ErrorBarsY(x[w], low, high, **keys)
        plt.add(p)

        return p

def _add_error_bars(plt, x, y, xrng, yrng, keys_in):
    from .biggles import SymmetricErrorBarsY,SymmetricErrorBarsX

    xerr=keys_in.get('xerr',None)
    yerr=keys_in.get('yerr',None)

    if xerr is not None or yerr is not None:

        xlog = keys_in.get('xlog',False)
        ylog = keys_in.get('ylog',False)

        keys = copy.copy(keys_in)
        if 'errlinetype' in keys:
            if 'errlinetype' in keys:
                keys['linetype'] = keys['errlinetype']
            else:
                keys['linetype'] = 'solid'
        if 'errlinewidth' in keys:
            keys['linewidth'] = keys['errlinewidth']
        if 'errlinecolor' in keys:
            keys['linecolor'] = keys['errlinecolor']

        if yerr is not None:
            if ylog:
                add_log_error_bars(plt, 'y', x, y, yerr, yrng, **keys)
            else:
                p_yerr=SymmetricErrorBarsY(x, y, yerr, **keys)
                plt.add(p_yerr)
        if xerr is not None:
            if xlog:
                add_log_error_bars(plt, 'y', x, y, xerr, xrng, **keys)
            else:
                p_xerr=SymmetricErrorBarsX(x, y, xerr, **keys)
                plt.add(p_xerr)


def _get_range_and_subpts(xin, yin, keys):
    """
    keys['xerr'] and 'yerr' could be modified to be 1-d arrays in the range,
    so make sure keys was already a copy of original keys
    """

    x, y = _get_pts(xin, yin, keys)

    xerr=keys.get('xerr',None)
    yerr=keys.get('yerr',None)

    xlog = keys.get('xlog',False)
    ylog = keys.get('ylog',False)

    xrng = keys.get('xrange',None)
    yrng = keys.get('yrange',None)

    # For log, Don't plot points less than zero
    w=None
    if xlog and ylog:
        xrng = get_log_plot_range(x, err=xerr, input_range=xrng)
        yrng = get_log_plot_range(y, err=yerr, input_range=yrng)
        w,=numpy.where( (x > xrng[0]) & (y > yrng[0]) )
    elif xlog:
        xrng = get_log_plot_range(x, err=xerr, input_range=xrng)
        w,=numpy.where( x > xrng[0])
    elif ylog:
        yrng = get_log_plot_range(y, err=yerr, input_range=yrng)
        w,=numpy.where( y > yrng[0])

    if w is not None:
        if w.size == 0:
            raise ValueError("no points > 0 for log plot")
        x = x[w]
        y = y[w]

        if xerr is not None:
            xerr=xerr[w]
        if yerr is not None:
            yerr=yerr[w]

    keys['xerr']=xerr
    keys['yerr']=yerr

    return x, y, xrng, yrng

def _get_pts(xin, yin, keys):
    """
    xerr and yerr keys may be modified so make sure keys is
    already a copy of original dict
    """
    xpts=numpy.array(xin, ndmin=1, copy=False)
    ypts=numpy.array(yin, ndmin=1, copy=False)
    xerr = keys.get('xerr',None)
    yerr = keys.get('yerr',None)
    if xerr is not None:
        xerr=numpy.array(xerr, ndmin=1, copy=False)

        keys['xerr']=xerr
    if yerr is not None:
        yerr=numpy.array(yerr, ndmin=1, copy=False)
        keys['yerr']=yerr

    return xpts, ypts

def _get_plot_object(plt, keys):
    if plt is None:
        plt = biggles.FramedPlot(**keys)
    else:
        for key,value in keys.iteritems():
            if hasattr(plt,key):
                setattr(plt,key,value)

    return plt
