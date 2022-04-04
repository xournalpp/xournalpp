#!/usr/bin/env bash
#
# This is a script used to export the gdk pixbuf loaders cache. Used for local
# development/testing.

if [ $# -eq 0 ]; then
  echo 'Please provide the path of the custom gdk pixbuf file to be created'
  exit 1
fi

dest_file="$1"

# Remove all lines starting with # and replace the relative paths with absolute paths
gdk-pixbuf-query-loaders | sed '/^#/d' | sed 's:lib/:/Users/xournal-dev/gtk/inst/lib/:g' > "$dest_file"
