## Containers, Components and Style
Biggles provides a set of objects useful in the creation of scientific plots. Since the goal is complete customization, plots are created by composing sets of simple objects. Biggles objects break into two categories,
[containers](containers.md) and [components](components.md). A common set of [keywords](style.md)
controls the style of these objects.

**Containers:** Containers are things like plots and tables, and can be turned into graphical output. Containers can contain other containers. The containers are

* FramedArray
* FramedPlot
* HammerAitoffPlot
* Plot
* Table  

**Components:** Components can't be visualized on their own, but only when added to containers. They include

 - **lines**: Curve, DataArc, DataLine/Line, ErrorBarsX/Y, LineX/Y, LowerLimit, UpperLimit, Slope,
   SymmetricErrorBarsX/Y, PlotArc, PlotLine, Geodesic
 - **points/polygons**: Circle(s), Ellipse(s), Polygon, Point(s), DataBox, PlotBox
 - **filled regions**: Countour(s), Density, Histogram, FillAbove, FillBelow, FillBetween
 - **labels**: DataLabel/Label, PlotKey, PlotLabel,
 - **inset panels**: DataInset, PlotInset/Inset


## Lightweight Plotting Routines
Biggles provides a few simple routines to allow you to quickly make plots during interactive work.

 - [plot](https://github.com/biggles-plot/biggles/blob/master/biggles/func.py#L45): Make simple plots of lines and points in 2D.
 - [plot_hist](https://github.com/biggles-plot/biggles/blob/master/biggles/func.py#L134): Make simple histograms.

See the function doc strings (linked above) for more details.

## Configuration
Biggles looks for two configuration files when it loads: the site-wide [`config_base.py`](https://github.com/biggles-plot/biggles/blob/master/biggles/config_base.py) (located in the same directory as the biggles source files, the variable `CONFIG_BASE` is the base config file), and per-user `~/.biggles`. User configuration options override site configuration options.

The format should be clear from looking at the variable `CONFIG_BASE` in `config_base.py`, and you can get all the details [here](https://docs.python.org/2/library/configparser.html).

Here is an example `.biggles` file:

```
[default]
fontface = HersheySerif # Sets the default font face. HersheySerif or HersheySans is
                        #  recommended if you have any TeX math-mode material.
fontsize_min = 1.25     # Sets the minimum fontsize (relative to the size of the
                        #  plotting window).
symboltype = diamond    # Sets the default symbol type.
symbolsize = 2.0        # Sets the default symbol size.

[screen]
width = 640             # The width (in pixels) of the X window produced by .show().
height = 640            # The height (in pixels) of the X window produced by .show().
persistent = no         # Normally every invocation of .show() creates a new X window.
                        #  This can be annoying during interactive use, so biggles
                        #  reuses the X window when it thinks it's not being called
                        #  by a script. Set this to no to disable this behavior.

[printer]
command = lpr           # Sets the command used to print plots. Specifically, postscript
                        #  output is piped to this command.
paper = letter          # Sets the printer paper size.
```

You can set these parameters interactively as well, using the `configure()` function. For instance, to change the default printer command in the middle of a script, you would say

```python
biggles.configure('printer', 'command', 'lpr -Plaser')
```

The first argument is the section name, the second the parameter name, and the third the parameter's value. If you omit the first argument the section name is taken to be default, i.e.

```python
biggles.configure('fontface', 'HersheySerif')
```

is equivalent to

```python
biggles.configure('default', 'fontface', 'HersheySerif')
```

## TeX emulation
Biggles includes a simple TeX emulator which recognizes subscripts, superscripts, and many of the math symbol definitions from Appendix F of [The TeXbook](https://www.pearsonhighered.com/program/Knuth-Computers-Typesetting-Volume-A-The-Te-Xbook/PGM61813.html). All text strings passed to biggles are interpreted as TeX.

A few notes and extensions:

 - You need to enclose math-mode material in `$`, i.e. `"$\alpha$"` not `"\alpha"`.
 - Normally, Python strings are interpreted as C strings. For example, `"\nu"` <==> [newline] + [character u]. One way
   around this is to use Python's raw string notation by prepending an `"r"` to the string (i.e., `r"\nu"`). Or you
   could say `"\\nu"`.
 - For a degree symbol (°), use `"\degree"` instead of `"^\circ"`.
