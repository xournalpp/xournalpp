#!/bin/bash -x
export PATH="$HOME/.new_local/bin:$PATH"

# Build for macOS, without GTK and without Poppler

if [ $# -eq 0 ]; then
  echo 'Please provide the path of your gtk installation'
  exit 1
fi

export PATH="$HOME/.local/bin:$1/inst/bin:$PATH"

mkdir -p ../build
cd ../build/ || exit
cmake -DCMAKE_INSTALL_PREFIX:PATH=$1/inst ..
make -j 4
make install

cd ../mac-setup/ || exit
./build-app.sh "$1"
