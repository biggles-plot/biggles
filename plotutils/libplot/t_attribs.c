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

/* This internal method is invoked before drawing any polyline.  It sets
   the relevant attributes of a Tektronix display (line type only, since
   cap type, join type, line width are not supported) to what they should
   be.

   Our canonical line types are the same as those of a Tektronix, except
   that we also support "dotdotdashed" lines.  So setting the line type is
   straightforward.  kermit's assignment of line types is different from
   that of a genuine Tektronix, though. */

#include "sys-defines.h"
#include "extern.h"

void
_pl_t_set_attributes (S___(Plotter *_plotter))
{
  if ((!(_plotter->tek_line_type_is_unknown))
      && (_plotter->tek_line_type == _plotter->drawstate->line_type))
    return;

  switch (_plotter->drawstate->line_type)
    {
    default:
    case PL_L_SOLID:
      /* ASCII ESC `, i.e. ^[` */
      _write_string (_plotter->data, "\033`");
      break;
    case PL_L_DOTTED:
      /* ASCII ESC a, i.e. ^[a */
      _write_string (_plotter->data, "\033a");	
      break;
      /* following two are interchanged in kermit emulator */
    case PL_L_DOTDASHED:
      if (_plotter->tek_display_type == TEK_DPY_KERMIT)
	/* ASCII ESC c, i.e. ^[c */
	_write_string (_plotter->data, "\033c"); 
      else
	/* ASCII ESC b, i.e. ^[b */
	_write_string (_plotter->data, "\033b"); 
      break;
    case PL_L_SHORTDASHED:
      if (_plotter->tek_display_type == TEK_DPY_KERMIT)
	/* ASCII ESC b, i.e. ^[b */
	_write_string (_plotter->data, "\033b"); 
      else
	/* ASCII ESC c, i.e. ^[c */
	_write_string (_plotter->data, "\033c"); 
      break;
    case PL_L_LONGDASHED:
      /* in kermit emulator, the following switches to "dotlongdashed"
	 rather than "longdashed", but we can live with that */
      /* ASCII ESC d, i.e. ^[d */
      _write_string (_plotter->data, "\033d");	
      break;
    case PL_L_DOTDOTDASHED:
      if (_plotter->tek_display_type == TEK_DPY_KERMIT)
	/* ASCII ESC e, i.e. ^[e */
	_write_string (_plotter->data, "\033e"); 
      else
	/* not supported on a genuine Tektronix, so punt */
	/* ASCII ESC b, i.e. ^[b */
	_write_string (_plotter->data, "\033b"); 
      break;
    case PL_L_DOTDOTDOTDASHED:
      /* not supported, so punt */
      /* ASCII ESC b, i.e. ^[b */
      _write_string (_plotter->data, "\033b");
      break;
    }

  /* Tek now agrees with us on line type */
  _plotter->tek_line_type = _plotter->drawstate->line_type;
  _plotter->tek_line_type_is_unknown = false;
}  

/* The reason for the kermit-specific modifications above is that according
   to kermit documentation, the MS-DOS kermit Tektronix emulator has a
   different ordering for line types:

        ` = solid		11111111 11111111
        a = dotted      	10101010 10101010 
        b = shortdashed		11110000 11110000
        c = dotdashed           11111010 11111010
        d = dotlongdashed       11111111 11001100
        e = dotdotdashed        11111100 10010010
        x = user defined (by ESC / Pn a)
        y = user defined (by ESC / Pn b)
        z = user defined (by ESC / Pn c)
	
   Incidentally, line type characters recognized by VT-type terminals in
   Tektronix emulator mode also allegedly differ.  According to an old doc
   file,

        ` = solid
        a = dotted
	b = shortdashed
	c = dotdashed
	d = dotlongdashed
	h = solid (bold) 
        i = dotted (bold) 
        j = shortdashed (bold)
        k = dotdashed (bold)
        l = dotlongdashed (bold)

   Interestingly, BSD atoplot(1) recognizes "dotlongdashed",
   "dotshortdashed", and "dotdotdashed" (with final "ed" omitted), besides
   the five canonical Tektronix line types.  So when atoplot(1) was
   written, there must have been plot(1) filters for output devices that
   supported those additional types.  Presumably on VT-type terminals? */
