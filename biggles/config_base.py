#
# $Id: config.ini,v 1.94 2008/11/28 00:38:20 mrnolta Exp $
#
# Copyright (C) 2000-2001 Mike Nolta <mike@nolta.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#

# --------------------------------------------------
# biggles checks here if it fails to find an option
# in other sections
#
CONFIG_BASE = """\
[default]

fillcolor       = 0xf0f0f0
filltype        = 0x0
fontface        = HersheySerif
fontsize        = 3.0
fontsize_min    = 1.25
symboltype      = diamond
symbolsize      = 2.0
textangle       = 0.0
texthalign      = center
textvalign      = center

# --------------------------------------------------
[screen]

# dimensions of window (pixels)
#
width           = 640
height          = 640

# make window persistent when using interactively?
#
persistent      = no

# --------------------------------------------------
[printer]

# command which postscript will be piped to
#
command         = lpr

# --------------------------------------------------
[postscript]

# paper size name; accepts ISO sizes ("a0",..,"a4"),
# ANSI sizes ("a",..,"e"), "letter", "ledger", "tabloid",
# and "b5".
#
paper           = letter

# size of output region on the page
# valid units are in,pt,cm,mm
#
width           = 7.5in
height          = 7.5in

# --------------------------------------------------
# default object parameters
#

[Contours]
func_color      = None
func_linetype   = None
func_linewidth  = None
levels          = 10

[FramedArray]
cellspacing     = 0.0
gutter          = 0.1
label_offset    = 0.9
label_size      = 2.7
uniform_limits  = 0
xlabel          = None
ylabel          = None

[FramedPlot]
frame.grid_style  = {linetype: dot}
frame.tickdir     = -1
frame1.draw_grid  = 0
gutter            = 0.1
xlog              = 0
ylog              = 0

[Geodesic]
divisions       = 100

[HammerAitoffPlot]
aspect_ratio    = 0.5
ribs_b          = 2
ribs_l          = 3
ribs_style      = {linetype: dot, linewidth: 0.5}

[Histogram]
drop_to_zero    = 1

[Plot]
gutter          = 0.0
xlog            = 0
xrange          = None
ylog            = 0
yrange          = None

[PlotKey]
key_height      = 2.0
key_hsep        = 2.0
key_vsep        = 2.0
key_width       = 4.0
kw_defaults     = {fontsize: 2.5, texthalign: left}

[Table]
align_interiors = 0
cellpadding     = 0.0
cellspacing     = 2.0

# --------------------------------------------------
# internal; can change/disappear at any time
#

[_FillComponent]
filltype        = 0x1

[Text]
fontface        = HersheySerif
fontsize        = 3.0
justify         = left
halign          = center
lineheight      = 1.2
valign          = center

[_HalfAxis]
draw_axis         = 1
draw_grid         = 0
draw_nothing      = 0
draw_spine        = 1
draw_subticks     = None
draw_ticks        = 1
draw_ticklabels   = 1
grid_style        = {linetype: dot}
intercept         = None
label             = None
label_offset      = 1.0
label_style       = {fontsize: 3.0}
log               = None
range             = None
spine_style       = {}
subticks          = None
subticks_size     = 0.75
subticks_style    = {}
tickdir           = -1
ticks             = None
ticks_size        = 1.5
ticks_style       = {}
ticklabels        = None
ticklabels_dir    = 1
ticklabels_offset = 1.5
ticklabels_style  = {fontsize: 3.0}

[_ErrorBar]
barsize         = 0.5

[_ErrorLimit]
size            = 0.6

[_LabelComponent]
kw_defaults     = {fontsize: 3.0,
                   textangle: 0.0,
                   texthalign: center,
                   textvalign: center}

[_LabelsComponent]
kw_defaults     = {fontsize: 2.5,
                   textangle: 0.0,
                   texthalign: center,
                   textvalign: center}

[_LineComponent]
kw_defaults     = {linewidth: 1.0}

[_PlotContainer]
aspect_ratio    = None
page_margin     = 0.1
title           = None
title_offset    = 1.0
title_style     = {fontsize: 3.0}
"""
