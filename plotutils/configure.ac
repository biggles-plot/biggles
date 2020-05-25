# Configure template for the GNU plotutils package.
# Copyright (C) 1989-2009 Free Software Foundation, Inc.
# Process this file with autoconf to produce a configure script.

AC_INIT([GNU plotutils],[2.6],[bug-plotutils@gnu.org])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR(graph/graph.c)
AM_INIT_AUTOMAKE([plotutils],[2.6])
AM_CONFIG_HEADER(config.h:config.hin)

# The following will be defined in config.h, if appropriate.

# Miscellaneous.
AH_TEMPLATE([HAVE_NULL_FLUSH],
	[Define to 1 if in your libc, fflush(NULL) flushes all outstreams.])
AH_TEMPLATE([_HPUX_SOURCE],
	[Define to 1 under HP/UX to get matherr-related things from math.h.])

# Threading-related.
AH_TEMPLATE([PTHREAD_SUPPORT], 
	[Define to 1 if your libc includes support for pthreads.])

# X11-related.
AH_TEMPLATE([USE_MOTIF], 
	[Define to request that Motif should be used, if X11 is used.])
AH_TEMPLATE([X_THREAD_SUPPORT],
	[Define to signal support in libXt/libX11 for multithreading.])
AH_TEMPLATE([HAVE_DBE_SUPPORT], 
	[Define if -lXext has support for the DBE X11 protocol extension.])
AH_TEMPLATE([HAVE_MBX_SUPPORT], 
	[Define if -lXext has support for the MBX X11 protocol extension.])

# PNG-related.
AH_TEMPLATE([HAVE_LIBPNG], 
	[Define if libpng is available.])
AH_TEMPLATE([INCLUDE_PNG_SUPPORT], 
	[Define to request PNG support (requires HAVE_LIBPNG, HAVE_PNG_H).])

# Fonts supported by Plotters of various types.
AH_TEMPLATE([USE_LJ_FONTS_IN_X], 
	[Define to enable support for the 45 LaserJet fonts in X output.])
AH_TEMPLATE([USE_LJ_FONTS_IN_PS], 
	[Define to enable support for the 45 LaserJet fonts in PS output.])
AH_TEMPLATE([USE_PS_FONTS_IN_PCL], 
	[Define to enable support for the 35 PS fonts in PCL output.])

# Did installer set the CFLAGS and CXXFLAGS environ variables before
# running configure?  Our default CFLAGS and CXXFLAGS differ from
# autoconf's, but we won't override installer-specified values.
if test "x$CFLAGS" = "x"; then
  CFLAGS_NOT_SET_BY_INSTALLER="yes"
else
  CFLAGS_NOT_SET_BY_INSTALLER="no"
fi
if test "x$CXXFLAGS" = "x"; then
  CXXFLAGS_NOT_SET_BY_INSTALLER="yes"
else
  CXXFLAGS_NOT_SET_BY_INSTALLER="no"
fi

# We now use libtool to make and installed a shared library.  This should
# invoke the tests AC_PROG_CC, AC_OBJEXT, AC_PROG_INSTALL, and
# AC_PROG_MAKE_SET, or equivalents, so we don't perform them explicitly.
# Note: this can apparently alter CFLAGS, on a few platforms, e.g., on
# SCO OpenServer 5 (i.e. *-*-sco3.2v5*), "-belf" is added.
AC_PROG_LIBTOOL

# Determine extension (e.g. ".exe") on executables, if any.
AC_EXEEXT

# Compiler characteristics and typedefs.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL

# Check whether fflush(NULL) works
AC_MSG_CHECKING(whether flushing a null FILE pointer works)
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
int main()
{
  FILE *fpw, *fpr;
  char test_array[7];

  if ((fpw = fopen ("conftest0", "w")) == (FILE *)0
      || fwrite ("FOOBAR", 1, 7, fpw) != 7
      || fflush ((FILE *)0) < 0
      || (fpr = fopen ("conftest0", "r")) == (FILE *)0
      || fread (test_array, 1, 7, fpr) != 7)
    exit (1);
  else
    exit(0); }]])],[AC_MSG_RESULT(yes); AC_DEFINE(HAVE_NULL_FLUSH)],[AC_MSG_RESULT(no)],[AC_MSG_RESULT(no)])

