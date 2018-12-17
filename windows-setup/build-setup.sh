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

mkdir setup
mkdir setup/bin
mkdir setup/lib
mkdir setup/share

echo "build windows launcher"
./build-launcher.sh
# done in launcher build script: cp xournalpp.exe setup/bin/

echo "copy binaries"

cp ../build/src/xournalpp.exe ./setup/bin/xournalpp_bin.exe
ldd ../build/src/xournalpp.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/

echo "copy ui"

cp -r ../ui setup/

mkdir -p setup/share/po/cs/LC_MESSAGES
mkdir -p setup/share/po/zh_HK/LC_MESSAGES
mkdir -p setup/share/po/de/LC_MESSAGES
mkdir -p setup/share/po/pl/LC_MESSAGES
mkdir -p setup/share/po/zh_TW/LC_MESSAGES
mkdir -p setup/share/po/zh/LC_MESSAGES

cp -r ../po/cs.mo setup/share/po/cs/LC_MESSAGES/xournalpp.mo
cp -r ../po/zh_HK.mo setup/share/po/zh_HK/LC_MESSAGES/xournalpp.mo
cp -r ../po/de.mo setup/share/po/de/LC_MESSAGES/xournalpp.mo
cp -r ../po/ps.mo setup/share/po/pl/LC_MESSAGES/xournalpp.mo
cp -r ../po/zh_TW.mo setup/share/po/zh_TW/LC_MESSAGES/xournalpp.mo
cp -r ../po/zh.mo setup/share/po/zh/LC_MESSAGES/xournalpp.mo

echo "copy pixbuf libs"
cp -r /mingw64/lib/gdk-pixbuf-2.0 setup/lib

echo "copy pixbuf lib dependencies"
ldd /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/

echo "copy icons"
cp -r /mingw64/share/icons setup/share/

echo "copy glib shared"
cp -r /mingw64/share/glib-2.0 setup/share/

echo "pack setup"
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" xournalpp.nsi

echo "finished"
