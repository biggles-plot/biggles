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

/* This file contains the closepl method, which is a standard part of
   libplot.  It closes a Plotter object.

   For an AIPlotter objects, we prepare an Illustrator header for the page.
   After the header comes the graphics code for all objects, which we have
   saved in a resizable outbuf structure for the page.  Then a trailer.

   AI format supports only single-page documents. */

#include "sys-defines.h"
#include "extern.h"

/* song and dance to define time_t, and declare both time() and ctime() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* for time_t on some pre-ANSI Unix systems */
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>		/* for time() on some pre-ANSI Unix systems */
#include <time.h>		/* for ctime() */
#else  /* not TIME_WITH_SYS_TIME, include only one (prefer <sys/time.h>) */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else  /* not HAVE_SYS_TIME_H */
#include <time.h>
#endif /* not HAVE_SYS_TIME_H */
#endif /* not TIME_WITH_SYS_TIME */

static const char * const _ai_symbol_reencoding = "32/space\n/exclam\n/universal\n/numbersign\n/existential\n/percent\n/ampersand\n/suchthat\n/parenleft\n/parenright\n/asteriskmath\n/plus\n/comma\n/minus\n/period\n/slash\n/zero\n/one\n/two\n/three\n/four\n/five\n/six\n/seven\n/eight\n/nine\n/colon\n/semicolon\n/less\n/equal\n/greater\n/question\n/congruent\n/Alpha\n/Beta\n/Chi\n/Delta\n/Epsilon\n/Phi\n/Gamma\n/Eta\n/Iota\n/theta1\n/Kappa\n/Lambda\n/Mu\n/Nu\n/Omicron\n/Pi\n/Theta\n/Rho\n/Sigma\n/Tau\n/Upsilon\n/sigma1\n/Omega\n/Xi\n/Psi\n/Zeta\n/bracketleft\n/therefore\n/bracketright\n/perpendicular\n/underscore\n/radicalex\n/alpha\n/beta\n/chi\n/delta\n/epsilon\n/phi\n/gamma\n/eta\n/iota\n/phi1\n/kappa\n/lambda\n/mu\n/nu\n/omicron\n/pi\n/theta\n/rho\n/sigma\n/tau\n/upsilon\n/omega1\n/omega\n/xi\n/psi\n/zeta\n/braceleft\n/bar\n/braceright\n/similar\n161/Upsilon1\n/minute\n/lessequal\n/fraction\n/infinity\n/florin\n/club\n/diamond\n/heart\n/spade\n/arrowboth\n/arrowleft\n/arrowup\n/arrowright\n/arrowdown\n/degree\n/plusminus\n/second\n/greaterequal\n/multiply\n/proportional\n/partialdiff\n/bullet\n/divide\n/notequal\n/equivalence\n/approxequal\n/ellipsis\n/arrowvertex\n/arrowhorizex\n/carriagereturn\n/aleph\n/Ifraktur\n/Rfraktur\n/weierstrass\n/circlemultiply\n/circleplus\n/emptyset\n/intersection\n/union\n/propersuperset\n/reflexsuperset\n/notsubset\n/propersubset\n/reflexsubset\n/element\n/notelement\n/angle\n/gradient\n/registerserif\n/copyrightserif\n/trademarkserif\n/product\n/radical\n/dotmath\n/logicalnot\n/logicaland\n/logicalor\n/arrowdblboth\n/arrowdblleft\n/arrowdblup\n/arrowdblright\n/arrowdbldown\n/lozenge\n/angleleft\n/registersans\n/copyrightsans\n/trademarksans\n/summation\n/parenlefttp\n/parenleftex\n/parenleftbt\n/bracketlefttp\n/bracketleftex\n/bracketleftbt\n/bracelefttp\n/braceleftmid\n/braceleftbt\n/braceex\n241/angleright\n/integral\n/integraltp\n/integralex\n/integralbt\n/parenrighttp\n/parenrightex\n/parenrightbt\n/bracketrighttp\n/bracketrightex\n/bracketrightbt\n/bracerighttp\n/bracerightmid\n/bracerightbt\n";

