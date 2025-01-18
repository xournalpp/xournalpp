# MacOS Build (.app)

This file describes how to build and package Xournal++ into a `.app` bundle.

If you are only interested in building Xournal++ for local development, see
the below sections on the dependency setup for Homebrew and Macports.

For packaging into a `.app` bundle, see the section on gtk-osx.

If you run into any problems, see the FAQ section at the bottom of this file to
see if there may be helpful advice there.

## Make sure the Development environment is installed

Before you use *any* of the methods listed above, you need to install developer
tools.

Run `xcode-select --install` in a terminal to install developer tools.

## Xournal++ Mac Homebrew Build

**We do not officially support builds with Homebrew. They are solely for convenience.**

It is highly recommended to either use an official or nightly release.
Should you still want to build your own version please refer to the app-build above.

### Install Homebrew
https://brew.sh/

````sh
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
````

### Install dependencies
````sh
brew install cmake ninja pkg-config gtk+3 poppler librsvg adwaita-icon-theme libzip portaudio libsndfile
````

### Build Xournal++:
````sh
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake .. -GNinja
cmake --build .
````

## Bundling Xournal++ into a `.app` with gtk-osx

This uses the official [gtk-osx][gtk-osx] project, which builds GTK using the
jhbuild tool and a curated set of dependencies that allows GTK to work on macOS.

**Warning**: The build process is flaky and may break at any time. It will
require debugging and patience.

For the easiest build experience, do not have MacPorts or Homebrew installed.
* If you have any of them installed, you need to create a new user, and use the
  new user for the whole process. This is because the jhbuild packages are meant
  to be self-contained, and environment variables from MacPorts or Homebrew
  configurations may interfere with the jhbuild package builds.
* Even if you create a new user, packages built by jhbuild may still "find"
  MacPorts or Homebrew packages. If this is the case, you can either uninstall
  the corresponding packages; or (for advanced users) you can clone gtk-osx
  and manually create a jhbuild _moduleset_ that explicitly ensures that such
  packages are disabled (see the FAQ).
* One possible way to use jhbuild alongside brew is to unlink all brew modules
  before running jhbuild. After finishing the build, they can be relinked.
  Note: this is untested though, and might fail.
* By default, gtk-osx will dump multiple folders such as `~/gtk`, `~/Sources`,
  etc. into the home directory. If you prefer having the folders in a different
  location, you can set the `HOME` environment variable before running these
  steps. NOTE: this could potentially break things since your shell rc files
  will not be loaded; change `HOME` at your own risk.

### A note on build dependencies

All of the following steps, except for the last one, only need to be done once.

Please take the OS version dependent problems for each step into account.
The errors will vary with different versions of the libraries.
If you encounter a yet unknown error, feel free to submit a pull request to add it to this set of instructions.

### Step 1: Install jhbuild

Before you begin, make sure you are logged in as the jhbuild user and are in the
home directory.

To install jhbuild, run the following commands. **Do not blindly copy and paste.**
Make sure you understand what each command does as you run them one-by-one.

```sh
# Download the setup script
curl -LR https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh -o gtk-osx-setup.sh

# If jhbuild is already installed with a different python version, you MUST delete it first.
rm -rf ~/gtk ~/.new_local ~/.config/jhbuildrc* ~/.cache/jhbuild ~/Source

# workaround for openssl 1.1.1q compilation error:
# https://github.com/openssl/openssl/issues/18720
export OPENSSL_CFLAGS='-Wno-error=implicit-function-declaration'

# Build jhbuild
# NOTE: if you are using macOS Ventura, please see the FAQ below.
bash gtk-osx-setup.sh

# permanently add jhbuild to path
echo 'export PATH="$HOME"/.new_local/bin:"$PATH"' >> ~/.zshrc
```

You will either need to relogin as the jhbuild user, or you will instead need to
execute the following to put jhbuild on `PATH`:

