## Makefile.am for plotutils/libplot.

lib_LTLIBRARIES = libplot.la

EXTRA_DIST = DEDICATION HUMOR README-cgm README-gif README-hpgl README-tek VERSION

libplot_la_LDFLAGS = -version-info 4:4:2

INCLUDES = $(X_CFLAGS) -I$(srcdir)/../include -DLIBPLOT

MISRC = mi_alloc.c mi_api.c mi_arc.c mi_canvas.c mi_fllarc.c mi_fllrct.c \
mi_fplycon.c mi_gc.c mi_ply.c mi_plycon.c mi_plygen.c mi_plypnt.c	 \
mi_plyutil.c mi_spans.c mi_widelin.c mi_zerarc.c mi_zerolin.c mi_version.c

GSRC = g_affine.c g_alabel.c g_alab_her.c g_arc.c g_attrib.c g_bez.c	   \
g_box.c g_circ.c g_clipper.c g_closepl.c g_colors.c g_cntrlify.c g_dash.c  \
g_dash2.c g_defplot.c g_defstate.c g_ellipse.c g_endpath.c g_error.c	   \
g_erase.c g_flushpl.c g_font.c g_fontdb.c g_fontd2.c g_havecap.c	   \
g_her_glyph.c g_integer.c g_line.c g_linewidth.c g_mark.c g_matrix.c	   \
g_miscmi.c g_move.c g_openpl.c g_outbuf.c g_outfile.c g_pagetype.c	   \
g_param.c g_param2.c g_path.c g_pentype.c g_point.c g_relative.c g_range.c \
g_retrieve.c g_savestate.c g_space.c g_subpaths.c g_vector.c g_version.c   \
g_write.c g_xmalloc.c g_xstring.c

MSRC = m_attribs.c m_closepl.c m_defplot.c m_emit.c m_erase.c m_mark.c	\
m_openpl.c m_path.c m_point.c m_text.c

BSRC = b_closepl.c b_defplot.c b_erase.c b_openpl.c b_path.c b_point.c

RSRC = r_attribs.c r_closepl.c r_color.c r_defplot.c r_erase.c r_openpl.c \
r_path.c r_point.c

TSRC = t_attribs.c t_closepl.c t_color.c t_color2.c t_defplot.c t_erase.c \
t_openpl.c t_path.c t_point.c t_tek_md.c t_tek_mv.c t_tek_vec.c

HSRC = h_attribs.c h_closepl.c h_color.c h_defplot.c h_erase.c h_font.c	\
h_openpl.c h_path.c h_point.c h_text.c

FSRC = f_closepl.c f_color.c f_color2.c f_defplot.c f_erase.c f_openpl.c \
f_path.c f_point.c f_retrieve.c f_text.c

CSRC = c_attribs.c c_closepl.c c_color.c c_defplot.c c_emit.c c_erase.c	\
c_mark.c c_openpl.c c_path.c c_point.c c_text.c

PSRC = p_closepl.c p_color.c p_color2.c p_defplot.c p_erase.c p_openpl.c \
p_path.c p_point.c p_text.c

ASRC = a_attribs.c a_color.c a_closepl.c a_defplot.c a_erase.c a_openpl.c \
a_path.c a_point.c a_text.c

SSRC = s_closepl.c s_color.c s_defplot.c s_erase.c s_openpl.c s_path.c	\
s_point.c s_text.c

ISRC = i_closepl.c i_color.c i_defplot.c i_erase.c i_openpl.c i_path.c	\
i_point.c i_rle.c

NSRC = n_defplot.c n_write.c

ZSRC = z_defplot.c z_write.c

XSRC = x_afftext.c x_attribs.c x_closepl.c x_color.c x_defplot.c x_erase.c x_flushpl.c \
x_openpl.c x_path.c x_point.c x_retrieve.c x_savestate.c x_text.c

YSRC = y_closepl.c y_defplot.c y_erase.c y_openpl.c

