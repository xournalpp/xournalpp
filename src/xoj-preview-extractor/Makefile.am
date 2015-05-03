
bin_PROGRAMS = xournal-thumbnailer

xournal_thumbnailer_SOURCES = \
	xournal-thumbnailer.cpp \
	../util/XojPreviewExtractor.cpp

xournal_thumbnailer_CPPFLAGS = \
	$(AM_CPPFLAGS) `pkg-config --cflags glib-2.0`

xournal_thumbnailer_LDFLAGS = \
	`pkg-config --libs glib-2.0`

xournal_thumbnailer_LDADD = \
	-lz

desktop-install:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-install-rule xournal-thumbnailer-xoj.schemas

desktop-uninstall:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-uninstall-rule xournal-thumbnailer-xoj.schemas
