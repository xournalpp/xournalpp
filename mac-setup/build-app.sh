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

mkdir -p Xournal++.app/Contents/Resources

export bundle_etc="./Xournal++.app/Contents/Resources/etc"
export GTK_IM_MODULE_FILE="$bundle_etc/gtk-2.0/gtk.immodules"
export GDK_PIXBUF_MODULE_FILE="$bundle_etc/gtk-2.0/gdk-pixbuf.loaders"

mkdir -p ./Xournal++.app/Contents/Resources/etc/gtk-2.0/
gdk-pixbuf-query-loaders >./Xournal++.app/Contents/Resources/etc/gtk-2.0/gdk-pixbuf.loaders
sed -i -e "s:$1/inst/:@executable_path/../Resources/:g" ./Xournal++.app/Contents/Resources/etc/gtk-2.0/gdk-pixbuf.loaders

echo "Copy GTK Schema"
mkdir -p ./Xournal++.app/Contents/Resources/share/glib-2.0/schemas
cp -rp "$1"/inst/share/glib-2.0/schemas ./Xournal++.app/Contents/Resources/share/glib-2.0/

echo "Copy UI"
cp -rp ../ui ./Xournal++.app/Contents/Resources/
sed -i -e 's/GDK_CONTROL_MASK/GDK_META_MASK/g' ./Xournal++.app/Contents/Resources/ui/main.glade

echo "Create zip"
zip -r Xournal++.zip Xournal++.app

echo "finished"
