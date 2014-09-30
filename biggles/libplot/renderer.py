#
# $Id: renderer.py,v 1.17 2004/03/08 23:30:05 mrnolta Exp $
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

import math
from ._libplot_pywrap import Plotter

from .tex2libplot import tex2libplot

## polygon clipping

def sh_inside( p, dim, boundary, side ):
    return side*p[dim] >= side*boundary

def sh_intersection( s, p, dim, boundary ):
    mid = not dim
    g = 0.
    if p[dim] != s[dim]:
        g = (boundary - s[dim])/(p[dim] - s[dim])
    q = [0,0]
    q[dim] = boundary
    q[mid] = s[mid] + g*(p[mid] - s[mid])
    return q[0], q[1]

def sutherland_hodgman( polygon, dim, boundary, side ):
    out = []
    s = polygon[-1]
    s_inside = sh_inside( s, dim, boundary, side )
    for p in polygon:
        p_inside = sh_inside( p, dim, boundary, side )

        crosses = (p_inside and not s_inside) or \
                  (not p_inside and s_inside)
        if crosses:
            out.append( sh_intersection(s, p, dim, boundary) )

        if p_inside:
            out.append( p )

        s = p
        s_inside = p_inside

    return out

class RendererState(object):

    def __init__( self ):
        self.current = {}
        self.saved = []

    def set( self, name, value ):
        self.current[name] = value

    def get( self, name, notfound=None ):
        if self.current.has_key(name):
            return self.current[name]
        for i in range(len(self.saved)):
            d = self.saved[i]
            if d.has_key(name):
                return d[name]
        return notfound

    def save( self ):
        self.saved.insert( 0, self.current )
        self.current = {}

    def restore( self ):
        self.current = self.saved.pop(0)

def _hexcolor( hextriplet, scale=1 ):
    s = float(scale) / 0xff
    r = s * ((hextriplet >> 16) & 0xff)
    g = s * ((hextriplet >>  8) & 0xff)
    b = s * ((hextriplet >>  0) & 0xff)
    return r, g, b

def _set_color( pl, color ):
    if type(color) == type(''):
        #raw.set_colorname_fg( pl, color )
        pl.set_colorname_fg(color)
    else:
        r,g,b = _hexcolor( color )
        #raw.set_color_fg( pl, r, g, b )
        pl.set_color_fg(r, g, b)

def _set_pen_color( pl, color ):
    if type(color) == type(''):
        #raw.set_colorname_pen( pl, color )
        pl.set_colorname_pen(color)
    else:
        r,g,b = _hexcolor( color )
        #raw.set_color_pen( pl, r, g, b )
        pl.set_color_pen(r, g, b)

def _set_fill_color( pl, color ):
    if type(color) == type(''):
        #raw.set_colorname_fill( pl, color )
        pl.set_colorname_fill(color)
    else:
        r,g,b = _hexcolor( color )
        #raw.set_color_fill( pl, r, g, b )
        pl.set_color_fill(r, g, b)

_pl_line_type = {
        "dot"           : "dotted",
        "dash"          : "shortdashed",
        "dashed"        : "shortdashed",
}

def _set_line_type( pl, type ):
    pl_type = _pl_line_type.get( type, type )
    #raw.set_line_type( pl, pl_type )
    pl.set_line_type(pl_type )

