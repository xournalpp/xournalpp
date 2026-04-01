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

# Some race condition makes the CI sometimes fail on this step with "resource busy"
# This fix is from https://github.com/create-dmg/create-dmg/pull/139
unmounting_attempts=0
until
    echo "Unmounting disk image..."
    (( unmounting_attempts++ ))
    hdiutil detach "$TMP_MOUNTPOINT"
    exit_code=$?
    (( exit_code ==  0 )) && break            # nothing goes wrong
    (( exit_code != 16 )) && exit $exit_code  # exit with the original exit code
    # The above statement returns 1 if test failed (exit_code == 16).
    #   It can make the code in the {do... done} block to be executed
do
    (( unmounting_attempts == 3 )) && exit 16  # patience exhausted, exit with code EBUSY
    echo "Wait a moment..."
    sleep $(( 1 * (2 ** unmounting_attempts) ))
done
unset unmounting_attempts

# hdituil convert refuses to run if the file already exists, so delete it
rm -f "$OUT_NAME"
hdiutil convert "$TMP_DMG" -format UDZO -o "$OUT_NAME"

# Cleanup
rm "$TMP_DMG"
