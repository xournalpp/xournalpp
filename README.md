# Xournal++

[![Build Status](https://travis-ci.org/xournalpp/xournalpp.svg?branch=string_new)](https://travis-ci.org/xournalpp/xournalpp)
[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# GTK-3 branch merged
Now the GTK-3 branch is merged, with really much changes.
**A lot of bugs are fixed, but maybe there are new one.**
The GTK-2 version is still available as master_gtk2_stable.


## Description

Xournal++ is not Xournal! It is a ground-up rewrite of Xournal in a different language (C++ instead of C). The purpose
was to create a more flexible application that significantly extends the functionality of Xournal.

At the moment, Xournal is very stable software while Xournal++ is not yet. If you want stability, you might be more
interested in the original Xournal project, which you can find at [sourceforge](http://sourceforge.net/projects/xournal/)
or, for some in-development features of Xournal, at [github.com/dmgerman/xournal](https://github.com/dmgerman/xournal).

New features in Xournal++ include:

* enhanced support for image insertion
* better eraser
* significantly reduced memory usage and code to detect memory leaks
* LaTeX support (requires a working LaTeX install and ```-DENABLE_MATHTEX=ON``` flag when configuring)
* advanced page sorting (a sidebar, page up/down, etc.)
* bug reporting, autosave, and auto backup tools

Hopefully you'll enjoy it!


## Building

Aside from legacy releases, this is currently the way to install Xournal++. For complete building documentation refer to wiki page:
[Installation](https://github.com/xournalpp/xournalpp/wiki/Installing).

There will be some binaries appearing in the future, so ideally you can make use of those as well.
They'll be at [Xournal++ releases](https://github.com/xournalpp/xournalpp/releases).

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
sudo apt-get install cmake libboost-all-dev libcppunit-dev dvipng texlive
liblcms2-dev libopenjpeg-dev libjpeg-dev fontconfig librsvg2-dev libgtk-3-dev
libpoppler-dev libpoppler-cpp-dev libpoppler-glib-dev libpoppler-private-dev
libxml2-dev
````

Basic steps are:
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
make
````

On Ubuntu 16.04, you may need to configure cmake with `-DBUILD_POPPLER=ON` due
to #234.

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
For development create a fork, and use the master as base.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via badge on top.
