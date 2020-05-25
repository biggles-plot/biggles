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

/* The following Postscript procset defines a set of macros and constants
   that libplot uses when rendering graphics.  It is split into several
   shorter pieces, because some compilers have difficulty with strings that
   are too long.  It is #included by p_defplot.c.

   The procset was largely written by John Interrante
   <interran@uluru.stanford.edu>, formerly of the InterViews team.  (Thanks
   to John for generously providing it, and for helpful comments.)
   For more information see the InterViews distribution at
   ftp://interviews.stanford.edu.

   The present procset, version 1.1, includes a minor modification.
   Originally, FontBBox was assumed always to be an executable array.  Many
   fonts are designed this way, but it is not required.  The assumption
   gave problems on some versions of ghostscript.  Thanks to Alex
   Cherepanov for the bug fix.  */

#define PS_PROCSET_NAME "GNU_libplot"
#define PS_PROCSET_VERSION "1.1"

static const char * const _ps_fontproc =
"\
/ISOLatin1Encoding [\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/space/exclam/quotedbl/numbersign/dollar/percent/ampersand/quoteright\n\
/parenleft/parenright/asterisk/plus/comma/minus/period/slash\n\
/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon\n\
/less/equal/greater/question/at/A/B/C/D/E/F/G/H/I/J/K/L/M/N\n\
/O/P/Q/R/S/T/U/V/W/X/Y/Z/bracketleft/backslash/bracketright\n\
/asciicircum/underscore/quoteleft/a/b/c/d/e/f/g/h/i/j/k/l/m\n\
/n/o/p/q/r/s/t/u/v/w/x/y/z/braceleft/bar/braceright/asciitilde\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n\
/.notdef/dotlessi/grave/acute/circumflex/tilde/macron/breve\n\
/dotaccent/dieresis/.notdef/ring/cedilla/.notdef/hungarumlaut\n\
/ogonek/caron/space/exclamdown/cent/sterling/currency/yen/brokenbar\n\
/section/dieresis/copyright/ordfeminine/guillemotleft/logicalnot\n\
/hyphen/registered/macron/degree/plusminus/twosuperior/threesuperior\n\
/acute/mu/paragraph/periodcentered/cedilla/onesuperior/ordmasculine\n\
/guillemotright/onequarter/onehalf/threequarters/questiondown\n\
/Agrave/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla\n\
/Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex\n\
/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde/Odieresis\n\
/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute\n\
/Thorn/germandbls/agrave/aacute/acircumflex/atilde/adieresis\n\
/aring/ae/ccedilla/egrave/eacute/ecircumflex/edieresis/igrave\n\
/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute/ocircumflex\n\
/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex/udieresis\n\
/yacute/thorn/ydieresis\n\
] def\n\
/reencodeISO {\n\
dup dup findfont dup length dict begin\n\
{ 1 index /FID ne { def }{ pop pop } ifelse } forall\n\
/Encoding ISOLatin1Encoding def\n\
currentdict end definefont\n\
} def\n";

/* The following is split into substrings, because some compilers (e.g., 
   MS VC++) can't handle very long strings. */

