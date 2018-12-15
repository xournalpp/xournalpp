# Xournal++ Windows Build

**Windows Build is working now, but Xournal++ is not yet fully supported
on Windows.**

See also [Linux Build](LinuxBuild.md)

Pull requests with fixes to the Code **and to this manual** are welcome!
This manual is not yet completed.


![Screenshot](main-win.png?raw=true "Xournal++ Screenshot on Win10")

## Preparation
Install MSYS2

Start Mingw-w64 64bit. (Always Check **64bit** not 32bit and not MSYS2)

Update MSYS (do this multiple times,
close Terminal after each update)

```bash
pacman -Syuu
```

## Install GIT
```bash
pacman -S git
```

## Install Build tools
```bash
pacman -S mingw-w64-x86_64-cmake \
pacman -S make \
pacman -S mingw-w64-x86_64-toolchain \
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
```
(this is a duplicate of the lines above, probably only this line is needed.
Can anybody confirm this?)
-> press enter multiple times / confirm all default values

```bash
pacman -S mingw-w64-x86_64-gcc
```

## Install dependencies

```bash
pacman -S mingw-w64-x86_64-boost \
pacman -S mingw-w64-x86_64-poppler \
pacman -S mingw-w64-x86_64-gtk3
```

## Get sources and build

```bash
git clone https://github.com/xournalpp/xournalpp.git
cd xournalpp/
mkdir build
cd build/
```

```bash
cmake ..
make
```

## Packaging and Setup
There is no Script yet. This two StackOverflow entries describe how it works.

https://stackoverflow.com/questions/49092784/how-to-distribute-a-gtk-application-on-windows
https://stackoverflow.com/questions/26738025/gtk-icon-missing-when-running-in-ms-windows/34673860#34673860

All libpixbufloader*.dll should be copied.

loaders.cache don't need to be changed.

But the dependencies of the libpixbufloader*.dll needs also to be copied into
the bin directory.