# Checks for header files: ANSI C, POSIX, and nonstandard Unix headers.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h limits.h float.h)
AC_CHECK_HEADERS(malloc.h pthread.h strings.h unistd.h values.h sys/select.h sys/stdtypes.h sys/time.h sys/types.h sys/wait.h)
AC_HEADER_TIME

# HP/UX needs a cpp definition for math.h to define matherr-related things
AC_EGREP_HEADER(_HPUX_SOURCE, math.h, [AC_EGREP_HEADER(matherr, math.h, [AC_DEFINE(_HPUX_SOURCE)])])

# Sui generis.
AC_FUNC_ALLOCA

# Checks for programs.  Automake needs AM_PROG_LEX, not AC_PROG_LEX;
# ignore the bogus warning about AC_PROG_LEX being invoked twice.
AC_PROG_YACC
AM_PROG_LEX

# Should libplot/libplotter support multithreading via pthread mutexes?
# Check to see whether libc includes dummy ("weak") pthread functions, so
# that executables may be linked with a library that uses mutexes without
# also linking with -lpthread.  (This is the case with glibc under
# Linux.)
AC_CHECK_LIB(c, pthread_mutex_init, [AC_DEFINE(PTHREAD_SUPPORT)])

# Do we have the thread-safe library functions ctime_r(), localtime_r()?
AC_CHECK_FUNCS(ctime_r localtime_r)

# Do we have the POSIX waitpid() function?
AC_CHECK_FUNCS(waitpid)

# Other non-X library functions and auxiliary libraries.
# 
# SGI's have matherr in libmx.a, not libm.a
AC_CHECK_LIB(mx, matherr, LIBS="$LIBS -lmx", LIBS="$LIBS -lm")
AC_CHECK_FUNCS(memcpy memmove strchr strcasecmp strerror matherr j0 erf)
# lgamma() and gamma() both compute the log of the gamma function.  There
# are old systems out there which do not have lgamma (the name was
# introduced after BSD 4.2), but which do have gamma.  Also some systems,
# new and old, have lgamma but not gamma.
AC_CHECK_FUNCS(lgamma gamma)

# Prepare to support X.  If the user gave the command-line option
# --without-x, AC_PATH_XTRA will set no_x to "yes".  Otherwise, it will
# add appropriate preprocessor flags to X_CFLAGS, and appropriate linker
# flags to X_LIBS.  It also checks for system-specific X libraries, and
# adds them to X_PRE_LIBS or X_EXTRA_LIBS, as appropriate.  (The former
# is where "-lSM -lICE" goes, in X11R6.)  We define the X_TOOLKIT_LIBS
# and X_BASIC_LIBS argument vectors ourselves, and if X is to be
# supported, we'll concatenate the whole lot of these into a command line
# for libtool.  See, e.g., the libplot_la_LIBADD line in
# libplot/Makefile.am.

AC_PATH_XTRA
AC_SUBST(X_CFLAGS)
# Following five are concatenated together into a linker specification.
AC_SUBST(X_LIBS)
AC_SUBST(X_TOOLKIT_LIBS)
AC_SUBST(X_PRE_LIBS)
AC_SUBST(X_BASIC_LIBS)
AC_SUBST(X_EXTRA_LIBS)

X_TOOLKIT_LIBS="-lXt"
X_BASIC_LIBS="-lXext -lX11"

# Check whether libXt has thread support.  Some platforms may have
# pthread support in libc, but no thread support in libXt/libX11.
our_saved_LDFLAGS="$LDFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
AC_CHECK_LIB(Xt, XtToolkitThreadInitialize, [AC_DEFINE(X_THREAD_SUPPORT)],[],$X_PRE_LIBS $X_BASIC_LIBS $X_EXTRA_LIBS)
LDFLAGS="$our_saved_LDFLAGS"

# Check in -lXext for double buffering extensions to X11, and check
# also whether appropriate header files are present.  (Some systems have one
# but not the other.)
our_saved_LDFLAGS="$LDFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
AC_CHECK_LIB(Xext, XdbeQueryExtension, [AC_DEFINE(HAVE_DBE_SUPPORT)], [], -lX11 "$X_EXTRA_LIBS")
AC_CHECK_LIB(Xext, XmbufQueryExtension, [AC_DEFINE(HAVE_MBX_SUPPORT)], [], -lX11 "$X_EXTRA_LIBS")
LDFLAGS="$our_saved_LDFLAGS"

our_saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"

AC_CHECK_HEADERS([X11/Xlib.h])
AC_CHECK_HEADERS([X11/extensions/Xdbe.h], [], [],
[#if HAVE_X11_XLIB_H
# include <X11/Xlib.h>
# endif
])
AC_CHECK_HEADERS([X11/extensions/multibuf.h], [], [],
[#if HAVE_X11_XLIB_H
# include <X11/Xlib.h>
# endif
])

CPPFLAGS="$our_saved_CPPFLAGS"

# Allow installer to specify location of Athena widgets (i.e. location of
# libraries lib/libXaw.so and lib/libXmu.so, and header files).  If the
# argument of --with-athena is a directory DIR, we'll add an `-L DIR/lib'
# option to the head of X_LIBS and an `-I DIR/include' option to the head
# of X_CFLAGS.

AC_ARG_WITH(athena, [  --with-athena[=DIR]       use Athena widgets: includes and libraries in DIR])

# If --with-motif is specified, then `-lXm' will be added to the head of
# X_TOOLKIT_LIBS, instead of `-lXaw -lXmu', and --with-athena will be
# ignored.  We allow the installer to specify DIR, the location of Motif.
# If the argument of --with-motif is a directory, an `-L DIR/lib' option
# will be added to the head of X_LIBS and an `-I DIR/include' option to
# the head of C_FLAGS.

AC_ARG_WITH(motif, [  --with-motif[=DIR]        use Motif widgets: includes and libraries in DIR])

# If --with-motif is specified, we always check for the Xpm image
# library, and if we find it, then we place it at the beginning of
# X_BASIC_LIBS, since Motif 2.x requires it (it may be included in -lXm
# already, though).  We also allow the installer to specify a location
# for Xpm. If DIR is specified as an argument to --with-xpm, DIR/include
# should contain xpm.h, and DIR/lib should contain the library libxpm.

AC_ARG_WITH(xpm, [  --with-xpm[=DIR]          use Xpm with Motif: include and library in DIR])

# NOTE: Motif 2.x also seems to require the extension library -lXp for
# printing, so if --with-motif is specified we always check for libXp.
# If we find it, we'll place `-lXp' before `-lXext -lX11' in
# X_BASIC_LIBS.  Also, Motif on Sys-V derived systems requires `-lPW'.
# If we find libPW, we'll add `-lPW' to LIBS, but precede it by -lc
# because some of its definitions may conflict with the standard library.
# Similarly we always check for libgen, which some versions of Motif on
# Solaris need.

# NOTE: For each `with' option, there are four possible values for the
# associated shell variable, all of which we must handle:
# 
#     1. ""    (if neither `--with-foo' nor `--without-foo' was specified)
#     2. "no"  (obtained if `--without-foo' was specified)
#     3. "yes" (obtained if `--with-foo' was specified, without an argument)
#     4. DIR   (obtained if `--with-foo DIR' was specified)

case "x$with_motif" in
xno|x)
  case "x$with_athena" in
  xyes|x)
    X_TOOLKIT_LIBS="-lXaw -lXmu $X_TOOLKIT_LIBS"
    ;;
  xno) ;;
  *)
    X_TOOLKIT_LIBS="-lXaw -lXmu $X_TOOLKIT_LIBS"
    X_CFLAGS="$X_CFLAGS -I$with_athena/include"
    X_LIBS="$X_LIBS -L$with_athena/lib"
    ;;
  esac
  ;;
