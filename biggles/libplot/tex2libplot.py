#
# $Id: tex2libplot.py,v 1.5 2002/08/18 22:04:07 mrnolta Exp $
#
# Copyright (C) 2000 Mike Nolta <mike@nolta.net>
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

#
# This is just a quick and dirty converter from simple TeX strings
# to libplot Hershey font strings. Basically just a lookup table.
#

import re, string

class TeXLexer(object):

    re_control_sequence = re.compile( r"^\\[a-zA-Z]+[ ]?|^\\[^a-zA-Z][ ]?" )

    def __init__( self, str ):
        self.str = str
        self.len = len(str)
        self.pos = 0
        self.token_stack = []

    def get_token( self ):
        if self.pos == self.len:
            return None

        if len(self.token_stack) > 0:
            return self.token_stack.pop()

        str = self.str[self.pos:]
        m = self.re_control_sequence.match(str)
        if m is not None:
            token = m.group()
            self.pos = self.pos + len(token)
            ## consume trailing space
            if len(token) > 2 and token[-1] == ' ':
                token = token[:-1]
        else:
            token = str[0]
            self.pos = self.pos + 1

        return token

    def put_token( self, token ):
        self.token_stack.append( token )

    def peek( self ):
        token = self.get_token()
        self.put_token( token )
        return token

_common_token_dict = {
        r'\\'                           : '\\',
        r'\$'                           : r'$',
        r'\%'                           : r'%',
        r'\#'                           : r'#',
        r'\&'                           : r'&',
#       r'\~'                           : r'~',
        r'\{'                           : r'{',
        r'\}'                           : r'}',
        r'\_'                           : r'_',
#       r'\^'                           : r'^',

        r'~'                            : r' ',
        r'\/'                           : r'\r^',

        ## special letters (p52)
#       r'\oe'                          : r'',
#       r'\OE'                          : r'',
        r'\ae'                          : r'\ae',
        r'\AE'                          : r'\AE',
        r'\aa'                          : r'\oa',
        r'\AA'                          : r'\oA',
        r'\o'                           : r'\/o',
        r'\O'                           : r'\/O',
#       r'\l'                           : r'',
#       r'\L'                           : r'',
        r'\ss'                          : r'\ss',

        ## ignore stray brackets
        r'{'                            : r'',
        r'}'                            : r'',
}

_text_token_dict = {
        ## punctuation (p52)
        r'\`'                           : r'\`',
        r"\'"                           : r"\'",
        r'\^'                           : r'\^',
        r'\"'                           : r'\:',
        r'\~'                           : r'\~',
        r'\c'                           : r'\,',

        ## non-math symbols (p438)
        r'\S'                           : r'\sc',
        r'\P'                           : r'\ps',
        r'\dag'                         : r'\dg',
        r'\ddag'                        : r'\dd',
}

