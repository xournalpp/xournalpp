# go to script directory
cd "${0%/*}"

git clone https://github.com/erikd/libsndfile.git

cd libsndfile

mkdir build
cd build

cmake -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=/mingw64 ..
make -j8
make install
