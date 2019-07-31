# Xournal++ Linux Build

## Install dependencies

Please create pull requests (or file issues) if you have more precise dependencies.

Lua is needed for plugins, if it is missing, the plugins will be disabled.

### Arch
```bash
sudo pacman -S cmake gtk3 base-devel libxml2 cppunit portaudio libsndfile \
poppler-glib texlive-bin texlive-pictures gettext libzip
```

### For Fedora/CentOS/RHEL:
```bash
sudo dnf install gcc-c++ cmake gtk3-devel libxml2-devel cppunit-devel portaudio-devel libsndfile-devel \
poppler-glib-devel texlive-scheme-basic texlive-dvipng 'tex(standalone.cls)' gettext libzip-devel
```

### For Ubuntu/Debian:
````bash
sudo apt-get install cmake libgtk-3-dev libpoppler-glib-dev portaudio19-dev libsndfile-dev \
libcppunit-dev dvipng texlive libxml2-dev liblua5.3-dev libzip-dev
````

### For OpenSuse:
```bash
sudo zypper install cmake gtk3-devel cppunit-devel portaudio-devel libsndfile-devel \
texlive-dvipng texlive libxml2-devel libpoppler-glib-devel libzip-devel
```

### For Solus:
```bash
sudo eopkg it -c system.devel
sudo eopkg it cmake libgtk-3-devel libxml2-devel poppler-devel libzip-devel \
portaudio-devel libsndfile-devel alsa-lib-devel cppunit-devel lua-devel
```

## Compiling

The basic steps to compile Xournal++ are:

````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
cmake --build .
```

Use `cmake-gui ..` to graphically configure compilation.

With Cairo 1.16 PDF Bookmarks will be possible, but this Version is not yet
common available, therefore the Cairo PDF Export is without PDF Bookmarks.

The binary executable will be placed in the `build/src/` subdirectory.

### Packaging and Installation

After compilation, you can generate a `.tar.gz` file containing the binary and
everything it needs with

```bash
cmake --build . --package
```

If you don't want to make a package, you can install Xournal++ into your user
folder with

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build . --target install
./cmake/postinst configure
```

If you want to install Xournal++ systemwide directly from the build directory
(not recommended: generate a native package or an AppImage/Flatpak instead), run

```bash
sudo cmake --build . --target install
./cmake/postinst configure
```
