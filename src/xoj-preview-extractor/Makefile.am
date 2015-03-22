
bin_PROGRAMS = xournal-thumbnailer

xournal_thumbnailer_SOURCES = \
    xournal-thumbnailer.cpp

AM_CPPFLAGS = \
    -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

xournal_thumbnailer_LDADD = \
    -lboost_iostreams-mt -lboost_locale-mt

xournal_thumbnailer_LDFLAGS = \
    -Os

desktop-install:
    GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-install-rule xournal-thumbnailer-xoj.schemas

desktop-uninstall:
    GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-uninstall-rule xournal-thumbnailer-xoj.schemas
