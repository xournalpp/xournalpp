export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

rm -rf ./gtk-mac-integration
git clone https://gitlab.gnome.org/GNOME/gtk-mac-integration.git

cd ./gtk-mac-integration
./autogen.sh
./configure --prefix=$HOME/gtk/inst
make -j4
make install

cd ..
