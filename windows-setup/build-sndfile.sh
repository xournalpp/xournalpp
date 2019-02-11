# go to script directory
cd "${0%/*}"

git clone https://github.com/erikd/libsndfile.git

cd libsndfile

mkdir build
cd build

cmake ..
make -j8
make install
