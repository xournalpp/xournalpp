# Xournal++ Mac Build (development)

**To create an .app see [Mac Setup](../mac-setup/README.md)**

## Install Homebrew
https://brew.sh/

````bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
````

## Install dependencies
````bash
brew install cmake pkg-config gtk+3 poppler librsvg adwaita-icon-theme
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
