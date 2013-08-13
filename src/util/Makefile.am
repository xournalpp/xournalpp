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
	String.cpp \
	pixbuf-utils.cpp \
	Range.cpp \
	Rectangle.cpp \
	Util.cpp \
	XInputUtils.cpp \
	XournalType.cpp

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

libutil_la_CXXFLAGS = \
	-I$(top_srcdir)/src/util \
	-g -rdynamic \
	-Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable

