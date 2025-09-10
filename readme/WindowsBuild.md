# Windows Build

![Screenshot](./main-win.png?raw=true "Xournal++ Screenshot on Win10")

## Install Dependencies

Xournal++ requires the following software to build:

1. Install [MSYS2](https://www.msys2.org/) to a short path without spaces.
2. Install [NSIS](https://nsis.sourceforge.io/Download) to the standard directory.

### Update MSYS2

Open a MSYS2 console (**not** the MINGW64 console) and run the following command twice. Reopen the MSYS2 console each time you run the command.

```sh
pacman -Syuu
```

### Install Build tools

Open a MINGW64 console (Always check if it says **MINGW64** - not 32bit and not MSYS2)

All following steps in this document happen in this console, unless specified otherwise.

```sh
pacman -S \
  mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-imagemagick \
  mingw-w64-x86_64-gettext \
  patch \
  make \
  git
```

If prompted, confirm or use all default values.

### Install dependencies

```sh
pacman -S \
  mingw-w64-x86_64-poppler \
  mingw-w64-x86_64-gtk3 \
  mingw-w64-x86_64-libsndfile \
  mingw-w64-x86_64-libzip \
  mingw-w64-x86_64-lua \
  mingw-w64-x86_64-portaudio \
  mingw-w64-x86_64-qpdf
```

If prompted, confirm or use all default values.

## Building and Testing

See [Compile.md](./Compile.md)

## Packaging and Setup

Create the installer with
```sh
./windows-setup/package.sh build/
```

The installer will be located at `build/xournalpp-setup.exe`. This
command will also create a portable version of Xournal++ located in
`build/dist`.
