#!/bin/bash -x
export PATH="$HOME/.new_local/bin:$PATH"
export LIBRARY_PATH="$HOME/gtk/inst/lib:$LIBRARY_PATH"
# go to script directory
cd "${0%/*}" || exit

curl -L http://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz -o pa_stable_v190700_20210406.tgz
tar xzf pa_stable_*.tgz

cd portaudio || exit

./configure --enable-cxx --disable-mac-universal --prefix=$HOME/gtk/inst
make -j2
make install
