#!/bin/bash

PA_CHECKSUM=b7e9b9c53d993f6d110487ef56a3d4529d21b2f1
PA_FILENAME=pa_stable_v190700_20210406.tgz

# go to script directory
cd "${0%/*}"

wget http://files.portaudio.com/archives/${PA_FILENAME}

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

