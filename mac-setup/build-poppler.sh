export PATH="$HOME/.local/bin:$PATH"

curl -L https://github.com/uclouvain/openjpeg/archive/v2.3.0.tar.gz -o openjpeg.tar.gz
tar xf openjpeg.tar.gz
cd openjpeg-*
mkdir build
cd build
echo "Build OpenJpeg"
pwd
$HOME/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j8
make install
cd ..
cd ..


ln -s $HOME/gtk/inst/lib/gio $HOME/gtk/inst/lib/gio-2.0

curl https://poppler.freedesktop.org/poppler-0.72.0.tar.xz -o poppler.tar.xz
tar xf poppler.tar.xz
cd poppler-*
mkdir build
cd build
echo "Build Poppler"
pwd
$HOME/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j8
cd ..
cd ..
