ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src ui

if HAVE_DOXYGEN
SUBDIRS += doc

.PHONY: doc

doc:
	$(MAKE) -C $(abs_top_builddir)/doc doc

endif


EXTRA_DIST = \
	autogen.sh \
	po/LINGUAS \
	po/de.mo \
	xournal.xml x-xoj.desktop xournalpp.desktop

poppler clean-local:
	for dir in $(SUBDIRS); do \
		if [ $$dir == "src" ]; then \
			$(MAKE) -C $$dir $@; \
		else \
			$(MAKE) -C $$dir; \
		fi; \
	done

desktop-install:
	if test "$(datadir)" = "/usr/share"; then \
	  desktopdir=/usr/share; \
	else \
	  desktopdir=/usr/local/share; \
	fi; \
	$(mkinstalldirs) $(DESTDIR)$$desktopdir/icons/hicolor/scalable/apps; \
	$(mkinstalldirs) $(DESTDIR)$$desktopdir/icons/hicolor/scalable/mimetypes; \
	$(mkinstalldirs) $(DESTDIR)$$desktopdir/mime/packages; \
	$(mkinstalldirs) $(DESTDIR)$$desktopdir/applications; \
	$(mkinstalldirs) $(DESTDIR)/usr/share/mimelnk/application; \
	$(INSTALL_DATA) $(srcdir)/ui/pixmaps/xournalpp.svg $(DESTDIR)$$desktopdir/icons/hicolor/scalable/apps; \
	$(INSTALL_DATA) $(srcdir)/ui/pixmaps/xoj.svg $(DESTDIR)$$desktopdir/icons/hicolor/scalable/mimetypes; \
	if test ! -e $(DESTDIR)$$desktopdir/icons/hicolor/scalable/mimetypes/gnome-mime-application-x-xoj.svg; then \
	  ln -s xoj.svg $(DESTDIR)$$desktopdir/icons/hicolor/scalable/mimetypes/gnome-mime-application-x-xoj.svg; \
	fi; \
	$(INSTALL_DATA) $(srcdir)/xournal.xml $(DESTDIR)$$desktopdir/mime/packages; \
	$(INSTALL_DATA) $(srcdir)/xournalpp.desktop $(DESTDIR)$$desktopdir/applications; \
	$(INSTALL_DATA) $(srcdir)/x-xoj.desktop $(DESTDIR)/usr/share/mimelnk/application; \
	if test -z "$(DESTDIR)"; then \
	  echo "Updating desktop, mime, and icon databases."; \
	  update-desktop-database; \
	  update-mime-database $$desktopdir/mime; \
	  gtk-update-icon-cache -f -t $$desktopdir/icons/hicolor; \
	else \
	  echo "*** Not updating desktop, mime, and icon databases. After install, run:"; \
	  echo "***   update-desktop-database"; \
	  echo "***   update-mime-database $$desktopdir/mime"; \
	  echo "***   gtk-update-icon-cache -f -t $$desktopdir/icons/hicolor"; \
	fi
