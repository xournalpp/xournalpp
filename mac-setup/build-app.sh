#!/bin/bash

## Mac Setup script
## 1. do the build, will not be called from this script
## 2. call this script
## 3. an .app will be packed

# go to script directory
cd "${0%/*}"

# delete old app, if there
echo "clean old app"

export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

rm -rf ./Xournal++.app
rm ./Xournal++.zip

echo "prepare gtk-mac-bundler"
if [ ! -d "gtk-mac-bundler" ]; then
  git clone https://gitlab.gnome.org/GNOME/gtk-mac-bundler.git
  cd gtk-mac-bundler
else
  cd gtk-mac-bundler
  git pull
fi

make install
cd ..

echo "create package"

gtk-mac-bundler xournalpp.bundle

mkdir -p Xournal++.app/Contents/Resources

export bundle_etc="./Xournal++.app/Contents/Resources/etc"
export GTK_IM_MODULE_FILE="$bundle_etc/gtk-2.0/gtk.immodules"
export GDK_PIXBUF_MODULE_FILE="$bundle_etc/gtk-2.0/gdk-pixbuf.loaders"

mkdir -p ./Xournal++.app/Contents/Resources/etc/gtk-2.0/
gdk-pixbuf-query-loaders > ./Xournal++.app/Contents/Resources/etc/gtk-2.0/gdk-pixbuf.loaders
sed -i -e "s:$HOME/gtk/inst/:@executable_path/../Resources/:g" ./Xournal++.app/Contents/Resources/etc/gtk-2.0/gdk-pixbuf.loaders

echo "Copy UI"

cp -rp ../ui ./Xournal++.app/Contents/Resources/

supportedLocales=("cs" "de" "it" "pl" "zh" "zh_TW" "zh_HK")
for locale in "${supportedLocales[@]}" ; do
	echo "Copy locale $locale"
	mkdir -p setup/share/locale/$locale/LC_MESSAGES

	# Xournal Translation
	cp -r ../build/po/$locale.gmo ./Xournal++.app/Contents/Resources/share/locale/$locale/LC_MESSAGES/xournalpp.mo
done

echo "Create zip"
zip -r Xournal++.zip Xournal++.app

echo "finished"
