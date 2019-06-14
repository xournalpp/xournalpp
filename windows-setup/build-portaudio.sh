#!/bin/bash

PA_CHECKSUM=56c596bba820d90df7d057d8f6a0ec6bf9ab82e8
PA_FILENAME=pa_stable_v190600_20161030.tgz

# go to script directory
cd "${0%/*}"

wget http://www.portaudio.com/archives/${PA_FILENAME}

#Check the checksum of the downloaded archive
printf "Checking integrity of downloaded files..."
printf "${PA_CHECKSUM} *${PA_FILENAME}" | sha1sum -c --strict -
if [ $? != 0 ]; then
  exit 1
fi

echo "Unpacking..."
tar xzf pa_stable_*.tgz

cd portaudio

echo "Installing..."
./configure --enable-cxx
make -j2
make install

