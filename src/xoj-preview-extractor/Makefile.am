
bin_PROGRAMS = xournal-thumbnailer

xournal_thumbnailer_SOURCES = \
	xournal-thumbnailer.cpp \
	../util/XojPreviewExtractor.cpp

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	$(BOOST_CPPFLAGS) $(GLIBMM_CFLAGS)

AM_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS) $(BOOST_LOCALE_LDFLAGS) $(BOOST_IOSTREAMS_LDFLAGS)

LIBS = $(BOOST_SYSTEM_LIBS) $(BOOST_LOCALE_LIBS) $(BOOST_IOSTREAMS_LIBS) $(GLIBMM_LIBS)

desktop-install:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-install-rule xournal-thumbnailer-xoj.schemas

desktop-uninstall:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-uninstall-rule xournal-thumbnailer-xoj.schemas

all:
	strip xournal-thumbnailer
