# Linux Build

## Introduction

Xournal++ is programmed with C++17 and needs the `<optional>` header and one filesystem library, either the STL or the boost implementation.
Therefore it is required to install a compiler implementing those features.
We recommend using at least GCC 8 or Clang 9.

Please create file an issue or create a pull request if you require more precise dependencies.

Lua is needed for plugins; if it is missing, the plugins will be disabled.

### Install Dependencies

The minimum required CMake version is 3.13, but we recommend using >=3.15. Also, either `make` or `ninja` must be installed.

#### For Arch Linux:
```sh
sudo pacman -S cmake gtk3 base-devel libxml2 portaudio libsndfile \
  poppler-glib texlive-bin texlive-pictures gettext libzip lua53 lua53-lgi \
  gtksourceview4
```

#### For Fedora:
```sh
sudo dnf install gcc-c++ cmake gtk3-devel libxml2-devel portaudio-devel libsndfile-devel \
  poppler-glib-devel texlive-scheme-basic texlive-dvipng 'tex(standalone.cls)' gettext libzip-devel \
  librsvg2-devel lua-devel lua-lgi gtksourceview4-devel
```

#### For CentOS/RHEL:
```sh
sudo dnf install gcc-c++ cmake gtk3-devel libxml2-devel cppunit-devel portaudio-devel libsndfile-devel \
  poppler-glib-devel texlive-scheme-basic texlive-dvipng 'tex(standalone.cls)' gettext libzip-devel \
  librsvg2-devel gtksourceview4-devel
```

#### For Ubuntu/Debian and Raspberry Pi OS:
```sh
sudo apt-get install cmake libgtk-3-dev libpoppler-glib-dev portaudio19-dev libsndfile-dev \
  dvipng texlive libxml2-dev liblua5.3-dev libzip-dev librsvg2-dev gettext lua-lgi \
  libgtksourceview-4-dev
```

#### For openSUSE:
```sh
sudo zypper install cmake gtk3-devel portaudio-devel libsndfile-devel \
  texlive-dvipng texlive libxml2-devel libpoppler-glib-devel libzip-devel librsvg-devel lua-devel lua-lgi \
  gtksourceview4-devel
```

#### For Solus:
```sh
sudo eopkg it -c system.devel
sudo eopkg it cmake libgtk-3-devel libxml2-devel poppler-devel libzip-devel \
  portaudio-devel libsndfile-devel alsa-lib-devel lua-devel \
  librsvg-devel gettext libgtksourceview-devel
```

## Building and Testing

See [Compile.md](./Compile.md)

## Building Documentation

The code documentation is generated using Doxygen. Both Doxygen and graphviz must be
installed. For example, with apt:

```sh
sudo apt install doxygen graphviz
```

Then, execute `doxygen` in the root directory of the repository. The documentation
can be found in `doc/html` and `doc/latex`. Launch a server hosting the files with
`python3 -m http.server 8000` and visit the URL that the command shows.

## Packaging and Installation

### `.deb` packages

```sh
cmake .. -DCPACK_GENERATOR="DEB" ..
cmake --build . --target package
```

### `.rpm` packages

TODO

### AppImage

The quickest way to generate an AppImage is to first generate the `.tar.gz`
package and then use that with the `azure-pipelines/util/build_appimage.sh`
script.

```sh
cmake .. -DCPACK_GENERATOR="TGZ"
cmake --build . --target package
../azure-pipelines/util/build_appimage.sh
```

The `build_appimage.sh` script will automatically download LinuxDeploy, copy the
`.tar.gz` files and required libraries and resources into a `appimage_staging`
directory, and run LinuxDeploy on the prepared app dir.

By default, the `build_appimage.sh` script will copy the Adwaita GTK theme and
the Adwaita icon theme into the AppImage.

### Flatpak

The Flatpak manifest for Xournal++ is located at
https://github.com/flathub/com.github.xournalpp.xournalpp, which should be
cloned into a separate directory before building.

```sh
git clone https://github.com/flathub/com.github.xournalpp.xournalpp xournalpp-flatpak
cd xournalpp-flatpak
```

By default, the Flatpak manifest will build the latest stable version of
Xournal++. You can change the built version to a specific commit by editing the
commit information of the manifest to the desired commit (also specify tags if
building a stable version):

```diff
   - name: xournalpp
     buildsystem: cmake-ninja
     sources:
       - type: git
         url: https://github.com/xournalpp/xournalpp
-        commit: 14e9012b94e005112387dbb7d2ed59274d542885
-        tag: 1.0.10
+        commit: a911a3911df7c588c23997a29ad6a2e8d48b4aea
+        tag: 1.0.15
```

You can also build your local clone of Xournal++ by changing the source type to
`dir` and specifying the path to the clone.

### Installation from source

__We highly discourage installation from source__, as it may lead to issues when
upgrading to newer versions later on. Please think about creating a native
package, an AppImage or Flatpak instead. Instructions are above.

If you don't want to make a package, you can install Xournal++ into your user
folder (or any other folder) by specifying `CMAKE_INSTALL_PREFIX`:

```sh
cmake .. -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build . --target install
./cmake/postinst configure
```

If you want to install Xournal++ system-wide directly from the build directory, run

```sh
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --build . --target install
./cmake/postinst configure
```
