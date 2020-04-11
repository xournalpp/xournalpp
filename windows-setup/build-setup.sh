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
ldd ../build/src/xournalpp.exe | grep '\/mingw.*\.dll' -o | sort -u | xargs -I{} cp "{}" setup/bin/
# CI workaround: copy libcrypto and libssl in case they are not already copied.
ldd ../build/src/xournalpp.exe | grep -E 'lib(ssl|crypto)[^\.]*\.dll' -o | sort -u | xargs -I{} cp "/mingw64/bin/{}" setup/bin/

echo "copy ui"

cp -r ../ui setup/

supportedLocales=("cs" "de" "it" "pl" "zh" "zh_TW" "zh_HK")
for locale in "${supportedLocales[@]}" ; do
	echo "Copy locale $locale"
	mkdir -p setup/share/locale/$locale/LC_MESSAGES
	
	# Xournal Translation
	cp -r ../build/po/$locale.gmo setup/share/locale/$locale/LC_MESSAGES/xournalpp.mo

	# GTK / GLib Translation
	cp -r /usr/share/locale/$locale/LC_MESSAGES/glib20.mo			setup/share/locale/$locale/LC_MESSAGES/glib20.mo

	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gdk-pixbuf.mo		setup/share/locale/$locale/LC_MESSAGES/gdk-pixbuf.mo
	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gtk30.mo			setup/share/locale/$locale/LC_MESSAGES/gtk30.mo
	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gtk30-properties.mo	setup/share/locale/$locale/LC_MESSAGES/gtk30-properties.mo
done

echo "copy pixbuf libs"
cp -r /mingw64/lib/gdk-pixbuf-2.0 setup/lib

echo "copy pixbuf lib dependencies"
ldd /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/

echo "copy icons"
cp -r /mingw64/share/icons setup/share/

echo "copy glib shared"
cp -r /mingw64/share/glib-2.0 setup/share/

echo "copy gspawn-win64-helper"
cp /mingw64/bin/gspawn-win64-helper.exe setup/bin
cp /mingw64/bin/gspawn-win64-helper-console.exe setup/bin

echo "pack setup"
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" xournalpp.nsi

echo "finished"

