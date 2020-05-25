## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tek2plot

tek2plot_SOURCES = tek2plot.c Tektable.c fontlist.c
tek2plot_LDADD = ../lib/libcommon.a ../libplot/libplot.la
noinst_HEADERS = Tekparse.h

INCLUDES = -I$(srcdir)/../include

CLEANFILES = fontlist.c

SUBDIRS = teksamples

# remove executables installed by previous versions of the package

OBSOLETEBINS = tek2tek tek2hpgl tek2fig tek2ps tek2X

install-exec-hook:
	-for f in $(OBSOLETEBINS) ; do rm -f $(bindir)/$$f; done

fontlist.c: $(top_srcdir)/lib/fontlist.c
	@rm -f fontlist.c ; if $(LN_S) $(top_srcdir)/lib/fontlist.c fontlist.c ; then true ; else cp -p $(top_srcdir)/lib/fontlist.c fontlist.c ; fi