static const char * const _ai_zapf_dingbats_reencoding = "32/space\n/a1\n/a2\n/a202\n/a3\n/a4\n/a5\n/a119\n/a118\n/a117\n/a11\n/a12\n/a13\n/a14\n/a15\n/a16\n/a105\n/a17\n/a18\n/a19\n/a20\n/a21\n/a22\n/a23\n/a24\n/a25\n/a26\n/a27\n/a28\n/a6\n/a7\n/a8\n/a9\n/a10\n/a29\n/a30\n/a31\n/a32\n/a33\n/a34\n/a35\n/a36\n/a37\n/a38\n/a39\n/a40\n/a41\n/a42\n/a43\n/a44\n/a45\n/a46\n/a47\n/a48\n/a49\n/a50\n/a51\n/a52\n/a53\n/a54\n/a55\n/a56\n/a57\n/a58\n/a59\n/a60\n/a61\n/a62\n/a63\n/a64\n/a65\n/a66\n/a67\n/a68\n/a69\n/a70\n/a71\n/a72\n/a73\n/a74\n/a203\n/a75\n/a204\n/a76\n/a77\n/a78\n/a79\n/a81\n/a82\n/a83\n/a84\n/a97\n/a98\n/a99\n/a100\n160/space\n/a101\n/a102\n/a103\n/a104\n/a106\n/a107\n/a108\n/a112\n/a111\n/a110\n/a109\n/a120\n/a121\n/a122\n/a123\n/a124\n/a125\n/a126\n/a127\n/a128\n/a129\n/a130\n/a131\n/a132\n/a133\n/a134\n/a135\n/a136\n/a137\n/a138\n/a139\n/a140\n/a141\n/a142\n/a143\n/a144\n/a145\n/a146\n/a147\n/a148\n/a149\n/a150\n/a151\n/a152\n/a153\n/a154\n/a155\n/a156\n/a157\n/a158\n/a159\n/a160\n/a161\n/a163\n/a164\n/a196\n/a165\n/a192\n/a166\n/a167\n/a168\n/a169\n/a170\n/a171\n/a172\n/a173\n/a162\n/a174\n/a175\n/a176\n/a177\n/a178\n/a179\n/a193\n/a180\n/a199\n/a181\n/a200\n/a182\n241/a201\n/a183\n/a184\n/a197\n/a185\n/a194\n/a198\n/a186\n/a195\n/a187\n/a188\n/a189\n/a190\n/a191\n";

