#
# $Id: confit.py,v 1.2 2001/04/26 04:12:15 mrnolta Exp $
#
# Copyright (C) 2001 Mike Nolta <mike@nolta.net>
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

import ConfigParser

def _atox( x ):
    x = x.strip()
    if x == "None":
        return None
    if x == 'False':
        return False
    if x == 'True':
        return True
    try: return int( x, 0 )
    except ValueError:
        try: return float(x)
        except ValueError:
            pass
    if x[0] == "{" and x[-1] == "}":
        style = {}
        pairs = map( lambda c: c.strip(), x[1:-1].split(",") )
        for pair in pairs:
            if pair == "":
                continue
            key,val = pair.split( ":" )
            style[ key.strip() ] = _atox( val.strip() )
        return style
    return x

class Confit(object):

    default_sect = "default"

    def __init__( self ):
        self._sections = {}
        self._deprecated = {}

    def _defaults( self ):
        return self._sections[ self.default_sect ]

    def _get( self, section, option ):
        return self._sections[section][option]

    def _set( self, section, option, value ):
        if not self._sections.has_key( section ):
            self._sections[section] = {}
        self._sections[section][option] = value

    def deprecated( self, old, new ):
        self._deprecated[old] = new

    def get( self, section, option, notfound=None ):
        try: rval = self._get( section, option )
        except KeyError:
            try: rval = self._defaults()[option]
            except KeyError:
                rval = notfound
        return rval

    def get_section( self, section ):
        if not self._sections.has_key( section ):
            return None
        return self._sections[section]

    def set( self, section, option, value ):
        if self._deprecated.has_key( (section,option) ):
            sect,opt = self._deprecated[ (section,option) ]
            self._set( sect, opt, value )
        else:
            self._set( section, option, value )

    def read( self, filename ):
        cp = ConfigParser.ConfigParser()
        cp.read( filename )

        for section in cp.sections():
            for option in cp.options( section ):
                if option == "__name__":
                    continue
                a = cp.get( section, option, raw=1 )
                self.set( section, option, _atox(a) )

    def readfp( self, fp ):
        cp = ConfigParser.ConfigParser()
        cp.readfp( fp )

        for section in cp.sections():
            for option in cp.options( section ):
                if option == "__name__":
                    continue
                a = cp.get( section, option, raw=1 )
                self.set( section, option, _atox(a) )
