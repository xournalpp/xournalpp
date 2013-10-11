#!/bin/bash
#This will produce a deb package using checkinstall.
checkinstall --pkgversion 20130111 --pkgname xournalpp --provides xournalpp --requires "gtk+-2.0, poppler-glib, poppler, libglade-2.0, gthread-2.0, librsvg-2.0, zlib" --maintainer "wilson@wbrenna.ca" -D make install
