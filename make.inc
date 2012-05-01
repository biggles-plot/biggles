#
# $Id: make.inc,v 1.9 2010/04/09 20:50:18 mrnolta Exp $
#
# Copyright (C) 2001 Mike Nolta <mrnolta@users.sourceforge.net>
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

PYTHON			= python
CC			= gcc
CCSHARED		= gcc -shared
CFLAGS			= -Wall -O2 -fPIC
OS			= $(shell uname)

## compile flags for the libplot library (part of GNU plotutils)

LIBPLOT_PREFIX		= /usr
LIBPLOT_CPPFLAGS	= -I$(LIBPLOT_PREFIX)/include
LIBPLOT_LDFLAGS		= -L$(LIBPLOT_PREFIX)/lib64 -L$(LIBPLOT_PREFIX)/lib \
		-lplot -L/usr/X11R6/lib64 -L/usr/X11R6/lib \
		-lXaw -lXmu -lXt -lSM -lICE -lXext -lX11

NUMPY_CPPFLAGS	= -I$(shell $(PYTHON) -c 'import numpy; print numpy.get_include()')

## these options probably don't need to be changed

PYPREFIX	= $(shell $(PYTHON) -c 'import sys; print sys.prefix')
PYVERSION	= $(shell $(PYTHON) -c 'import sys; print sys.version[:3]')
PYINCLUDE	= $(PYPREFIX)/include/python$(PYVERSION)
PYSITEPKGS	= $(PYPREFIX)/lib/python$(PYVERSION)/site-packages
BIGGLESDIR	= $(PYSITEPKGS)/biggles
BASE_CPPFLAGS	= -I$(PYINCLUDE)
BASE_LDFLAGS	= -lm

ifeq ($(OS),Darwin)
CCSHARED	= gcc -undefined dynamic_lookup
BASE_LDFLAGS	= -Wl,-F. -bundle -L$(PYPREFIX)/lib -lpng
ifneq (,$(findstring Framework,$(PYPREFIX)))
BASE_LDFLAGS   += -framework Python
endif
endif

