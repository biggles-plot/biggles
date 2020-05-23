## Process this file with automake to produce Makefile.in

bin_PROGRAMS = plotfont

plotfont_SOURCES = plotfont.c fontlist.c
plotfont_LDADD = ../lib/libcommon.a ../libplot/libplot.la

INCLUDES = -I$(srcdir)/../include

CLEANFILES = fontlist.c

fontlist.c: $(top_srcdir)/lib/fontlist.c
	@rm -f fontlist.c ; if $(LN_S) $(top_srcdir)/lib/fontlist.c fontlist.c ; then true ; else cp -p $(top_srcdir)/lib/fontlist.c fontlist.c ; fi
