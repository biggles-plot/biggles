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

/* 13 functions in traditional (pre-GNU) libplot */
#define arc(xc,yc,x0,y0,x1,y1) pl_arc(xc,yc,x0,y0,x1,y1)
#define box(x0,y0,x1,y1) pl_box(x0,y0,x1,y1)
#define circle(x,y,r) pl_circle(x,y,r)
#define closepl() pl_closepl()
#define cont(x,y) pl_cont(x,y)
#define erase() pl_erase()
#define label(s) pl_label(s)
#define line(x0,y0,x1,y1) pl_line(x0,y0,x1,y1)
#define linemod(s) pl_linemod(s)
#define move(x,y) pl_move(x,y)
#define openpl() pl_openpl()
#define point(x,y) pl_point(x,y)
#define space(x0,y0,x1,y1) pl_space(x0,y0,x1,y1)

/* 46 additional functions in GNU libplot, plus 1 obsolete function
   [pl_outfile]. */
#define outfile(outfile) pl_outfile(outfile)
#define alabel(x_justify,y_justify,s) pl_alabel(x_justify,y_justify,s)
#define arcrel(dxc,dyc,dx0,dy0,dx1,dy1) pl_arcrel(dxc,dyc,dx0,dy0,dx1,dy1)
#define bezier2(x0,y0,x1,y1,x2,y2) pl_bezier2(x0,y0,x1,y1,x2,y2)
#define bezier2rel(dx0,dy0,dx1,dy1,dx2,dy2) pl_bezier2rel(dx0,dy0,dx1,dy1,dx2,dy2)
#define bezier3(x0,y0,x1,y1,x2,y2,x3,y3) pl_bezier3(x0,y0,x1,y1,x2,y2,x3,y3)
#define bezier3rel(dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3) pl_bezier3rel(dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3)
#define bgcolor(red,green,blue) pl_bgcolor(red,green,blue)
#define bgcolorname(name) pl_bgcolorname(name)
#define boxrel(dx0,dy0,dx1,dy1) pl_boxrel(dx0,dy0,dx1,dy1)
#define capmod(s) pl_capmod(s)
#define circlerel(dx,dy,r) pl_circlerel(dx,dy,r)
#define closepath() pl_closepath()
#define color(red,green,blue) pl_color(red,green,blue)
#define colorname(name) pl_colorname(name)
#define contrel(dx,dy) pl_contrel(dx,dy)
#define ellarc(xc,yc,x0,y0,x1,y1) pl_ellarc(xc,yc,x0,y0,x1,y1)
#define ellarcrel(dxc,dyc,dx0,dy0,dx1,dy1) pl_ellarcrel(dxc,dyc,dx0,dy0,dx1,dy1)
#define ellipse(x,y,rx,ry,angle) pl_ellipse(x,y,rx,ry,angle)
#define ellipserel(dx,dy,rx,ry,angle) pl_ellipserel(dx,dy,rx,ry,angle)
#define endpath() pl_endpath()
#define endsubpath() pl_endsubpath()
#define fillcolor(red,green,blue) pl_fillcolor(red,green,blue)
#define fillcolorname(name) pl_fillcolorname(name)
#define fillmod(s) pl_fillmod(s)
#define filltype(level) pl_filltype(level)
#define flushpl() pl_flushpl()
#define fontname(s) pl_fontname(s)
#define fontsize(size) pl_fontsize(size)
#define havecap(s) pl_havecap(s)
#define joinmod(s) pl_joinmod(s)
#define labelwidth(s) pl_labelwidth(s)
#define linedash(n,dashes,offset) pl_linedash(n,dashes,offset)
#define linerel(dx0,dy0,dx1,dy1) pl_linerel(dx0,dy0,dx1,dy1)
#define linewidth(size) pl_linewidth(size)
#define marker(x,y,type,size) pl_marker(x,y,type,size)
#define markerrel(dx,dy,type,size) pl_markerrel(dx,dy,type,size)
#define moverel(x,y) pl_moverel(x,y)
#define orientation(direction) pl_orientation(direction)
#define pencolor(red,green,blue) pl_pencolor(red,green,blue)
#define pencolorname(name) pl_pencolorname(name)
#define pentype(level) pl_pentype(level)
#define pointrel(dx,dy) pl_pointrel(dx,dy)
#define restorestate() pl_restorestate()
#define savestate() pl_savestate()
#define space2(x0,y0,x1,y1,x2,y2) pl_space2(x0,y0,x1,y1,x2,y2)
#define textangle(angle) pl_textangle(angle)

