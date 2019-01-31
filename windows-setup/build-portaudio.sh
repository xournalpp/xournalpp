#!/bin/bash

# go to script directory
cd "${0%/*}"

wget http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz

tar xzf pa_stable_*.tgz

cd portaudio

./configure --enable-cxx
make -j2
make install

