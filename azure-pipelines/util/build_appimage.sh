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

# linuxdeploy and linuxdeploy GTK plugin locations
LINUXDEPLOY=${LINUXDEPLOY:-"linuxdeploy.AppImage"}
LINUXDEPLOY_PLUGIN_GTK="linuxdeploy-plugin-gtk.sh"
LINUXDEPLOY_PLUGIN_GETTEXT="linuxdeploy-plugin-gettext.sh"


# Download linuxdeploy and its Gtk and Gettext plugins, if they do not yet exist
if [[ ! -f $LINUXDEPLOY_PLUGIN_GTK ]]; then
wget -c "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh"
chmod +x "$LINUXDEPLOY_PLUGIN_GTK"
fi

if [[ ! -f $LINUXDEPLOY_PLUGIN_GETTEXT ]]; then
wget -c "https://raw.githubusercontent.com/linuxdeploy/misc-plugins/master/gettext/linuxdeploy-plugin-gettext.sh"
chmod +x "$LINUXDEPLOY_PLUGIN_GETTEXT"
fi

if [[ ! -f $LINUXDEPLOY ]]; then
wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" -O "$LINUXDEPLOY"
chmod +x "$LINUXDEPLOY"
fi

# Extract tar contents to APPDIR; tar file already contains a top
# level dir, so remove it.
TAR_NAME=$(find packages/xournalpp-*.tar.gz | head -n 1)
if [[ ! -d "$APPDIR" ]]; then
    tar xf "$TAR_NAME" --one-top-level="$APPDIR"/usr --strip=1
fi

ICON_FILE="$APPDIR"/usr/share/icons/hicolor/scalable/apps/com.github.xournalpp.xournalpp.svg
DESKTOP_FILE="$APPDIR"/usr/share/applications/com.github.xournalpp.xournalpp.desktop
echo "Use the icon file $ICON_FILE and the desktop file $DESKTOP_FILE"

appimage_name='appimage'

export UPD_INFO="gh-releases-zsync|xournalpp|xournalpp|latest|$appimage_name.zsync"

# call through linuxdeploy
./"$LINUXDEPLOY" --appdir="$APPDIR" --plugin gtk --plugin gettext --output "$appimage_name" --icon-file="$ICON_FILE" --desktop-file="$DESKTOP_FILE"
