# Xournal++

[![Build Status](https://travis-ci.org/xournalpp/xournalpp.svg?branch=string_new)](https://travis-ci.org/xournalpp/xournalpp)
[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
<a href="https://scan.coverity.com/projects/xournal">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/17046/badge.svg"/>
</a>
(Currently run manually)


## Features

Xournal++ is a hand note taking software written in C++ with the target of flexibility, functionality and speed.
Stroke recognizer and other parts are based on Xournal Code, which you can find at [sourceforge](http://sourceforge.net/projects/xournal/)

Xournal++ features:
* Support for Pen preassure, e.g. Wacom Tablet
* Support for annotating PDFs
* PDF Export (with and without paper style)
* PNG Export (with and without transparent background)
* Allow to map different tools / colors etc. to stylus buttons / mouse buttons
* Sidebar with Page Previews with advanced page sorting, PDF Bookmarks and Layers (Layers currently not enabled by default)
* enhanced support for image insertion
* Eraser with multiple configurations
* Significantly reduced memory usage and code to detect memory leaks compared to Xournal
* LaTeX support (requires a working LaTeX install)
* bug reporting, autosave, and auto backup tools
* Customizeable toolbar, with multiple configurations, e.g. to optimize toolbar for portrait / landscape
* Page Template definitions
* Shape drawing (line, arrow, circle, rect)
* Shape resizing and rotation
* Rotation snapping every 45 degrees
* Rect snapping to grid 
* Audio recording and playback alongside with handwritten notes

![Screenshot](readme/main.png?raw=true "Xournal++ Screenshot")

Hopefully you'll enjoy it!

## How to use audio record and playback feature:

- Install `vlc` and make sure `curl` and `arecord` are present on your system 
- Make sure you installed xournalpp with `desktop-install` command (see install steps)
- Under `vlc` settings: 
- - enable `http interface` (see [this](https://github.com/azrafe7/vlc4youtube/blob/master/instructions/how-to-enable-vlc-web-interface.md) ) with blank username and password "password"
- - tick `Allow only one instance` under `Interface -> Instance`
- set the folder where do you want to store audio recordings under settings -> `audio recording`
- Choose `Toolbar Left` layout under `View->Toolbars->Toolbar Left`

You're ready to go! 
Just press the red button to start/stop recording and use the `Play Object` tool to click on a stroke and listen to the corresponding audio.

If you need to, you can edit the recording audio gain in the script under `/usr/local/bin/xopp-recording.sh` (an option for this will be availabe through xournalpp's settings in the future)

## Installing
### Ubuntu and derivates
````bash
sudo add-apt-repository ppa:andreasbutti/xournalpp-master
sudo apt update
sudo apt install xournalpp
````

### OpenSuse
https://build.opensuse.org/package/show/home:badshah400:Staging/xournalpp-gtk3

Build by https://github.com/badshah400

### Windows
**Windows is not fully tested.** If you find errors, please let us know over the
Issues page here on Github.
If you are a developer, working with Windows and would like to help us, please
contact also contact us over an Issue. 

https://github.com/xournalpp/xournalpp/releases

### Mac OS X
Work in Progress, it's possible to build, but no release yet.


## Building

[Linux Build](readme/LinuxBuild.md)

[Windows Build](readme/WindowsBuild.md)

## Fileformat
The fileformat *.xopp is an XML which is .gz compressed. PDFs are not embedded
into the file, so if the PDF is deleted, the background is lost.
*.xopp is basically the same fileformat as *.xoj, which is used by Xournal.
Therefor Xournal++ reads *.xoj files, and can also export *.xoj.
On exporting to *.xoj all Xournal++ specific Extension are lost, like addtional
Background types.
*.xopp can theretically be read by Xournal, as long as you do not use any new
feature, Xournal does not open files at all if there are new attributes or
unknown values, because of this Xournal++ will add the extension .xopp to all
saved files.
All new files will be saved as *.xopp, if an *.xoj file is opened which was
created by Xournal, the Save-As dialog will be displayed on save. If the *.xoj
file was by Xournal++ created, Xournal++ overwrite the file on save, and does
not change the extension.

## Development
For developping new features, write a Ticket, so others know what you are doing.
For development create a fork, and use the master as base. Create a Pull request for each fix.
Do not create big pull requests, as long as you don't break anything features also can be
merged, even if they are not 100% finished.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via badge on top.

## Code documentation

The code documentation is generated using Doxygen.

In order to generate the documentation yourself, first install Doxygen and graphviz, i.e.

```bash
sudo apt install doxygen
sudo apt install graphviz
```

on Debian or Ubuntu. Finally, type in `doxygen` in the root directory of the repository.
The documentation can be found in `doc/html` and `doc/latex`. Conveniently display the
documentation with `python3 -m http.server 8000` and visit the shown URL to view the
documentation.