static const char * const _ps_procset[] =
{"\
/none null def\n\
/numGraphicParameters 17 def\n\
/stringLimit 65535 def\n\
/arrowHeight 8 def\n\
/eoFillRule true def\n\n\
/Begin { save numGraphicParameters dict begin } def\n\
/End { end restore } def\n\
\n\
/SetB {\n\
dup type /nulltype eq {\n\
pop\n\
false /brushRightArrow idef\n\
false /brushLeftArrow idef\n\
true /brushNone idef\n\
} {\n\
/brushDashOffset idef\n\
/brushDashArray idef\n\
0 ne /brushRightArrow idef\n\
0 ne /brushLeftArrow idef\n\
/brushWidth idef\n\
false /brushNone idef\n",
"} ifelse\n\
} def\n\
\n\
/SetCFg {\n\
/fgblue idef\n\
/fggreen idef\n\
/fgred idef\n\
} def\n\
\n\
/SetCBg {\n\
/bgblue idef\n\
/bggreen idef\n\
/bgred idef\n\
} def\n\
\n\
/SetF {\n\
/printSize idef\n\
/printFont idef\n\
} def\n\
\n\
/SetP {\n\
dup type /nulltype eq {\n\
pop true /patternNone idef\n\
} {\n\
/patternGrayLevel idef\n\
patternGrayLevel -1 eq {\n\
/patternString idef\n\
} if\n\
false /patternNone idef\n\
} ifelse\n\
} def\n\
\n",
"/BSpl {\n\
0 begin\n\
storexyn\n\
newpath\n\
n 1 gt {\n\
0 0 0 0 0 0 1 1 true subspline\n\
n 2 gt {\n\
0 0 0 0 1 1 2 2 false subspline\n\
1 1 n 3 sub {\n\
/i exch def\n\
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline\n\
} for\n\
n 3 sub dup n 2 sub dup n 1 sub dup 2 copy false subspline\n\
} if\n\
n 2 sub dup n 1 sub dup 2 copy 2 copy false subspline\n\
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if\n\
brushNone not { istroke } if\n",
"0 0 1 1 leftarrow\n\
n 2 sub dup 1 sub dup rightarrow\n\
} if\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/Circ {\n\
newpath\n\
0 360 arc\n\
closepath\n\
patternNone not { ifill } if\n\
brushNone not { istroke } if\n\
} def\n\
\n",
"/CBSpl {\n\
0 begin\n\
dup 2 gt {\n\
storexyn\n\
newpath\n\
n 1 sub dup 0 0 1 1 2 2 true subspline\n\
1 1 n 3 sub {\n\
/i exch def\n\
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline\n\
} for\n",
"n 3 sub dup n 2 sub dup n 1 sub dup 0 0 false subspline\n\
n 2 sub dup n 1 sub dup 0 0 1 1 false subspline\n\
patternNone not { ifill } if\n\
brushNone not { istroke } if\n\
} {\n\
Poly\n\
} ifelse\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/Elli {\n\
0 begin\n\
newpath\n\
4 2 roll\n\
translate\n\
scale\n\
0 0 1 0 360 arc\n\
closepath\n\
patternNone not { ifill } if\n\
brushNone not { istroke } if\n\
end\n\
} dup 0 1 dict put def\n\
\n\
/Line {\n\
0 begin\n\
2 storexyn\n",
"newpath\n\
x 0 get y 0 get moveto\n\
x 1 get y 1 get lineto\n\
brushNone not { istroke } if\n\
0 0 1 1 leftarrow\n\
0 0 1 1 rightarrow\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/MLine {\n\
0 begin\n\
storexyn\n\
newpath\n\
n 1 gt {\n\
x 0 get y 0 get moveto\n\
1 1 n 1 sub {\n\
/i exch def\n\
x i get y i get lineto\n\
} for\n\
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if\n\
brushNone not { istroke } if\n",
"0 0 1 1 leftarrow\n\
n 2 sub dup n 1 sub dup rightarrow\n\
} if\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/Poly {\n\
3 1 roll\n\
newpath\n\
moveto\n\
-1 add\n\
{ lineto } repeat\n\
closepath\n\
patternNone not { ifill } if\n\
brushNone not { istroke } if\n\
} def\n\
\n",
"/Rect {\n\
0 begin\n\
/t exch def\n\
/r exch def\n\
/b exch def\n\
/l exch def\n\
newpath\n\
l b moveto\n\
l t lineto\n\
r t lineto\n\
r b lineto\n\
closepath\n\
patternNone not { ifill } if\n\
brushNone not { istroke } if\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/Text {\n\
ishow\n\
} def\n\
\n\
/idef {\n\
dup where { pop pop pop } { exch def } ifelse\n\
} def\n\
\n\
/ifill {\n\
0 begin\n\
gsave\n",
"patternGrayLevel -1 ne {\n\
fgred bgred fgred sub patternGrayLevel mul add\n\
fggreen bggreen fggreen sub patternGrayLevel mul add\n\
fgblue bgblue fgblue sub patternGrayLevel mul add setrgbcolor\n\
eoFillRule { eofill } { fill } ifelse\n\
} {\n\
eoFillRule { eoclip } { clip } ifelse\n\
originalCTM setmatrix\n\
pathbbox /t exch def /r exch def /b exch def /l exch def\n\
/w r l sub ceiling cvi def\n\
/h t b sub ceiling cvi def\n\
/imageByteWidth w 8 div ceiling cvi def\n\
/imageHeight h def\n",
"bgred bggreen bgblue setrgbcolor\n\
eoFillRule { eofill } { fill } ifelse\n\
fgred fggreen fgblue setrgbcolor\n\
w 0 gt h 0 gt and {\n\
l b translate w h scale\n\
w h true [w 0 0 h neg 0 h] { patternproc } imagemask\n\
} if\n\
} ifelse\n\
grestore\n\
end\n\
} dup 0 8 dict put def\n\
\n",
"/istroke {\n\
gsave\n\
brushDashOffset -1 eq {\n\
[] 0 setdash\n\
1 setgray\n\
} {\n\
brushDashArray brushDashOffset setdash\n\
fgred fggreen fgblue setrgbcolor\n\
} ifelse\n",
"brushWidth setlinewidth\n\
originalCTM setmatrix\n\
stroke\n\
grestore\n\
} def\n\
\n\
/ishow {\n\
0 begin\n\
gsave\n\
fgred fggreen fgblue setrgbcolor\n\
/fontDict printFont findfont printSize scalefont dup setfont def\n\
/descender fontDict begin 0 /FontBBox load 1 get FontMatrix end\n\
transform exch pop def\n\
/vertoffset 1 printSize sub descender sub def {\n\
0 vertoffset moveto show\n\
/vertoffset vertoffset printSize sub def\n\
} forall\n\
grestore\n\
end\n\
} dup 0 3 dict put def\n\
\n",
"/patternproc {\n\
0 begin\n\
/patternByteLength patternString length def\n\
/patternHeight patternByteLength 8 mul sqrt cvi def\n\
/patternWidth patternHeight def\n\
/patternByteWidth patternWidth 8 idiv def\n\
/imageByteMaxLength imageByteWidth imageHeight mul\n\
stringLimit patternByteWidth sub min def\n\
/imageMaxHeight imageByteMaxLength imageByteWidth idiv patternHeight idiv\n\
patternHeight mul patternHeight max def\n",
"/imageHeight imageHeight imageMaxHeight sub store\n\
/imageString imageByteWidth imageMaxHeight mul patternByteWidth add string def\n\
0 1 imageMaxHeight 1 sub {\n\
/y exch def\n\
/patternRow y patternByteWidth mul patternByteLength mod def\n\
/patternRowString patternString patternRow patternByteWidth getinterval def\n\
/imageRow y imageByteWidth mul def\n\
0 patternByteWidth imageByteWidth 1 sub {\n\
/x exch def\n\
imageString imageRow x add patternRowString putinterval\n\
} for\n\
} for\n",
"imageString\n\
end\n\
} dup 0 12 dict put def\n\
\n\
/min {\n\
dup 3 2 roll dup 4 3 roll lt { exch } if pop\n\
} def\n\
\n\
/max {\n\
dup 3 2 roll dup 4 3 roll gt { exch } if pop\n\
} def\n\
\n\
/midpoint {\n\
0 begin\n\
/y1 exch def\n\
/x1 exch def\n\
/y0 exch def\n\
/x0 exch def\n\
x0 x1 add 2 div\n\
y0 y1 add 2 div\n\
end\n\
} dup 0 4 dict put def\n\
\n",
"/thirdpoint {\n\
0 begin\n\
/y1 exch def\n\
/x1 exch def\n\
/y0 exch def\n\
/x0 exch def\n",
"x0 2 mul x1 add 3 div\n\
y0 2 mul y1 add 3 div\n\
end\n\
} dup 0 4 dict put def\n\
\n\
/subspline {\n\
0 begin\n\
/movetoNeeded exch def\n\
y exch get /y3 exch def\n\
x exch get /x3 exch def\n\
y exch get /y2 exch def\n\
x exch get /x2 exch def\n\
y exch get /y1 exch def\n\
x exch get /x1 exch def\n\
y exch get /y0 exch def\n\
x exch get /x0 exch def\n\
x1 y1 x2 y2 thirdpoint\n\
/p1y exch def\n\
/p1x exch def\n\
x2 y2 x1 y1 thirdpoint\n\
/p2y exch def\n\
/p2x exch def\n",
"x1 y1 x0 y0 thirdpoint\n\
p1x p1y midpoint\n\
/p0y exch def\n\
/p0x exch def\n\
x2 y2 x3 y3 thirdpoint\n\
p2x p2y midpoint\n\
/p3y exch def\n\
/p3x exch def\n\
movetoNeeded { p0x p0y moveto } if\n\
p1x p1y p2x p2y p3x p3y curveto\n\
end\n\
} dup 0 17 dict put def\n\
\n\
/storexyn {\n\
/n exch def\n\
/y n array def\n\
/x n array def\n\
n 1 sub -1 0 {\n\
/i exch def\n\
y i 3 2 roll put\n\
x i 3 2 roll put\n\
} for\n\
} def\n\
\n",
"/arrowhead {\n\
0 begin\n\
transform originalCTM itransform\n",
"/taily exch def\n\
/tailx exch def\n\
transform originalCTM itransform\n\
/tipy exch def\n\
/tipx exch def\n\
/dy tipy taily sub def\n\
/dx tipx tailx sub def\n\
/angle dx 0 ne dy 0 ne or { dy dx atan } { 90 } ifelse def\n\
gsave\n\
originalCTM setmatrix\n\
tipx tipy translate\n\
angle rotate\n\
newpath\n\
arrowHeight neg arrowWidth 2 div moveto\n\
0 0 lineto\n\
arrowHeight neg arrowWidth 2 div neg lineto\n\
patternNone not {\n\
originalCTM setmatrix\n",
"/padtip arrowHeight 2 exp 0.25 arrowWidth 2 exp mul add sqrt brushWidth mul\n\
arrowWidth div def\n\
/padtail brushWidth 2 div def\n\
tipx tipy translate\n\
angle rotate\n\
padtip 0 translate\n\
arrowHeight padtip add padtail add arrowHeight div dup scale\n\
arrowheadpath\n\
ifill\n\
} if\n\
brushNone not {\n\
originalCTM setmatrix\n\
tipx tipy translate\n\
angle rotate\n\
arrowheadpath\n\
istroke\n\
} if\n\
grestore\n\
end\n\
} dup 0 9 dict put def\n\
\n",
"/arrowheadpath {\n\
newpath\n\
arrowHeight neg arrowWidth 2 div moveto\n\
0 0 lineto\n\
arrowHeight neg arrowWidth 2 div neg lineto\n\
} def\n\
\n\
/leftarrow {\n\
0 begin\n\
y exch get /taily exch def\n\
x exch get /tailx exch def\n\
y exch get /tipy exch def\n\
x exch get /tipx exch def\n\
brushLeftArrow { tipx tipy tailx taily arrowhead } if\n\
end\n\
} dup 0 4 dict put def\n\
\n",
"/rightarrow {\n\
0 begin\n\
y exch get /tipy exch def\n\
x exch get /tipx exch def\n\
y exch get /taily exch def\n\
x exch get /tailx exch def\n\
brushRightArrow { tipx tipy tailx taily arrowhead } if\n\
end\n\
} dup 0 4 dict put def\n",
""};
