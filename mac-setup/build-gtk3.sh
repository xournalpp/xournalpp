#!/bin/bash -x
# missing six python lib:
# after first failure download, unpack and install with
# /Users/yourname/gtk/inst/bin/python setup.py install
export PATH="$HOME/.new_local/bin:$PATH"
export CFLAGS=-Wno-implicit-function-declaration $CFLAGS

curl -L -O https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh
chmod +x gtk-osx-setup.sh
./gtk-osx-setup.sh
jhbuild bootstrap-gtk-osx
jhbuild build python3
jhbuild build meta-gtk-osx-bootstrap meta-gtk-osx-gtk3