class LibplotRenderer(Plotter):

    def __init__( self, ll, ur, type='X', parameters=None, file=None ):
        if file is None:
            filename=""
        else:
            filename=file

        self.lowerleft = ll
        self.upperright = ur
        super(LibplotRenderer,self).__init__(type,parameters,filename)
        #self.pl = raw.new( type, parameters, file )
        #self.pl = Plotter(type, parameters, file)

    def open( self ):
        self.state = RendererState()
        #raw.begin_page( self.pl )
        self.begin_page()
        #args = (self.pl,) + self.lowerleft + self.upperright
        args = self.lowerleft + self.upperright
        #raw.space( *args )
        self.space( *args )
        #raw.clear( self.pl )
        self.clear()

    #def close( self ):
    #    if self.pl is not None:
    #        #raw.end_page( self.pl )
    #        self.pl.end_page()
    def close( self ):
        self.end_page()

    def __enter__(self):
        return self
    def __exit__(self, exception_type, exception_value, traceback):
        pass
        #self.flush()


    '''
    def delete( self ):
        """ 
        if not hasattr(self, 'pl'):
            return
        if self.pl is not None:
            #raw.delete( self.pl )
            del self.pl
            self.pl = None
        """
        pass
    '''
    """
    def __del__( self ):
        self.delete()
    """

    ## state commands

    __pl_style_func = {
            "color"         : _set_color,
            "linecolor"     : _set_pen_color,
            "fillcolor"     : _set_fill_color,
            "linetype"      : _set_line_type,
            "linewidth"     : Plotter.set_line_size,
            "filltype"      : Plotter.set_fill_level,
            "fillmode"      : Plotter.set_fill_type,
            "fontface"      : Plotter.set_font_type,
            "fontsize"      : Plotter.set_font_size,
            "textangle"     : Plotter.set_string_angle,
            #"linewidth"     : raw.set_line_size,
            #"filltype"      : raw.set_fill_level,
            #"fillmode"      : raw.set_fill_type,
            #"fontface"      : raw.set_font_type,
            #"fontsize"      : raw.set_font_size,
            #"textangle"     : raw.set_string_angle,

    }

    def set( self, key, value ):
        self.state.set( key, value )
        if LibplotRenderer.__pl_style_func.has_key(key):
            method = LibplotRenderer.__pl_style_func[key]
            method(self, value)
            #apply( method, (self.pl, value) )

    def get( self, parameter, notfound=None ):
        return self.state.get( parameter, notfound )

    def save_state( self ):
        self.state.save()
        #raw.gsave( self.pl )
        self.gsave()

    def restore_state( self ):
        self.state.restore()
        #raw.grestore( self.pl )
        self.grestore()

    ## drawing commands

    def move( self, p ):
        #raw.move( self.pl, p[0], p[1] )
        super(LibplotRenderer,self).move(p[0], p[1])

    def lineto( self, p ):
        #raw.lineto( self.pl, p[0], p[1] )
        super(LibplotRenderer,self).lineto(p[0], p[1])

    def linetorel( self, p ):
        #raw.linetorel( self.pl, p[0], p[1] )
        super(LibplotRenderer,self).linetorel(p[0], p[1])

    def line( self, p, q ):
        cr = self.get( "cliprect" )
        if cr is None:
            #raw.line( self.pl, p[0], p[1], q[0], q[1] )
            super(LibplotRenderer,self).line(p[0], p[1], q[0], q[1])
        else:
            #raw.clipped_line( self.pl, \
            #        cr[0], cr[1], cr[2], cr[3], \
            #        p[0], p[1], q[0], q[1] )
            self.clipped_line(cr[0], cr[1], cr[2], cr[3],
                              p[0], p[1], q[0], q[1])


    def rect( self, p, q ):
        #raw.rect( self.pl, p[0], p[1], q[0], q[1] )
        super(LibplotRenderer,self).rect(p[0], p[1], q[0], q[1])

    def circle( self, p, r ):
        #raw.circle( self.pl, p[0], p[1], r )
        super(LibplotRenderer,self).circle(p[0], p[1], r)

    def ellipse( self, p, rx, ry, angle=0. ):
        #raw.ellipse( self.pl, p[0], p[1], rx, ry, angle )
        super(LibplotRenderer,self).ellipse(p[0], p[1], rx, ry, angle)

    def arc( self, c, p, q ):
        #raw.arc( self.pl, c[0], c[1], p[0], p[1], q[0], q[1] )
        super(LibplotRenderer,self).arc(c[0], c[1], p[0], p[1], q[0], q[1])

    __pl_symbol_type = {
            "none"                          : 0,
            "dot"                           : 1,
            "plus"                          : 2,
            "asterisk"                      : 3,
            "circle"                        : 4,
            "cross"                         : 5,
            "square"                        : 6,
            "triangle"                      : 7,
            "diamond"                       : 8,
            "star"                          : 9,
            "inverted triangle"             : 10,
            "starburst"                     : 11,
            "fancy plus"                    : 12,
            "fancy cross"                   : 13,
            "fancy square"                  : 14,
            "fancy diamond"                 : 15,
            "filled circle"                 : 16,
            "filled square"                 : 17,
            "filled triangle"               : 18,
            "filled diamond"                : 19,
            "filled inverted triangle"      : 20,
            "filled fancy square"           : 21,
            "filled fancy diamond"          : 22,
            "half filled circle"            : 23,
            "half filled square"            : 24,
            "half filled triangle"          : 25,
            "half filled diamond"           : 26,
            "half filled inverted triangle" : 27,
            "half filled fancy square"      : 28,
            "half filled fancy diamond"     : 29,
            "octagon"                       : 30,
            "filled octagon"                : 31,
    }

    def symbol( self, p ):
        self.symbols( [p[0]], [p[1]] )

    def symbols( self, x, y ):
        DEFAULT_SYMBOL_TYPE = "square"
        DEFAULT_SYMBOL_SIZE = 0.01
        type_str = self.state.get( "symboltype", DEFAULT_SYMBOL_TYPE )
        size = self.state.get( "symbolsize", DEFAULT_SYMBOL_SIZE )
        if len(type_str) == 1:
            type = ord(type_str[0])
        else:
            type = LibplotRenderer.__pl_symbol_type.get( type_str )

        cr = self.get( "cliprect" )
        if cr is None:
            #raw.symbols( self.pl, x, y, type, size )
            super(LibplotRenderer,self).symbols(x, y, type, size)
        else:
            #raw.clipped_symbols( self.pl, x, y, type, size,
            #        cr[0], cr[1], cr[2], cr[3] )
            self.clipped_symbols(x, y, type, size,
                                 cr[0], cr[1], cr[2], cr[3])


    def colored_symbols( self, x, y, c ):
        DEFAULT_SYMBOL_TYPE = "square"
        DEFAULT_SYMBOL_SIZE = 0.01
        type_str = self.state.get( "symboltype", DEFAULT_SYMBOL_TYPE )
        size = self.state.get( "symbolsize", DEFAULT_SYMBOL_SIZE )
        if len(type_str) == 1:
            type = ord(type_str[0])
        else:
            type = LibplotRenderer.__pl_symbol_type.get( type_str )

        cr = self.get( "cliprect" )
        if cr is None:
            # This will cause an error: not written yet
            #raw.colored_symbols( self.pl, x, y, type, size )
            super(LibplotRenderer,self).colored_symbols(x, y, type, size)
        else:
            self.clipped_colored_symbols(x, y, c,
                                         type, size,
                                         cr[0], cr[1],
                                         cr[2], cr[3])

    def density_plot( self, densgrid, ((xmin,ymin), (xmax,ymax)) ):
        #raw.density_plot( self.pl, densgrid,
        #                  xmin, xmax, ymin, ymax )
        super(LibplotRenderer,self).density_plot(densgrid, xmin, xmax, ymin, ymax)


    def color_density_plot( self, densgrid, ((xmin,ymin), (xmax,ymax)) ):
        #raw.color_density_plot( self.pl, densgrid,
        #                        xmin, xmax, ymin, ymax )
        super(LibplotRenderer,self).color_density_plot(densgrid,
                                                       xmin, xmax, ymin, ymax)


    def curve( self, x, y ):
        cr = self.get( "cliprect" )
        if cr is None:
            #raw.curve( self.pl, x, y )
            super(LibplotRenderer,self).curve(x, y)
        else:
            #raw.clipped_curve( self.pl, x, y,
            #        cr[0], cr[1], cr[2], cr[3] )
            self.clipped_curve(x, y,
                               cr[0], cr[1], cr[2], cr[3])

    def polygon( self, points ):
        pts = points
        cr = self.get( "cliprect" )
        if cr is not None:
            pts = sutherland_hodgman( pts, 0, cr[0], +1 )
            pts = sutherland_hodgman( pts, 0, cr[1], -1 )
            pts = sutherland_hodgman( pts, 1, cr[2], +1 )
            pts = sutherland_hodgman( pts, 1, cr[3], -1 )
        self.move( pts[0] )
        map( self.lineto, pts[1:] )

    ## text commands

    __pl_text_align = {
            "center"        : ord('c'),
            "baseline"      : ord('x'),
            "left"          : ord('l'),
            "right"         : ord('r'),
            "top"           : ord('t'),
            "bottom"        : ord('b'),
    }

    def text( self, p, str ):
        plstr = tex2libplot( str )
        hstr = self.state.get( "texthalign", "center" )
        vstr = self.state.get( "textvalign", "center" )
        hnum = LibplotRenderer.__pl_text_align.get( hstr )
        vnum = LibplotRenderer.__pl_text_align.get( vstr )
        #raw.move( self.pl, p[0], p[1] )
        #raw.string( self.pl, hnum, vnum, plstr )
        super(LibplotRenderer,self).move(p[0], p[1] )
        self.string(hnum, vnum, plstr )

    def textwidth( self, str ):
        plstr = tex2libplot( str )
        #return raw.get_string_width( self.pl, plstr )
        return self.get_string_width(plstr )

    def textheight( self, str ):
        return self.state.get( "fontsize" )     ## XXX: kludge?

