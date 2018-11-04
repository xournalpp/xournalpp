# Xournal++

This fork is intended to improve and customize xournalpp to my very own use cases and taste. 

## Changes so far

- [x] Improved pasting screenshots from clipboard (better image positioning an sizing)
- [x] Changed "graph" journal style color form blue to grey 
- [x] On the fly text size increasing/decreasing with CTRL + Alt + plus / minus
- [x] On the fly text bold toggle with CTRL + b
- [x] Go to next/prev. page with right/left arrows (instead of just pag. down / pag. up)
- [x] Save a timestamp for each stroke 
- [x] Record audio and have it play according to stroke timestamps (done via external sw for the moment)
- [ ] Zoom to the viewer's center (instead of top left corner of the page)


![UI](/doc/app/UI.png "UI")

## Description

Xournal++ is not Xournal! It is a ground-up rewrite of Xournal in a different language (C++ instead of C). The purpose
was to create a more flexible application that significantly extends the functionality of Xournal.

At the moment, Xournal is very stable software while Xournal++ is not. If you want stability, you might be more
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
````bash
dnf groups install "C Development Tools and Libraries"
dnf install cmake libglade2-devel texlive-scheme-basic texlive-dvipng glibmm24-devel gtk2-devel gtk+-devel boost boost-devel poppler-glib-devel
````

For Ubuntu/Debian:

````bash
sudo apt-get install cmake libboost-all-dev libcppunit-dev dvipng texlive
liblcms2-dev libopenjpeg-dev libjpeg-dev fontconfig librsvg2-dev libglade2-dev
libpoppler-dev libpoppler-cpp-dev libpoppler-glib-dev libpoppler-private-dev
````

Basic steps are: (need to compile with -fpermissive due to const library changes in poppler)
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake -DCMAKE_CXX_FLAGS="-fpermissive" ..
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

For now branches aren't organized too well, but (currently) the most of development happens in `development`.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via badge on top.
