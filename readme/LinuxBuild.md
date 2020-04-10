# Xournal++ Linux Build

## Install dependencies

Xournal++ is programmed with c++17 and needs the <optional> header and one filesystem library, either the STL or the boost implementation.
Therefore it is required to install a compiler implementing those features.
We recommend g++-8 or clang-9 and above.

Please create pull requests (or file issues) if you have more precise dependencies.

Lua is needed for plugins, if it is missing, the plugins will be disabled.


### CMake Generator

The installation instructions don't assume any specific build tool (other than CMake), 
but they do require make, ninja, or another supported CMake generator. It is required 
that such a tool is installed in order to build xournalpp.

The minimum required CMake version is 3.10, but we recommend to use >=3.15.

### Distribution specific commands

#### For Arch
```bash
sudo pacman -S cmake gtk3 base-devel libxml2 cppunit portaudio libsndfile \
poppler-glib texlive-bin texlive-pictures gettext libzip
```

#### For Fedora/CentOS/RHEL:
```bash
sudo dnf install gcc-c++ cmake gtk3-devel libxml2-devel cppunit-devel portaudio-devel libsndfile-devel \
poppler-glib-devel texlive-scheme-basic texlive-dvipng 'tex(standalone.cls)' gettext libzip-devel
```

#### For Ubuntu/Debian:
````bash
sudo apt-get install cmake libgtk-3-dev libpoppler-glib-dev portaudio19-dev libsndfile-dev \
libcppunit-dev dvipng texlive libxml2-dev liblua5.3-dev libzip-dev
````

#### For openSUSE:
```bash
sudo zypper install cmake gtk3-devel cppunit-devel portaudio-devel libsndfile-devel \
texlive-dvipng texlive libxml2-devel libpoppler-glib-devel libzip-devel
```

#### For Solus:
```bash
sudo eopkg it -c system.devel
sudo eopkg it cmake libgtk-3-devel libxml2-devel poppler-devel libzip-devel \
portaudio-devel libsndfile-devel alsa-lib-devel cppunit-devel lua-devel
```

## Compiling

The basic steps to compile Xournal++ are:

```bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
cmake --build .
# For a faster build, set the flag -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

Use `cmake-gui ..` to graphically configure compilation.

With Cairo 1.16 PDF Bookmarks will be possible, but this Version is not yet
common available, therefore the Cairo PDF Export is without PDF Bookmarks.

The binary executable will be placed in the `build/src/` subdirectory.

## Packaging and Installation

### Creating Packages for Package Managers

Please ensure that the `translations` target has been built before
attempting to generate any package.
```bash
cmake --build . --target translations
```

After compilation, select which packages you want to generate (see the relevant
sections below) and then run the `package` target. The generated packages will
be located in `build/packages`. For example:

```bash
cmake .. -DCPACK_GENERATOR="TGZ;DEB"  # Generate .tar.gz and .deb packages
cmake --build . --target package
```

By default, a standalone `.tar.gz` package will be generated. For
distro-agnostic packaging platforms such as AppImages and Flatpaks, see the
relevant sections below.

#### .deb packages

```bash
cmake .. -DCPACK_GENERATOR="DEB" ..
cmake --build . --target package
```

#### .rpm packages

TODO

#### AppImage

The quickest way to generate an AppImage is to first generate the `.tar.gz`
package and then use that with the `azure-pipelines/util/build_appimage.sh`
script.

```bash
cmake .. -DPACK_GENERATOR="TGZ"
cmake --build . --target package
../azure-pipelines/util/build_appimage.sh
```

The `build_appimage.sh` script will automatically download LinuxDeploy, copy the
`.tar.gz` files and required libraries and resources into a `appimage_staging`
directory, and run LinuxDeploy on the prepared app dir.

By default, the `build_appimage.sh` script will copy the Adwaita GTK theme and
the Adwaita icon theme into the AppImage.

#### Flatpak

The Flatpak manifest for Xournal++ is located at
https://github.com/flathub/com.github.xournalpp.xournalpp, which should be
cloned into a separate directory before building.

```bash
git clone https://github.com/flathub/com.github.xournalpp.xournalpp xournalpp-flatpak
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

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build . --target install
./cmake/postinst configure
```

If you want to install Xournal++ systemwide directly from the build directory, run

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --build . --target install
./cmake/postinst configure
```
