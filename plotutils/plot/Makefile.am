## Process this file with automake to produce Makefile.in

bin_PROGRAMS = plot

plot_SOURCES = plot.c fontlist.c
plot_LDADD = ../lib/libcommon.a ../libplot/libplot.la

INCLUDES = -I$(srcdir)/../include

CLEANFILES = fontlist.c

# remove executables installed by previous versions of the package

OBSOLETEBINS = plot2plot plot2tek plot2hpgl plot2fig plot2ps plot2X

install-exec-hook:
	-for f in $(OBSOLETEBINS) ; do rm -f $(bindir)/$$f; done

fontlist.c: $(top_srcdir)/lib/fontlist.c
	@rm -f fontlist.c ; if $(LN_S) $(top_srcdir)/lib/fontlist.c fontlist.c ; then true ; else cp -p $(top_srcdir)/lib/fontlist.c fontlist.c ; fi
