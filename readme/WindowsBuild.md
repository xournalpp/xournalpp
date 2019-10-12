# Xournal++ Windows Build

**Windows Build is working now, but Xournal++ is not yet fully supported
on Windows.**

See also [Linux Build](LinuxBuild.md)

Pull requests with fixes to the Code **and to this manual** are welcome!
This manual is not yet completed.


![Screenshot](main-win.png?raw=true "Xournal++ Screenshot on Win10")

## Preparation
Install [MSYS2](https://www.msys2.org/) to a short path without spaces.
Install [NSIS](https://nsis.sourceforge.io/Download) to the standard Folder.

Start Mingw-w64 64bit. (Always check if it says **MINGW64** - not 32bit and not MSYS2)

Update MSYS2 (do this multiple times, close the Terminal after each update)

```bash
pacman -Syuu
```

## Install GIT
```bash
pacman -S git
```

## Install Build tools
```bash
pacman -S mingw-w64-x86_64-toolchain \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-make \
          mingw-w64-x86_64-ninja \
          patch \
          make \
          mingw-w64-x86_64-cppunit
```
-> press enter multiple times / confirm all default values

## Install dependencies

```bash
pacman -S mingw-w64-x86_64-poppler \
          mingw-w64-x86_64-gtk3 \
          mingw-w64-x86_64-libsndfile \
          mingw-w64-x86_64-libzip
```
-> press enter multiple times / confirm all default values

## Get sources

```bash
git clone https://github.com/xournalpp/xournalpp.git
cd xournalpp/
```

## sndfile / PortAudio
Build/Install portaudio with
```bash
windows-setup/build-portaudio.sh
```

### Lua
Build/Install lua with
```bash
windows-setup/build-lua.sh
```

## Build
```bash
mkdir build
cd build/
cmake ..
cmake --build .
```

You can run Xournal++ with
```bash
./src/xournalpp.exe
```
or package it in an installer (see below).

## Packaging and Setup
Create the installer with
```bash
windows-setup/build-setup.sh
```

The installer will be located at `windows-setup/xournalpp-setup.exe`