_math_token_dict = {

        r'*'                            : r'\**',

        ## spacing
#       r' '                            : r'',
        r'\ '                           : r' ',
        r'\quad'                        : r'\r1',       # 1 em
        r'\qquad'                       : r'\r1\r1',    # 2 em
        r'\,'                           : r'\r6',       # 3/18 em
#       r'\>'                           : r'',          # 4/18 em
#       r'\;'                           : r'',          # 5/18 em
        r'\!'                           : r'\l6',       # -1/6 em

        ## lowercase greek
        r'\alpha'                       : r'\*a',
        r'\beta'                        : r'\*b',
        r'\gamma'                       : r'\*g',
        r'\delta'                       : r'\*d',
        r'\epsilon'                     : r'\*e',
#       r'\varepsilon'                  : r'',
        r'\zeta'                        : r'\*z',
        r'\eta'                         : r'\*y',
        r'\theta'                       : r'\*h',
        r'\vartheta'                    : r'\+h',
        r'\iota'                        : r'\*i',
        r'\kappa'                       : r'\*k',
        r'\lambda'                      : r'\*l',
        r'\mu'                          : r'\*m',
        r'\nu'                          : r'\*n',
        r'\xi'                          : r'\*c',
        r'\pi'                          : r'\*p',
#       r'\varpi'                       : r'',
        r'\rho'                         : r'\*r',
#       r'\varrho'                      : r'',
        r'\sigma'                       : r'\*s',
        r'\varsigma'                    : r'\ts',
        r'\tau'                         : r'\*t',
        r'\upsilon'                     : r'\*u',
        r'\phi'                         : r'\*f',
        r'\varphi'                      : r'\+f',
        r'\chi'                         : r'\*x',
        r'\psi'                         : r'\*q',
        r'\omega'                       : r'\*w',

        ## uppercase greek
        r'\Alpha'                       : r'\*A',
        r'\Beta'                        : r'\*B',
        r'\Gamma'                       : r'\*G',
        r'\Delta'                       : r'\*D',
        r'\Epsilon'                     : r'\*E',
        r'\Zeta'                        : r'\*Z',
        r'\Eta'                         : r'\*Y',
        r'\Theta'                       : r'\*H',
        r'\Iota'                        : r'\*I',
        r'\Kappa'                       : r'\*K',
        r'\Lambda'                      : r'\*L',
        r'\Mu'                          : r'\*M',
        r'\Nu'                          : r'\*N',
        r'\Xi'                          : r'\*C',
        r'\Pi'                          : r'\*P',
        r'\Rho'                         : r'\*R',
        r'\Sigma'                       : r'\*S',
        r'\Tau'                         : r'\*T',
        r'\Upsilon'                     : r'\*U',
        r'\Phi'                         : r'\*F',
        r'\Chi'                         : r'\*X',
        r'\Psi'                         : r'\*Q',
        r'\Omega'                       : r'\*W',

        ## miscellaneous
        r'\aleph'                       : r'\Ah',
        r'\hbar'                        : r'\hb',
        r'\ell'                         : r'\#H0662',
        r'\wp'                          : r'\wp',
        r'\Re'                          : r'\Re',
        r'\Im'                          : r'\Im',
        r'\partial'                     : r'\pd',
        r'\infty'                       : r'\if',
        r'\prime'                       : r'\fm',
        r'\emptyset'                    : r'\es',
        r'\nabla'                       : r'\gr',
        r'\surd'                        : r'\sr',
#       r'\top'                         : r'',
#       r'\bot'                         : r'',
        r'\|'                           : r'\||',
        r'\angle'                       : r'\/_',
#       r'\triangle'                    : r'',
        r'\backslash'                   : r'\\',
        r'\forall'                      : r'\fa',
        r'\exists'                      : r'\te',
        r'\neg'                         : r'\no',
#       r'\flat'                        : r'',
#       r'\natural'                     : r'',
#       r'\sharp'                       : r'',
        r'\clubsuit'                    : r'\CL',
        r'\diamondsuit'                 : r'\DI',
        r'\heartsuit'                   : r'\HE',
        r'\spadesuit'                   : r'\SP',
        r'\int'                         : r'\is',

        ## binary operations
        r'\pm'                          : r'\+-',
        r'\mp'                          : r'\-+',
#       r'\setminus'                    : r'',
        r'\cdot'                        : r'\md',
        r'\times'                       : r'\mu',
        r'\ast'                         : r'\**',
#       r'\star'                        : r'',
#       r'\diamond'                     : r'',
#       r'\circ'                        : r'',
        r'\bullet'                      : r'\bu',
        r'\div'                         : r'\di',
        r'\cap'                         : r'\ca',
        r'\cup'                         : r'\cu',
#       r'\uplus'                       : r'',
#       r'\sqcap'                       : r'',
#       r'\sqcup'                       : r'',
#       r'\triangleleft'                : r'',
#       r'\triangleright'               : r'',
#       r'\wr'                          : r'',
#       r'\bigcirc'                     : r'',
#       r'\bigtriangleup'               : r'',
#       r'\bigtriangledown'             : r'',
#       r'\vee'                         : r'',
#       r'\wedge'                       : r'',
        r'\oplus'                       : r'\c+',
#       r'\ominus'                      : r'',
        r'\otimes'                      : r'\c*',
#       r'\oslash'                      : r'',
        r'\odot'                        : r'\SO',
        r'\dagger'                      : r'\dg',
        r'\ddagger'                     : r'\dd',
#       r'\amalg'                       : r'',

        ## relations
        r'\leq'                         : r'\<=',
#       r'\prec'                        : r'',
#       r'\preceq'                      : r'',
        r'\ll'                          : r'<<',
        r'\subset'                      : r'\SB',
#       r'\subseteq'                    : r'',
#       r'\sqsubseteq'                  : r'',
        r'\in'                          : r'\mo',
#       r'\vdash'                       : r'',
#       r'\smile'                       : r'',
#       r'\frown'                       : r'',
        r'\geq'                         : r'\>=',
#       r'\succ'                        : r'',
#       r'\succeq'                      : r'',
        r'\gg'                          : r'>>',
        r'\supset'                      : r'\SS',
#       r'\supseteq'                    : r'',
#       r'\sqsupseteq'                  : r'',
#       r'\ni'                          : r'',
#       r'\dashv'                       : r'',
        r'\mid'                         : r'|',
        r'\parallel'                    : r'\||',
        r'\equiv'                       : r'\==',
        r'\sim'                         : r'\ap',
        r'\simeq'                       : r'\~-',
#       r'\asymp'                       : r'',
        r'\approx'                      : r'\~~',
        r'\cong'                        : r'\=~',
#       r'\bowtie'                      : r'',
        r'\propto'                      : r'\pt',
#       r'\models'                      : r'',
#       r'\doteq'                       : r'',
        r'\perp'                        : r'\pp',

        ## arrows
        r'\leftarrow'                   : r'\<-',
        r'\Leftarrow'                   : r'\lA',
        r'\rightarrow'                  : r'\->',
        r'\Rightarrow'                  : r'\rA',
        r'\leftrightarrow'              : r'\<>',
        r'\Leftrightarrow'              : r'\hA',
#       r'\mapsto'                      : r'',
#       r'\hookleftarrow'               : r'',
#       r'\leftharpoonup'               : r'',
#       r'\leftharpoondown'             : r'',
#       r'\rightleftharpoons'           : r'',
#       ...
        r'\uparrow'                     : r'\ua',
        r'\Uparrow'                     : r'\uA',
        r'\downarrow'                   : r'\da',
        r'\Downarrow'                   : r'\dA',
#       r'\updownarrow'                 : r'',
#       r'\Updownarrow'                 : r'',
#       r'\nearrow'                     : r'',
#       r'\searrow'                     : r'',
#       r'\swarrow'                     : r'',
#       r'\nwarrow'                     : r'',

        ## openings
        r'\lbrack'                      : r'[',
        r'\lbrace'                      : r'{',
        r'\langle'                      : r'\la',
#       r'\lfloor'                      : r'',
#       r'\lceil'                       : r'',

        ## closings
        r'\rbrack'                      : r']',
        r'\rbrace'                      : r'}',
        r'\rangle'                      : r'\ra',
#       r'\rfloor'                      : r'',
#       r'\rceil'                       : r'',

        ## alternate names
        r'\ne'                          : r'\!=',
        r'\neq'                         : r'\!=',
        r'\le'                          : r'\<=',
        r'\ge'                          : r'\>=',
        r'\to'                          : r'\->',
        r'\gets'                        : r'\<-',
#       r'\owns'                        : r'',
        r'\land'                        : r'\AN',
        r'\lor'                         : r'\OR',
        r'\lnot'                        : r'\no',
        r'\vert'                        : r'|',
        r'\Vert'                        : r'\||',

        ## extensions
        r'\degree'                      : r'\de',
        r'\deg'                         : r'\de',
        r'\degr'                        : r'\de',
        r'\arcdeg'                      : r'\de',
}

