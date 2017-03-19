## FramedPlot
This object represents a framed plot, where the axes surround the plotting region, instead of intersecting it. You build a plot by adding components, using:

```python
p = FramedPlot()
p.add(component, ...)
p += component
p += component1, component2, component3
```

where p is a `FramedPlot` object. Components are rendered in the order they're added.

**Basic Attributes:**

Many of these can be sent as keywords during construction of the `FramedPlot` instance.

- axis labels

```python
.xlabel = None | string  
.ylabel = None | string
```

- axis scaling

```python
.xlog = 0 | 1  
.ylog = 0 | 1
```
  If 1 use log scaling, otherwise linear.

- axis range

```python
.xrange = None | (number, number)  
.yrange = None | (number, number)
```

These attributes should be sufficient for casual use, but often you'll want greater control over the frame.

**Axis Attributes:**

Each side of the frame is an independent axis object: `p.x1` (bottom), `p.y1` (left), `p.x2` (top), and `p.y2` (right).
The axis attributes below apply to each of these objects. So for example, to label the right side of the frame, you would say:

```python
p.y2.label = "something"
```

You can set the following attributes.

- labels, ranges and scaling

```python
.label = None | string
.label_offset = number
.label_style = dictionary
.log = 0 | 1
.range = None | (number, number)
```

  The label, log, and range attributes are the same as the ones above. For instance, when you set `p.xlog` you're actually setting `p.x1.log`. The `.label_offset` and `.label_style` attributes let you control the placement and style of the axis label.

- grid lines

```python
.grid_style = dictionary
.draw_grid = 0 | 1
```

  Grid lines are parallel to and coincident with the ticks.

- tick properties

```python
.tickdir = +1 | -1
```

  This controls the direction the ticks and subticks are drawn in. If +1 they point toward the ticklabels and if -1 they point away from the ticklabels.

```python
.ticks = None | integer | number-list
.ticks_size = number
.ticks_style = dictionary
.draw_ticks = 0 | 1
```

  If `.ticks` is set to None they will be automagically generated. If set to an integer n, n equally spaced ticks will be drawn. You can provide your own values by setting `.ticks` to a sequence.

```python
.ticklabels = None | string-list
.ticklabels_dir = +1 | -1
.ticklabels_offset = number
.ticklabels_style = dictionary
.draw_ticklabels = 0 | 1
```

  Ticklabels are the labels marking the values of the ticks. You can provide your own labels by setting `.ticklabels` to a list of strings.

- sub/minor tick properties

```python
.subticks = None | integer | number-list
.subticks_size = number
.subticks_style = dictionary
.draw_subticks = None | 0 | 1
```

  Similar to `.ticks`, except when `.subticks` is set to an integer it sets the number of subticks drawn between ticks, not the total number of subticks. If `.draw_subticks` is set to None subticks will be drawn only if ticks are drawn.

- axis spines

```python
.spine_style = dictionary
.draw_spine = 0 | 1
```

  The spine is the line perpendicular to the ticks.

```python
.draw_axis = 0 | 1
.draw_nothing = 0 | 1
```

  If `.draw_axis` is 0 the spine, ticks, and subticks are not drawn; otherwise it has no effect. If `.draw_nothing` is 1 nothing is drawn; otherwise it has no effect.

So let's say you wanted to color all the ticks red. You could write:

```python
p.x1.ticks_style["color"] = "red"
p.x2.ticks_style["color"] = "red"
p.y1.ticks_style["color"] = "red"
p.y2.ticks_style["color"] = "red"
```

but it's tedious, and hazardous for your hands. `FramedPlot` provides a mechanism for manipulating groups of axes, through the use of the following pseudo-attributes:

```python
.frame          ==>    .x1, .x2, .y1, .y2
.frame1         ==>    .x1, .y1
.frame2         ==>    .x2, .y2
.x              ==>    .x1, .x2
.y              ==>    .y1, .y2
```

which lets you write

```python
p.frame.ticks_style["color"] = "red"
```
instead.

- Managed PlotKey

You can always add a PlotKey to any plot, but sometimes it is
convenient to let the `FramedPlot` manage a PlotKey for you. You
can do this by sending a key on construction

```python
key=PlotKey(0.1, 0.9, halign='left')
plt=FramedPlot(key=key)
```

then any components added to the plot are also added to the
key if they have a `.label` attribute.


## Plot
`Plot` behaves the same as `FramedPlot`, except no axes, axis labels, or titles are drawn.

**Attributes:** Identical to `FramedPlot` except the title/label options.


## FramedArray(nrows, ncols)