if NO_PNG
if NO_X
ALLSRC = $(MISRC) $(GSRC) $(BSRC) $(MSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC)
else
ALLSRC = $(MISRC) $(GSRC) $(BSRC) $(MSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(XSRC) $(YSRC)
endif
else
if NO_X
ALLSRC = $(MISRC) $(GSRC) $(BSRC) $(MSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(ZSRC)
else
ALLSRC = $(MISRC) $(GSRC) $(BSRC) $(MSRC) $(TSRC) $(RSRC) $(HSRC) $(FSRC) $(CSRC) $(PSRC) \
$(ASRC) $(SSRC) $(ISRC) $(NSRC) $(ZSRC) $(XSRC) $(YSRC)
endif
endif

EXTRA_libplot_la_SOURCES = x_afftext.c x_attribs.c x_closepl.c x_color.c x_defplot.c \
x_erase.c x_flushpl.c x_openpl.c x_path.c x_point.c x_retrieve.c	 \
x_savestate.c x_text.c y_closepl.c y_defplot.c y_erase.c y_openpl.c

libplot_la_SOURCES = apinewc.c apioldc.c apioldcc.c $(ALLSRC)

MIHEADERS = xmi.h mi_api.h mi_arc.h mi_fllarc.h mi_fply.h mi_gc.h mi_line.h \
mi_ply.h mi_scanfill.h mi_spans.h mi_widelin.h mi_zerarc.h

noinst_HEADERS = extern.h g_arc.h g_colorname.h g_control.h g_cntrlify.h \
g_her_metr.h g_jis.h g_pagetype.h h_roman8.h i_rle.h p_header.h x_afftext.h \
$(MIHEADERS)

CLEANFILES = $(MISRC) $(MIHEADERS)

if NO_PNG
if NO_X
libplot_la_LIBADD =
else
libplot_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS)
endif
else
if NO_X
libplot_la_LIBADD = -lpng -lz
else
libplot_la_LIBADD = $(X_LIBS) $(X_TOOLKIT_LIBS) $(X_PRE_LIBS) $(X_BASIC_LIBS) $(X_EXTRA_LIBS) -lpng -lz
endif
endif


## execute the following command in ../libxmi to produce the commands
## that follow
##(for i in mi_*.c; do echo "$i": "\$(top_srcdir)/libxmi/$i"; echo "	rm -f $i"" ; if \$(LN_S) \$(top_srcdir)/libxmi/$i $i"" ; then true ; else cp -p \$(top_srcdir)/libxmi/$i $i"" ; fi"; echo; done)

mi_alloc.c: $(top_srcdir)/libxmi/mi_alloc.c $(MIHEADERS)
	@rm -f mi_alloc.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_alloc.c mi_alloc.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_alloc.c mi_alloc.c ; fi

mi_api.c: $(top_srcdir)/libxmi/mi_api.c $(MIHEADERS)
	@rm -f mi_api.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_api.c mi_api.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_api.c mi_api.c ; fi

mi_arc.c: $(top_srcdir)/libxmi/mi_arc.c $(MIHEADERS)
	@rm -f mi_arc.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_arc.c mi_arc.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_arc.c mi_arc.c ; fi

mi_canvas.c: $(top_srcdir)/libxmi/mi_canvas.c $(MIHEADERS)
	@rm -f mi_canvas.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_canvas.c mi_canvas.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_canvas.c mi_canvas.c ; fi

mi_fllarc.c: $(top_srcdir)/libxmi/mi_fllarc.c $(MIHEADERS)
	@rm -f mi_fllarc.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_fllarc.c mi_fllarc.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fllarc.c mi_fllarc.c ; fi

mi_fllrct.c: $(top_srcdir)/libxmi/mi_fllrct.c $(MIHEADERS)
	@rm -f mi_fllrct.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_fllrct.c mi_fllrct.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fllrct.c mi_fllrct.c ; fi

mi_fplycon.c: $(top_srcdir)/libxmi/mi_fplycon.c $(MIHEADERS)
	@rm -f mi_fplycon.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_fplycon.c mi_fplycon.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_fplycon.c mi_fplycon.c ; fi

mi_gc.c: $(top_srcdir)/libxmi/mi_gc.c $(MIHEADERS)
	@rm -f mi_gc.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_gc.c mi_gc.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_gc.c mi_gc.c ; fi

mi_ply.c: $(top_srcdir)/libxmi/mi_ply.c $(MIHEADERS)
	@rm -f mi_ply.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_ply.c mi_ply.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_ply.c mi_ply.c ; fi

mi_plycon.c: $(top_srcdir)/libxmi/mi_plycon.c $(MIHEADERS)
	@rm -f mi_plycon.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_plycon.c mi_plycon.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plycon.c mi_plycon.c ; fi

mi_plygen.c: $(top_srcdir)/libxmi/mi_plygen.c $(MIHEADERS)
	@rm -f mi_plygen.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_plygen.c mi_plygen.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plygen.c mi_plygen.c ; fi

mi_plypnt.c: $(top_srcdir)/libxmi/mi_plypnt.c $(MIHEADERS)
	@rm -f mi_plypnt.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_plypnt.c mi_plypnt.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plypnt.c mi_plypnt.c ; fi

mi_plyutil.c: $(top_srcdir)/libxmi/mi_plyutil.c $(MIHEADERS)
	@rm -f mi_plyutil.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_plyutil.c mi_plyutil.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_plyutil.c mi_plyutil.c ; fi

mi_spans.c: $(top_srcdir)/libxmi/mi_spans.c $(MIHEADERS)
	@rm -f mi_spans.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_spans.c mi_spans.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_spans.c mi_spans.c ; fi

mi_widelin.c: $(top_srcdir)/libxmi/mi_widelin.c $(MIHEADERS)
	@rm -f mi_widelin.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_widelin.c mi_widelin.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_widelin.c mi_widelin.c ; fi

mi_zerarc.c: $(top_srcdir)/libxmi/mi_zerarc.c $(MIHEADERS)
	@rm -f mi_zerarc.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_zerarc.c mi_zerarc.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_zerarc.c mi_zerarc.c ; fi

mi_zerolin.c: $(top_srcdir)/libxmi/mi_zerolin.c $(MIHEADERS)
	@rm -f mi_zerolin.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_zerolin.c mi_zerolin.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_zerolin.c mi_zerolin.c ; fi

mi_version.c: $(top_srcdir)/libxmi/mi_version.c $(MIHEADERS)
	@rm -f mi_version.c ; if $(LN_S) $(top_srcdir)/libxmi/mi_version.c mi_version.c ; then true ; else cp -p $(top_srcdir)/libxmi/mi_version.c mi_version.c ; fi

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
