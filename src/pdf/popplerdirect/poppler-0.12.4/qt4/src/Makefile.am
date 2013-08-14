INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	-I$(top_srcdir)/qt			\
	$(FONTCONFIG_CFLAGS)			\
	$(POPPLER_QT4_CFLAGS)

SUFFIXES: .moc

poppler_includedir = $(includedir)/poppler/qt4

poppler_include_HEADERS =			\
	poppler-qt4.h				\
	poppler-link.h				\
	poppler-annotation.h			\
	poppler-form.h				\
	poppler-optcontent.h			\
	poppler-export.h			\
	poppler-page-transition.h		\
	poppler-page-transition-private.h

lib_LTLIBRARIES = libpoppler-qt4.la

libpoppler_qt4_la_SOURCES =			\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-fontinfo.cc			\
	poppler-embeddedfile.cc			\
	poppler-textbox.cc			\
	poppler-link.cc				\
	poppler-annotation.cc			\
	poppler-link-extractor.cc		\
	poppler-optcontent.cc			\
	poppler-optcontent.h			\
	poppler-page-transition.cc		\
	poppler-sound.cc			\
	poppler-form.cc				\
	poppler-base-converter.cc		\
	poppler-ps-converter.cc			\
	poppler-pdf-converter.cc		\
	poppler-qiodeviceoutstream.cc		\
	poppler-private.cc			\
	poppler-movie.cc			\
	poppler-annotation-helper.h		\
	poppler-page-private.h			\
	poppler-link-extractor-private.h	\
	poppler-annotation-private.h		\
	poppler-converter-private.h		\
	poppler-qiodeviceoutstream-private.h	\
	poppler-optcontent-private.h		\
	poppler-export.h			\
	poppler-private.h

libpoppler_qt4_la_LIBADD = 			\
	$(top_builddir)/poppler/libpoppler.la	\
	$(FONTCONFIG_LIBS)				\
	$(POPPLER_QT4_LIBS)

if BUILD_SPLASH_OUTPUT
libpoppler_qt4_la_LIBADD += 			\
	$(top_builddir)/poppler/libpoppler-arthur.la
endif

libpoppler_qt4_la_LDFLAGS = -version-info 5:0:2 @create_shared_lib@

# This rule lets GNU make create any *.moc from the equivalent *.h
.h.moc:
	$(AM_V_GEN) $(MOCQT4) -i $< -o $@

poppler-optcontent.lo: poppler-optcontent.moc

clean-generic:
	rm -f *.moc