Use this container if you want to plot an array of similar plots. To add a component to a specific cell, use

```python
a[i,j].add(component, ...)
a[i,j] += component
a[i,j] += component1, component2, component3
```

where `a` is a `FramedArray` object, `i` is the row number, and `j` is the column number. You can also add a component to all the cells at once using:

```python
a.add(component, ...)
a += component
a += component1, component2, component3
```

Attributes: (in addition to the basic `FramedPlot` ones)

- cell layout

```python
.cellspacing = number
[i,j].visible = True | False
.row_fractions = number-list
.col_fractions = number-list
```

  The `[i,j].visible` attribute turns the `i,j` panel on or off. The `.row_fractions` and `.col_fractions` set the fractional height or width of the total plot each row or column spans.

- cell limits

```python
.uniform_limits = 0 | 1
```

  If set to 1 every cell will have the same limits. Otherwise they are only forced to be the same across rows and down
  columns.


## Table (nrows, ncols)
This container allows you to arrange other containers in a grid. To add a container to a specific cell, use

```python
t[i,j] = container
```

where `t` is the `Table object`, `i` is the row number, and `j` is the column number. Rows and columns are numbered starting from 0, with `t[0,0]` being the upper left corner cell.

**Attributes:**

- cell layout

```python
.cellpadding = number
.cellspacing = number
.align_interiors = True | False
```

Setting `.align_interiors` attempts to align the axes interiors so that the output looks more like a `FramedArray`.


## HammerAitoffPlot(l0=0, b0=0, rot=0)
This plot implements Hammer-Aitoff coordinates, which are an equal-area projection of the sphere into the plane,
commonly used in astrophysics. The spherical coordinates `l` and `b` are used where `l` runs from `-pi` to `pi` and `b`
from `-pi/2` to `pi/2`. The equator is `b=0`, and `b=+/-pi/2` are the north/south poles. You build a plot by adding components, using:

```python
p = HammerAitoffPlot(l0=0, b0=0, rot=0)
p.add(component, ...)
p += component
p += component1, component2, component3
```

where `p` is a `HammerAitoffPlot` object. Components are rendered in the order they're added. You can specify the coordinates of the plot center using `(l0, b0)` and a rotation about that point (`rot`).

If you want to use `HammerAitoffPlot` to plot maps of the globe, then `l` and `b` are east longitude and north
latitude. Here's an [example](https://github.com/nolta/biggles/blob/master/examples/example7.py).

**Attributes:**

- ribs

```python
.ribs_l = integer
.ribs_b = integer
.ribs_style = dictionary
```


## Common Container Methods
These methods and attributes are common to all containers.

 - aspect ratio

```python
.aspect_ratio = None | number
```
  Force the aspect ratio (height divided by width) to be a particular value. If None the container fills the available space.

 - page margin

```python
.page_margin = number
```

  Extra padding applied when rendering the head container (ie, the container from which `.show()`, `.write_XXX()`, etc was called).

 - title

```python
.title = None | string
```
  Draw a plot-centered supertitle.

 - title offset

```python
.title_offset = number
```

  The distance between the title and the container's contents.

 - title style

```python
.title_style = dictionary
```

  Want a red title? Try `p.title_style["color"] = "red"`.

 - show

```python
.show ()
```
  Plot the object in an X window.

 - write

```python
.write(filename [, type=, ..])
```

  Write the plot to do disk.  The type will be inferred from the
  extension, or can be forced using type=.  Extra keywords can be
  sent depending on the type.

```python
.write("myplot.eps" [, **kw])
```

  Save plot as an Encapsulated PostScript (EPS) file.  Force eps with.
  `type="eps"` Additional keywords for eps creation can be sent.

```python
.write("myplot.pdf", [, **kw])
```
  Save plot as a PDF file.  Additional keywords for eps creation can be sent.

```python
.write("myplot.png" [, dpi=100, **kw])
```
  Save plot as a PNG file.  Additional keywords for eps creation can be sent
  (the image is converted from eps)

```python
.write("myplot.jpg" [, dpi=100, **kw])
```

  Save plot as a jpeg file.

Additional keywords for eps creation can be sent (the image is converted
from eps)

 - write_img

```python
.write_img ( [type,] width, height, filename )
```

  Write non anti-aliased image.  This method is much faster than the standard
  `write()` method with ant-aliasing, but the curves will have a jagged look.
  Valid types are "png", "svg", and "gif". Note that the GIF images produced do
  not use (patented) LZW compression. If filename is "-" output is sent to
  stdout. If `type` is omitted the last three letters of filename are used.
