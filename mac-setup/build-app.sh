#!/bin/bash
export PATH="$HOME/.new_local/bin:$PATH"

## Mac Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. an .app will be packed

if [ $# -eq 0 ]; then
  echo 'Please provide the path of your gtk installation'
  exit 1
fi

# go to script directory
cd "${0%/*}" || exit

# delete old app, if there
echo "clean old app"

export PATH="$HOME/.local/bin:$1/inst/bin:$PATH"

rm -rf ./Xournal++.app
rm ./Xournal++.zip

echo "prepare gtk-mac-bundler"
if [ ! -d "gtk-mac-bundler" ]; then
  git clone https://gitlab.gnome.org/GNOME/gtk-mac-bundler.git
  cd gtk-mac-bundler || exit
else
  cd gtk-mac-bundler || exit
  git pull
fi

make install
cd ..

echo "create package"

export GTKDIR="$1/inst"

gtk-mac-bundler xournalpp.bundle

echo "Replace GDK_CONTROL_MASK by GDK_META_MASK in main.glade"
sed -i -e 's/GDK_CONTROL_MASK/GDK_META_MASK/g' ./Xournal++.app/Contents/Resources/ui/main.glade

echo "Create zip"
zip -r Xournal++.zip Xournal++.app

echo "finished"
