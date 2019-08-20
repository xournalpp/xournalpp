export PATH="$HOME/.new_local/bin:$PATH"

curl -L https://libzip.org/download/libzip-1.5.2.tar.gz -o libzip.tar.gz
tar xf libzip.tar.gz
cd libzip-*
mkdir build
cd build
echo "Build LibZip"
pwd
$HOME/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j8
make install
cd ..
cd ..
