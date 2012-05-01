#
# $Id: python-biggles.spec,v 1.28 2010/04/09 21:28:15 mrnolta Exp $
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
Version: 1.6.6
Release: 1
Name: python-biggles
Summary: high-level scientific plotting module for Python
Packager: Michael Nolta <mrnolta@users.sourceforge.net>
Source: http://download.sourceforge.net/biggles/python-biggles-%{version}.tar.gz
URL: http://biggles.sourceforge.net/
Copyright: GPL
Group: Applications/Graphics
Requires: plotutils
Requires: python
BuildRoot: /var/tmp/%{name}-%{version}-buildroot
Prefix: /usr

%define pythonversion %(python -V 2>&1 | cut -c8-10)
%define bigglesdir %{prefix}/lib/python%{pythonversion}/site-packages/biggles

%description

Biggles is a Python module for creating publication-quality 2D scientific
plots. It supports multiple output formats (postscript, x11, png, svg, gif),
understands simple TeX, and sports a high-level, elegant interface. It's
intended for technical users with sophisticated plotting needs.

%prep
%setup -q

%build
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{bigglesdir}
install -m644 src/config.ini %{buildroot}%{bigglesdir}/config.ini
install -m644 src/*.py  %{buildroot}%{bigglesdir}/
install -m644 src/*.pyc %{buildroot}%{bigglesdir}/
install -m755 src/*.so  %{buildroot}%{bigglesdir}/
mkdir -p %{buildroot}%{bigglesdir}/libplot
install -m644 src/libplot/*.py  %{buildroot}%{bigglesdir}/libplot
install -m644 src/libplot/*.pyc %{buildroot}%{bigglesdir}/libplot
install -m755 src/libplot/*.so  %{buildroot}%{bigglesdir}/libplot

%files
%defattr(-,root,root)
%doc COPYING CREDITS ChangeLog README examples
%dir %{bigglesdir}
%config %{bigglesdir}/config.ini
%{bigglesdir}/*.py
%{bigglesdir}/*.pyc
%{bigglesdir}/*.so
%dir %{bigglesdir}/libplot
%{bigglesdir}/libplot/*.py
%{bigglesdir}/libplot/*.pyc
%{bigglesdir}/libplot/*.so

%changelog
* Sat Nov 03 2001 Michael Nolta <mrnolta@users.sourceforge.net>
- Removed *.pyo files. Made code slower.

* Fri Oct 26 2001 Michael Nolta <mrnolta@users.sourceforge.net>
- Updated for new file layout. Added *.pyo files.

* Sun Jan 25 2001 Michael Nolta <mrnolta@users.sourceforge.net>
- Added support for *.so files.

* Sun Dec 17 2000 Michael Nolta <mrnolta@users.sourceforge.net>
- Added *.pyc files.

* Tue Nov 24 2000 Michael Nolta <mrnolta@users.sourceforge.net>
- Created.
