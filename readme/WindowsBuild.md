# Xournal++ Windows Build

![Screenshot](main-win.png?raw=true "Xournal++ Screenshot on Win10")

## Preparation
1. Install [MSYS2](https://www.msys2.org/) to a short path without spaces.
2. Install [NSIS](https://nsis.sourceforge.io/Download) to the standard directory.
3. Start Mingw-w64 64bit. (Always check if it says **MINGW64** - not 32bit and not MSYS2)

This will open a console. All following steps happen in this console.

## Update MSYS2

Do this multiple times, close the Terminal after each update
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
          mingw-w64-x86_64-ninja \
          patch \
          mingw-w64-x86_64-cppunit \
          make
```
-> press enter multiple times / confirm all default values

## Install dependencies

```bash
pacman -S mingw-w64-x86_64-poppler \
          mingw-w64-x86_64-gtk3 \
          mingw-w64-x86_64-libsndfile \
          mingw-w64-x86_64-libzip \
          mingw-w64-x86_64-lua
```
-> press enter multiple times / confirm all default values

## Get sources

```bash
git clone https://github.com/xournalpp/xournalpp.git
cd xournalpp/
```

## Install sndfile / PortAudio

Build/Install portaudio with
```bash
windows-setup/build-portaudio.sh
```

## Build Xournal++

```bash
mkdir build
cd build/
cmake ..
cmake --build .
```
## Modify Path Environment Variable

Add `C:\msys64\mingw64\bin` and `C:\msys64\usr\bin` to the top of 
your PATH environment variable in the Windows Advanced system 
settings (assuming default installation folder for MSYS2). 

You can now run Xournal++ with
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
