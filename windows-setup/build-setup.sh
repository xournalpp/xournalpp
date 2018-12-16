#!/bin/bash

## Windows Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. all DLLs, and additional needed files are copied to
##    the folder "setup"
## 4. NSIS is called, to create the setup

# go to script directory
cd "${0%/*}"

# delete old setup, if there
echo "clean setup folder"

rm -rf ./setup
rm -rf xournalpp-setup.exe

echo "build windows launcher"
./build-launcher.sh

mkdir setup
mkdir setup/bin
mkdir setup/lib
mkdir setup/share

echo "copy binaries"

cp ../build/src/xournalpp.exe ./setup/bin/xournalpp_bin.exe
ldd ../build/src/xournalpp.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/

echo "copy ui"

cp -r ../ui setup/
cp xournalpp.ico setup/
cp -r ../po setup/ui/

echo "copy pixbuf libs"
cp -r /mingw64/lib/gdk-pixbuf-2.0 setup/lib

echo "copy pixbuf lib dependencies"
ldd /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/

echo "copy icons"
cp -r /mingw64/share/icons setup/share/

echo "pack setup"
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" xournalpp.nsi

echo "finished"
