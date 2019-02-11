# go to script directory
cd "${0%/*}"

export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

git clone https://github.com/erikd/libsndfile.git

cd libsndfile

mkdir build
cd build

$HOME/gtk/inst/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j8
make install

# Fix linker flags
cp portaudio-2.0.pc /Users/andreas/gtk/inst/lib/pkgconfig

