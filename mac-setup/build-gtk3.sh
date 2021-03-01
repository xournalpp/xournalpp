# missing six python lib:
# after first failure download, unpack and install with
# /Users/yourname/gtk/inst/bin/python setup.py install

curl -L -O https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh
chmod +x gtk-osx-setup.sh

./gtk-osx-setup.sh
export PATH="$HOME/.new_local/bin:$PATH"
jhbuild run bootstrap-gtk-osx
jhbuild run build meta-gtk-osx-bootstrap meta-gtk-osx-gtk3
