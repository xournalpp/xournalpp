# Xournal++ Mac Build

## Install Homebrew
https://brew.sh/

````bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
````

## Install dependencies
````bash
brew install cmake pkg-config gtk+3 poppler-glib poppler librsvg adwaita-icon-theme
````

## Basic steps are:
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake .. -DENABLE_MATHTEX=OFF
make
````

## APP Building
Currently not ready, Xournal++ can be executed from Build directory with
````bash
./src/xournalpp
````

Work in progress

