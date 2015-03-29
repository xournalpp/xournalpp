#!/bin/bash
#This will produce a deb package using checkinstall.
checkinstall --install=no --pkgversion 1.0.2 --pkgname xournalpp --provides xournalpp --requires "libgtk2.0-0, libglade2-0, librsvg2-2" --maintainer "wilson@wbrenna.ca" -D make install
