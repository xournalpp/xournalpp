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
rm -rf ./setup
mkdir setup
mkdir setup/bin

cp ../build/src/xournalpp.exe ./setup/bin
ldd ../build/src/xournalpp.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" setup/bin/





