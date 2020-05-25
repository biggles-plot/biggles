if NO_LIBPLOTTER
ADD_LIBPLOTTER =
else
ADD_LIBPLOTTER = pic2plot.test
endif

TESTS = spline.test ode.test graph.test plot2plot.test plot2hpgl.test plot2pcl.test plot2fig.test plot2cgm.test plot2ps.test plot2svg.test tek2plot.test $(ADD_LIBPLOTTER)

EXTRA_DIST = spline.test ode.test graph.test plot2plot.test plot2hpgl.test plot2pcl.test plot2fig.test plot2cgm.test plot2ps.test plot2svg.test tek2plot.test pic2plot.test spline.xout ode.xout graph.xout plot2plot.xout plot2hpgl.xout plot2hpgl.yout plot2pcl.xout plot2pcl.yout plot2fig.xout plot2cgm.xout plot2ps.xout plot2svg.xout tek2plot.xout pic2plot.xout sample.pic
				     
TESTS_ENVIRONMENT = SRCDIR=$(srcdir) PS_FONTS_IN_PCL=$(ps_fonts_in_pcl)

CLEANFILES = graph.out ode.out ode.dos plot2fig.out plot2hpgl.out plot2cg0.out plot2cg1.out plot2plot.out plot2ps0.out plot2ps1.out plot2svg.out spline.out spline.dos tek2plot.out pic2plot.out
