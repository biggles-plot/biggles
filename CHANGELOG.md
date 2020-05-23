2.0.0
========================

* Bundle plotutils, including fixes for recent libpng
* Fix install instructions in readme (w/ help from MrAureliusR)
* Allow row and col fractions to be sent to Table 
* Internal: pass on plot constructor keywords to the base class
* Moved to github actions for CI

1.7.3 (not yet released)
========================

Features
--------
* Plots now show inline in jupyter notebooks.
* Added docs/examples for jupyter notebooks.
* Can set the default background color for plots using `bgcolor = ` in the 
  `[default]` section of the config.  This default effects plots shown on the 
  screen and non-antialiased plots written with write_img.  The default
  can specifically be overridden in the `[screen]` section and the new section
  configuring non-anti-aliased plots `[image_noaa]`.
* New config section for anti-aliased images `[image]` where currently you can
  set the resolution, which defaults to `dpi = 100`. This can be overridden
  in call to `write(..., dpi=)`
* New config section for non-anti-aliased images `[image_noaa]` where currently you can
  set the default `width =`, `height=`, `bgcolor=` and `color = `.  Default `width,height`
  is 640.  The colors default to those set in the `[default]` section.
* The `write_img` function can now be called with just the filename, since
  the default `width,height` can now be set in the `[image_noaa]` section.
  The other arguments can now be sent with keywords (old style still supported).
* Label for plot components like Points can be set at construction

Bug Fixes
----------

* make ScreenRender close itself after leaving the context
  manager (or being cleaned up). This prevents a seg fault that could occur
  when the X11 device was open and an error occured, and the device was not
  then closed.  This was basically a mistake on my part thinking
  that close would close the window, so I had not deleted it.
* Fixed keyword conflict in lightweight plotting routines.

Removed Features
----------------

* removed the `persistent = yes` feature for X11 windows.  This never
  worked.  It may be re-implemented in the future.


1.7.2 (14 Mar 2017)
===================

Features
--------
* Added `+=` syntax for adding components to a `PlotKey`.
* Added `+=` syntax for adding components to containers.
* Added `Polygon` component.

Bug Fixes
---------
* Added /usr/X11 to search path for linking plotutils. (Thank you @smaret!)

Code Changes
------------
* Updated examples to use new `+=` syntax.
* Updated code to PEP8 with `autopep8`.
* Updated travis-ci to ship to pypi on tags.
* Added `MANIFEST.in` file to include `README.rst` in the package.

1.7.0 (08 Mar 2017)
===================

Features
--------
* Increased default screen size to 640x640.
* Changed default font to `HersheySerif`.
* Changed default to non-persistent windows.
* Added new write functions for anti-aliased images.
* Added smooth keyword to histogram.
* Added option to change panel ratios in `FramedArray` using
  `row_fractions` and `col_fractions` keywords.
* Visibility of panels in `FramedArray` can be controlled
  with the visibility attribute, `arr[i,j].visible=False`.
* Added `\odot` to latex conversion, for sun symbol.
* New `func.plot` function with more functionality.
* Major classes and functions documented. This facilitates
  checking docs from an interactive session, e.g. `help()` or `?` in ipython.
* New documentation on GitHub wiki.

Bug Fixes
---------
* Fixed plotting of vertical and horizontal lines on log axes.
* Fixed bug in bounding box unions computing minimum.
* Added `try: finally:` blocks around compose operations. Fixes segfault
  when an exception is thrown while composing and more plotting is attempted.
* Fixed bug in `DataArc`/`PlotArc`.

Code Changes
------------
* Added Travis CI for testing.
* Converted to new style classes.
* Converted to extended calling syntax.
* Moved to spaces instead of tabs.

Removed Code
------------
* Removed old C wrapper for GNU plotutils.
* Removed old Makefile build system.
* Removed matlab compatible interface.
* Removed read_* I/O code from `func.py`.

1.6.7 (02 May 2012)
===================

* Fix segfault when libplot isn't built with X11 (reported by Daniel Ericsson
  & Sebastien Maret).

* Move to github.

1.6.6 (27 Nov 2008)
===================

* Fixed issue that caused all images to be 570x570 on some systems, due to a
  change in the python2.5 C API.

* Improved setup.py and Makefiles.

* Contours can now be used with PlotKey.

* Added Labels component. Similar to Points, but with text labels instead
  of symbols.

1.6.5 (20 Mar 2007)
===================

* Replaced Numeric with numpy.

1.6.4 (08 Mar 2004)
===================

* Phil Kromer contributed the new ColoredPoints and Density components.
  See example9.py for details.

* Todd Fox contributed Makefiles for building with MS Visual C++.

* Can now specify the width/height of postscript output, by passing (e.g.)
  width="5in" or height="10in" to the functions which produce postscript
  output. Default values are in the [postscript] section of config.ini.
  The old [printer]/paper option is now [postscript]/paper.

* Added UpperLimits, LowerLimits components. These produce symbols
  with half-arrows indicating the true value is below/above the
  point.

