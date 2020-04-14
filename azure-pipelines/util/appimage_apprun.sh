#!/usr/bin/env bash
#
# Custom AppRun for Xournal++

export APPDIR=${APPDIR:-$(readlink -f $(dirname "$0"))}
export PATH="$APPDIR"/usr/bin/:"$PATH"
export XDG_DATA_DIRS="$APPDIR"/usr/share:"$XDG_DATA_DIRS"

export GDK_PIXBUF_MODULEDIR="$APPDIR"/usr/lib/gdk-pixbuf-2.0/loaders
export GDK_PIXBUF_MODULE_FILE="$APPDIR"/usr/lib/gdk-pixbuf-2.0/loaders.cache
export GTK_PATH="$APPDIR"/usr/lib/gtk-3.0
export PANGO_LIBDIR="$APPDIR"/usr/lib

export LD_LIBRARY_PATH="$APPDIR"/usr/lib:"$GDK_PIXBUF_MODULEDIR":"$LD_LIBRARY_PATH"

# It's possible to use the system GTK theme, but different GTK versions can
# break themes. The user can set this manually if he or she really wants the
# theme to match.
export GTK_THEME=${GTK_THEME:-Adwaita}

"$APPDIR"/usr/bin/xournalpp $@
