noinst_LTLIBRARIES = libgoo.la

if ENABLE_XPDF_HEADERS

poppler_goo_includedir = $(includedir)/poppler/goo
poppler_goo_include_HEADERS =			\
	GooHash.h				\
	GooList.h				\
	GooTimer.h				\
	GooMutex.h				\
	GooString.h				\
	GooVector.h				\
	gtypes.h				\
	gmem.h					\
	gfile.h					\
	FixedPoint.h				\
	PNGWriter.h				\
	gstrtod.h

endif

INCLUDES =					\
	-I$(top_srcdir)

libgoo_la_SOURCES =				\
	gfile.cc				\
	gmempp.cc				\
	GooHash.cc				\
	GooList.cc				\
	GooTimer.cc				\
	GooString.cc				\
	gmem.cc					\
	FixedPoint.cc				\
	PNGWriter.cc				\
	gstrtod.cc