*)
  if test "x$with_motif" != "xyes"; then
    X_CFLAGS="$X_CFLAGS -I$with_motif/include"
    X_LIBS="$X_LIBS -L$with_motif/lib"
  fi

  X_TOOLKIT_LIBS="-lXm $X_TOOLKIT_LIBS"

  our_saved_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $X_LIBS"
  case "x$with_xpm" in
    xno|x)
      AC_CHECK_LIB(Xpm, XpmReadFileToPixmap, X_BASIC_LIBS="-lXpm $X_BASIC_LIBS", , -lXext -lX11) 
      ;;
    xyes)
      AC_CHECK_LIB(Xpm, XpmReadFileToPixmap, X_BASIC_LIBS="-lXpm $X_BASIC_LIBS", , -lXext -lX11) 
      ;;
    *)
      LDFLAGS="$LDFLAGS -L$with_xpm/lib"
      AC_CHECK_LIB(Xpm, XpmReadFileToPixmap, X_CFLAGS="$X_CFLAGS -I$with_xpm/include"; X_LIBS="$X_LIBS -L$with_xpm/lib"; X_BASIC_LIBS="-lXpm $X_BASIC_LIBS", , -lXext -lX11) 
      ;;
  esac
  LDFLAGS="$our_saved_LDFLAGS"

  our_saved_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS $X_LIBS"
  AC_CHECK_LIB(Xp, XpQueryExtension, X_BASIC_LIBS="-lXp $X_BASIC_LIBS", , -lXext -lX11) 
  AC_CHECK_LIB(gen, regex, LIBS="$LIBS -lc -lgen", LIBS="$LIBS") 
  AC_CHECK_LIB(PW, regex, LIBS="$LIBS -lc -lPW", LIBS="$LIBS") 
  LDFLAGS="$our_saved_LDFLAGS"

  AC_DEFINE(USE_MOTIF) 
  ;;
esac

# The libplot/libplotter Makefile.am files test the automake variable
# NO_X to determine whether X should be supported.
AM_CONDITIONAL(NO_X, test "x$no_x" = "xyes")

# Check for libpng and png.h, and if both are available, define the
# config.h variable INCLUDE_PNG_SUPPORT; otherwise define the automake
# variable NO_PNG.  The latter is used in libplot/Makefile.am and
# libplotter/Makefile.am.

# NOTE: the test for libpng checks for the function png_set_tRNS(),
# because it was added to libpng in version 0.95, which is the minimum
# version that we can use.

# ALSO NOTE: the test for libpng automatically checks for zlib, since it
# supplies the `-lz' argument to the linker.

AC_CHECK_HEADERS(png.h, png_support="yes", png_support="no")
AC_CHECK_LIB(png, png_set_tRNS, [AC_DEFINE(HAVE_LIBPNG)], png_support="no", -lz -lm)
AC_ARG_WITH(libpng, [  --without-libpng        omit PNG support even if libpng and zlib are present], if test "x${with_libpng}" = "xno"; then png_support="no"; fi)
if test "x${png_support}" = "xyes"; then AC_DEFINE(INCLUDE_PNG_SUPPORT) fi
AM_CONDITIONAL(NO_PNG, test "x${png_support}" = "xno")

# Font options.  The ps_fonts_in_pcl variable is used by test/plot2hpgl.test.
AC_ARG_ENABLE(ps-fonts-in-pcl,
[  --enable-ps-fonts-in-pcl   enable use of PS fonts in PCL and HP-GL/2 output],
[if test "x$enableval" = "xyes"; then
  echo enabling support for the 35 PS fonts in PCL 5 and HP-GL/2 output
  ps_fonts_in_pcl=1
  AC_DEFINE(USE_PS_FONTS_IN_PCL)
else
  ps_fonts_in_pcl=0
fi],
ps_fonts_in_pcl=0)
AC_SUBST(ps_fonts_in_pcl)

AC_ARG_ENABLE(lj-fonts-in-ps,
[  --enable-lj-fonts-in-ps    enable use of LaserJet fonts in PS output],
[if test "x$enableval" = "xyes"; then
  echo enabling support for the 45 LaserJet fonts in PS output
  AC_DEFINE(USE_LJ_FONTS_IN_PS)
fi])

