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
prefix=${MSYSTEM_PREFIX:-/mingw64}
echo "Set prefix to ${prefix}"

rm -rf "$setup_dir"
rm -rf xournalpp-setup.exe

mkdir "$setup_dir"
mkdir "$setup_dir"/lib

echo "copy installed files"
(cd ../build && cmake --install . --prefix ../windows-setup/"$setup_dir")

echo "copy libraries"
ldd ../build/xournalpp.exe | grep "${prefix}.*\.dll" -o | sort -u | xargs -I{} cp "{}" "$setup_dir"/bin/
# CI workaround: copy libcrypto and libssl in case they are not already copied.
ldd ../build/xournalpp.exe | grep -E 'lib(ssl|crypto)[^\.]*\.dll' -o | sort -u | xargs -I{} cp "${prefix}/bin/{}" "$setup_dir"/bin/

echo "Installing GTK/Glib translations"
# Copy system locale files
for trans in ../build/po/*.gmo; do
    # Bail if there are no translations at all
    [ -f "$trans" ] || break;

    # Retrieve locale from name of translation file
    locale=$(basename -s .gmo $trans)
    locale_no_country=$(echo $locale | sed 's/_.*//')

    # GTK / GLib Translation
    for f in "glib20.mo" "gdk-pixbuf.mo" "gtk30.mo" "gtk30-properties.mo"; do
        install -Dvm644 "$prefix"/share/locale/$locale/LC_MESSAGES/$f "$setup_dir"/share/locale/$locale/LC_MESSAGES/$f \
          || ([ "$locale" != "$locale_no_country" ] \
              && install -Dvm644 "$prefix"/share/locale/$locale_no_country/LC_MESSAGES/$f "$setup_dir"/share/locale/$locale_no_country/LC_MESSAGES/$f)
    done
done

echo "copy pixbuf libs"
cp -r "$prefix"/lib/gdk-pixbuf-2.0 "$setup_dir"/lib/

echo "copy pixbuf lib dependencies"
ldd "$prefix"/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll | grep "${prefix}.*\.dll" -o | xargs -I{} cp "{}" "$setup_dir"/bin/

echo "copy icons"
cp -r "$prefix"/share/icons "$setup_dir"/share/

echo "copy glib shared"
cp -r "$prefix"/share/glib-2.0 "$setup_dir"/share/

echo "copy poppler shared"
cp -r "$prefix"/share/poppler "$setup_dir"/share/

echo "copy gtksourceview shared"
cp -r "$prefix"/share/gtksourceview-4 "$setup_dir"/share

echo "copy gspawn-win64-helper"
cp "$prefix"/bin/gspawn-win64-helper.exe "$setup_dir"/bin
cp "$prefix"/bin/gspawn-win64-helper-console.exe "$setup_dir"/bin

echo "copy gdbus"
cp "$prefix"/bin/gdbus.exe "$setup_dir"/bin

echo "copy gtk3-demo"
cp "$prefix"/bin/gtk3-demo.exe "$setup_dir"/bin

echo "copy lua-lgi and dependencies"
cp "$prefix"/bin/libgirepository-1.0-1.dll "$setup_dir"/bin
mkdir -p "$setup_dir"/lib/lua/5.4/lgi
cp "$prefix"/lib/lua/5.4/lgi/corelgilua51.dll "$setup_dir"/lib/lua/5.4/lgi
cp "$prefix"/lib/libgirepository-1.0.dll.a "$setup_dir"/lib
mkdir "$setup_dir"/lib/girepository-1.0
cp "$prefix"/lib/girepository-1.0/*.typelib "$setup_dir"/lib/girepository-1.0
mkdir -p "$setup_dir"/share/lua/5.4
cp "$prefix"/share/lua/5.4/lgi.lua "$setup_dir"/share/lua/5.4
cp -r "$prefix"/share/lua/5.4/lgi/ "$setup_dir"/share/lua/5.4

echo "create installer"
bash make_version_nsh.sh
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" xournalpp.nsi

echo "finished"