```sh
export PATH="$HOME"/.new_local/bin:"$PATH"
```

### Step 2: Set up custom modulesets and configure jhbuild

Make sure you are in the home directory for this step.

jhbuild uses "modulesets" to define what dependencies to use for its builds.
The Xournal++ macOS build defines its own moduleset (in
`mac-setup/xournalpp.modules`) that can be used to build Xournal++ dependencies
that are not already included in the `gtk-osx` project.

First, clone the gtk-osx repository:
```sh
git clone https://gitlab.gnome.org/GNOME/gtk-osx.git
```

Then add the following lines to your `~/.config/jhbuildrc-custom` to make
jhbuild use the modulesets in the repository you just cloned:
```python
use_local_modulesets = True
moduleset = "gtk-osx.modules"

# This assumes you cloned gtk-osx into your home directory.
# If you're using a custom location, please change this.
modulesets_dir = os.path.expanduser("~/gtk-osx/modulesets-stable")
```

To allow older versions of macOS to run the produced `.app` file, you should
replace a line in `~/.config/jhbuildrc-custom`:
```diff
- setup_sdk()
+ setup_sdk(target='10.15')
```

Lastly, add the following lines to `~/.config/jhbuildrc-custom` to work around
bugs or make the build process more robust.
```python
# Workaround for https://gitlab.gnome.org/GNOME/glib/-/issues/2759
module_mesonargs['glib'] = mesonargs + ' -Dobjc_args=-Wno-error=declaration-after-statement'

# Fix freetype build finding brotli installed through brew or ports, causing the
# harfbuzz build to fail when jhbuild's pkg-config cannot find brotli.
module_cmakeargs['freetype-no-harfbuzz'] = ' -DFT_DISABLE_BROTLI=TRUE '
module_cmakeargs['freetype'] = ' -DFT_DISABLE_BROTLI=TRUE '

# portaudio may fail with parallel build, so disable parallel building.
module_makeargs['portaudio'] = ' -j1 '
```

You may also need to modify `~/.config/jhbuildrc` as follows:
```
# On the lines that have
#     module_cmakeargs['freetype'] = ...
#     module_cmakeargs['freetype-no-harfbuzz'] = ...
#
# Change the '=' to '+='.
# Then insert the following lines *before* those lines:

module_cmakeargs.setdefault('freetype', cmakeargs + ' ')
module_cmakeargs.setdefault('freetype-no-harfbuzz', cmakeargs + ' ')
```

### Step 3: Build required dependencies from gtk-osx

Many of the dependencies (e.g., gtk) are already configured by gtk-osx, and can
be installed with:

```sh
# note: the following may fail on the first run.
jhbuild bootstrap-gtk-osx

# Now we can build all dependencies.
# Warning: the following steps compile a lot of code and may take up to 1 hour to run.
jhbuild bootstrap-gtk-osx
jhbuild build meta-gtk-osx-gtk3 gtksourceview3
```

### Step 4: Clone the Xournal++ code

If you haven't already, you'll need to clone the Xournal++ code in order to
compile it and its dependencies from source:

```sh
git clone http://github.com/xournalpp/xournalpp.git
```

### Step 5: Build required dependencies not in gtk-osx

Some other dependencies are not provided by gtk-osx.
We provide a jhbuild moduleset that defines how these dependencies will be
downloaded and built.

To build them, run the following command in the `mac-setup` folder of the
`xournalpp` repository you just cloned:

```sh
jhbuild -m xournalpp.modules build meta-xournalpp-deps
```

### Last step: build Xournal++ and package it as .app

From the `mac-setup` directory, this can be done completely automatically using:
```sh
./complete-build.sh "$HOME"/gtk
```

Or manually with:
```sh
# from the top-level folder

# build and install to local gtk prefix
mkdir -p build && cd build
cmake .. -GNinja -DCMAKE_INSTALL_PREFIX:PATH="$HOME"/gtk/inst
cmake --build .
cmake --install .
cd ..

# build application
cd mac-setup
jhbuild run bash ./build-app.sh "$HOME"/gtk
```

