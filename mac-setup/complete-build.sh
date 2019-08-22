#!/usr/bin/env bash

# Build for macOS, without GTK and without Poppler

export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

mkdir -p ../build
cd ../build/
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j 4
make install

cd ../mac-setup/
./build-app.sh
