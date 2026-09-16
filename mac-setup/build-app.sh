#!/usr/bin/env bash

set -e

## Mac Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. an .app will be packed
#
# This script should be run from a jhbuild environment

if [ -z "$UNDER_JHBUILD" ]; then
  echo 'WARNING: this command needs to be run within a jhbuild-like environment.'
  echo 'The build will proceed, but do not be surprised if it fails!'
fi

if [ $# -eq 0 ]; then
  echo 'Please provide the path of your gtk installation'
  exit 1
fi

# go to script directory
cd "$(dirname "$0")" || exit

# delete old app, if there
echo "clean old app"
rm -rf ./Xournal++.app

echo "prepare gtk-mac-bundler"
GTK_MAC_BUNDLER_VENV="$PWD"/gtk-mac-bundler-venv
GTK_MAC_BUNDLER="$GTK_MAC_BUNDLER_VENV"/bin/gtk-mac-bundler
if [ ! -f "$GTK_MAC_BUNDLER" ]; then
  echo "Existing gtk-mac-bundler not found, installing a new one in $GTK_MAC_BUNDLER_VENV"
  python3 -m venv "$GTK_MAC_BUNDLER_VENV"

  if [ ! -d gtk-mac-bundler ]; then
    git clone --depth=1 https://gitlab.gnome.org/GNOME/gtk-mac-bundler.git
  fi
  pushd gtk-mac-bundler
  # The install script assumes it will install to some global path
  # Instead, we isolate it to a virtualenv.
  cat <<EOF > bundler/launcher.py
from . import main as bundler_main
import sys

def main():
    bundler_main.main(sys.argv[1:])
EOF

  cat <<EOF > pyproject.toml
[build-system]
requires = ["setuptools >= 58.0"]
build-backend = "setuptools.build_meta"

[project]
name = "gtk-mac-bundler"
version = "0.0.0"

[project.scripts]
gtk-mac-bundler = "bundler.launcher:main"
EOF

  echo 'include bundler/*.sh' > MANIFEST.in
  popd
  "$GTK_MAC_BUNDLER_VENV"/bin/pip install ./gtk-mac-bundler
fi

if [ ! -f "$GTK_MAC_BUNDLER" ]; then
  echo "error: gtk-mac-bundler does not appear to be installed!"
  echo "try deleting $GTK_MAC_BUNDLER_VENV and rerunning this script"
  exit 1
fi

echo "create package"

export GTKDIR="$1/inst"
[ ! -d "$GTKDIR" ] && echo "$GTKDIR doesn't exist!" && exit 1

"$GTK_MAC_BUNDLER" xournalpp.bundle

echo "Replace Ctrl by Meta in mainmenubar.ui"
sed -i -e 's/Ctrl/Meta/g' ./Xournal++.app/Contents/Resources/ui/mainmenubar.ui

echo "Create zip"
zip --filesync -r Xournal++.zip Xournal++.app

echo "finished"
