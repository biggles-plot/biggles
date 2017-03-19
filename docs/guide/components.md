## Conventions
1. All variables are scalars except

     - `"s"` means `s` is a string
     - `x[]` means `x` is a numeric sequence
     - `m[,]` means m is a 2D matrix
     - `(p)` means `p` is an (x,y) tuple

2. Abbreviated keywords are those for which the portion in brackets may be omitted.

3. The `Plot*` objects take plot coordinates, where the lower-left corner of the plot is at (0,0)
and the upper-right corner is at (1,1).

## Lines

**Curve(x[], y[])**

 - Draws lines connecting `(x[i], y[i])` to `(x[i+1], y[i+1])`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**DataArc(p, r, a0, a1) [== Arc] / PlotArc(p, r, a0, a1)**

 - Draw an arc about `p` of radius `r` from angle `a0` to `a1`. Angles are in radians.
 - Abbreviated keywords: None

**DataLine((p), (q)) [== Line] / PlotLine((p), (q))**

 - Draws a line connecting points `p` and `q`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**ErrorBarsX(y[], xerr_lo[], xerr_hi[]) / ErrorBarsY(x[], yerr_lo[], yerr_hi[])**

 - Draws X/Y error bars. Specifically, the bars extend from `(xerr_lo[i], y[i])` to `(xerr_hi[i], y[i])` for `ErrorBarsX`, and `(x[i], yerr_lo[i])` to `(x[i], yerr_hi[i])` for `ErrorBarsY`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**Geodesic((p)), (q))**

 - Draws a geodesic connecting points `p` and `q`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**LineX(x) / LineY(y)**

 - Draws a line parallel to the X/Y-axis.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**LowerLimits(x[], lim[])**

 - Draws lower limits of value `lim[]` at points `x[]`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**Slope(slope, p=(0,0))**

 - Draws the line `y = p[1] + slope*(x - p[0])`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**SymmetricErrorBarsX(x[], y[], err[]) / SymmetricErrorBarsY(x[], y[], err[])**

 - Draws error bars extending from `(x[i]-err[i], y[i])` to `(x[i]+err[i], y[i])` for `SymmetricErrorBarsX`, and
   `(x[i], y[i]-err[i])` to `(x[i], y[i]+err[i])` for `SymmetricErrorBarsY`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**UpperLimits(x[], lim[])**

 - Draws lower limits of value `lim[]` at points `x[]`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.


## Points and Polygons

**Circle(x, y, r) / Circles(x[], y[], r[])**

 - Draws circles centered at `(x,y)` with radius `r[]`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**ColoredPoint(x, y, c) / ColoredPoints(x, y, c)**

 - Draws a set of points with colors given by `c`.

**DataBox((p), (q)) [== Box] / PlotBox((p), (q))**

 - Draws the rectangle defined by points `p` and `q`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.

**Ellipse(x, y, rx, ry, angle=None) / Ellipses(x[], y[], rx[], ry[], angle[]=None)**

 - Draws ellipses centered at `(x,y)`, with x-radius `rx`, y-radius `ry`, and rotated counterclockwise by `angle`.

**Point(x, y) / Points(x[], y[])**

 - Draws symbols at the set of points `(x,y)`.
 - Abbreviated keywords: `[symbol]size`, `[symbol]type`.

**Polygon(x[], y[])**

 - Draws a polygon by connecting all of the points `(x,y)`.


## Images and Filled Regions
Countour(s), Density, Histogram, FillAbove, FillBelow, FillBetween

**Contour(z[,], x[], y[], z0) / Contours(z[,], x[]=None, y[]=None, (zrange)=None)**

 - Draws isocontours for a given 2D Numeric matrix `z`. The first index of `z` is plotted along the x-axis and the second index along the y-axis, at the values given by the vectors `x` and `y`.
 - Abbreviated keywords: `func_linestyle`, `func_color`, `func_linewidth`.

**Density(m[,], ((xmin,xmax), (ymin,ymax)))**

 - Renders an image. If `m[,]` is NxMx3 dimensional, the image will be colored, otherwise greyscale is used. The range
   tuple `((xmin,xmax), (ymin,ymax))` controls where the image is rendered.

**FillAbove(x[], y[]) / FillBelow(x[], y[])**

 - `FillAbove/Below` fills the region bounded below/above, respectively, by the curve `(x,y)`.

**FillBetween(xA[], yA[], xB[], yB[])**

 - Fill the region bounded by the curves `(xA,yA)` and `(xB,yB)`.

**Histogram(y[], x0=0, binsize=1)**

 - Draws a histogram of the `y` values. The x-range for `y[i]` is `(x0 + i*binsize, x0 + (i + 1)*binsize)`.
 - Abbreviated keywords: `[line]color`, `[line]type`, `[line]width`.


## Plot Keys and Labels

**DataLabel(x, y, "label") [== Label] / PlotLabel(x, y, "label")**

 - Write the text string at the position `(x,y)`. Alignment is governed by `halign` and `valign`.
 - Abbreviated keywords: `[font]face`, `[font]size`, `[text]angle`, `[text]halign`, `[text]valign`.

**PlotKey (x, y, components)**

 - Hmm... you should probably just take a look at the [example](https://github.com/biggles-plot/biggles/blob/master/examples/example2.py).
 - Abbreviated keywords: `[font]face`, `[font]size`, `[text]angle`, `[text]halign`, `[text]valign`.


## Inset Panels

**DataInset((p), (q), container) / PlotInset((p), (q), container) [== Inset]**

 - Draws container in the rectangle defined by points `p` and `q`.
