#!/bin/bash
## Build launcher

echo "Create icon for start menu"
ICON_ORIGIN="../ui/pixmaps/com.github.xournalpp.xournalpp.svg"
ICON_TARGET="setup/xournalpp.ico"

convert -resize 128x128 "$ICON_ORIGIN" "$ICON_TARGET"

if [[ ! -f "$ICON_TARGET"  ]]; then
	echo "Failed to create icon xournalpp.ico! "
fi

unamestr=$(uname)
if [[ "$unamestr" == 'Linux' ]]; then
	g++ xournalpp_launcher.cpp -o xournalpp_launcher
else
	windres xpp.rc -O coff -o xpp.res
	g++ xournalpp_launcher.cpp xpp.res -o setup/bin/xournalpp.exe -s -Os -mwindows
fi

