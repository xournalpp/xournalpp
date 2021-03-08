#!/bin/bash -x
# missing six python lib:
# after first failure download, unpack and install with
# /Users/yourname/gtk/inst/bin/python setup.py install
set -x
export PATH="$HOME/.new_local/bin:$PATH"
echo "$PWD"

curl -L -O https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh
chmod +x gtk-osx-setup.sh
./gtk-osx-setup.sh
jhbuild bootstrap-gtk-osx
jhbuild build meta-gtk-osx-bootstrap meta-gtk-osx-gtk3
