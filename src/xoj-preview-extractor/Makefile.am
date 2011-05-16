
bin_PROGRAMS = xournal-thumbnailer

xournal_thumbnailer_SOURCES = \
	xournal-thumbnailer.cpp

xournal_thumbnailer_CPPFLAGS = \
	$(AM_CPPFLAGS)

xournal_thumbnailer_LDADD = \
	-lz

desktop-install:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-install-rule xournal-thumbnailer-xoj.schemas

desktop-uninstall:
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`; gconftool-2 --makefile-uninstall-rule xournal-thumbnailer-xoj.schemas
