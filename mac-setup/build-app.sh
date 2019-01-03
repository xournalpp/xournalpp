#!/bin/bash

## Mac Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. an .app will be packed

# go to script directory
cd "${0%/*}"

# delete old app, if there
echo "clean old app"

export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

rm -rf ./Xournal++.app

echo "prepare gtk-mac-bundler"
if [ ! -d "gtk-mac-bundler" ]; then
  git clone https://gitlab.gnome.org/GNOME/gtk-mac-bundler.git
  cd gtk-mac-bundler
else
  cd gtk-mac-bundler
  git pull
fi

make install
cd ..

echo "create package"

gtk-mac-bundler xournalpp.bundle

echo "finished"
