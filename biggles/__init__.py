#
# $Id: __init__.py,v 1.82 2010/04/09 21:28:15 mrnolta Exp $
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

__version__ = '1.7.0'

from .biggles import (
    Circle,
    Circles,
    ColoredPoint,
    ColoredPoints,
    Curve,
    DataArc,
    DataBox,
    DataInset,
    DataLabel,
    DataLine,
    Density,
    Ellipse,
    Ellipses,
    ErrorBarsX,
    ErrorBarsY,
    FillAbove,
    FillBelow,
    FillBetween,
    FramedArray,
    FramedPlot,
    Geodesic,
    Histogram,
    Labels,
    LineX,
    LineY,
    LowerLimits,
    OldCustomFramedPlot,
    OldKey,
    Plot,
    PlotArc,
    PlotBox,
    PlotInset,
    PlotKey,
    PlotLabel,
    PlotLine,
    Point,
    Points,
    Slope,
    SymmetricErrorBarsX,
    SymmetricErrorBarsY,
    Table,
    Text,
    UpperLimits,
    multipage,
    X11_is_running,
)

from .config import               \
    configure

from .contour import              \
    Contour                 ,\
    Contours

from .func import             \
    plot                    ,\
    make_hist               ,\
    make_histc              ,\
    plot_hist

from .hammer import               \
    HammerAitoffPlot

# aliases
Arc = DataArc
Box = DataBox
Inset = PlotInset
Label = DataLabel
Line = DataLine
SymmetricXErrorBars = SymmetricErrorBarsX
SymmetricYErrorBars = SymmetricErrorBarsY
XErrorBars = ErrorBarsX
YErrorBars = ErrorBarsY

class _deprecated:

    def __init__( self, obj, old, new, harrass=1 ):
        self.obj = obj
        self.old = old
        self.new = new
        self.harrass = harrass

    def __call__( self, *args, **kw ):
        import sys
        if self.harrass == 1:
            sys.stderr.write( \
                    "biggles: %s is deprecated - use %s instead\n" \
                    % (self.old, self.new) )
        return apply( self.obj, args, kw )

# XXX:deprecated 1.0
Plot2D = _deprecated( FramedPlot, "Plot2D", "FramedPlot" )

# XXX:deprecated 1.3
ErrorEllipses = _deprecated( Ellipses, "ErrorEllipses", "Ellipses"  )
LabelData = _deprecated( DataLabel, "LabelData", "DataLabel" )
LabelPlot = _deprecated( PlotLabel, "LabelPlot", "PlotLabel" )
LineKey = _deprecated( OldKey, "LineKey", "PlotKey" )
LineSlope = _deprecated( Slope, "LineSlope", "Slope" )
SymbolKey = _deprecated( OldKey, "SymbolKey", "PlotKey" )

# XXX:deprecated 1.5
CustomFramedPlot = _deprecated( \
        OldCustomFramedPlot, "CustomFramedPlot", "FramedPlot", 0 )

try:
    del _biggles
    del biggles
    del config
    del contour
    del func
    del geometry
    del hammer
except NameError:
    pass
