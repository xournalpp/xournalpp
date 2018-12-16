#!/bin/bash
## Build launcher

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
	g++ xournalpp_launcher.cpp -o xournalpp_launcher
else
	windres xpp.rc -O coff -o xpp.res
	g++ xournalpp_launcher.cpp xpp.res -o setup/bin/xournalpp.exe -mwindows
fi

