## Build Xournal++ .app
Do not install macports or homebrew. If you have installed it, you need to
create a new user, and use this for the whole process. jhbuild does not work,
if there is such an environment installed.

### Make sure the Development environment is installed
Open a Terminal, and type in **git**, confirm popup from Appstore with "Install" to install development tools.

### Build Libraries, needs to be once

#### 1. Build GTK
Execute in this folder.
````bash
./build-gtk3.sh
````

The build will fail on expat:
````
configure: error: C compiler cannot create executables
````
* Press 4 to start a shell
* Copy the command from above: ./configure --prefix /Users/..../gtk/inst
* Execute configure, it will work now
* Execute make install
* exit

Press 2 to ignore the error, as the build was manually successfully executed

The build will fail again. (missing python module six)
Download from here: https://pypi.org/project/six/
Execute
````bash
$HOME/gtk/inst/bin/python setup.py install
````

Build again. It should now build
````bash
./build-gtk3.sh
````

#### 2. Build Poppler
Execute in this folder.
````bash
./build-poppler.sh
````

#### 3. Build Mac integration
Execute in this folder.
````bash
./build-mac-integration.sh
````

#### 4. PortAudio

````bash
./build-portaudio.sh
````

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

#### 5. sndfile

````bash
./build-sndfile.sh
````


### Build Xournal++ and .app
````bash
complete-build.sh
````

Technical it does:

#### Build Xournal++
````bash
export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j 4
make install
````

#### Build App
````bash
./build-app.sh
````
