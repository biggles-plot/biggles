## Makefile.am for plotutils.
## Process this file with automake to produce Makefile.in in all directories.

# plot, tek2plot, plotfont, graph, and hersheydemo are linked with libplot.
# spline, double, and ode are not, however.  All use the routines in lib.
#
# pic2plot is written in C++ but is currently linked with libplot, not
# libplotter.  (It could easily be modified to use libplotter rather than
# libplot.)  We build it only if --enable-libplotter is specified, though;
# which is a bit contradictory.

# Since the libxmi 2D rasterization library is distributed as a separate
# package, by default we don't build and install it as a standalone
# library.  (It's always compiled into libplot and libplotter, though.)

if NO_LIBPLOTTER
ADD_LIBPLOTTER =
else
ADD_LIBPLOTTER = pic2plot libplotter
endif

if NO_LIBXMI
ADD_LIBXMI =
else
ADD_LIBXMI = libxmi
endif

SUBDIRS = lib spline double ode ode-examples libplot $(ADD_LIBXMI) plot tek2plot plotfont graph hersheydemo $(ADD_LIBPLOTTER) include info doc fonts test

# files with nonstandard names in this directory
EXTRA_DIST = COMPAT INSTALL.pkg INSTALL.fonts KNOWN_BUGS ONEWS PROBLEMS

# remove files that are symlinks, and shouldn't be part of the 
# distribution
dist-hook:
	for i in mi_alloc.c mi_api.c mi_arc.c mi_canvas.c mi_fllarc.c mi_fllrct.c mi_fplycon.c mi_gc.c mi_ply.c mi_plycon.c mi_plygen.c mi_plypnt.c mi_plyutil.c mi_spans.c mi_widelin.c mi_zerarc.c mi_zerolin.c mi_version.c; do rm -f $(distdir)/libplot/$$i; done
	for i in xmi.h mi_api.h mi_arc.h mi_fllarc.h mi_fply.h mi_gc.h mi_line.h mi_ply.h mi_scanfill.h mi_spans.h mi_widelin.h mi_zerarc.h; do rm -f $(distdir)/libplot/$$i; done
	rm -f $(distdir)/libplotter/*.cc
	rm -f $(distdir)/libplotter/*.h
	for i in graph plot tek2plot plotfont pic2plot; do rm -f $(distdir)/$$i/fontlist.c; done