def map_text_token( token ):
    if _text_token_dict.has_key(token):
        return _text_token_dict[token]
    else:
        return _common_token_dict.get( token, token )

def map_math_token( token ):
    if _math_token_dict.has_key(token):
        return _math_token_dict[token]
    else:
        return _common_token_dict.get( token, token )

def math_group( lexer ):
    output = ''
    bracketmode = 0
    while 1:
        token = lexer.get_token()
        if token is None:
            break

        if token == '{':
            bracketmode = 1
        elif token == '}':
            break
        else:
            output = output + map_math_token( token )
            if not bracketmode:
                break
    return output

font_code = [ r'\f0', r'\f1', r'\f2', r'\f3' ]

def tex2libplot( str ):
    output = ''
    mathmode = 0
    font_stack = []
    font = 1

    lexer = TeXLexer( str )
    while 1:
        token = lexer.get_token()
        if token is None:
            break

        append = ''

        if token == '$':
            mathmode = not mathmode
        elif token == '{':
            font_stack.append( font )
        elif token == '}':
            old_font = font_stack.pop()
            if old_font != font:
                font = old_font
                append = font_code[font]
        elif token == r'\rm':
            font = 1
            append = font_code[font]
        elif token == r'\it':
            font = 2
            append = font_code[font]
        elif token == r'\bf':
            font = 3
            append = font_code[font]
        elif not mathmode:
            append = map_text_token( token )
        elif token == '_':
            append = r'\sb' + math_group(lexer) + r'\eb'
            if lexer.peek() == '^':
                append = r'\mk' + append + r'\rt'
        elif token == '^':
            append = r'\sp' + math_group(lexer) + r'\ep'
            if lexer.peek() == '_':
                append = r'\mk' + append + r'\rt'
        else:
            append = map_math_token( token )

        output = output + append

    return output
