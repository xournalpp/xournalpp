# Xournal++ Linux Build

## Install dependencies

Please create pull requests (or file issues) if you have more precise dependencies.

### For Fedora/CentOS/RHEL:
```bash
sudo dnf install cmake gtk3-devel libxml2-devel cppunit-devel portaudio-devel libsndfile-devel \
poppler-glib-devel texlive-scheme-basic texlive-dvipng 'tex(standalone.cls)' gettext
```

### For Ubuntu/Debian:
````bash
sudo apt-get install cmake libgtk-3-dev libpoppler-glib-dev portaudio19-dev libsndfile-dev \
libcppunit-dev dvipng texlive libxml2-dev
````

### For OpenSuse:
```bash
sudo zypper install cmake gtk3-devel cppunit-devel portaudio-devel libsndfile-devel \
texlive-dvipng texlive libxml2-devel \
libpoppler-glib-devel
```


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

With Cairo 1.16 PDF Bookmarks will be possible, but this Version is not yet
common available, therefore the Cairo PDF Export is without PDF Bookmarks.

The binary executable will be in `build/src/` subdirectory.

To install all needed files execute:
```bash
sudo make install
```

