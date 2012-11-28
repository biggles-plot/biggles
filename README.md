
Biggles : Scientific Plotting with/in/for Python
================================================

Biggles is a Python module for creating publication-quality 2D scientific
plots. It supports multiple output formats (postscript, x11, png, svg, gif),
understands simple TeX, and sports a high-level, elegant interface. It's
intended for technical users with sophisticated plotting needs. 

Simple Example
--------------

Here's a simple biggles script:

    #!/usr/bin/env python

    import biggles

    x = [1, 2, 3, 4, 5]
    y = [5, 4, 3, 2, 1]

    p = biggles.FramedPlot()
    p.add( biggles.Curve(x, y) )
    p.show()

This produces an X window with a framed plot of the curve y(x).
You can find more in the 'examples/' directory included with the
source distribution.

Installation
------------

Macports:

    $ sudo port install plotutils +x11
    $ sudo port install py27-biggles

