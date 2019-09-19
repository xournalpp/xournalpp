#!/usr/bin/env bash
#
# Custom AppRun for Xournal++

export APPDIR=${APPDIR:-$(readlink -f $(dirname "$0"))}

export PATH="$APPDIR"/usr/bin/:"$PATH"
export GDK_PIXBUF_MODULEDIR="$APPDIR"/usr/lib/gdk-pixbuf-2.0/loaders
export GDK_PIXBUF_MODULE_FILE="$APPDIR"/usr/lib/gdk-pixbuf-2.0/loaders.cache
export GTK_PATH="$APPDIR"/usr/lib/gtk-3.0
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$GDK_PIXBUF_MODULEDIR"
export PANGO_LIBDIR="$APPDIR"/usr/lib

"$APPDIR"/usr/bin/xournalpp
