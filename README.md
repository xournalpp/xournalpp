# Xournal++

[![Build Status](https://travis-ci.org/xournalpp/xournalpp.svg?branch=string_new)](https://travis-ci.org/xournalpp/xournalpp)
[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Features

Xournal++ is a hand note taking software written in C++ with the target of flexibility, functionality and speed.
Stroke recognizer and other parts are based on Xournal Code, which you can find at [sourceforge](http://sourceforge.net/projects/xournal/)

Xournal++ features:
* Support for Pen preassure, e.g. Wacom Tablet
* Support for annotating PDFs
* Allow to map different tools / colors etc. to stylus buttons / mouse buttons
* Sidebar with Page Previews with advanced page sorting, PDF Bookmarks and Layers (Layers currently not enabled by default)
* enhanced support for image insertion
* Eraser with multipe configurations
* Significantly reduced memory usage and code to detect memory leaks compared to Xournal
* LaTeX support (requires a working LaTeX install and ```-DENABLE_MATHTEX=ON``` flag when configuring)
* bug reporting, autosave, and auto backup tools
* Customizeable toolbar, with multiple configurations, e.g. to optimize toolbar for portrait / landscape
* Page Template definitions
* Shape drawing (line, arrow, circle, rect)

![Screenshot](readme/main.png?raw=true "Xournal++ Screenshot")

Hopefully you'll enjoy it!


## Building

Aside from legacy releases, this is currently the way to install Xournal++. For complete building documentation refer to wiki page:
[Installation](https://github.com/xournalpp/xournalpp/wiki/Installing).

There will be some binaries appearing in the future, so ideally you can make use of those as well.
They'll be at [Xournal++ releases](https://github.com/xournalpp/xournalpp/releases).

Current releases are out of Date, we are searching for Maintainer for PPA etc.
[Issue for Contact](https://github.com/xournalpp/xournalpp/issues/176)


### Install dependencies
For Fedora/CentOS/RHEL:
```diff
- Probably not correct for new GTK3 build, create a pull or a Ticket if you have the correct list
```
````bash
dnf groups install "C Development Tools and Libraries"
dnf install cmake libglade2-devel texlive-scheme-basic texlive-dvipng glibmm24-devel gtk2-devel gtk+-devel boost boost-devel poppler-glib-devel
````

For Ubuntu/Debian:
````bash
sudo apt-get install cmake libboost-all-dev libcppunit-dev dvipng texlive \
liblcms2-dev libjpeg-dev fontconfig librsvg2-dev libgtk-3-dev \
libpoppler-dev libpoppler-cpp-dev libpoppler-glib-dev libpoppler-private-dev \
libxml2-dev libopenjpeg-dev
````
(On Ubuntu 18.04, remove the last `libopenjpeg-dev`, it's not in the repository any more).

Basic steps are:
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
make
````

On Ubuntu 16.04, `-DBUILD_POPPLER=ON` is automatically added due to #234.

If you'd like to enable mathtex support you should add `-DENABLE_MATHTEX=ON` to cmake command or use `cmake-gui ..`
to see graphically all available options. However, this should already be enabled by default.

The binary executable will be in `build/src/` subdirectory.

To install all needed files execute:
```bash
make install
```

If you want to install desktop file and thumbnailer execute:
```bash
make desktop-install
```

## Development
For developping new features, write a Ticket, so others know what you are doing.
For development create a fork, and use the master as base. Create a Pull request for each fix.
Do not create big pull requests, as long as you don't break anything features also can be
merged, even if they are not 100% finished.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via badge on top.
