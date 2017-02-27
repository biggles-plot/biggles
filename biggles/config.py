#
# $Id: config.py,v 1.34 2003/04/17 00:51:02 mrnolta Exp $
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

import os, sys, io
import confit
from config_base import CONFIG_BASE

_config = confit.Confit()

# XXX:deprecated 1.4
_config.deprecated( ('screen','guess_interactive'), ('screen','persistent') )

# XXX:deprecated 1.5
_config.deprecated( \
        ('HammerAitoffPlot','num_b_ribs'), ('HammerAitoffPlot','ribs_b') )
_config.deprecated( \
        ('HammerAitoffPlot','num_l_ribs'), ('HammerAitoffPlot','ribs_l') )
_config.deprecated( \
        ("readcolumn","use_numeric"), ("read_column","return_numpy") )
_config.deprecated( \
        ("readcolumn","comment_char"), ("read_column","comment_char") )

# XXX:deprecated 1.6.4
_config.deprecated( ('printer','paper'), ('postscript','paper') )
if sys.version_info < (3, 0, 0):
    with io.StringIO(CONFIG_BASE.decode('utf-8')) as fp:
        _config.readfp(fp)
else:
    with io.StringIO(CONFIG_BASE) as fp:
        _config.readfp(fp)

if os.environ.has_key( "HOME" ):
    USERCONFIGFILE = os.path.join( os.environ["HOME"], ".biggles" )
    if os.path.exists( USERCONFIGFILE ):
        _config.read( USERCONFIGFILE )

def interactive():
    return hasattr( sys, "ps1" )

def bool( section, option ):
    global _config
    try:
        x = _config.get( section, option, "no" )
        if x[0] == 'y':
            return 1
    except:
        return 0

def value( section, option, notfound=None ):
    global _config
    return _config.get( section, option, notfound )

def options( section ):
    global _config
    return _config.get_section( section )

def configure( *args ):
    global _config
    if len(args) == 2:
        _config.set( "default", args[0], args[1] )
    elif len(args) == 3:
        _config.set( args[0], args[1], args[2] )
