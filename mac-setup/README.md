## Build Xournal++ .app
Do not install macports or homebrew.

Open a Terminal, and type in **git**, confirm popup from Appstore with "Install" to install development tools.

Build GTK3
./build-gtk3.sh

After first failure (missing python module six)
# download TODO link, unpack and install with
# /Users/yourname/gtk/inst/bin/python setup.py install

Build Xournal++
./build-app.sh


Work in progress
./configure --prefix /Users/andreas/gtk/inst --disable-gtk2-engine  
