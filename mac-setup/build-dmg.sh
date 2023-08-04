#!/usr/bin/env bash

[ $# -lt 2 ] && echo "Usage: $0 <.app folder> <output path>" && exit 1

APP_NAME="$1"
OUT_NAME="$2"

TMP_DMG=$(mktemp).dmg
TMP_MOUNTPOINT=$(mktemp -d)
echo "Creating temporary dmg: $TMP_DMG"
echo "Mount point at: $TMP_MOUNTPOINT"
mkdir -p "$TMP_MOUNTPOINT"

# Create the image.
# If there is not enough space when copying, the size should be increased.
hdiutil create -size 256m -fs HFS+ -volname "Xournal++" "$TMP_DMG"
hdiutil attach "$TMP_DMG" -mountpoint "$TMP_MOUNTPOINT"
echo "Copying app"
cp -r "$APP_NAME" "$TMP_MOUNTPOINT"/
ln -s /Applications/ "$TMP_MOUNTPOINT"/Applications
hdiutil detach "$TMP_MOUNTPOINT"

# hdituil convert refuses to run if the file already exists, so delete it
rm "$OUT_NAME"
hdiutil convert "$TMP_DMG" -format UDZO -o "$OUT_NAME"

# Cleanup
rm "$TMP_DMG"
