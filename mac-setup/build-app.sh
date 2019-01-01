#!/bin/bash

## Mac Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. an .app will be packed

# go to script directory
cd "${0%/*}"

# delete old app, if there
echo "clean old app"

rm -rf ./Xournal++.app

echo "prepare macdylibbundler"
if [ ! -d "macdylibbundler" ]; then
  git clone https://github.com/auriamg/macdylibbundler.git macdylibbundler
  cd macdylibbundler
else
  cd macdylibbundler
  git pull
fi

make -j 2

cd ..
echo "prepare package"

mkdir -p Xournal++.app/Contents/MacOS
cp ../build/src/xournalpp ./Xournal++.app/Contents/MacOS/xournalpp
./macdylibbundler/dylibbundler -od -b -x ./Xournal++.app/Contents/MacOS/xournalpp -d ./Xournal++.app/Contents/libs/

cp icon/xournalpp.icns ./Xournal++.app/Contents/Resources/xournalpp.icns
cp Info.plist ./Xournal++.app/Contents/Info.plist


echo "finished"
