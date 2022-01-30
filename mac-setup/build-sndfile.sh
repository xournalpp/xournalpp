#!/bin/bash -x
export PATH="$HOME/.new_local/bin:$HOME/gtk/inst/bin:$PATH"
# go to script directory
cd "${0%/*}" || exit

git clone https://github.com/erikd/libsndfile.git

cd libsndfile || exit

mkdir build
cd build || exit

# TODO add libogg and libvorbis for actual audio support

"$HOME"/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH="$HOME"/gtk/inst ..
make -j8
make install
