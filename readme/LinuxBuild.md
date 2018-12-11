# Xournal++ Linux Build

Aside from legacy releases, this is currently the way to install Xournal++. For complete building documentation refer to wiki page:
[Installation](https://github.com/xournalpp/xournalpp/wiki/Installing).

There will be some binaries appearing in the future, so ideally you can make use of those as well.
They'll be at [Xournal++ releases](https://github.com/xournalpp/xournalpp/releases).

Current releases are out of Date, we are searching for Maintainer for PPA etc.
[Issue for Contact](https://github.com/xournalpp/xournalpp/issues/176)


## Install dependencies
### For Fedora/CentOS/RHEL:
````bash
dnf groups install "C Development Tools and Libraries"
dnf install cmake texlive-scheme-basic texlive-dvipng poppler-glib-devel
# AND SOME MORE please create Pull / write ticket if you have the exact dependencies
````

### For Ubuntu/Debian:
````bash
sudo apt-get install cmake libgtk-3-dev libpoppler-glib-dev \
libboost-filesystem-dev libboost-locale-dev libboost-iostreams-dev \
libcppunit-dev dvipng texlive 
````

## Basic steps are:
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
make
````

Use `cmake-gui ..` to see graphically all available options.

With `-DADVANCED_PDF_EXPORT_POPPLER=ON` the internal Poppler API is used, and
PDFs are exported with Bookmarks, but this is only working with some Poppler
Versions.
If not working, add `-DBUILD_POPPLER=ON` to build Poppler manually.

With Cairo 1.16 PDF Bookmarks will be possible, but this Version is not yet
common available, therefore the Cairo PDF Export is without PDF Bookmarks.

The binary executable will be in `build/src/` subdirectory.

To install all needed files execute:
```bash
make install
```

