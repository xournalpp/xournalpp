# Windows Build

![Screenshot](./main-win.png?raw=true "Xournal++ Screenshot on Win10")

### Install Dependencies

Xournall requires the following software to build:

1. Install [MSYS2](https://www.msys2.org/) to a short path without spaces.
2. Install [NSIS](https://nsis.sourceforge.io/Download) to the standard directory.
3. Start Mingw-w64 64bit. (Always check if it says **MINGW64** - not 32bit and not MSYS2)

This will open a console. The following steps happen in this console:

### Update MSYS2

Do this multiple times, close the Terminal after each update

```sh
pacman -Syuu
```

### Install Git

```sh
pacman -S git
```

### Install Build tools

```sh
pacman -S mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  patch \
  make \
  mingw-w64-x86_64-imagemagick
```

If prompted, confirm or use all default values.

### Install dependencies

```sh
pacman -S mingw-w64-x86_64-poppler \
  mingw-w64-x86_64-gtk3 \
  mingw-w64-x86_64-libsndfile \
  mingw-w64-x86_64-libzip \
  mingw-w64-x86_64-lua \
  mingw-w64-x86_64-portaudio
```

If prompted, confirm or use all default values.

## Building and Testing

See [Compile.md](./Compile.md)

## Modify Path Environment Variable

Add `C:\msys64\mingw64\bin` and `C:\msys64\usr\bin` to the top of 
your PATH environment variable in the Windows Advanced system 
settings (assuming default installation folder for MSYS2). 

You can now run Xournal++ with
```sh
./xournalpp.exe
```
or package it in an installer (see below).

## Packaging and Setup

Create the installer with
```sh
./windows-setup/package.sh
```

The installer will be located at `windows-setup/xournalpp-setup.exe`. This
command will also create a portable version of Xournal++ located in
`windows-setup/dist`.