/* 32 floating point counterparts to some of the above (all GNU additions) */
#define ffontname(s) pl_ffontname(s)
#define ffontsize(s) pl_ffontsize(s)
#define flabelwidth(s) pl_flabelwidth(s)
#define ftextangle(angle) pl_ftextangle(angle)
#define farc(xc,yc,x0,y0,x1,y1) pl_farc(xc,yc,x0,y0,x1,y1)
#define farcrel(dxc,dyc,dx0,dy0,dx1,dy1) pl_farcrel(dxc,dyc,dx0,dy0,dx1,dy1)
#define fbezier2(x0,y0,x1,y1,x2,y2) pl_fbezier2(x0,y0,x1,y1,x2,y2)
#define fbezier2rel(dx0,dy0,dx1,dy1,dx2,dy2) pl_fbezier2rel(dx0,dy0,dx1,dy1,dx2,dy2)
#define fbezier3(x0,y0,x1,y1,x2,y2,x3,y3) pl_fbezier3(x0,y0,x1,y1,x2,y2,x3,y3)
#define fbezier3rel(dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3) pl_fbezier3rel(dx0,dy0,dx1,dy1,dx2,dy2,dx3,dy3)
#define fbox(x0,y0,x1,y1) pl_fbox(x0,y0,x1,y1)
#define fboxrel(dx0,dy0,dx1,dy1) pl_fboxrel(dx0,dy0,dx1,dy1)
#define fcircle(x,y,r) pl_fcircle(x,y,r)
#define fcirclerel(dx,dy,r) pl_fcirclerel(dx,dy,r)
#define fcont(x,y) pl_fcont(x,y)
#define fcontrel(dx,dy) pl_fcontrel(dx,dy)
#define fellarc(xc,yc,x0,y0,x1,y1) pl_fellarc(xc,yc,x0,y0,x1,y1)
#define fellarcrel(dxc,dyc,dx0,dy0,dx1,dy1) pl_fellarcrel(dxc,dyc,dx0,dy0,dx1,dy1)
#define fellipse(x,y,rx,ry,angle) pl_fellipse(x,y,rx,ry,angle)
#define fellipserel(dx,dy,rx,ry,angle) pl_fellipserel(dx,dy,rx,ry,angle)
#define fline(x0,y0,x1,y1) pl_fline(x0,y0,x1,y1)
#define flinedash(n,dashes,offset) pl_flinedash(n,dashes,offset)
#define flinerel(dx0,dy0,dx1,dy1) pl_flinerel(dx0,dy0,dx1,dy1)
#define flinewidth(size) pl_flinewidth(size)
#define fmarker(x,y,type,size) pl_fmarker(x,y,type,size)
#define fmarkerrel(dx,dy,type,size) pl_fmarkerrel(dx,dy,type,size)
#define fmove(x,y) pl_fmove(x,y)
#define fmoverel(dx,dy) pl_fmoverel(dx,dy)
#define fpoint(x,y) pl_fpoint(x,y)
#define fpointrel(dx,dy) pl_fpointrel(dx,dy)
#define fspace(x0,y0,x1,y1) pl_fspace(x0,y0,x1,y1)
#define fspace2(x0,y0,x1,y1,x2,y2) pl_fspace2(x0,y0,x1,y1,x2,y2)

/* 6 floating point operations with no integer counterpart (GNU additions) */
#define fconcat(m0,m1,m2,m3,m4,m5) pl_fconcat(m0,m1,m2,m3,m4,m5)
#define fmiterlimit(limit) pl_fmiterlimit(limit)
#define frotate(theta) pl_frotate(theta)
#define fscale(x,y) pl_fscale(x,y)
#define fsetmatrix(m0,m1,m2,m3,m4,m5) pl_fsetmatrix(m0,m1,m2,m3,m4,m5)
#define ftranslate(x,y) pl_ftranslate(x,y)

/* 4 functions specific to the C binding (for construction/destruction of
   Plotters, and setting of Plotter parameters) */
#define newpl(type,infile,outfile,errfile) pl_newpl(type,infile,outfile,errfile)
#define selectpl(handle) pl_selectpl(handle)
#define deletepl(handle) pl_deletepl(handle)
#define parampl(parameter,value) pl_parampl(parameter,value)
