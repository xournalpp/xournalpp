# MacOS Build (.app)

Do not install with MacPorts or Homebrew. If you have installed it, you need to
create a new user, and use this for the whole process. jhbuild does not work,
if there is such an environment installed.

One possible way to use jhbuild alongside brew is to unlink all brew modules before running jhbuild. After finishing the build they can be relinked. This is untested though and might fail.

## Make sure the Development environment is installed
Open a Terminal, and type in `git`, confirm popup from the App Store with "Install" to install development tools.

## Build Libraries (needs to be done once)
Please take the OS version dependent problems for each step into account. The errors will vary with different versions of the libraries. If you encounter a yet unknown error, feel free to add it to this set of instructions.

### 1. Build GTK
```sh
./build-gtk3.sh
```
#### Potential Errors
##### itstool (version 2.0.6 on macOS High Sierra 10.13)
itstool might fail in configure step searching for libxml2 python bindings.

Follow these steps to resolve the issue:
1. Open a shell with option 4
2. Change Python to version 2.7 with `export PYTHON=/usr/bin/python2.7`
3. Run the configure step manually with `./configure --prefix "$HOME/gtk/inst"`
4. Exit the shell with `exit`
5. Continue the build with option 2

##### expat (macOS Mojave 10.14)
The build might fail on expat:
```sh
configure: error: C compiler cannot create executables
```

Follow these steps to resolve the issue:
1. Open a shell with option 4
2. Run the configure step manually with `./configure --prefix "$HOME/gtk/inst"`
3. Exit the shell with `exit`
4. Continue the build with option 2

### 2. Start a jhbuild shell
```sh
"$HOME"/.new_local/bin/jhbuild shell
```

### 3. Build Poppler
Execute in this folder.
```sh
./build-poppler.sh
```

### 4. Build PortAudio

```sh
./build-portaudio.sh
```

### 5. Build LibZip

```sh
./build-libzip.sh
```
#### Potential Errors
##### Unknown module (macOS Mojave 10.14)

If there is an error like:
```sh
xcode-select: error: tool 'xcodebuild' requires Xcode, but active developer directory'/Library/Developer/CommandLineTools' is a command line tools instance
```

Follow these steps to resolve the issue:
1. Install Xcode (get it from [here](https://developer.apple.com/xcode/)) if you don't have it yet.
2. Accept the Terms and Conditions.
3. Ensure Xcode app is in the /Applications directory (**NOT** /Users/{user}/Applications).
4. Point xcode-select to the Xcode app Developer directory using the following command:
5. sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
6. Make sure your Xcode app path is correct.
   1. Xcode: /Applications/Xcode.app/Contents/Developer
   2. Xcode-beta: /Applications/Xcode-beta.app/Contents/Developer
   
Steps are from this [source](https://stackoverflow.com/questions/17980759/xcode-select-active-developer-directory-error/17980786#17980786). A big thanks to tjmetha and Rob Bednark!

### 6. Build sndfile
**Recording and playing audio is not yet supported for MacOS.**
```sh
./build-sndfile.sh
```

### 7. Build adwaita icon theme
```sh
jhbuild build adwaita-icon-theme
```

### 8. Build GtkSourceView
```sh
jhbuild build gtksourceview3
```

## Build Xournal++ and package it as .app

### Automated step

```sh
./complete-build.sh $HOME/gtk
```

### Manual steps

#### Build Xournal++
```sh
export PATH="$HOME/.local/bin:$HOME/gtk/inst/bin:$PATH"

cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/gtk/inst ..
make -j 4
make install
```

#### Build App
```sh
./build-app.sh "$HOME/gtk"
```

## Xournal++ Mac Homebrew Build

**We do not officially support builds with Homebrew. They are solely for convenience.**

It is highly recommended to either use an official or nightly release.
Should you still want to build your own version please refer to the app-build above.

### Install dependencies

Homebrew must be installed. See [https://brew.sh](https://brew.sh) for installation instructions

```sh
brew install cmake pkg-config gtk+3 poppler librsvg adwaita-icon-theme libzip portaudio libsndfile
```

### Building and Testing

See [Compile.md](./Compile.md)
