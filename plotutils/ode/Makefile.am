## Process this file with automake to produce Makefile.in

bin_PROGRAMS = ode

ode_SOURCES = am.c ama.c bessel.c eu.c expr.c float.c global.c main.c misc.c prt.c rk.c rka.c specfun.c stperr.c sym.c yywrap.c gram.y lex.l
noinst_HEADERS = extern.h num.h ode.h
ode_LDADD = ../lib/libcommon.a @LEXLIB@

AM_YFLAGS = -d

INCLUDES = -I$(srcdir)/../include

EXTRA_DIST = INTERNALS gram.h
