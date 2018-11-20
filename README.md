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


![UI](/doc/ui/main.png "UI")

## How to use audio record and playback feature:

- Install `vlc` and make sure `curl` and `arecord` are present on your system 
- Copy the scripts under `launcher` folder to their corresponding absolute path
- Under `vlc` settings: 
- - enable `http interface` (see [this](https://github.com/azrafe7/vlc4youtube/blob/master/instructions/how-to-enable-vlc-web-interface.md) ) with blank username and password "password"
- - tick `Allow only one instance` under `Interface -> Instance`
- Launch `xournalpp` using the launcher I provided `xournalpp-ts-launcher.sh` 
- Choose `Toolbar Left` layout under `View->Toolbars->Toolbar Left`

You're ready to go! 
Just press the red button to start/stop recording and use the `Play Object` tool to click on a stroke and listen to the corresponding audio

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

Basic steps are: (need to compile with -fpermissive due to const library changes in poppler)
````bash
git clone http://github.com/morrolinux/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake -DCMAKE_CXX_FLAGS="-fpermissive" ..
make
````

If you're on Arch and you're having issues getting it to compile, please try to downgrade those two packages with `downgrade` command.

### Non-Arch users

If your build fails, try the following (I haven't tested it yet as I'm on Arch) (thanks to Gianluca Viganò)
- Download and replace those two files from the upstream:
- - xournalpp/src/config.h.in
- - xournalpp/src/mathtex/config.h.in
- Rename xournalpp/src/pdf/popplerdirect/workaround/poppler-0.62.0 to poppler-0.67.0 (should match your poppler version)
- Edit `poppler-0.67.0` occurrences accordingly inside xournalpp/src/pdf/popplerdirect/workaround/workaround.h.in 
- re-try from cmake command 

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
For development create a fork, and use the master as base. Create a Pull request for each fix.
Do not create big pull requests, as long as you don't break anything features also can be
merged, even if they are not 100% finished.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via badge on top.
