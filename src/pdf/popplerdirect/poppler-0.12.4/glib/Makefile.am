if BUILD_CAIRO_OUTPUT

cairo_includes =				\
	$(CAIRO_CFLAGS)

cairo_libs =					\
	$(top_builddir)/poppler/libpoppler-cairo.la	\
	$(CAIRO_LIBS)

endif

SUBDIRS = . reference demo
INCLUDES =					\
	-DG_LOG_DOMAIN=\"Poppler\"		\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/poppler			\
	$(cairo_includes)			\
	$(POPPLER_GLIB_CFLAGS)			\
	$(GDK_CFLAGS)				\
	$(FREETYPE_CFLAGS)			\
	$(FONTCONFIG_CFLAGS)                    \
	$(cairo_includes)			\
	$(POPPLER_GLIB_DISABLE_DEPRECATED)	\
	$(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES)

stamp_files = stamp-poppler-enums.h

poppler-enums.h: stamp-poppler-enums.h
	@true

stamp-poppler-enums.h: poppler-enums.h.template $(poppler_glib_public_headers)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbh \
	&& (cmp -s xgen-etbh poppler-enums.h || cp xgen-etbh poppler-enums.h ) \
	&& rm -f xgen-etbh \
	&& echo timestamp > $(@F)

poppler-enums.c: poppler-enums.c.template $(poppler_glib_public_headers)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-etbc \
	&& (cmp -s xgen-etbc poppler-enums.c || cp xgen-etbc poppler-enums.c ) \
	&& rm -f xgen-etbc

EXTRA_DIST = \
	poppler-enums.h.template	\
	poppler-enums.c.template

poppler_glib_public_headers =			\
	poppler-action.h			\
	poppler-date.h				\
	poppler-document.h			\
	poppler-page.h				\
	poppler-attachment.h			\
	poppler-form-field.h			\
	poppler-annot.h				\
	poppler-layer.h				\
	poppler.h

poppler_glib_includedir = $(includedir)/poppler/glib

poppler_glib_include_HEADERS =			\
	$(poppler_glib_public_headers)		\
	poppler-enums.h				\
	poppler-features.h

lib_LTLIBRARIES = libpoppler-glib.la
libpoppler_glib_la_SOURCES =			\
	$(BUILT_SOURCES)			\
	poppler-action.cc			\
	poppler-date.cc				\
	poppler-document.cc			\
	poppler-page.cc				\
	poppler-attachment.cc			\
	poppler-form-field.cc			\
	poppler-annot.cc			\
	poppler-layer.cc			\
	poppler.cc				\
	poppler-private.h

libpoppler_glib_la_LIBADD =				\
	$(top_builddir)/poppler/libpoppler.la		\
	$(POPPLER_GLIB_LIBS)				\
	$(GDK_LIBS)					\
	$(FREETYPE_LIBS)				\
	$(FONTCONFIG_LIBS)				\
	$(cairo_libs)

libpoppler_glib_la_LDFLAGS = -version-info 4:0:0 @create_shared_lib@ @auto_import_flags@

if BUILD_WITH_GDK
noinst_PROGRAMS = test-poppler-glib

test_poppler_glib_SOURCES =			\
       test-poppler-glib.cc

test_poppler_glib_LDADD =			\
	$(top_builddir)/poppler/libpoppler.la	\
	libpoppler-glib.la			\
	$(POPPLER_GLIB_LIBS)			\
	$(GDK_LIBS)				\
	$(FREETYPE_LIBS)			\
	$(cairo_libs)
endif

BUILT_SOURCES =					\
	poppler-enums.c				\
	poppler-enums.h				\
	poppler-features.h

CLEANFILES = $(BUILT_SOURCES) $(stamp_files)
DISTCLEANFILES = $(BUILT_SOURCES) $(stamp_files)
MAINTAINERCLEANFILES = $(BUILT_SOURCES) $(stamp_files)
