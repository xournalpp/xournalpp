#!/bin/bash

# Windows packaging script. This does the following:
# 1. Run "install" target from CMake into setup folder
# 2. Copy runtime dependencies into setup folder
# 3. Create version file and execute NSIS to create installer

# go to script directory
cd $(dirname $(readlink -f "$0"))
# delete old setup, if there
echo "clean dist folder"

setup_dir=dist

rm -rf "$setup_dir"
rm -rf xournalpp-setup.exe

mkdir "$setup_dir"
mkdir "$setup_dir"/lib

echo "copy installed files"
(cd ../build && cmake .. -DCMAKE_INSTALL_PREFIX= && DESTDIR=../windows-setup/"$setup_dir" cmake --build . --target install)

echo "copy libraries"
ldd ../build/xournalpp.exe | grep '\/mingw.*\.dll' -o | sort -u | xargs -I{} cp "{}" "$setup_dir"/bin/
# CI workaround: copy libcrypto and libssl in case they are not already copied.
ldd ../build/xournalpp.exe | grep -E 'lib(ssl|crypto)[^\.]*\.dll' -o | sort -u | xargs -I{} cp "/mingw64/bin/{}" "$setup_dir"/bin/

# Copy system locale files
for trans in ../build/po/*.gmo; do
    # Bail if there are no translations at all
    [ -f "$trans" ] || break;

	# Retrieve locale from name of translation file
	locale=$(basename -s .gmo $trans)

	# GTK / GLib Translation
	cp -r /usr/share/locale/$locale/LC_MESSAGES/glib20.mo "$setup_dir"/share/locale/$locale/LC_MESSAGES/glib20.mo

	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gdk-pixbuf.mo "$setup_dir"/share/locale/$locale/LC_MESSAGES/gdk-pixbuf.mo
	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gtk30.mo "$setup_dir"/share/locale/$locale/LC_MESSAGES/gtk30.mo
	cp -r /mingw64/share/locale/$locale/LC_MESSAGES/gtk30-properties.mo	"$setup_dir"/share/locale/$locale/LC_MESSAGES/gtk30-properties.mo
done

echo "copy pixbuf libs"
cp -r /mingw64/lib/gdk-pixbuf-2.0 "$setup_dir"/lib/

echo "copy pixbuf lib dependencies"
ldd /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" "$setup_dir"/bin/

echo "copy icons"
cp -r /mingw64/share/icons "$setup_dir"/share/

echo "copy glib shared"
cp -r /mingw64/share/glib-2.0 "$setup_dir"/share/

echo "copy poppler shared"
cp -r /mingw64/share/poppler "$setup_dir"/share/

echo "copy gtksourceview shared"
cp -r /mingw64/share/gtksourceview-5 "$setup_dir"/share

echo "copy gspawn-win64-helper"
cp /mingw64/bin/gspawn-win64-helper.exe "$setup_dir"/bin
cp /mingw64/bin/gspawn-win64-helper-console.exe "$setup_dir"/bin

echo "copy gdbus"
cp /mingw64/bin/gdbus.exe "$setup_dir"/bin

echo "create installer"
bash make_version_nsh.sh
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" xournalpp.nsi

echo "finished"

