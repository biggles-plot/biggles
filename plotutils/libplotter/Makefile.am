## Makefile.am for plotutils/libplotter.

lib_LTLIBRARIES = libplotter.la

libplotter_la_LDFLAGS = -version-info 4:4:2

INCLUDES = $(X_CFLAGS) -I$(srcdir)/../include -DLIBPLOT -DLIBPLOTTER

MISRC = mi_alloc.cc mi_api.cc mi_arc.cc mi_canvas.cc mi_fllarc.cc	\
mi_fllrct.cc mi_fplycon.cc mi_gc.cc mi_ply.cc mi_plycon.cc mi_plygen.cc	\
mi_plypnt.cc mi_plyutil.cc mi_spans.cc mi_widelin.cc mi_zerarc.cc	\
mi_zerolin.cc mi_version.cc

GSRC = g_affine.cc g_alabel.cc g_alab_her.cc g_attrib.cc g_arc.cc g_bez.cc  \
g_box.cc g_circ.cc g_clipper.cc g_closepl.cc g_colors.cc g_cntrlify.cc	    \
g_dash.cc g_dash2.cc g_defplot.cc g_defstate.cc g_ellipse.cc g_endpath.cc   \
g_error.cc g_erase.cc g_flushpl.cc g_font.cc g_fontdb.cc g_fontd2.cc	    \
g_havecap.cc g_her_glyph.cc g_integer.cc g_line.cc g_linewidth.cc g_mark.cc \
g_matrix.cc g_miscmi.cc g_move.cc g_openpl.cc g_outbuf.cc g_outfile.cc	    \
g_pagetype.cc g_param.cc g_param2.cc g_path.cc g_pentype.cc g_point.cc	    \
g_relative.cc g_range.cc g_retrieve.cc g_savestate.cc g_space.cc	    \
g_subpaths.cc g_vector.cc g_version.cc g_write.cc g_xmalloc.cc g_xstring.cc

BSRC = b_closepl.cc b_defplot.cc b_erase.cc b_openpl.cc b_path.cc	\
b_point.cc

MSRC = m_attribs.cc m_closepl.cc m_defplot.cc m_emit.cc m_erase.cc	\
m_mark.cc m_openpl.cc m_path.cc m_point.cc m_text.cc

RSRC = r_attribs.cc r_closepl.cc r_color.cc r_defplot.cc r_erase.cc	\
r_openpl.cc r_path.cc r_point.cc

TSRC = t_attribs.cc t_closepl.cc t_color.cc t_color2.cc t_defplot.cc	\
t_erase.cc t_openpl.cc t_path.cc t_point.cc t_tek_md.cc t_tek_mv.cc	\
t_tek_vec.cc

HSRC = h_attribs.cc h_closepl.cc h_color.cc h_defplot.cc h_erase.cc	\
h_font.cc h_openpl.cc h_path.cc h_point.cc h_text.cc

FSRC = f_closepl.cc f_color.cc f_color2.cc f_defplot.cc f_erase.cc	\
f_openpl.cc f_path.cc f_point.cc f_retrieve.cc f_text.cc

ISRC = i_closepl.cc i_color.cc i_defplot.cc i_erase.cc i_openpl.cc	\
i_path.cc i_point.cc i_rle.cc

NSRC = n_defplot.cc n_write.cc 

ZSRC = z_defplot.cc z_write.cc 

CSRC = c_attribs.cc c_closepl.cc c_color.cc c_defplot.cc c_emit.cc	\
c_erase.cc c_mark.cc c_openpl.cc c_path.cc c_point.cc c_text.cc

PSRC = p_closepl.cc p_color.cc p_color2.cc p_defplot.cc p_erase.cc	\
p_openpl.cc p_path.cc p_point.cc p_text.cc

ASRC = a_attribs.cc a_color.cc a_closepl.cc a_defplot.cc a_erase.cc	\
a_openpl.cc a_path.cc a_point.cc a_text.cc

SSRC = s_closepl.cc s_color.cc s_defplot.cc s_erase.cc s_openpl.cc	\
s_path.cc s_point.cc s_text.cc

XSRC = x_afftext.cc x_attribs.cc x_closepl.cc x_color.cc x_defplot.cc   \
x_erase.cc x_flushpl.cc x_openpl.cc x_path.cc x_point.cc x_retrieve.cc  \
x_savestate.cc x_text.cc

YSRC = y_closepl.cc y_defplot.cc y_erase.cc y_openpl.cc