Once the `Xournal++.app` bundle is created, you can install it in your
Applications directory and run it.

At the time of writing, it is not possible to run the application from the
`build` folder (at least if you're using the special build user). If you have
found a nice way to get this to run, it would be great if you could contribute
the steps to this document!

### FAQ

#### gtk-osx-setup.sh fails on macOS Ventura (13.0 or newer)

Likely due to improved security protections on macOS Ventura, the
`gtk-osx-setup.sh` script will no longer install gtk-osx correctly. This can be
observed if running `jhbuild` or any associated commands results in a message
such as `zsh : killed`.

The reason why this error happens is that `gtk-osx-setup.sh` will copy
`/bin/bash` to `~/.new_local/bin/bash` so that it is always on `PATH`; however,
`~/.new_local/bin/bash` will always be killed immediately. This causes some
steps in `gtk-osx-setup.sh` to fail silently (e.g., `pyenv` which is used to run
`jhbuild`).

This issue has already been fixed in gtk-osx, but you may encounter it if you
are installing an older version of gtk-osx for whatever reason. To work around
this issue, you can edit `gtk-osx-setup.sh` and comment out the line `cp
/bin/bash "$DEVPREFIX/bash"` (or similar).

#### itstool build error (version 2.0.6 on macOS >= 10.13)
itstool might fail in configure step searching for libxml2 python bindings.

If you are using Python 3.10 (check with `jhbuild run python --version`), there
is a bug in `libxml2` that causes it to segfault when imported by itstool.

To work around this problem, you will need to reinstall jhbuild and gtk-osx,
starting from the very first step. Before you run the install steps, run the
following command to force Python 3.9 to be installed:

```sh
# Override Python version in setup script
export PYTHON_VERSION=3.9.15
```

#### Unknown module error

If there is an error like:
````sh
xcode-select: error: tool 'xcodebuild' requires Xcode, but active developer directory '/Library/Developer/CommandLineTools' is a command line tools instance
````

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

#### gobject-introspection fails to build

When running `jhbuild build gtk-osx-meta`, the following error may occur:

```sh
ModuleNotFoundError: No module named 'distutils.msvccompiler'
ninja: build stopped: subcommand failed.
*** Error during phase build of gobject-introspection: ########## Error running ninja   *** [20/33]
```

This is due to an issue in `gobject-introspection` where deprecated code from
Python's `setuptools` is being invoked:
https://gitlab.gnome.org/GNOME/gobject-introspection/-/issues/438

To work around this bug, run `jhbuild run pip install -U setuptools`. This
should install a version of setuptools that is newer than 65.0.2, which should
have the deprecated code back.

Confirm that the workaround works by seeing if this command prints "OK":
```sh
jhbuild run python3 -c 'import distutils.msvccompiler; print("OK")'
```


#### Modifying the custom moduleset to work around dependency errors

If you have MacPorts or Homebrew installed, the jhbuild packages may pick up on
packages outside of jhbuild during the build process. Consequently, this can
result in build or configure time failures.

To work around this problem, you can then manually edit the configure flags for
each package in `gtk-osx/modulesets-stable`, as needed. These flags will be
picked up when you run `jhbuild -afc <package_name>` to rebuild a given package
`<package_name>`.

Alternatively, you can edit `~/.config/jhbuildrc-custom` to add additional flags
to the build tool(s). For example, setting
```python
module_cmakeargs['poppler'] = " -DCMAKE_BUILD_TYPE=Debug "
```
will build poppler in debug mode. See
[this page](https://gnome.pages.gitlab.gnome.org/jhbuild/config-reference.html#config-reference-variables)
for a list of configuration variables.



[gtk-osx]: https://gitlab.gnome.org/GNOME/gtk-osx
