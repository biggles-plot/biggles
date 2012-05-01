#
# $Id: Makefile,v 1.11 2001/11/03 20:52:17 mrnolta Exp $
#
# Copyright (C) 2000-2001 Mike Nolta <mrnolta@users.sourceforge.net>
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

include make.inc

SRCDIR		= ./src
EXAMPLESDIR	= ./examples

.PHONY: all install uninstall clean

all:
	cd $(SRCDIR) && $(MAKE) all
	cd $(SRCDIR)/libplot && $(MAKE) all

install: all
	[ -d $(BIGGLESDIR) ] || mkdir -m755 $(BIGGLESDIR)
	install -m644 $(SRCDIR)/*.ini $(BIGGLESDIR)
	install -m644 $(SRCDIR)/*.py  $(BIGGLESDIR)
	install -m644 $(SRCDIR)/*.pyc $(BIGGLESDIR)
	install -m755 $(SRCDIR)/*.so  $(BIGGLESDIR)
	[ -d $(BIGGLESDIR)/libplot ] || mkdir -m755 $(BIGGLESDIR)/libplot
	install -m644 $(SRCDIR)/libplot/*.py  $(BIGGLESDIR)/libplot
	install -m644 $(SRCDIR)/libplot/*.pyc $(BIGGLESDIR)/libplot
	install -m755 $(SRCDIR)/libplot/*.so  $(BIGGLESDIR)/libplot

uninstall:
	[ -d $(BIGGLESDIR) ] && rm -rf $(BIGGLESDIR)

clean:
	cd $(SRCDIR) && $(MAKE) clean
	cd $(SRCDIR)/libplot && $(MAKE) clean
	-cd $(EXAMPLESDIR) && rm *.png *.eps

