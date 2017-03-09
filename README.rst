================================================
Biggles : Scientific Plotting with/in/for Python
================================================

.. image:: https://travis-ci.org/biggles-plot/biggles.svg?branch=master
    :target: https://travis-ci.org/biggles-plot/biggles

Biggles is a Python module for creating publication-quality 2D scientific
plots. It supports multiple output formats (postscript, x11, png, svg, gif),
understands simple TeX, and sports a high-level, elegant interface. It's
intended for technical users with sophisticated plotting needs.

Simple Example
--------------

Here's a simple biggles script::

    >>> import biggles
    >>> x = [1, 2, 3, 4, 5]
    >>> y = [5, 4, 3, 2, 1]
    >>> p = biggles.FramedPlot()
    >>> p.add(biggles.Curve(x, y))
    >>> p.show()

This produces an X window with a framed plot of the curve y(x).
You can find more in the `examples <https://github.com/biggles-plot/biggles/tree/master/examples>`_
directory included with the source distribution. Also, see the `gallery <http://biggles-plot.github.io/biggles>`_
on the web.

See the full `documentation <https://github.com/biggles-plot/biggles/wiki>`_ for more details.

Installation
------------

Biggles requires `numpy <http://www.numpy.org/>`_ and GNU `plotutils <http://www.gnu.org/software/plotutils/>`_.

**Macports**::

    $ sudo port install plotutils +x11
    $ pip install biggles

**Homebrew**::

    $ brew install plotutils --with-x11
    $ pip install biggles

**Debian/Ubuntu**::

    $ sudo apt-get libplot-dev plotutils
    $ pip install biggles

**Windows**

This kind of install is not well tested at the moment. Please report bugs if you find them!
