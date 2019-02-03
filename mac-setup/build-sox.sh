# go to script directory
cd "${0%/*}"

export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

git clone https://git.code.sf.net/p/sox/code sox-code

cd sox-code
git checkout tags/sox-14.4.2 -b sox-14.4.2

mkdir build
cd build

$HOME/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j8
make install
