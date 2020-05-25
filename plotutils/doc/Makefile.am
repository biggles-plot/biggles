libplotdatadir = $(datadir)/libplot
libplotdata_DATA = README colors.txt h-fonts.txt h-glyphs.txt hershey.bib kana.txt kanji.txt

# files with nonstandard names in this directory
EXTRA_DIST = colors.txt h-fonts.txt h-glyphs.txt hershey.bib kana.txt kanji.txt

# remove documentation files installed by previous versions of the package,
# with obsolete names

OBSOLETEDOC = hershey-demo.c hershey-fonts hershey-number hershey.doc h-fonts.doc h-glyphs.doc kana.doc kanji.doc demo-page h-demo.c

install-data-hook:
	-for f in $(OBSOLETEDOC) ; do rm -f $(datadir)/libplot/$$f; done
