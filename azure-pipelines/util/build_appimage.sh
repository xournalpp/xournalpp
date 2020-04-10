#!/usr/bin/env bash
#
# Script to package an AppImage. Assumes it is being run from the build
# directory.

# AppImage root
APPDIR=${APPDIR:-"appimage_staging"}

if [ -z "$APPDIR" ]; then
    echo "Error: APPDIR must be a defined environment variable."
    exit 1
fi

# linuxdeploy location
LINUXDEPLOY=${LINUXDEPLOY:-"linuxdeploy.AppImage"}

# Extract tar contents to APPDIR; tar file already contains a top
# level dir, so remove it.
TAR_NAME=$(ls packages/xournalpp-*.tar.gz | head -n 1)
if [[ ! -d "$APPDIR" ]]; then
    tar xf $TAR_NAME --one-top-level="$APPDIR"/usr --strip=1
fi

# Download linuxdeploy if it doesn't exist
if [[ ! -f $LINUXDEPLOY ]]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O "$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

set -x

# Package GTK libraries and dependencies. The following code is from
# https://docs.appimage.org/packaging-guide/manual.html#bundling-gtk-libraries
mkdir -p "$APPDIR"/usr/lib

# GDK-Pixbuf
gdk_pixbuf_moduledir=$(pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0)
gdk_pixbuf_cache_file=$(pkg-config --variable=gdk_pixbuf_cache_file gdk-pixbuf-2.0)
gdk_pixbuf_libdir_bundle="lib/gdk-pixbuf-2.0"
gdk_pixbuf_cache_file_bundle="$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"/loaders.cache
mkdir -p "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
cp -a "$gdk_pixbuf_moduledir" "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
cp -a "$gdk_pixbuf_cache_file" "$APPDIR"/usr/"${gdk_pixbuf_libdir_bundle}"
sed -i -e "s|${gdk_pixbuf_moduledir}/||g" "$gdk_pixbuf_cache_file_bundle"

# RSVG
export GDK_PIXBUF_MODULEDIR="${APPDIR}"/usr/lib/gdk-pixbuf-2.0/loaders
RSVG_LIBDIR=$(pkg-config --variable=libdir librsvg-2.0)
if [ x"${RSVG_LIBDIR}" != "x" ]; then
    export GDK_PIXBUF_MODULE_FILE="${APPDIR}"/usr/lib/gdk-pixbuf-2.0/loaders.cache
    echo "cp -a ${RSVG_LIBDIR}/librsvg*.so* $APPDIR/usr/lib"
    cp -a "${RSVG_LIBDIR}"/librsvg*.so* "$APPDIR"/usr/lib
fi

# Copy Adwaita GTK theme and icon theme
mkdir -p "$APPDIR"/usr/share/themes "$APPDIR"/usr/share/icons
cp -r /usr/share/themes/Adwaita "$APPDIR"/usr/share/themes
cp -r /usr/share/icons/Adwaita "$APPDIR"/usr/share/icons

set +x

# Generate AppImage
APPRUN=$(dirname $0)/appimage_apprun.sh
./linuxdeploy.AppImage --appdir="$APPDIR" --custom-apprun="$APPRUN" --output appimage
