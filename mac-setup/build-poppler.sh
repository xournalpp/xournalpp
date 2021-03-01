#!/bin/bash -x
export PATH="$HOME/.new_local/bin:$PATH"

curl -L https://github.com/uclouvain/openjpeg/archive/v2.3.0.tar.gz -o openjpeg.tar.gz
tar xf openjpeg.tar.gz
cd openjpeg-* || exit
mkdir build
cd build || exit
echo "Build OpenJpeg"
pwd
"$HOME"/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH="$HOME"/gtk/inst ..
make -j8
make install
cd ..
cd ..

export LIBRARY_PATH="$HOME/gtk/inst/lib:$LIBRARY_PATH"

jhbuild build freetype fontconfig
jhbuild buildone -acf cairo

curl https://poppler.freedesktop.org/poppler-0.72.0.tar.xz -o poppler.tar.xz
tar xf poppler.tar.xz
cd poppler-* || exit
mkdir build
cd build || exit
echo "Build Poppler"
pwd
"$HOME"/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH="$HOME"/gtk/inst ..
make -j8
make install