class NonInteractiveScreenRenderer( LibplotRenderer ):

    def __init__( self, width, height ):
        ll = 0, 0
        ur = width, height
        parameters = {
                "BITMAPSIZE": "%dx%d" % (width, height),
                "VANISH_ON_DELETE": "no",
        }
        super(NonInteractiveScreenRenderer,self).__init__(ll,
                                                          ur,
                                                          "X",
                                                          parameters )
    def __exit__(self, exception_type, exception_value, traceback):
        """
        we don't close screen
        """
        self.flush()

class InteractiveScreenRenderer( LibplotRenderer ):

    def __init__( self, width, height ):
        ll = 0, 0
        ur = width, height
        parameters = {
                "BITMAPSIZE": "%dx%d" % (width, height),
                "VANISH_ON_DELETE": "yes",
        }
        super(InteractiveScreenRenderer,self).__init__(ll,
                                                       ur,
                                                       "X",
                                                       parameters )

    def close( self ):
        #raw.flush( self.pl )
        self.flush()

    def __exit__(self, exception_type, exception_value, traceback):
        """
        we don't close screen
        """
        self.flush()

    '''
    def delete( self ):
        #raw.flush( self.pl )
        self.flush()
    '''

_saved_screen_renderer = None

def ScreenRenderer( persistent=0, width=512, height=512 ):

    if persistent == 1:
        global _saved_screen_renderer
        if _saved_screen_renderer is None:
            _saved_screen_renderer \
                    = InteractiveScreenRenderer( width, height )
        _saved_screen_renderer.clear()
        return _saved_screen_renderer
    else:
        return NonInteractiveScreenRenderer( width, height )

def _str_size_to_pts( str ):
    import re
    m = re.compile(r"([\d.]+)([^\s]+)").match(str)
    num_xx = float(m.group(1))
    units = m.group(2)
    # convert to postscipt pt = in/72
    xx2pt = { "in":72, "pt":1, "mm":2.835, "cm":28.35 }
    num_pt = int(num_xx*xx2pt[units])
    return num_pt

class PSRenderer( LibplotRenderer ):

    def __init__( self, filename, paper="", width="", height="", **kw ):
        ll = 0, 0
        ur = _str_size_to_pts(width), _str_size_to_pts(height)
        pagesize = "%s,xsize=%s,ysize=%s" % (paper,width,height)
        for key,val in kw.items():
            pagesize = pagesize +","+ key +"="+ val
        parameters = { "PAGESIZE": pagesize }
        super(PSRenderer,self).__init__(ll, ur, "ps", parameters, filename)

class ImageRenderer( LibplotRenderer ):

    def __init__( self, type, width, height, file ):
        ll = 0, 0
        ur = width, height
        parameters = { "BITMAPSIZE": "%dx%d" % (width, height) }
        super(ImageRenderer,self).__init__(ll, ur, type, parameters, file)
