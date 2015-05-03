noinst_LTLIBRARIES = \
	libutil.la

libutil_la_SOURCES = \
	logger/Logger.cpp \
	serializing/BinObjectEncoding.cpp \
	serializing/HexObjectEncoding.cpp \
	serializing/InputStreamException.cpp \
	serializing/ObjectEncoding.cpp \
	serializing/ObjectInputStream.cpp \
	serializing/ObjectOutputStream.cpp \
	CrashHandler.cpp \
	GzHelper.cpp \
	LatexAction.cpp \
	OutputStream.cpp \
	PageRange.cpp \
	Stacktrace.cpp \
	StringUtils.cpp \
	pixbuf-utils.cpp \
	Range.cpp \
	Rectangle.cpp \
	Util.cpp \
	XojPreviewExtractor.cpp \
	XInputUtils.cpp \
	XournalType.cpp

AM_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I"$(abs_top_builddir)/src/pdf/popplerdirect" \
	@PACKAGE_CFLAGS@

libutil_la_CXXFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/util \
	-I$(top_srcdir)/src/pdf/popplerdirect/poppler-0.24.1 \
	-I$(top_srcdir)/src/pdf/popplerdirect/poppler-0.24.1/poppler \
	-g -rdynamic \
	-Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable

$(abs_top_builddir)/src/pdf/popplerdirect/poppler-config.h:
	$(abs_top_builddir)/src/pdf/popplerdirect/workaround/workaround.sh build

BUILT_SOURCES = $(abs_top_builddir)/src/pdf/popplerdirect/poppler-config.h
