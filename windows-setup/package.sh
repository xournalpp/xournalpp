#!/bin/bash

# Windows packaging script. This does the following:
# 1. Run "install" target from CMake into setup folder
# 2. Copy runtime dependencies into setup folder
# 3. Create version file and execute NSIS to create installer

if [ -z $1 ]; then
    build_dir="$(pwd)"
else
    build_dir="$(cd $1; pwd)"
fi
setup_dir_name="dist"  # Same as in xournalpp.nsis
installer_name="xournalpp-setup.exe"  # Same as in xournalpp.nsis
setup_dir="$build_dir/$setup_dir_name"
script_dir=$(dirname $(readlink -f "$0"))
echo "Installing to $setup_dir and making installer $build_dir/$installer_name"

prefix=${MSYSTEM_PREFIX:-/mingw64}
echo "Set prefix to ${prefix}"

# delete old setup, if there
echo "clean dist folder"
rm -rf "$setup_dir"
rm -rf "$build_dir/$installer_name"

mkdir "$setup_dir"
mkdir "$setup_dir"/lib

echo "copy installed files"
(cd $build_dir && cmake --install . --prefix "$setup_dir")

echo "copy libraries"
ldd "$build_dir/xournalpp.exe" | grep "${prefix}.*\.dll" -o | sort -u | xargs -I{} cp "{}" "$setup_dir"/bin/
# CI workaround: copy libcrypto and libssl in case they are not already copied.
ldd "$build_dir/xournalpp.exe" | grep -E 'lib(ssl|crypto)[^\.]*\.dll' -o | sort -u | xargs -I{} cp "${prefix}/bin/{}" "$setup_dir"/bin/

echo "Installing GTK/Glib translations"
# Copy system locale files
for trans in "$build_dir"/po/*.gmo; do
    # Bail if there are no translations at all
    [ -f "$trans" ] || break;

    # Retrieve locale from name of translation file
    locale=$(basename -s .gmo $trans)
    locale_no_country=$(echo $locale | sed 's/_.*//')

    # GTK / GLib Translation
    for f in "glib20.mo" "gdk-pixbuf.mo" "gtk30.mo" "gtk30-properties.mo"; do
        install -Dvm644 "$prefix/share/locale/$locale/LC_MESSAGES/$f" "$setup_dir/share/locale/$locale/LC_MESSAGES/$f" \
          || ([ "$locale" != "$locale_no_country" ] \
              && install -Dvm644 "$prefix/share/locale/$locale_no_country/LC_MESSAGES/$f" "$setup_dir/share/locale/$locale_no_country/LC_MESSAGES/$f")
    done
done

echo "copy pixbuf libs"
cp -r "$prefix"/lib/gdk-pixbuf-2.0 "$setup_dir"/lib/

echo "copy pixbuf lib dependencies"
# most of the dependencies are not linked directly, using strings to find them
find "$prefix/lib/gdk-pixbuf-2.0" -type f -name "*.dll" -exec strings {} \; | grep "^lib.*\.dll$" | grep -v "libpixbufloader" | sort | uniq | xargs -I{} cp "$prefix/bin/{}" "$setup_dir/bin/"

echo "copy icons"
cp -r "$prefix"/share/icons "$setup_dir"/share/

echo "copy glib shared"
cp -r "$prefix"/share/glib-2.0 "$setup_dir"/share/

echo "copy poppler shared"
cp -r "$prefix"/share/poppler "$setup_dir"/share/

echo "copy gtksourceview shared"
cp -r "$prefix"/share/gtksourceview-4 "$setup_dir"/share

echo "copy gspawn-win64-helper"
cp "$prefix"/bin/gspawn-win64-helper{,-console}.exe "$setup_dir"/bin/

echo "copy gdbus"
cp "$prefix"/bin/gdbus.exe "$setup_dir"/bin

echo "copy gtk3-demo"
cp "$prefix"/bin/gtk3-demo.exe "$setup_dir"/bin

echo "copy lua-gobject and dependencies"
cp "$prefix"/bin/libgirepository-2.0-0.dll "$setup_dir"/bin
mkdir -p "$setup_dir"/lib/lua/5.4/LuaGObject
cp "$prefix"/lib/lua/5.4/LuaGObject/lua_gobject_core.dll "$setup_dir"/lib/lua/5.4/LuaGObject
cp "$prefix"/lib/libgirepository-2.0.dll.a "$setup_dir"/lib
mkdir "$setup_dir"/lib/girepository-1.0
cp "$prefix"/lib/girepository-1.0/*.typelib "$setup_dir"/lib/girepository-1.0
mkdir -p "$setup_dir"/share/lua/5.4
cp "$prefix"/share/lua/5.4/LuaGObject.lua "$setup_dir"/share/lua/5.4
cp -r "$prefix"/share/lua/5.4/LuaGObject/ "$setup_dir"/share/lua/5.4

echo "copy qpdf"
cp "$prefix"/bin/libqpdf*.dll "$setup_dir"/bin
cp "$prefix"/lib/libqpdf* "$setup_dir"/lib

echo "create installer"
version=$(cat "$build_dir/VERSION" | sed '1!d')
"/c/Program Files (x86)/NSIS/Bin/makensis.exe" -NOCD       \
    -DXOURNALPP_VERSION="$version"                         \
    -DSETUP_DIR="$setup_dir"                               \
    -DOUTPUT_INSTALLER_FILE="$build_dir/$installer_name"   \
    -DSCRIPT_DIR="$script_dir"                             \
    "$script_dir/xournalpp.nsi"

echo "finished"