bool
_pl_a_end_page (S___(Plotter *_plotter))
{
  bool fonts_used = false;
  int i;

  /* Beginning of Page Header */
  {
    char *time_s;
    double x_min, x_max, y_min, y_max;
    double xmid, ymid;
    int ixmid, iymid;
    time_t clock;
    plOutbuf *page_header;
    
    /* first, prepare AI header, and write it to a plOutbuf */
    page_header = _new_outbuf ();
    
    /* compute center of viewport in device coors (i.e. points) */
    xmid = 0.5 * (_plotter->data->xmin + _plotter->data->xmax);
    ymid = 0.5 * (_plotter->data->ymin + _plotter->data->ymax);
    ixmid = IROUND(xmid);
    iymid = IROUND(ymid);      
    
    /* emit first few comment lines */
    sprintf (page_header->point, "\
%%!PS-Adobe-3.0\n\
%%%%Creator: GNU libplot drawing library %s\n\
%%%%For: (Unknown) (Unknown)\n\
%%%%Title: (Untitled)\n", 
	     PL_LIBPLOT_VER_STRING);
    _update_buffer (page_header);
    
    /* emit creation date and time, if possible */
    time(&clock);
    time_s = ctime(&clock);
    if (time_s != NULL)
      {
	char weekday[32], month[32], day[32], hour_min_sec[32], year[32];
	int num_matched;
	
	num_matched = sscanf (time_s, "%s %s %s %s %s",
			      weekday, month, day, hour_min_sec, year);
	if (num_matched == 5)
	  {
	    sprintf (page_header->point, "\
%%%%CreationDate: (%s %s %s) (%s)\n",
		     day, month, year, hour_min_sec);
	    _update_buffer (page_header);
	  }
      }
    
    /* emit bounding box for the page */
    _bbox_of_outbuf (_plotter->data->page, &x_min, &x_max, &y_min, &y_max);
    if (x_min > x_max || y_min > y_max) /* no objects */
      /* place degenerate box at center of page */
      sprintf (page_header->point, "\
%%%%BoundingBox: %d %d %d %d\n",
	       ixmid, iymid, ixmid, iymid);
    else
      /* emit true bounding box */
      sprintf (page_header->point, "\
%%%%BoundingBox: %d %d %d %d\n",
	       IROUND(x_min - 0.5), IROUND(y_min - 0.5),
	       IROUND(x_max + 0.5), IROUND(y_max + 0.5));
    _update_buffer (page_header);
    if (_plotter->ai_version >= AI_VERSION_5)
      /* emit hi-res bounding box too */
      {
	if (x_min > x_max || y_min > y_max) /* empty page */
	  /* place degenerate box at center of page */
	    sprintf (page_header->point, "\
%%%%HiResBoundingBox: %.4f %.4f %.4f %.4f\n",
		     xmid, ymid, xmid, ymid);
	else
	  /* emit true bounding box */
	  sprintf (page_header->point, "\
%%%%HiResBoundingBox: %.4f %.4f %.4f %.4f\n",
		   x_min, y_min, x_max, y_max);
	_update_buffer (page_header);
      }
    
    /* emit process colors used */
    sprintf (page_header->point, "\
%%%%DocumentProcessColors:");
    _update_buffer (page_header);
    if (_plotter->ai_cyan_used)
      {
	sprintf (page_header->point, " Cyan");
	_update_buffer (page_header);
      }
    if (_plotter->ai_magenta_used)
      {
	sprintf (page_header->point, " Magenta");
	_update_buffer (page_header);
      }
    if (_plotter->ai_yellow_used)
      {
	sprintf (page_header->point, " Yellow");
	_update_buffer (page_header);
      }
    if (_plotter->ai_black_used)
      {
	sprintf (page_header->point, " Black");
	_update_buffer (page_header);
      }
    sprintf (page_header->point, "\n");
    _update_buffer (page_header);      
    
    /* tell AI to include any PS [or PCL] fonts that are needed */
    sprintf (page_header->point, "\
%%%%DocumentFonts: ");
    _update_buffer (page_header);
    for (i = 0; i < PL_NUM_PS_FONTS; i++)
      if (_plotter->data->page->ps_font_used[i])
	{
	  if (fonts_used)	/* not first font */
	    sprintf (page_header->point, 
		     "%%%%+ %s\n", _pl_g_ps_font_info[i].ps_name);
	  else		/* first font */
	    sprintf (page_header->point, 
		     "%s\n", _pl_g_ps_font_info[i].ps_name);
	  _update_buffer (page_header);
	  fonts_used = true;
	}
    for (i = 0; i < PL_NUM_PCL_FONTS; i++)
      if (_plotter->data->page->pcl_font_used[i])
	{
	  if (fonts_used)	/* not first font */
	    sprintf (page_header->point, 
		     "%%%%+ %s\n", _pl_g_pcl_font_info[i].ps_name);
	  else		/* first font */
	    sprintf (page_header->point, 
		     "%s\n", _pl_g_pcl_font_info[i].ps_name);
	  _update_buffer (page_header);
	  fonts_used = true;
	}
    if (!fonts_used)
      {
	sprintf (page_header->point, "\n");
	_update_buffer (page_header);
      }
    
    /* tell AI or print spooler that we need procsets */
    if (_plotter->ai_version == AI_VERSION_5)
      {
	sprintf (page_header->point, "\
%%%%DocumentNeededResources: procset Adobe_level2_AI5 1.0 0\n\
%%%%+ procset Adobe_typography_AI5 1.0 0\n\
%%%%+ procset Adobe_Illustrator_AI6_vars Adobe_Illustrator_AI6\n\
%%%%+ procset Adobe_Illustrator_AI5 1.0 0\n");
	_update_buffer (page_header);

	/* claim to be AI 7.0 (that's what `3' means) */
	sprintf (page_header->point, "\
%%AI5_FileFormat 3\n");
	_update_buffer (page_header);
      }
    else			/* AI_VERSION_3 */
      {
	sprintf (page_header->point, "\
%%%%DocumentNeededResources: procset Adobe_packedarray 2.0 0\n\
%%%%+ procset Adobe_cmykcolor 1.1 0\n\
%%%%+ procset Adobe_cshow 1.1 0\n\
%%%%+ procset Adobe_customcolor 1.0 0\n\
%%%%+ procset Adobe_typography_AI3 1.0 1\n\
%%%%+ procset Adobe_pattern_AI3 1.0 0\n\
%%%%+ procset Adobe_Illustrator_AI3 1.0 1\n");
	_update_buffer (page_header);
      }
    
    /* AI3 directives. */
    
    /* tell AI whether or not we're monochrome */
    sprintf (page_header->point, "\
%%AI3_ColorUsage: ");
    _update_buffer (page_header);
    if (_plotter->ai_cyan_used || _plotter->ai_magenta_used || _plotter->ai_yellow_used)
      sprintf (page_header->point, "Color\n");
    else
      sprintf (page_header->point, "Black&White\n");
    _update_buffer (page_header);
    
    /* no linked images are embedded in this file */
    sprintf (page_header->point, "\
%%AI7_ImageSettings: 0\n");
    _update_buffer (page_header);
    
    /* place degenerate template box at center of viewport (used for
       centering, in case size of artboard changes between successive
       versions of AI) */
    sprintf (page_header->point, "\
%%AI3_TemplateBox: %d %d %d %d\n",
	     ixmid, iymid, ixmid, iymid);
    _update_buffer (page_header);
    
    /* nominal imageable area of the page (used only by Macintosh version
       of AI?): we specify our horizontal range, and the full page height */
    sprintf (page_header->point, "\
%%AI3_TileBox: %d %d %d %d\n",
	     IROUND(_plotter->data->xmin),
	     0,
	     IROUND(_plotter->data->xmax),
	     IROUND(72 * _plotter->data->page_data->ysize));
    _update_buffer (page_header);
    
    sprintf (page_header->point, "\
%%AI3_DocumentPreview: None\n");
    _update_buffer (page_header);
      
    /* AI5 directives. */
    
    /* Note: AI5_ArtFlags consists of nine 0/1 flags that describe the
       settings found in the Document Setup dialogue; meaning of each `1'
       is: use page setup / use print tiles / show placed images / preview
       patterns / split long paths / tile full pages / use printer's
       default screens / use auto default screens [meaningful only if
       previous flag = 1] / use compatible gradient printing */

    /* Note: we could add an %AI5_OpenToView line, between the
       %AI5_NumLayers line and the %AI5_OpenViewLayers line.  Such a line
       would look like
       
       %AI5_OpenToView: -318 780 -2 624 379 18 0 1 98 74 0 0

       with 12 parameters.  It specifies the size, location, and
       orientation of the artwork within the AI display, when the file is
       opened.  Parameters are:

       1,2. (ulx, uly).  Position, in integer artwork coors, of upper left
       corner of AI's artwork window.
       3. zoom.  Integer zoom factor (a negative number represents 1/x).
       4,5. w,h.  Width and height of the artwork window, in pixels.
       Allegedly floating point, not integer.
       6. view_style.  An integer specifying what's displayed 
       (e.g. 25=artwork, 26=preview, 30=preview selection).
       7. ruler.  0/1 flag, 1 means show ruler.
       8. tiling. 0/1 flag, 1 means show tiling.
       9,10. (ul_monx, ul_mony).  Upper left corner of the artwork window
       on the monitor, in integer coordinates. (!)
       11. grid. 0/1 flag, 1 means display grid.
       12. snap_grid.  0/1 flag, 1 means snap to grid. */
    
    if (_plotter->ai_version >= AI_VERSION_5)
      {
	sprintf (page_header->point, "\
%%AI5_ArtSize: %d %d\n\
%%AI5_RulerUnits: %d\n\
%%AI5_ArtFlags: 1 0 0 1 0 0 1 1 0\n\
%%AI5_TargetResolution: 800\n\
%%AI5_NumLayers: 1\n\
%%AI5_OpenViewLayers: 7\n",
		 /* For `ArtSize' (size of the artboard in points), we
		    specify the entire physical page. */
		 /* page width */
		 IROUND(72 * _plotter->data->page_data->xsize),
		 /* page height */
		 IROUND(72 * _plotter->data->page_data->ysize),
		 /* label AI's rulers with centimeters or inches
		    (4 = cm, 0 = in) */
		 _plotter->data->page_data->metric ? 4 : 0);
	_update_buffer (page_header);
      }
    
    /* following three may be used only by old Macintosh versions of AI? */
    
    /* so-called page origin, taken to be lower left corner of nominal
       imageable area (see above) */
    sprintf (page_header->point, "\
%%%%PageOrigin:%d %d\n",
	     IROUND(_plotter->data->xmin), 0);
    _update_buffer (page_header);
    
    /* paper rectangle, relative to the lower left corner of the nominal
       imageable area (see above) */
    sprintf (page_header->point, "\
%%%%AI3_PaperRect:%d %d %d %d\n",
	     -IROUND(_plotter->data->xmin),
	     IROUND(72 * _plotter->data->page_data->ysize),
	     IROUND(72 * _plotter->data->page_data->xsize - _plotter->data->xmin),
	     0);
    _update_buffer (page_header);
    
    /* margins on all sides of the paper, i.e. the offsets between page
       edges and the nominal imageable area (see above) */
    sprintf (page_header->point, "\
%%%%AI3_Margin:%d %d %d %d\n",
	     IROUND(_plotter->data->xmin),
	     0,
	     IROUND(-(72 * _plotter->data->page_data->xsize - _plotter->data->xmax)),
	     0);
    _update_buffer (page_header);
    
    /* gridlines; parameters are:
       num. horizontal points between gridlines /
       num. horizontal subdivisions /
       num. vertical points between gridlines /
       num. vertical subdivisions /
       gridlines in front/back of artwork (0/1) /
       grid style lines/dots (0/1) /
       RGB for gridlines / 
       RGB for subdivisions */
    
    if (_plotter->data->page_data->metric)
      /* visible grid spacing = 1 cm, 3 subdivisions / division */
      sprintf (page_header->point, "\
%%AI7_GridSettings: %.4f 3 %.4f 3 1 0 0.8 0.8 0.8 0.9 0.9 0.9\n",
	       72.0/2.54, 72.0/2.54);
    else
      /* visible grid spacing = 1 in, 8 subdivisions / division */
      sprintf (page_header->point, "\
%%AI7_GridSettings: 72 8 72 8 1 0 0.8 0.8 0.8 0.9 0.9 0.9\n");
    _update_buffer (page_header);
    
    sprintf (page_header->point, "\
%%%%EndComments\n");
    _update_buffer (page_header);
    
    /* Prolog section: include the procsets */
    if (_plotter->ai_version == AI_VERSION_5)
      sprintf (page_header->point, "\
%%%%BeginProlog\n\
%%%%IncludeResource: procset Adobe_level2_AI5 1.0 0\n\
%%%%IncludeResource: procset Adobe_typography_AI5 1.0 0\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI6_vars Adobe_Illustrator_AI6\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI5 1.0 0\n\
%%%%EndProlog\n");
    else			/* AI_VERSION_3 */
      sprintf (page_header->point, "\
%%%%BeginProlog\n\
%%%%IncludeResource: procset Adobe_packedarray 2.0 0\n\
Adobe_packedarray /initialize get exec\n\
%%%%IncludeResource: procset Adobe_cmykcolor 1.1 0\n\
%%%%IncludeResource: procset Adobe_cshow 1.1 0\n\
%%%%IncludeResource: procset Adobe_customcolor 1.0 0\n\
%%%%IncludeResource: procset Adobe_typography_AI3 1.0 1\n\
%%%%IncludeResource: procset Adobe_pattern_AI3 1.0 0\n\
%%%%IncludeResource: procset Adobe_Illustrator_AI3 1.0 1\n\
%%%%EndProlog\n");
    _update_buffer (page_header);
      
    /* beginning of Setup section */
    sprintf (page_header->point, "\
%%%%BeginSetup\n");
    _update_buffer (page_header);
    
    /* include fonts if any */
    if (fonts_used)
      {
	for (i = 0; i < PL_NUM_PS_FONTS; i++)
	  if (_plotter->data->page->ps_font_used[i])
	    {
	      sprintf (page_header->point, "\
%%%%IncludeFont: %s\n", 
		       _pl_g_ps_font_info[i].ps_name);
	      _update_buffer (page_header);
	    }
	for (i = 0; i < PL_NUM_PCL_FONTS; i++)
	  if (_plotter->data->page->pcl_font_used[i])
	    {
	      sprintf (page_header->point, "\
%%%%IncludeFont: %s\n", 
		       _pl_g_pcl_font_info[i].ps_name);
	      _update_buffer (page_header);
	    }
      }
    
    /* do setup of procsets */
    if (_plotter->ai_version == AI_VERSION_5)
      sprintf (page_header->point, "\
Adobe_level2_AI5 /initialize get exec\n\
Adobe_Illustrator_AI5_vars Adobe_Illustrator_AI5 Adobe_typography_AI5 /initialize get exec\n\
Adobe_ColorImage_AI6 /initialize get exec\n\
Adobe_Illustrator_AI5 /initialize get exec\n");
    else			/* AI_VERSION_3 */
      sprintf (page_header->point, "\
Adobe_cmykcolor /initialize get exec\n\
Adobe_cshow /initialize get exec\n\
Adobe_customcolor /initialize get exec\n\
Adobe_typography_AI3 /initialize get exec\n\
Adobe_pattern_AI3 /initialize get exec\n\
Adobe_Illustrator_AI3 /initialize get exec\n");
    _update_buffer (page_header);

    if (fonts_used)
      /* do whatever font reencodings are needed */
      {
	/* don't modify StandardEncoding */
	sprintf (page_header->point, "[\n\
TE\n");
	_update_buffer (page_header);
	
	/* reencode each used font */
	for (i = 0; i < PL_NUM_PS_FONTS; i++)
	  if (_plotter->data->page->ps_font_used[i])
	    {
	      const char *reencoding;
	      
	      if (_pl_g_ps_font_info[i].iso8859_1)	/* ISO-Latin-1 font */
		reencoding = "";
	      else if (strcmp (_pl_g_ps_font_info[i].ps_name, "ZapfDingbats")== 0)
		reencoding = _ai_zapf_dingbats_reencoding;
	      else if (strcmp (_pl_g_ps_font_info[i].ps_name, "Symbol") == 0)
		reencoding = _ai_symbol_reencoding;
	      else		/* don't know what to do */
		reencoding = "";
	      sprintf (page_header->point, "\
%%AI3_BeginEncoding: _%s %s\n\
[%s/_%s/%s 0 0 0 TZ\n\
%%AI3_EndEncoding AdobeType\n",
		       _pl_g_ps_font_info[i].ps_name, _pl_g_ps_font_info[i].ps_name,
                       reencoding,
		       _pl_g_ps_font_info[i].ps_name, _pl_g_ps_font_info[i].ps_name);
	      _update_buffer (page_header);
	    }
	for (i = 0; i < PL_NUM_PCL_FONTS; i++)
	  if (_plotter->data->page->pcl_font_used[i])
	    {
	      sprintf (page_header->point, "\
%%AI3_BeginEncoding: _%s %s\n\
[/_%s/%s 0 0 0 TZ\n\
%%AI3_EndEncoding TrueType\n",
		       _pl_g_pcl_font_info[i].ps_name, _pl_g_pcl_font_info[i].ps_name,
		       _pl_g_pcl_font_info[i].ps_name, _pl_g_pcl_font_info[i].ps_name);
	      _update_buffer (page_header);
	    }
      }
    /* end of Setup section */
    sprintf (page_header->point, "\
%%%%EndSetup\n");
    _update_buffer (page_header);
    
    if (_plotter->ai_version >= AI_VERSION_5)
      /* objects will belong to layer #1 (if layers are supported) */
      {
	/* 10 layer attributes, of which the first 6 are 0/1 flags, with
	   `1' meaning: visible / preview / enabled / printing layer /
	   dimmed / has multilayer masks.  The next attribute is a color
	   ID for the layer (0 = light blue), and the final three
	   attributes are intensities of R,G,B, on a 0..255 scale
	   (79,128,255 apparently being what AI normally uses) */
        sprintf (page_header->point, "\
%%AI5_BeginLayer\n\
1 1 1 1 0 0 0 79 128 255 Lb\n\
(Layer 1) Ln\n");
	_update_buffer (page_header);
      }

    /* place header in the plOutbuf of page */
    _plotter->data->page->header = page_header;
  }
  /* End of Page Header */
  
  /* Beginning of Page Trailer */
  {
    plOutbuf *page_trailer;

    page_trailer = _new_outbuf ();
    
    if (_plotter->ai_version >= AI_VERSION_5)
      /* after outputing objects, must end layer */
      {
	sprintf (page_trailer->point, "\
LB\n\
%%AI5_EndLayer--\n");
	_update_buffer (page_trailer);
      }
    
    sprintf (page_trailer->point, "\
%%%%PageTrailer\n\
gsave annotatepage grestore showpage\n");
    _update_buffer (page_trailer);
    
    /* trailer: terminate procsets */
    if (_plotter->ai_version == AI_VERSION_5)
      sprintf (page_trailer->point, "\
%%%%Trailer\n\
Adobe_Illustrator_AI5 /terminate get exec\n\
Adobe_ColorImage_AI6 /terminate get exec\n\
Adobe_typography_AI5 /terminate get exec\n\
Adobe_level2_AI5 /terminate get exec\n\
%%%%EOF\n");
    else			/* AI_VERSION_3 */
      sprintf (page_trailer->point, "\
%%%%Trailer\n\
Adobe_Illustrator_AI3 /terminate get exec\n\
Adobe_pattern_AI3 /terminate get exec\n\
Adobe_typography_AI3 /terminate get exec\n\
Adobe_customcolor /terminate get exec\n\
Adobe_cshow /terminate get exec\n\
Adobe_cmykcolor /terminate get exec\n\
Adobe_packedarray /terminate get exec\n\
%%%%EOF\n");
    _update_buffer (page_trailer);

    /* place header in the plOutbuf of the page */
    _plotter->data->page->trailer = page_trailer;    
  }
  /* End of Page Trailer */
  
  return true;
}
