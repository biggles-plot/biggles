/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* This is a header file for the XAffText module, which was originally
   independent of libplot.  The module is in x_afftext.c.  It may be used
   independently of libplot.

   To use the module independently of libplot, simply do not specify
   "-DLIBPLOT" at compile time.

   The module supplies two external functions, which are generalizations of
   the core X11 function XDrawString: XAffDrawRotString and
   XAffDrawAffString.  They draw, respectively, a rotated text string and
   (more generally) a matrix-transformed text string, using a specified
   core X font.  The rotation angle and transformation matrix are specified
   by the user.  The matrix is passed as a 4-element array, with the
   element ordering convention, and sign conventions, being those of the
   Matrix XLFD extension.

   `XAffText' is an abbreviation of `X11 affinely transformed text'.  The
   module was inspired by Alan Richardson's xvertext module for displaying
   rotated text strings in X11, using the core X fonts.  It works in a
   similar way.  (It retrieves a bitmap from the X server into an XImage,
   transforms the XImage, monochrome pixel by pixel, and sends it back to a
   bitmap on the server, for use as a stipple.)  But it supports arbitrary
   transformation matrices, and pays extra attention to pixel-level
   accuracy.  It uses integer arithmetic when possible. */

#include <X11/Xlib.h>

#ifdef LIBPLOT
/* Change the names of the two external functions of the module by
   prepending "_x_" to them, for consistency with other internal (but
   externally visible) X11-related functions in libplot.  */
#define XAffDrawAffString _pl_XAffDrawAffString
#define XAffDrawRotString _pl_XAffDrawRotString
#endif

extern int XAffDrawAffString (Display *dpy, Drawable drawable, GC gc, XFontStruct *font, int x, int y, double a[4], const char *text);
extern int XAffDrawRotString (Display *dpy, Drawable drawable, GC gc, XFontStruct *font, int x, int y, double angle, const char *text);