AC_ARG_ENABLE(lj-fonts-in-x,
[  --enable-lj-fonts-in-x     enable use of LaserJet fonts on X displays],
[if test "x$enableval" = "xyes"; then
  echo enabling support for the 45 LaserJet fonts on X displays
  AC_DEFINE(USE_LJ_FONTS_IN_X)
fi])

# Do we build libplotter, the C++ library, and other C++ software?
AC_ARG_ENABLE(libplotter, [  --enable-libplotter     build the C++ Plotter class library and C++ software], echo enabling construction of the C++ class library; no_libplotter="no", no_libplotter="yes"; extralib="")
AM_CONDITIONAL(NO_LIBPLOTTER, test "x$no_libplotter" = "xyes")

# Do we build and install a standalone version of the libxmi
# scan-conversion library?  (Not done by default because it's separately
# distributed.)
AC_ARG_ENABLE(libxmi, [  --enable-libxmi         build the libxmi scan-conversion library], echo enabling construction of the libxmi scan-conversion library; no_libxmi="no", no_libxmi="yes")
AM_CONDITIONAL(NO_LIBXMI, test "x$no_libxmi" = "xyes")

# Override default autoconf value "-g -O2" or "-g" for CFLAGS and
# CXXFLAGS, set by AC_PROG_CC and AC_PROG_CXX.  Provided, that is,
# installer didn't set CFLAGS via an environment variable before running
# configure.  We don't use "-g" when compiling libplot or libplotter,
# since debugging versions of those libraries would be huge.  Perhaps we
# should use it when compiling the executables, though?

# Note: on a very few platforms where libtool adds a command-line option
# to CFLAGS (see above; this includes SCO OpenServer 5, where "-belf" is
# added), this way of doing things will not work, i.e., the installer
# will need to add by hand the option that libtool would have added.
# E.g., on SCO OpenServer5 the installer may need to set the environment
# variable CFLAGS to "-O -belf" or "-O2 -belf".

if test "x$CFLAGS_NOT_SET_BY_INSTALLER" = "xyes"; then
if test "x$GCC" = "xyes"; then
  CFLAGS="-O2"
else
  CFLAGS="-O"
fi
fi

if test "x$CXXFLAGS_NOT_SET_BY_INSTALLER" = "xyes"; then
if test "x$GXX" = "xyes"; then
  CXXFLAGS="-O2"
else
  CXXFLAGS="-O"
fi
fi

# Check for gcc strength-reduce bug (taken from WINE config).  Could do
# the same for g++, but we'll assume anyone doing any C++ compiling has
# installed a modern compiler.
if test "x${GCC}" = "xyes"; then
  AC_CACHE_CHECK(for gcc strength-reduce bug, ac_cv_c_gcc_strength_bug,
                 AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main()
{
  static int Array[[3]];
  unsigned int B = 3;
  int i;
  for (i=0; i < B; i++)
    Array[[i]] = i - 3;
  exit (Array[[1]] != -2);
}]])],
[ac_cv_c_gcc_strength_bug="no"],[ac_cv_c_gcc_strength_bug="yes"],[ac_cv_c_gcc_strength_bug="yes"]) )
  if test "$ac_cv_c_gcc_strength_bug" = "yes"
  then
    CFLAGS="$CFLAGS -fno-strength-reduce"
  fi
fi

AC_CONFIG_FILES([Makefile double/Makefile graph/Makefile hersheydemo/Makefile pic2plot/Makefile pic2plot/doc/Makefile pic2plot/libgroff/Makefile pic2plot/include/Makefile info/Makefile lib/Makefile libplot/Makefile libplotter/Makefile libxmi/Makefile libxmi/info/Makefile plot/Makefile tek2plot/Makefile tek2plot/teksamples/Makefile plotfont/Makefile spline/Makefile ode/Makefile ode-examples/Makefile include/Makefile doc/Makefile fonts/Makefile fonts/bdf/Makefile fonts/pcf/Makefile fonts/pfb/Makefile test/Makefile])
AC_OUTPUT
