export PATH="$HOME/.local/bin:$PATH"

# missing six python lib:
# after first failure download, unpack and install with
# /Users/yourname/gtk/inst/bin/python setup.py install

curl https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh -o gtk-osx-setup.sh
chmod +x gtk-osx-setup.sh
./gtk-osx-setup.sh

jhbuild bootstrap
jhbuild build python meta-gtk-osx-bootstrap meta-gtk-osx-gtk3
