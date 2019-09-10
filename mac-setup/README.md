# Build Xournal++ .app
Do not install macports or homebrew. If you have installed it, you need to
create a new user, and use this for the whole process. jhbuild does not work,
if there is such an environment installed.

One possible way to use jhbuild alongside brew is to unlink all brew modules before running jhbuild. After finishing the build they can be relinked. This is untested though and might fail.

## Make sure the Development environment is installed
Open a Terminal, and type in `git`, confirm popup from Appstore with "Install" to install development tools.

## Build Libraries (needs to be done once)
Please take the OS version dependent problems for each step into account. The errors will vary with different versions of the libraries. If you encounter a yet unknown error, feel free to add it to this set of instructions.

### 1. Build GTK
````bash
./build-gtk3.sh
````
#### Potential Errors
##### itstool (version 2.0.6 on macOS High Sierra 10.13)
itstool might fail in configure step searching for libxml2 python bindings.

Follow these steps to resolve the issue:
- Open a shell with option 4
- Change Python to version 2.7 with `export PYTHON=/usr/bin/python2.7`
- Run the configure step manually with `./configure --prefix $HOME/gtk/inst`
- Exit the shell with `exit`
- Continue the build with option 2

##### expat (macOS Mojave 10.14)
The build might fail on expat:
````bash
configure: error: C compiler cannot create executables
````

Follow these steps to resolve the issue:
- Open a shell with option 4
- Run the configure step manually with `./configure --prefix $HOME/gtk/inst`
- Exit the shell with `exit`
- Continue the build with option 2

### 2. Start a jhbuild shell
````bash
$HOME/.new_local/bin/jhbuild shell
````

### 3. Build Poppler
Execute in this folder.
````bash
./build-poppler.sh
````

### 4. Build PortAudio

````bash
./build-portaudio.sh
````

### 5. Build LibZip

````bash
./build-libzip.sh
````
#### Potential Errors
##### Unknown module (macOS Mojave 10.14)
**TODO this needs to be improved to meet the standard of this tutorial**

If there is an error like:
xcode-select: error: tool 'xcodebuild' requires Xcode, but active developer directory '/Library/Developer/CommandLineTools' is a command line tools instance

https://stackoverflow.com/questions/17980759/xcode-select-active-developer-directory-error/17980786#17980786
https://github.com/VCVRack/Rack/issues/144

**Because autoreconf may not run easy, may you change configure directly at Line 15866**
(search for "Could not find 10.5 to 10.12 SDK")

Add the following to Makefile.in at line 261

````
elif xcodebuild -version -sdk macosx10.13 Path >/dev/null 2>&1 ; then  
    mac_version_min="-mmacosx-version-min=10.5"
    mac_sysroot="-isysroot `xcodebuild -version -sdk macosx10.13 Path`"
elif xcodebuild -version -sdk macosx10.14 Path >/dev/null 2>&1 ; then  
    mac_version_min="-mmacosx-version-min=10.6"
    mac_sysroot="-isysroot `xcodebuild -version -sdk macosx10.14 Path`"
````

````bash
export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"
autoreconf -if && ./configure --enable-cxx --prefix=$HOME/gtk/inst
````

### 6. Build sndfile
**TODO There is currently no support for Opus files so recording and playback might fail.**
````bash
./build-sndfile.sh
````

### 7. Build adwaita icon theme
````bash
jhbuild build adwaita-icon-theme
````

## Build Xournal++ and package it as .app
````bash
./complete-build.sh $HOME/gtk
````

Technical it does:

### Build Xournal++
````bash
export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j 4
make install
````

### Build App
````bash
./build-app.sh $HOME/gtk
````