* Added TeX codes \`,\',\^,\",\~ for character accents.

1.6.3 (16 Jul 2002)
===================

* Mike Romberg fixed bugs related to packaging biggles with imputil.

1.6.2 (23 May 2002)
===================

* Spiros Papadimitriou contributed an experimental piddle port.

* You can now specify partial ranges for plots. So "p.xrange = 2, None"
  will set the lower bound of the plot p to be 2, and the upper bound
  will be guessed from the data contained in the plot.

* Replaced "!= None" with "is not None" everywhere.

* Fixed bug with added space in scientific notation ticklabels.

1.6.1 (14 Nov 2001)
===================

* Martin Lamar ported the code to Windows.

* Replaced "== None" with "is None" everywhere (again thanks to Martin).
  This should fix crashes with recent versions of numpy.

1.6.0 (07 Nov 2001)
===================

* Clipping!

* Better performance for plots with lots of data. Parts of the code
  were vectorized, and a new integrated libplot interface was written.
  Biggles no longer uses the python-libplot module.

* Jamie Mazer contributed code for Matlab emulation, accessible by
  "import biggles.matlab".

* Added the multipage() function for multipage postscript output.
  Thanks to Olivier Andrieu for the original patch.

* Changed the default plot window size to 512x512 from 570x570.

* Bug fixes: better tick generation on huge-ranged log axes; the
  "thousand tiny lines" problem with contour plots.

1.5.0 (31 Aug 2001)
===================

* Completely rewrote FramedPlot. Fully backward compatible with
  the old FramedPlot, it adds a tremendous number of new features.
  CustomFramedPlot is now deprecated.

* Configuration options are now read when objects are instantiated,
  instead of during class definition. This means you can change them
  in the middle of a script, using the new configure() function.

* Added read_rows(),read_matrix() functions. They read text data files,
  returning a list of rows and a matrix, respectively. Numeric Python
  arrays are returned by default. The readcolumn() function has been
  renamed read_column().

* TeX font control sequences \it and \bf are now recognized. Spaces
  are no longer ignored during math mode.

* You can now specify single characters as plotting symbols.

* Added DataBox/PlotBox components.

* Added .align_interiors option to Table.

* Added .drop_to_zero option to Histogram.

* Added .title_style attribute to all containers.

* The container methods .save_as_XXX() were renamed .write_XXX().
  While the old names are not deprecated, they will no longer
  appear in the documentation.

* Bug fixes: fixed crash when used by a CGI script, dropped minus
  sign for axis labels which are powers of ten, compile problem on
  Suns.

1.4.0 (06 Mar 2001)
===================

* Biggles now requires the Numeric Python module, and is no longer
  a pure Python module.

* Added support for contour plots with new Contours() component.

* HammerAitoffPlot(): new & improved geodesic algorithm; added
  support for rotated coordinates.

* Added FillAbove(), FillBelow() components. Fill*() objects now
  work with PlotKey().

* More robust interactive session detection. The config.ini option
  "[screen] guess_interactive" has been renamed "persistent".
  Removed interactive() function.

* Better tick guesses for log axes.

* Bug fixes: apparent data scaling by FramedArray when aspect_ratio
  is set; several related to calculating limits.

1.3.0 (17 Dec 2000)
===================

* Breaks old Table cell{spacing,padding} values; multipying them by
  100 should work in most cases.

* Renamed Label{Data,Plot} -> {Data,Plot}Label, LineSlope -> Slope,
  and ErrorEllipses -> Ellipses. The old names still work but are
  deprecated.

* {Line,Symbol}Key have been superceded by PlotKey and are now
  deprecated.

* New plot type: CustomFramedPlot(). Similar to FramedPlot(), but
  allows finer control over the frame style.

* New plot type: HammerAitoffPlot(). An equal-area projection of the
  sphere, Hammer-Aitoff plots are commonly used in astrophysics.

* New FramedArray options: .uniform_limits & .cellspacing.

* New components: Circle(s), Ellipse, Geodesic, Point, PlotInset,
  and PlotLine.

* You can now specify a minimum fontsize with "fontsize_min" in
  config.ini.

* Various speed improvements, bug fixes, and infrastructure work.

1.2.1 (31 Oct 2000)
===================

* Reorganized config.ini file.

* The readcolumn() function can be configured to return Numeric arrays.

* Added interactive() function to let users control X window behavior.

* Fixed a couple of axis bugs triggered by very large limits.

1.2.0 (09 Oct 2000)
===================

* Added distutils setup.py file. [contributed by Berthold H�llmann]

* Added FramedArray container for grouping similar plots.

* Added SymmetricErrorBars[XY] helper functions.

* You can now say "t[i,j]=x" instead of "t.set(i,j,x)" for Table
  objects.

* Broke readcolumn() so users can pass in their own string -> number
  conversion functions. Also, readcolumn now ignores blank lines and
  those starting with a comment character.

* When used interactively, biggles now reuses the same X window
  insteading of creating a new window each time .show() is called.

* Fixed stupid printing bug.

1.1.0 (22 Sep 2000)
===================

* LineKey, SymbolKey, and Inset now take plot coordinates instead of
  data coordinates. Plot coordinates run from 0 -> 1 for each axis.
  Sorry about the breakage.

* Inset coordinates now specify the frame location, not the bounding
  box. More breakage.

* Replaced Label with LabelData & LabelPlot. LabelData works the same
  as Label, and LabelPlot takes plot coordinates instead of data
  coordinates.

* Added LineSlope, Line[XY] components.

* Added readcolumn() function. Returns a list of column values from
  a flat ASCII file.

* More namespace cleanup.

1.0.3 (02 Sep 2000)
===================

* Fixed a couple of layout bugs. Handling of large tables and
  elongated plots is greatly improved. [thanks to Jamie Mazer]

* .save_as_{eps,img}() now recognize the filename "-" as stdout.

* Better log ticks.

* Namespace cleanup.

1.0.2 (18 Aug 2000)
===================

* Top level layout overhauled. Ugly-hack-that-sorta-worked replaced
  with Elegant Solution. Naturally, it's quite a bit slower.

* Added "cellpadding" and "cellspacing" to Table, similar to HTML
  tables.

1.0.1 (14 Aug 2000)
===================

* Prettier tick labels.

* Fixed stupid aspect_ratio bug.

1.0.0 (12 Aug 2000)
===================

* First stable release.