if NO_PNG
if NO_X
ALLSRC = $(MISRC) $(GSRC) $(MSRC) $(BSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC)
else
ALLSRC = $(MISRC) $(GSRC) $(MSRC) $(BSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(XSRC) $(YSRC)
endif
else
if NO_X
ALLSRC = $(MISRC) $(GSRC) $(MSRC) $(BSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(ZSRC)
else
ALLSRC = $(MISRC) $(GSRC) $(MSRC) $(BSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(ZSRC) $(XSRC) $(YSRC)
endif
endif


MIHEADERS = xmi.h mi_api.h mi_arc.h mi_fllarc.h mi_fply.h mi_gc.h mi_line.h \
mi_ply.h mi_scanfill.h mi_spans.h mi_widelin.h mi_zerarc.h

ALLHEADERS = extern.h g_arc.h g_colorname.h g_control.h g_cntrlify.h \
g_her_metr.h \
g_jis.h g_pagetype.h h_roman8.h i_rle.h p_header.h x_afftext.h

noinst_HEADERS = $(ALLHEADERS) $(MIHEADERS)

CLEANFILES = apioldcc.cc $(ALLSRC) $(ALLHEADERS) $(MIHEADERS)

libplotter_la_SOURCES = apioldcc.cc $(ALLSRC)

if NO_PNG
if NO_X
libplotter_la_LIBADD =
else
libplotter_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS)
endif
else
if NO_X
libplotter_la_LIBADD = -lpng -lz
else
libplotter_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS) -lpng -lz
endif
endif

## execute the following command in ../libplot to produce the commands
## that follow
## (for i in ?_*.c; do echo "$i"c: "\$(top_srcdir)/libplot/$i"; echo "	rm -f $i"c" ; if \$(LN_S) \$(top_srcdir)/libplot/$i $i"c" ; then true ; else cp -p \$(top_srcdir)/libplot/$i $i"c" ; fi"; echo; done)

apioldcc.cc: $(top_srcdir)/libplot/apioldcc.c $(ALLHEADERS)
	@rm -f apioldcc.cc ; if $(LN_S) $(top_srcdir)/libplot/apioldcc.c apioldcc.cc ; then true ; else cp -p $(top_srcdir)/libplot/apioldcc.c apioldcc.cc ; fi

a_attribs.cc: $(top_srcdir)/libplot/a_attribs.c $(ALLHEADERS)
	@rm -f a_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/a_attribs.c a_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_attribs.c a_attribs.cc ; fi

a_closepl.cc: $(top_srcdir)/libplot/a_closepl.c $(ALLHEADERS)
	@rm -f a_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/a_closepl.c a_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_closepl.c a_closepl.cc ; fi

a_color.cc: $(top_srcdir)/libplot/a_color.c $(ALLHEADERS)
	@rm -f a_color.cc ; if $(LN_S) $(top_srcdir)/libplot/a_color.c a_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_color.c a_color.cc ; fi

a_defplot.cc: $(top_srcdir)/libplot/a_defplot.c $(ALLHEADERS)
	@rm -f a_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/a_defplot.c a_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_defplot.c a_defplot.cc ; fi

a_path.cc: $(top_srcdir)/libplot/a_path.c $(ALLHEADERS)
	@rm -f a_path.cc ; if $(LN_S) $(top_srcdir)/libplot/a_path.c a_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_path.c a_path.cc ; fi

a_erase.cc: $(top_srcdir)/libplot/a_erase.c $(ALLHEADERS)
	@rm -f a_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/a_erase.c a_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_erase.c a_erase.cc ; fi

a_openpl.cc: $(top_srcdir)/libplot/a_openpl.c $(ALLHEADERS)
	@rm -f a_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/a_openpl.c a_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_openpl.c a_openpl.cc ; fi

a_point.cc: $(top_srcdir)/libplot/a_point.c $(ALLHEADERS)
	@rm -f a_point.cc ; if $(LN_S) $(top_srcdir)/libplot/a_point.c a_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_point.c a_point.cc ; fi

a_text.cc: $(top_srcdir)/libplot/a_text.c $(ALLHEADERS)
	@rm -f a_text.cc ; if $(LN_S) $(top_srcdir)/libplot/a_text.c a_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/a_text.c a_text.cc ; fi

s_closepl.cc: $(top_srcdir)/libplot/s_closepl.c $(ALLHEADERS)
	@rm -f s_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/s_closepl.c s_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_closepl.c s_closepl.cc ; fi

s_color.cc: $(top_srcdir)/libplot/s_color.c $(ALLHEADERS)
	@rm -f s_color.cc ; if $(LN_S) $(top_srcdir)/libplot/s_color.c s_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_color.c s_color.cc ; fi

s_defplot.cc: $(top_srcdir)/libplot/s_defplot.c $(ALLHEADERS)
	@rm -f s_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/s_defplot.c s_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_defplot.c s_defplot.cc ; fi

s_path.cc: $(top_srcdir)/libplot/s_path.c $(ALLHEADERS)
	@rm -f s_path.cc ; if $(LN_S) $(top_srcdir)/libplot/s_path.c s_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_path.c s_path.cc ; fi

s_erase.cc: $(top_srcdir)/libplot/s_erase.c $(ALLHEADERS)
	@rm -f s_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/s_erase.c s_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_erase.c s_erase.cc ; fi

s_openpl.cc: $(top_srcdir)/libplot/s_openpl.c $(ALLHEADERS)
	@rm -f s_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/s_openpl.c s_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_openpl.c s_openpl.cc ; fi

s_point.cc: $(top_srcdir)/libplot/s_point.c $(ALLHEADERS)
	@rm -f s_point.cc ; if $(LN_S) $(top_srcdir)/libplot/s_point.c s_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_point.c s_point.cc ; fi

s_text.cc: $(top_srcdir)/libplot/s_text.c $(ALLHEADERS)
	@rm -f s_text.cc ; if $(LN_S) $(top_srcdir)/libplot/s_text.c s_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/s_text.c s_text.cc ; fi

f_closepl.cc: $(top_srcdir)/libplot/f_closepl.c $(ALLHEADERS)
	@rm -f f_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/f_closepl.c f_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_closepl.c f_closepl.cc ; fi

f_color.cc: $(top_srcdir)/libplot/f_color.c $(ALLHEADERS)
	@rm -f f_color.cc ; if $(LN_S) $(top_srcdir)/libplot/f_color.c f_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_color.c f_color.cc ; fi

f_color2.cc: $(top_srcdir)/libplot/f_color2.c $(ALLHEADERS)
	@rm -f f_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/f_color2.c f_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_color2.c f_color2.cc ; fi

f_defplot.cc: $(top_srcdir)/libplot/f_defplot.c $(ALLHEADERS)
	@rm -f f_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/f_defplot.c f_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_defplot.c f_defplot.cc ; fi

f_path.cc: $(top_srcdir)/libplot/f_path.c $(ALLHEADERS)
	@rm -f f_path.cc ; if $(LN_S) $(top_srcdir)/libplot/f_path.c f_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_path.c f_path.cc ; fi

f_erase.cc: $(top_srcdir)/libplot/f_erase.c $(ALLHEADERS)
	@rm -f f_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/f_erase.c f_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_erase.c f_erase.cc ; fi

f_openpl.cc: $(top_srcdir)/libplot/f_openpl.c $(ALLHEADERS)
	@rm -f f_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/f_openpl.c f_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_openpl.c f_openpl.cc ; fi

f_point.cc: $(top_srcdir)/libplot/f_point.c $(ALLHEADERS)
	@rm -f f_point.cc ; if $(LN_S) $(top_srcdir)/libplot/f_point.c f_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_point.c f_point.cc ; fi

f_retrieve.cc: $(top_srcdir)/libplot/f_retrieve.c $(ALLHEADERS)
	@rm -f f_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/f_retrieve.c f_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_retrieve.c f_retrieve.cc ; fi

f_text.cc: $(top_srcdir)/libplot/f_text.c $(ALLHEADERS)
	@rm -f f_text.cc ; if $(LN_S) $(top_srcdir)/libplot/f_text.c f_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/f_text.c f_text.cc ; fi

g_affine.cc: $(top_srcdir)/libplot/g_affine.c $(ALLHEADERS)
	@rm -f g_affine.cc ; if $(LN_S) $(top_srcdir)/libplot/g_affine.c g_affine.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_affine.c g_affine.cc ; fi

g_alab_her.cc: $(top_srcdir)/libplot/g_alab_her.c $(ALLHEADERS)
	@rm -f g_alab_her.cc ; if $(LN_S) $(top_srcdir)/libplot/g_alab_her.c g_alab_her.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_alab_her.c g_alab_her.cc ; fi

g_alabel.cc: $(top_srcdir)/libplot/g_alabel.c $(ALLHEADERS)
	@rm -f g_alabel.cc ; if $(LN_S) $(top_srcdir)/libplot/g_alabel.c g_alabel.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_alabel.c g_alabel.cc ; fi

g_arc.cc: $(top_srcdir)/libplot/g_arc.c $(ALLHEADERS)
	@rm -f g_arc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_arc.c g_arc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_arc.c g_arc.cc ; fi

g_attrib.cc: $(top_srcdir)/libplot/g_attrib.c $(ALLHEADERS)
	@rm -f g_attrib.cc ; if $(LN_S) $(top_srcdir)/libplot/g_attrib.c g_attrib.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_attrib.c g_attrib.cc ; fi

g_bez.cc: $(top_srcdir)/libplot/g_bez.c $(ALLHEADERS)
	@rm -f g_bez.cc ; if $(LN_S) $(top_srcdir)/libplot/g_bez.c g_bez.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_bez.c g_bez.cc ; fi

g_box.cc: $(top_srcdir)/libplot/g_box.c $(ALLHEADERS)
	@rm -f g_box.cc ; if $(LN_S) $(top_srcdir)/libplot/g_box.c g_box.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_box.c g_box.cc ; fi

g_circ.cc: $(top_srcdir)/libplot/g_circ.c $(ALLHEADERS)
	@rm -f g_circ.cc ; if $(LN_S) $(top_srcdir)/libplot/g_circ.c g_circ.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_circ.c g_circ.cc ; fi

g_clipper.cc: $(top_srcdir)/libplot/g_clipper.c $(ALLHEADERS)
	@rm -f g_clipper.cc ; if $(LN_S) $(top_srcdir)/libplot/g_clipper.c g_clipper.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_clipper.c g_clipper.cc ; fi

g_closepl.cc: $(top_srcdir)/libplot/g_closepl.c $(ALLHEADERS)
	@rm -f g_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_closepl.c g_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_closepl.c g_closepl.cc ; fi

g_cntrlify.cc: $(top_srcdir)/libplot/g_cntrlify.c $(ALLHEADERS)
	@rm -f g_cntrlify.cc ; if $(LN_S) $(top_srcdir)/libplot/g_cntrlify.c g_cntrlify.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_cntrlify.c g_cntrlify.cc ; fi

g_colors.cc: $(top_srcdir)/libplot/g_colors.c $(ALLHEADERS)
	@rm -f g_colors.cc ; if $(LN_S) $(top_srcdir)/libplot/g_colors.c g_colors.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_colors.c g_colors.cc ; fi

g_dash.cc: $(top_srcdir)/libplot/g_dash.c $(ALLHEADERS)
	@rm -f g_dash.cc ; if $(LN_S) $(top_srcdir)/libplot/g_dash.c g_dash.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_dash.c g_dash.cc ; fi

g_dash2.cc: $(top_srcdir)/libplot/g_dash2.c $(ALLHEADERS)
	@rm -f g_dash2.cc ; if $(LN_S) $(top_srcdir)/libplot/g_dash2.c g_dash2.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_dash2.c g_dash2.cc ; fi

g_defplot.cc: $(top_srcdir)/libplot/g_defplot.c $(ALLHEADERS)
	@rm -f g_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/g_defplot.c g_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_defplot.c g_defplot.cc ; fi

g_defstate.cc: $(top_srcdir)/libplot/g_defstate.c $(ALLHEADERS)
	@rm -f g_defstate.cc ; if $(LN_S) $(top_srcdir)/libplot/g_defstate.c g_defstate.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_defstate.c g_defstate.cc ; fi

g_ellipse.cc: $(top_srcdir)/libplot/g_ellipse.c $(ALLHEADERS)
	@rm -f g_ellipse.cc ; if $(LN_S) $(top_srcdir)/libplot/g_ellipse.c g_ellipse.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_ellipse.c g_ellipse.cc ; fi

g_endpath.cc: $(top_srcdir)/libplot/g_endpath.c $(ALLHEADERS)
	@rm -f g_endpath.cc ; if $(LN_S) $(top_srcdir)/libplot/g_endpath.c g_endpath.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_endpath.c g_endpath.cc ; fi

g_erase.cc: $(top_srcdir)/libplot/g_erase.c $(ALLHEADERS)
	@rm -f g_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/g_erase.c g_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_erase.c g_erase.cc ; fi

g_error.cc: $(top_srcdir)/libplot/g_error.c $(ALLHEADERS)
	@rm -f g_error.cc ; if $(LN_S) $(top_srcdir)/libplot/g_error.c g_error.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_error.c g_error.cc ; fi

g_flushpl.cc: $(top_srcdir)/libplot/g_flushpl.c $(ALLHEADERS)
	@rm -f g_flushpl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_flushpl.c g_flushpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_flushpl.c g_flushpl.cc ; fi

g_font.cc: $(top_srcdir)/libplot/g_font.c $(ALLHEADERS)
	@rm -f g_font.cc ; if $(LN_S) $(top_srcdir)/libplot/g_font.c g_font.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_font.c g_font.cc ; fi

g_fontd2.cc: $(top_srcdir)/libplot/g_fontd2.c $(ALLHEADERS)
	@rm -f g_fontd2.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontd2.c g_fontd2.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontd2.c g_fontd2.cc ; fi

g_fontdb.cc: $(top_srcdir)/libplot/g_fontdb.c $(ALLHEADERS)
	@rm -f g_fontdb.cc ; if $(LN_S) $(top_srcdir)/libplot/g_fontdb.c g_fontdb.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_fontdb.c g_fontdb.cc ; fi

g_havecap.cc: $(top_srcdir)/libplot/g_havecap.c $(ALLHEADERS)
	@rm -f g_havecap.cc ; if $(LN_S) $(top_srcdir)/libplot/g_havecap.c g_havecap.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_havecap.c g_havecap.cc ; fi

g_her_glyph.cc: $(top_srcdir)/libplot/g_her_glyph.c $(ALLHEADERS)
	@rm -f g_her_glyph.cc ; if $(LN_S) $(top_srcdir)/libplot/g_her_glyph.c g_her_glyph.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_her_glyph.c g_her_glyph.cc ; fi

g_integer.cc: $(top_srcdir)/libplot/g_integer.c $(ALLHEADERS)
	@rm -f g_integer.cc ; if $(LN_S) $(top_srcdir)/libplot/g_integer.c g_integer.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_integer.c g_integer.cc ; fi

g_line.cc: $(top_srcdir)/libplot/g_line.c $(ALLHEADERS)
	@rm -f g_line.cc ; if $(LN_S) $(top_srcdir)/libplot/g_line.c g_line.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_line.c g_line.cc ; fi

g_linewidth.cc: $(top_srcdir)/libplot/g_linewidth.c $(ALLHEADERS)
	@rm -f g_linewidth.cc ; if $(LN_S) $(top_srcdir)/libplot/g_linewidth.c g_linewidth.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_linewidth.c g_linewidth.cc ; fi

g_mark.cc: $(top_srcdir)/libplot/g_mark.c $(ALLHEADERS)
	@rm -f g_mark.cc ; if $(LN_S) $(top_srcdir)/libplot/g_mark.c g_mark.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_mark.c g_mark.cc ; fi

g_matrix.cc: $(top_srcdir)/libplot/g_matrix.c $(ALLHEADERS)
	@rm -f g_matrix.cc ; if $(LN_S) $(top_srcdir)/libplot/g_matrix.c g_matrix.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_matrix.c g_matrix.cc ; fi

g_miscmi.cc: $(top_srcdir)/libplot/g_miscmi.c $(ALLHEADERS)
	@rm -f g_miscmi.cc ; if $(LN_S) $(top_srcdir)/libplot/g_miscmi.c g_miscmi.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_miscmi.c g_miscmi.cc ; fi

g_move.cc: $(top_srcdir)/libplot/g_move.c $(ALLHEADERS)
	@rm -f g_move.cc ; if $(LN_S) $(top_srcdir)/libplot/g_move.c g_move.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_move.c g_move.cc ; fi

g_openpl.cc: $(top_srcdir)/libplot/g_openpl.c $(ALLHEADERS)
	@rm -f g_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/g_openpl.c g_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_openpl.c g_openpl.cc ; fi

g_outbuf.cc: $(top_srcdir)/libplot/g_outbuf.c $(ALLHEADERS)
	@rm -f g_outbuf.cc ; if $(LN_S) $(top_srcdir)/libplot/g_outbuf.c g_outbuf.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_outbuf.c g_outbuf.cc ; fi

g_outfile.cc: $(top_srcdir)/libplot/g_outfile.c $(ALLHEADERS)
	@rm -f g_outfile.cc ; if $(LN_S) $(top_srcdir)/libplot/g_outfile.c g_outfile.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_outfile.c g_outfile.cc ; fi

g_pagetype.cc: $(top_srcdir)/libplot/g_pagetype.c $(ALLHEADERS)
	@rm -f g_pagetype.cc ; if $(LN_S) $(top_srcdir)/libplot/g_pagetype.c g_pagetype.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_pagetype.c g_pagetype.cc ; fi

g_param.cc: $(top_srcdir)/libplot/g_param.c $(ALLHEADERS)
	@rm -f g_param.cc ; if $(LN_S) $(top_srcdir)/libplot/g_param.c g_param.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_param.c g_param.cc ; fi

g_param2.cc: $(top_srcdir)/libplot/g_param2.c $(ALLHEADERS)
	@rm -f g_param2.cc ; if $(LN_S) $(top_srcdir)/libplot/g_param2.c g_param2.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_param2.c g_param2.cc ; fi

g_path.cc: $(top_srcdir)/libplot/g_path.c $(ALLHEADERS)
	@rm -f g_path.cc ; if $(LN_S) $(top_srcdir)/libplot/g_path.c g_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_path.c g_path.cc ; fi

g_pentype.cc: $(top_srcdir)/libplot/g_pentype.c $(ALLHEADERS)
	@rm -f g_pentype.cc ; if $(LN_S) $(top_srcdir)/libplot/g_pentype.c g_pentype.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_pentype.c g_pentype.cc ; fi

g_point.cc: $(top_srcdir)/libplot/g_point.c $(ALLHEADERS)
	@rm -f g_point.cc ; if $(LN_S) $(top_srcdir)/libplot/g_point.c g_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_point.c g_point.cc ; fi

g_range.cc: $(top_srcdir)/libplot/g_range.c $(ALLHEADERS)
	@rm -f g_range.cc ; if $(LN_S) $(top_srcdir)/libplot/g_range.c g_range.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_range.c g_range.cc ; fi

g_relative.cc: $(top_srcdir)/libplot/g_relative.c $(ALLHEADERS)
	@rm -f g_relative.cc ; if $(LN_S) $(top_srcdir)/libplot/g_relative.c g_relative.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_relative.c g_relative.cc ; fi

g_retrieve.cc: $(top_srcdir)/libplot/g_retrieve.c $(ALLHEADERS)
	@rm -f g_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/g_retrieve.c g_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_retrieve.c g_retrieve.cc ; fi

g_savestate.cc: $(top_srcdir)/libplot/g_savestate.c $(ALLHEADERS)
	@rm -f g_savestate.cc ; if $(LN_S) $(top_srcdir)/libplot/g_savestate.c g_savestate.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_savestate.c g_savestate.cc ; fi

g_space.cc: $(top_srcdir)/libplot/g_space.c $(ALLHEADERS)
	@rm -f g_space.cc ; if $(LN_S) $(top_srcdir)/libplot/g_space.c g_space.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_space.c g_space.cc ; fi

g_subpaths.cc: $(top_srcdir)/libplot/g_subpaths.c $(ALLHEADERS)
	@rm -f g_subpaths.cc ; if $(LN_S) $(top_srcdir)/libplot/g_subpaths.c g_subpaths.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_subpaths.c g_subpaths.cc ; fi

g_vector.cc: $(top_srcdir)/libplot/g_vector.c $(ALLHEADERS)
	@rm -f g_vector.cc ; if $(LN_S) $(top_srcdir)/libplot/g_vector.c g_vector.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_vector.c g_vector.cc ; fi

g_version.cc: $(top_srcdir)/libplot/g_version.c $(ALLHEADERS)
	@rm -f g_version.cc ; if $(LN_S) $(top_srcdir)/libplot/g_version.c g_version.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_version.c g_version.cc ; fi

g_write.cc: $(top_srcdir)/libplot/g_write.c $(ALLHEADERS)
	@rm -f g_write.cc ; if $(LN_S) $(top_srcdir)/libplot/g_write.c g_write.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_write.c g_write.cc ; fi

g_xmalloc.cc: $(top_srcdir)/libplot/g_xmalloc.c $(ALLHEADERS)
	@rm -f g_xmalloc.cc ; if $(LN_S) $(top_srcdir)/libplot/g_xmalloc.c g_xmalloc.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_xmalloc.c g_xmalloc.cc ; fi

g_xstring.cc: $(top_srcdir)/libplot/g_xstring.c $(ALLHEADERS)
	@rm -f g_xstring.cc ; if $(LN_S) $(top_srcdir)/libplot/g_xstring.c g_xstring.cc ; then true ; else cp -p $(top_srcdir)/libplot/g_xstring.c g_xstring.cc ; fi

h_attribs.cc: $(top_srcdir)/libplot/h_attribs.c $(ALLHEADERS)
	@rm -f h_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/h_attribs.c h_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_attribs.c h_attribs.cc ; fi

h_closepl.cc: $(top_srcdir)/libplot/h_closepl.c $(ALLHEADERS)
	@rm -f h_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/h_closepl.c h_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_closepl.c h_closepl.cc ; fi

h_color.cc: $(top_srcdir)/libplot/h_color.c $(ALLHEADERS)
	@rm -f h_color.cc ; if $(LN_S) $(top_srcdir)/libplot/h_color.c h_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_color.c h_color.cc ; fi

h_defplot.cc: $(top_srcdir)/libplot/h_defplot.c $(ALLHEADERS)
	@rm -f h_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/h_defplot.c h_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_defplot.c h_defplot.cc ; fi

h_erase.cc: $(top_srcdir)/libplot/h_erase.c $(ALLHEADERS)
	@rm -f h_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/h_erase.c h_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_erase.c h_erase.cc ; fi

h_font.cc: $(top_srcdir)/libplot/h_font.c $(ALLHEADERS)
	@rm -f h_font.cc ; if $(LN_S) $(top_srcdir)/libplot/h_font.c h_font.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_font.c h_font.cc ; fi

h_openpl.cc: $(top_srcdir)/libplot/h_openpl.c $(ALLHEADERS)
	@rm -f h_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/h_openpl.c h_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_openpl.c h_openpl.cc ; fi

h_path.cc: $(top_srcdir)/libplot/h_path.c $(ALLHEADERS)
	@rm -f h_path.cc ; if $(LN_S) $(top_srcdir)/libplot/h_path.c h_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_path.c h_path.cc ; fi

h_point.cc: $(top_srcdir)/libplot/h_point.c $(ALLHEADERS)
	@rm -f h_point.cc ; if $(LN_S) $(top_srcdir)/libplot/h_point.c h_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_point.c h_point.cc ; fi

h_text.cc: $(top_srcdir)/libplot/h_text.c $(ALLHEADERS)
	@rm -f h_text.cc ; if $(LN_S) $(top_srcdir)/libplot/h_text.c h_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/h_text.c h_text.cc ; fi

b_closepl.cc: $(top_srcdir)/libplot/b_closepl.c $(ALLHEADERS)
	@rm -f b_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/b_closepl.c b_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_closepl.c b_closepl.cc ; fi

b_defplot.cc: $(top_srcdir)/libplot/b_defplot.c $(ALLHEADERS)
	@rm -f b_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/b_defplot.c b_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_defplot.c b_defplot.cc ; fi

b_path.cc: $(top_srcdir)/libplot/b_path.c $(ALLHEADERS)
	@rm -f b_path.cc ; if $(LN_S) $(top_srcdir)/libplot/b_path.c b_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_path.c b_path.cc ; fi

b_erase.cc: $(top_srcdir)/libplot/b_erase.c $(ALLHEADERS)
	@rm -f b_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/b_erase.c b_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_erase.c b_erase.cc ; fi

b_openpl.cc: $(top_srcdir)/libplot/b_openpl.c $(ALLHEADERS)
	@rm -f b_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/b_openpl.c b_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_openpl.c b_openpl.cc ; fi

b_point.cc: $(top_srcdir)/libplot/b_point.c $(ALLHEADERS)
	@rm -f b_point.cc ; if $(LN_S) $(top_srcdir)/libplot/b_point.c b_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/b_point.c b_point.cc ; fi

n_defplot.cc: $(top_srcdir)/libplot/n_defplot.c $(ALLHEADERS)
	@rm -f n_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/n_defplot.c n_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_defplot.c n_defplot.cc ; fi

n_write.cc: $(top_srcdir)/libplot/n_write.c $(ALLHEADERS)
	@rm -f n_write.cc ; if $(LN_S) $(top_srcdir)/libplot/n_write.c n_write.cc ; then true ; else cp -p $(top_srcdir)/libplot/n_write.c n_write.cc ; fi

z_defplot.cc: $(top_srcdir)/libplot/z_defplot.c $(ALLHEADERS)
	@rm -f z_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/z_defplot.c z_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/z_defplot.c z_defplot.cc ; fi

z_write.cc: $(top_srcdir)/libplot/z_write.c $(ALLHEADERS)
	@rm -f z_write.cc ; if $(LN_S) $(top_srcdir)/libplot/z_write.c z_write.cc ; then true ; else cp -p $(top_srcdir)/libplot/z_write.c z_write.cc ; fi

i_closepl.cc: $(top_srcdir)/libplot/i_closepl.c $(ALLHEADERS)
	@rm -f i_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/i_closepl.c i_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_closepl.c i_closepl.cc ; fi

i_color.cc: $(top_srcdir)/libplot/i_color.c $(ALLHEADERS)
	@rm -f i_color.cc ; if $(LN_S) $(top_srcdir)/libplot/i_color.c i_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_color.c i_color.cc ; fi

i_defplot.cc: $(top_srcdir)/libplot/i_defplot.c $(ALLHEADERS)
	@rm -f i_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/i_defplot.c i_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_defplot.c i_defplot.cc ; fi

i_path.cc: $(top_srcdir)/libplot/i_path.c $(ALLHEADERS)
	@rm -f i_path.cc ; if $(LN_S) $(top_srcdir)/libplot/i_path.c i_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_path.c i_path.cc ; fi

i_erase.cc: $(top_srcdir)/libplot/i_erase.c $(ALLHEADERS)
	@rm -f i_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/i_erase.c i_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_erase.c i_erase.cc ; fi

i_openpl.cc: $(top_srcdir)/libplot/i_openpl.c $(ALLHEADERS)
	@rm -f i_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/i_openpl.c i_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_openpl.c i_openpl.cc ; fi

i_point.cc: $(top_srcdir)/libplot/i_point.c $(ALLHEADERS)
	@rm -f i_point.cc ; if $(LN_S) $(top_srcdir)/libplot/i_point.c i_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_point.c i_point.cc ; fi

i_rle.cc: $(top_srcdir)/libplot/i_rle.c $(ALLHEADERS)
	@rm -f i_rle.cc ; if $(LN_S) $(top_srcdir)/libplot/i_rle.c i_rle.cc ; then true ; else cp -p $(top_srcdir)/libplot/i_rle.c i_rle.cc ; fi

m_attribs.cc: $(top_srcdir)/libplot/m_attribs.c $(ALLHEADERS)
	@rm -f m_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/m_attribs.c m_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_attribs.c m_attribs.cc ; fi

m_closepl.cc: $(top_srcdir)/libplot/m_closepl.c $(ALLHEADERS)
	@rm -f m_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/m_closepl.c m_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_closepl.c m_closepl.cc ; fi

m_defplot.cc: $(top_srcdir)/libplot/m_defplot.c $(ALLHEADERS)
	@rm -f m_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/m_defplot.c m_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_defplot.c m_defplot.cc ; fi

m_mark.cc: $(top_srcdir)/libplot/m_mark.c $(ALLHEADERS)
	@rm -f m_mark.cc ; if $(LN_S) $(top_srcdir)/libplot/m_mark.c m_mark.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_mark.c m_mark.cc ; fi

m_path.cc: $(top_srcdir)/libplot/m_path.c $(ALLHEADERS)
	@rm -f m_path.cc ; if $(LN_S) $(top_srcdir)/libplot/m_path.c m_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_path.c m_path.cc ; fi

m_emit.cc: $(top_srcdir)/libplot/m_emit.c $(ALLHEADERS)
	@rm -f m_emit.cc ; if $(LN_S) $(top_srcdir)/libplot/m_emit.c m_emit.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_emit.c m_emit.cc ; fi

m_erase.cc: $(top_srcdir)/libplot/m_erase.c $(ALLHEADERS)
	@rm -f m_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/m_erase.c m_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_erase.c m_erase.cc ; fi

m_openpl.cc: $(top_srcdir)/libplot/m_openpl.c $(ALLHEADERS)
	@rm -f m_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/m_openpl.c m_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_openpl.c m_openpl.cc ; fi

m_point.cc: $(top_srcdir)/libplot/m_point.c $(ALLHEADERS)
	@rm -f m_point.cc ; if $(LN_S) $(top_srcdir)/libplot/m_point.c m_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_point.c m_point.cc ; fi

m_text.cc: $(top_srcdir)/libplot/m_text.c $(ALLHEADERS)
	@rm -f m_text.cc ; if $(LN_S) $(top_srcdir)/libplot/m_text.c m_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/m_text.c m_text.cc ; fi

c_attribs.cc: $(top_srcdir)/libplot/c_attribs.c $(ALLHEADERS)
	@rm -f c_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/c_attribs.c c_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_attribs.c c_attribs.cc ; fi

c_closepl.cc: $(top_srcdir)/libplot/c_closepl.c $(ALLHEADERS)
	@rm -f c_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/c_closepl.c c_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_closepl.c c_closepl.cc ; fi

c_color.cc: $(top_srcdir)/libplot/c_color.c $(ALLHEADERS)
	@rm -f c_color.cc ; if $(LN_S) $(top_srcdir)/libplot/c_color.c c_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_color.c c_color.cc ; fi

c_defplot.cc: $(top_srcdir)/libplot/c_defplot.c $(ALLHEADERS)
	@rm -f c_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/c_defplot.c c_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_defplot.c c_defplot.cc ; fi

c_emit.cc: $(top_srcdir)/libplot/c_emit.c $(ALLHEADERS)
	@rm -f c_emit.cc ; if $(LN_S) $(top_srcdir)/libplot/c_emit.c c_emit.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_emit.c c_emit.cc ; fi

c_path.cc: $(top_srcdir)/libplot/c_path.c $(ALLHEADERS)
	@rm -f c_path.cc ; if $(LN_S) $(top_srcdir)/libplot/c_path.c c_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_path.c c_path.cc ; fi

c_erase.cc: $(top_srcdir)/libplot/c_erase.c $(ALLHEADERS)
	@rm -f c_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/c_erase.c c_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_erase.c c_erase.cc ; fi

c_mark.cc: $(top_srcdir)/libplot/c_mark.c $(ALLHEADERS)
	@rm -f c_mark.cc ; if $(LN_S) $(top_srcdir)/libplot/c_mark.c c_mark.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_mark.c c_mark.cc ; fi

c_openpl.cc: $(top_srcdir)/libplot/c_openpl.c $(ALLHEADERS)
	@rm -f c_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/c_openpl.c c_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_openpl.c c_openpl.cc ; fi

c_point.cc: $(top_srcdir)/libplot/c_point.c $(ALLHEADERS)
	@rm -f c_point.cc ; if $(LN_S) $(top_srcdir)/libplot/c_point.c c_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_point.c c_point.cc ; fi

c_text.cc: $(top_srcdir)/libplot/c_text.c $(ALLHEADERS)
	@rm -f c_text.cc ; if $(LN_S) $(top_srcdir)/libplot/c_text.c c_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/c_text.c c_text.cc ; fi

p_closepl.cc: $(top_srcdir)/libplot/p_closepl.c $(ALLHEADERS)
	@rm -f p_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/p_closepl.c p_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_closepl.c p_closepl.cc ; fi

p_color.cc: $(top_srcdir)/libplot/p_color.c $(ALLHEADERS)
	@rm -f p_color.cc ; if $(LN_S) $(top_srcdir)/libplot/p_color.c p_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_color.c p_color.cc ; fi

p_color2.cc: $(top_srcdir)/libplot/p_color2.c $(ALLHEADERS)
	@rm -f p_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/p_color2.c p_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_color2.c p_color2.cc ; fi

p_defplot.cc: $(top_srcdir)/libplot/p_defplot.c $(ALLHEADERS)
	@rm -f p_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/p_defplot.c p_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_defplot.c p_defplot.cc ; fi

p_path.cc: $(top_srcdir)/libplot/p_path.c $(ALLHEADERS)
	@rm -f p_path.cc ; if $(LN_S) $(top_srcdir)/libplot/p_path.c p_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_path.c p_path.cc ; fi

p_erase.cc: $(top_srcdir)/libplot/p_erase.c $(ALLHEADERS)
	@rm -f p_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/p_erase.c p_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_erase.c p_erase.cc ; fi

p_openpl.cc: $(top_srcdir)/libplot/p_openpl.c $(ALLHEADERS)
	@rm -f p_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/p_openpl.c p_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_openpl.c p_openpl.cc ; fi

p_point.cc: $(top_srcdir)/libplot/p_point.c $(ALLHEADERS)
	@rm -f p_point.cc ; if $(LN_S) $(top_srcdir)/libplot/p_point.c p_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_point.c p_point.cc ; fi

p_text.cc: $(top_srcdir)/libplot/p_text.c $(ALLHEADERS)
	@rm -f p_text.cc ; if $(LN_S) $(top_srcdir)/libplot/p_text.c p_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/p_text.c p_text.cc ; fi

r_attribs.cc: $(top_srcdir)/libplot/r_attribs.c $(ALLHEADERS)
	@rm -f r_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/r_attribs.c r_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_attribs.c r_attribs.cc ; fi

r_closepl.cc: $(top_srcdir)/libplot/r_closepl.c $(ALLHEADERS)
	@rm -f r_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/r_closepl.c r_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_closepl.c r_closepl.cc ; fi

r_color.cc: $(top_srcdir)/libplot/r_color.c $(ALLHEADERS)
	@rm -f r_color.cc ; if $(LN_S) $(top_srcdir)/libplot/r_color.c r_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_color.c r_color.cc ; fi

r_defplot.cc: $(top_srcdir)/libplot/r_defplot.c $(ALLHEADERS)
	@rm -f r_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/r_defplot.c r_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_defplot.c r_defplot.cc ; fi

r_path.cc: $(top_srcdir)/libplot/r_path.c $(ALLHEADERS)
	@rm -f r_path.cc ; if $(LN_S) $(top_srcdir)/libplot/r_path.c r_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_path.c r_path.cc ; fi

r_erase.cc: $(top_srcdir)/libplot/r_erase.c $(ALLHEADERS)
	@rm -f r_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/r_erase.c r_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_erase.c r_erase.cc ; fi

r_openpl.cc: $(top_srcdir)/libplot/r_openpl.c $(ALLHEADERS)
	@rm -f r_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/r_openpl.c r_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_openpl.c r_openpl.cc ; fi

r_point.cc: $(top_srcdir)/libplot/r_point.c $(ALLHEADERS)
	@rm -f r_point.cc ; if $(LN_S) $(top_srcdir)/libplot/r_point.c r_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/r_point.c r_point.cc ; fi

t_attribs.cc: $(top_srcdir)/libplot/t_attribs.c $(ALLHEADERS)
	@rm -f t_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/t_attribs.c t_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_attribs.c t_attribs.cc ; fi

t_closepl.cc: $(top_srcdir)/libplot/t_closepl.c $(ALLHEADERS)
	@rm -f t_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/t_closepl.c t_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_closepl.c t_closepl.cc ; fi

t_color.cc: $(top_srcdir)/libplot/t_color.c $(ALLHEADERS)
	@rm -f t_color.cc ; if $(LN_S) $(top_srcdir)/libplot/t_color.c t_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_color.c t_color.cc ; fi

t_color2.cc: $(top_srcdir)/libplot/t_color2.c $(ALLHEADERS)
	@rm -f t_color2.cc ; if $(LN_S) $(top_srcdir)/libplot/t_color2.c t_color2.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_color2.c t_color2.cc ; fi

t_defplot.cc: $(top_srcdir)/libplot/t_defplot.c $(ALLHEADERS)
	@rm -f t_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/t_defplot.c t_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_defplot.c t_defplot.cc ; fi

t_path.cc: $(top_srcdir)/libplot/t_path.c $(ALLHEADERS)
	@rm -f t_path.cc ; if $(LN_S) $(top_srcdir)/libplot/t_path.c t_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_path.c t_path.cc ; fi

t_erase.cc: $(top_srcdir)/libplot/t_erase.c $(ALLHEADERS)
	@rm -f t_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/t_erase.c t_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_erase.c t_erase.cc ; fi

t_openpl.cc: $(top_srcdir)/libplot/t_openpl.c $(ALLHEADERS)
	@rm -f t_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/t_openpl.c t_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_openpl.c t_openpl.cc ; fi

t_point.cc: $(top_srcdir)/libplot/t_point.c $(ALLHEADERS)
	@rm -f t_point.cc ; if $(LN_S) $(top_srcdir)/libplot/t_point.c t_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_point.c t_point.cc ; fi

t_tek_md.cc: $(top_srcdir)/libplot/t_tek_md.c $(ALLHEADERS)
	@rm -f t_tek_md.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_md.c t_tek_md.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_md.c t_tek_md.cc ; fi

t_tek_mv.cc: $(top_srcdir)/libplot/t_tek_mv.c $(ALLHEADERS)
	@rm -f t_tek_mv.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_mv.c t_tek_mv.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_mv.c t_tek_mv.cc ; fi

t_tek_vec.cc: $(top_srcdir)/libplot/t_tek_vec.c $(ALLHEADERS)
	@rm -f t_tek_vec.cc ; if $(LN_S) $(top_srcdir)/libplot/t_tek_vec.c t_tek_vec.cc ; then true ; else cp -p $(top_srcdir)/libplot/t_tek_vec.c t_tek_vec.cc ; fi

x_afftext.cc: $(top_srcdir)/libplot/x_afftext.c $(ALLHEADERS)
	@rm -f x_afftext.cc ; if $(LN_S) $(top_srcdir)/libplot/x_afftext.c x_afftext.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_afftext.c x_afftext.cc ; fi

x_attribs.cc: $(top_srcdir)/libplot/x_attribs.c $(ALLHEADERS)
	@rm -f x_attribs.cc ; if $(LN_S) $(top_srcdir)/libplot/x_attribs.c x_attribs.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_attribs.c x_attribs.cc ; fi

x_closepl.cc: $(top_srcdir)/libplot/x_closepl.c $(ALLHEADERS)
	@rm -f x_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_closepl.c x_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_closepl.c x_closepl.cc ; fi

x_color.cc: $(top_srcdir)/libplot/x_color.c $(ALLHEADERS)
	@rm -f x_color.cc ; if $(LN_S) $(top_srcdir)/libplot/x_color.c x_color.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_color.c x_color.cc ; fi

x_defplot.cc: $(top_srcdir)/libplot/x_defplot.c $(ALLHEADERS)
	@rm -f x_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/x_defplot.c x_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_defplot.c x_defplot.cc ; fi

x_path.cc: $(top_srcdir)/libplot/x_path.c $(ALLHEADERS)
	@rm -f x_path.cc ; if $(LN_S) $(top_srcdir)/libplot/x_path.c x_path.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_path.c x_path.cc ; fi

x_erase.cc: $(top_srcdir)/libplot/x_erase.c $(ALLHEADERS)
	@rm -f x_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/x_erase.c x_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_erase.c x_erase.cc ; fi

x_flushpl.cc: $(top_srcdir)/libplot/x_flushpl.c $(ALLHEADERS)
	@rm -f x_flushpl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_flushpl.c x_flushpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_flushpl.c x_flushpl.cc ; fi

x_openpl.cc: $(top_srcdir)/libplot/x_openpl.c $(ALLHEADERS)
	@rm -f x_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/x_openpl.c x_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_openpl.c x_openpl.cc ; fi

x_point.cc: $(top_srcdir)/libplot/x_point.c $(ALLHEADERS)
	@rm -f x_point.cc ; if $(LN_S) $(top_srcdir)/libplot/x_point.c x_point.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_point.c x_point.cc ; fi

x_retrieve.cc: $(top_srcdir)/libplot/x_retrieve.c $(ALLHEADERS)
	@rm -f x_retrieve.cc ; if $(LN_S) $(top_srcdir)/libplot/x_retrieve.c x_retrieve.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_retrieve.c x_retrieve.cc ; fi

x_savestate.cc: $(top_srcdir)/libplot/x_savestate.c $(ALLHEADERS)
	@rm -f x_savestate.cc ; if $(LN_S) $(top_srcdir)/libplot/x_savestate.c x_savestate.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_savestate.c x_savestate.cc ; fi

x_text.cc: $(top_srcdir)/libplot/x_text.c $(ALLHEADERS)
	@rm -f x_text.cc ; if $(LN_S) $(top_srcdir)/libplot/x_text.c x_text.cc ; then true ; else cp -p $(top_srcdir)/libplot/x_text.c x_text.cc ; fi

y_closepl.cc: $(top_srcdir)/libplot/y_closepl.c $(ALLHEADERS)
	@rm -f y_closepl.cc ; if $(LN_S) $(top_srcdir)/libplot/y_closepl.c y_closepl.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_closepl.c y_closepl.cc ; fi

y_defplot.cc: $(top_srcdir)/libplot/y_defplot.c $(ALLHEADERS)
	@rm -f y_defplot.cc ; if $(LN_S) $(top_srcdir)/libplot/y_defplot.c y_defplot.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_defplot.c y_defplot.cc ; fi

y_erase.cc: $(top_srcdir)/libplot/y_erase.c $(ALLHEADERS)
	@rm -f y_erase.cc ; if $(LN_S) $(top_srcdir)/libplot/y_erase.c y_erase.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_erase.c y_erase.cc ; fi

y_openpl.cc: $(top_srcdir)/libplot/y_openpl.c $(ALLHEADERS)
	@rm -f y_openpl.cc ; if $(LN_S) $(top_srcdir)/libplot/y_openpl.c y_openpl.cc ; then true ; else cp -p $(top_srcdir)/libplot/y_openpl.c y_openpl.cc ; fi


## execute the following command in ../libplot to produce the commands
## that follow
## (for i in extern.h ?_*.h; do echo "$i": "\$(top_srcdir)/libplot/$i"; echo "	@rm -f $i"" ; if \$(LN_S) \$(top_srcdir)/libplot/$i $i"" ; then true ; else cp -p \$(top_srcdir)/libplot/$i $i"" ; fi"; echo; done)

extern.h: $(top_srcdir)/libplot/extern.h
	@rm -f extern.h ; if $(LN_S) $(top_srcdir)/libplot/extern.h extern.h ; then true ; else cp -p $(top_srcdir)/libplot/extern.h extern.h ; fi

g_arc.h: $(top_srcdir)/libplot/g_arc.h
	@rm -f g_arc.h ; if $(LN_S) $(top_srcdir)/libplot/g_arc.h g_arc.h ; then true ; else cp -p $(top_srcdir)/libplot/g_arc.h g_arc.h ; fi

g_cntrlify.h: $(top_srcdir)/libplot/g_cntrlify.h
	@rm -f g_cntrlify.h ; if $(LN_S) $(top_srcdir)/libplot/g_cntrlify.h g_cntrlify.h ; then true ; else cp -p $(top_srcdir)/libplot/g_cntrlify.h g_cntrlify.h ; fi

g_colorname.h: $(top_srcdir)/libplot/g_colorname.h
	@rm -f g_colorname.h ; if $(LN_S) $(top_srcdir)/libplot/g_colorname.h g_colorname.h ; then true ; else cp -p $(top_srcdir)/libplot/g_colorname.h g_colorname.h ; fi

g_control.h: $(top_srcdir)/libplot/g_control.h
	@rm -f g_control.h ; if $(LN_S) $(top_srcdir)/libplot/g_control.h g_control.h ; then true ; else cp -p $(top_srcdir)/libplot/g_control.h g_control.h ; fi

g_her_metr.h: $(top_srcdir)/libplot/g_her_metr.h
	@rm -f g_her_metr.h ; if $(LN_S) $(top_srcdir)/libplot/g_her_metr.h g_her_metr.h ; then true ; else cp -p $(top_srcdir)/libplot/g_her_metr.h g_her_metr.h ; fi

g_jis.h: $(top_srcdir)/libplot/g_jis.h
	@rm -f g_jis.h ; if $(LN_S) $(top_srcdir)/libplot/g_jis.h g_jis.h ; then true ; else cp -p $(top_srcdir)/libplot/g_jis.h g_jis.h ; fi

g_pagetype.h: $(top_srcdir)/libplot/g_pagetype.h
	@rm -f g_pagetype.h ; if $(LN_S) $(top_srcdir)/libplot/g_pagetype.h g_pagetype.h ; then true ; else cp -p $(top_srcdir)/libplot/g_pagetype.h g_pagetype.h ; fi

h_roman8.h: $(top_srcdir)/libplot/h_roman8.h
	@rm -f h_roman8.h ; if $(LN_S) $(top_srcdir)/libplot/h_roman8.h h_roman8.h ; then true ; else cp -p $(top_srcdir)/libplot/h_roman8.h h_roman8.h ; fi

i_rle.h: $(top_srcdir)/libplot/i_rle.h
	@rm -f i_rle.h ; if $(LN_S) $(top_srcdir)/libplot/i_rle.h i_rle.h ; then true ; else cp -p $(top_srcdir)/libplot/i_rle.h i_rle.h ; fi

p_header.h: $(top_srcdir)/libplot/p_header.h
	@rm -f p_header.h ; if $(LN_S) $(top_srcdir)/libplot/p_header.h p_header.h ; then true ; else cp -p $(top_srcdir)/libplot/p_header.h p_header.h ; fi

x_afftext.h: $(top_srcdir)/libplot/x_afftext.h
	@rm -f x_afftext.h ; if $(LN_S) $(top_srcdir)/libplot/x_afftext.h x_afftext.h ; then true ; else cp -p $(top_srcdir)/libplot/x_afftext.h x_afftext.h ; fi

## Similarly in ../libxmi, do
##(for i in mi_*.c; do echo "$i"c: "\$(top_srcdir)/libxmi/$i"; echo "	rm -f $i"c" ; if \$(LN_S) \$(top_srcdir)/libxmi/$i $i"c" ; then true ; else cp -p \$(top_srcdir)/libxmi/$i $i"c" ; fi"; echo; done)

mi_alloc.cc: $(top_srcdir)/libxmi/mi_alloc.c $(MIHEADERS) extern.h
	@rm -f mi_alloc.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_alloc.c mi_alloc.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_alloc.c mi_alloc.cc ; fi

mi_api.cc: $(top_srcdir)/libxmi/mi_alloc.c $(MIHEADERS) extern.h
	@rm -f mi_api.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_api.c mi_api.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_api.c mi_api.cc ; fi

mi_arc.cc: $(top_srcdir)/libxmi/mi_arc.c $(MIHEADERS) extern.h
	@rm -f mi_arc.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_arc.c mi_arc.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_arc.c mi_arc.cc ; fi

mi_canvas.cc: $(top_srcdir)/libxmi/mi_canvas.c $(MIHEADERS) extern.h
	@rm -f mi_canvas.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_canvas.c mi_canvas.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_canvas.c mi_canvas.cc ; fi

mi_fllarc.cc: $(top_srcdir)/libxmi/mi_fllarc.c $(MIHEADERS) extern.h
	@rm -f mi_fllarc.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_fllarc.c mi_fllarc.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fllarc.c mi_fllarc.cc ; fi

mi_fllrct.cc: $(top_srcdir)/libxmi/mi_fllrct.c $(MIHEADERS) extern.h
	@rm -f mi_fllrct.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_fllrct.c mi_fllrct.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fllrct.c mi_fllrct.cc ; fi

mi_fplycon.cc: $(top_srcdir)/libxmi/mi_fplycon.c $(MIHEADERS) extern.h
	@rm -f mi_fplycon.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_fplycon.c mi_fplycon.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fplycon.c mi_fplycon.cc ; fi

mi_gc.cc: $(top_srcdir)/libxmi/mi_gc.c $(MIHEADERS) extern.h
	@rm -f mi_gc.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_gc.c mi_gc.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_gc.c mi_gc.cc ; fi

mi_ply.cc: $(top_srcdir)/libxmi/mi_ply.c $(MIHEADERS) extern.h
	@rm -f mi_ply.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_ply.c mi_ply.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_ply.c mi_ply.cc ; fi

mi_plycon.cc: $(top_srcdir)/libxmi/mi_plycon.c $(MIHEADERS) extern.h
	@rm -f mi_plycon.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_plycon.c mi_plycon.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plycon.c mi_plycon.cc ; fi

mi_plygen.cc: $(top_srcdir)/libxmi/mi_plygen.c $(MIHEADERS) extern.h
	@rm -f mi_plygen.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_plygen.c mi_plygen.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plygen.c mi_plygen.cc ; fi

mi_plypnt.cc: $(top_srcdir)/libxmi/mi_plypnt.c $(MIHEADERS) extern.h
	@rm -f mi_plypnt.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_plypnt.c mi_plypnt.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plypnt.c mi_plypnt.cc ; fi

mi_plyutil.cc: $(top_srcdir)/libxmi/mi_plyutil.c $(MIHEADERS) extern.h
	@rm -f mi_plyutil.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_plyutil.c mi_plyutil.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plyutil.c mi_plyutil.cc ; fi

mi_spans.cc: $(top_srcdir)/libxmi/mi_spans.c $(MIHEADERS) extern.h
	@rm -f mi_spans.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_spans.c mi_spans.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_spans.c mi_spans.cc ; fi

mi_widelin.cc: $(top_srcdir)/libxmi/mi_widelin.c $(MIHEADERS) extern.h
	@rm -f mi_widelin.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_widelin.c mi_widelin.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_widelin.c mi_widelin.cc ; fi

mi_zerarc.cc: $(top_srcdir)/libxmi/mi_zerarc.c $(MIHEADERS) extern.h
	@rm -f mi_zerarc.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_zerarc.c mi_zerarc.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_zerarc.c mi_zerarc.cc ; fi

mi_zerolin.cc: $(top_srcdir)/libxmi/mi_zerolin.c $(MIHEADERS) extern.h
	@rm -f mi_zerolin.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_zerolin.c mi_zerolin.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_zerolin.c mi_zerolin.cc ; fi

mi_version.cc: $(top_srcdir)/libxmi/mi_version.c $(MIHEADERS) extern.h
	@rm -f mi_version.cc ; if $(LN_S) $(top_srcdir)/libxmi/mi_version.c mi_version.cc ; then true ; else cp -p $(top_srcdir)/libxmi/mi_version.c mi_version.cc ; fi


## execute the following command in ../libxmi to produce the commands
## that follow
## (for i in mi_*.h; do echo "$i": "\$(top_srcdir)/libxmi/$i"; echo "	@rm -f $i"" ; if \$(LN_S) \$(top_srcdir)/libxmi/$i $i"" ; then true ; else cp -p \$(top_srcdir)/libxmi/$i $i"" ; fi"; echo; done)

xmi.h: $(top_srcdir)/libxmi/xmi.h
	@rm -f xmi.h ; if $(LN_S) $(top_srcdir)/libxmi/xmi.h xmi.h ; then true ; else cp -p $(top_srcdir)/libxmi/xmi.h xmi.h ; fi

mi_api.h: $(top_srcdir)/libxmi/mi_api.h
	@rm -f mi_api.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_api.h mi_api.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_api.h mi_api.h ; fi

mi_arc.h: $(top_srcdir)/libxmi/mi_arc.h
	@rm -f mi_arc.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_arc.h mi_arc.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_arc.h mi_arc.h ; fi

mi_fllarc.h: $(top_srcdir)/libxmi/mi_fllarc.h
	@rm -f mi_fllarc.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_fllarc.h mi_fllarc.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fllarc.h mi_fllarc.h ; fi

mi_fply.h: $(top_srcdir)/libxmi/mi_fply.h
	@rm -f mi_fply.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_fply.h mi_fply.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fply.h mi_fply.h ; fi

mi_gc.h: $(top_srcdir)/libxmi/mi_gc.h
	@rm -f mi_gc.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_gc.h mi_gc.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_gc.h mi_gc.h ; fi

mi_line.h: $(top_srcdir)/libxmi/mi_line.h
	@rm -f mi_line.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_line.h mi_line.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_line.h mi_line.h ; fi

mi_ply.h: $(top_srcdir)/libxmi/mi_ply.h
	@rm -f mi_ply.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_ply.h mi_ply.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_ply.h mi_ply.h ; fi

mi_scanfill.h: $(top_srcdir)/libxmi/mi_scanfill.h
	@rm -f mi_scanfill.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_scanfill.h mi_scanfill.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_scanfill.h mi_scanfill.h ; fi

mi_spans.h: $(top_srcdir)/libxmi/mi_spans.h
	@rm -f mi_spans.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_spans.h mi_spans.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_spans.h mi_spans.h ; fi

mi_widelin.h: $(top_srcdir)/libxmi/mi_widelin.h
	@rm -f mi_widelin.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_widelin.h mi_widelin.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_widelin.h mi_widelin.h ; fi

mi_zerarc.h: $(top_srcdir)/libxmi/mi_zerarc.h
	@rm -f mi_zerarc.h ; if $(LN_S) $(top_srcdir)/libxmi/mi_zerarc.h mi_zerarc.h ; then true ; else cp -p $(top_srcdir)/libxmi/mi_zerarc.h mi_zerarc.h ; fi

##EOF
