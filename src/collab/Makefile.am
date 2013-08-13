noinst_LTLIBRARIES = \
	libcollab.la

libcollab_la_SOURCES = \
	Collaboration.cpp

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

libcollab_la_CXXFLAGS = \
	-I$(top_srcdir)/src/util \
	-g -rdynamic \
	-Wreturn-type -Wuninitialized -Wunused-value -Wunused-variable

