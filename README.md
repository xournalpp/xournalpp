# Xournal++

[![Build Status](https://travis-ci.org/xournalpp/xournalpp.svg?branch=string_new)](https://travis-ci.org/xournalpp/xournalpp)
[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Shout out
Recently we revisited the settings dialog to improve the feeling and usability.
While doing that we also added better descriptions, for which we require
new translations. If you would like to help us, please contact us by creating an issue
or write us on [Gitter](https://gitter.im/xournalpp/xournalpp)!

Partial translations, which need to be updated:
- Czech
- Polish
- Chinese

Full translations for all languages not mentioned previously except:
- English
- German
- Italian

Previous knowledge in creating translations using `gettext` would be nice but is not necessary.
We are happy to help you set everything up for translating.

**Thanks in advance!**

## Features

Xournal++ is a hand note taking software written in C++ with the target of flexibility, functionality and speed.
Stroke recognizer and other parts are based on Xournal Code, which you can find at [sourceforge](http://sourceforge.net/projects/xournal/)

Xournal++ features:
* Support for Pen preassure, e.g. Wacom Tablet
* Support for annotating PDFs
* Fill shape functionality
* PDF Export (with and without paper style)
* PNG Export (with and without transparent background)
* Allow to map different tools / colors etc. to stylus buttons / mouse buttons
* Sidebar with Page Previews with advanced page sorting, PDF Bookmarks and Layers (can be individually hidden, editing layer can be selected)
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
* Multi Language Support, Like English, German (Deutsch), Italian (Italiano)...
* Plugins using LUA Scripting

![Screenshot](readme/main.png?raw=true "Xournal++ Screenshot")

## Windows 10
![Screenshot](readme/main-win.png?raw=true "Xournal++ Screenshot on Win10")

## macOS High Sierra
![Screenshot](readme/main-mac.png?raw=true "Xournal++ Screenshot on macOS")

## Toolbar / Page Background / Layer
Multiple page background, easy selectable on the toolbar
![Screenshot](readme/background.png?raw=true "Xournal++ Screenshot")

Layer sidebar and advance Layer selection.
![Screenshot](readme/layer.png?raw=true "Xournal++ Screenshot")

Multiple predefined and fully customizeable Toolbar.
![Screenshot](readme/toolbar.png?raw=true "Xournal++ Screenshot")


## How to use audio record and playback feature:
- Go to `Edit > Preferences > Audio Recording` and set the `Audio Folder` as well as the appropriate `Input Device` and `Output Device`.

If you need to edit the recording audio gain, take a look at the preferences tab mentioned above.

**Please test this new feature in advance before relying on it to work. It could contain bugs specific to some hard-/software, which we have not yet found.**

### How to record
Just press the red button to start/stop recording and draw strokes using the `Pen` tool. The recording is associated with the drawn strokes and typed text while it is running.

### Play the recorded audio
Use the `Play Object` tool to click on a stroke or text node and listen to the corresponding audio. You can pause and stop the playback of the audio with the buttons next to the recording button in the toolbar.

## Installing
### Ubuntu and derivates
````bash
sudo add-apt-repository ppa:andreasbutti/xournalpp-master
sudo apt update
sudo apt install xournalpp
````

### OpenSuse
On openSUSE Tumbleweed, the released version of xournalpp is available from the main repository:
````bash
sudo zypper in xournalpp
````

For openSUSE Leap 15.0 and earlier, use the install link from [X11:Utilities](https://software.opensuse.org//download.html?project=X11%3AUtilities&package=xournalpp).

For all versions of openSUSE, bleeding edge packages synced to xournalpp git master on a weekly basis are available from [home:badshah400:Staging](https://software.opensuse.org//download.html?project=home%3Abadshah400%3AStaging&package=xournalpp).

### Arch Linux
The most recent stable release is available [in the [extra] repository](https://www.archlinux.org/packages/?q=xournalpp).

To build the latest state of the master branch yourself, use [this AUR package](https://aur.archlinux.org/packages/xournalpp-git/).

### Windows
**The windows Version has a Bug:**
Please start Xournal++, touch with the Pen, Quit Xournal++ and start again.
Then Pen input will be working, until you restart Windows. [#659](https://github.com/xournalpp/xournalpp/issues/659)

https://github.com/xournalpp/xournalpp/releases

### Mac OS X
Xournal++ will be deliverd with a patched GTK. Else pressure sensitivity is not working on Mac [#569](https://github.com/xournalpp/xournalpp/issues/569). (GTK-Issue)

https://github.com/xournalpp/xournalpp/releases

## Building

[Linux Build](readme/LinuxBuild.md)

[Mac Build](readme/MacBuild.md)

[Windows Build](readme/WindowsBuild.md)

## Fileformat
The fileformat *.xopp is an XML which is .gz compressed. PDFs are not embedded into the file, so if the PDF is deleted, the background is lost. *.xopp is basically the same fileformat as *.xoj, which is used by Xournal. Therefor Xournal++ reads *.xoj files, and can also export *.xoj. On exporting to *.xoj all Xournal++ specific Extension are lost, like addtional Background types.

*.xopp can theoretically be read by Xournal, as long as you do not use any new feature, Xournal does not open files at all if there are new attributes or unknown values, because of this Xournal++ will add the extension .xopp to all saved files.

All new files will be saved as *.xopp, if an *.xoj file is opened which was created by Xournal, the Save-As dialog will be displayed on save. If the *.xoj file was by Xournal++ created, Xournal++ overwrite the file on save, and does not change the extension.

**We are currently introducing a new file format that can efficiently store attached PDF files and other attachments internally. We will still allow for attachments that are linked to external files. Please refer to [#937](https://github.com/xournalpp/xournalpp/issues/937) for futher details.**

## Development
For developping new features, write a Ticket, so others know what you are doing.
For development create a fork, and use the master as base. Create a Pull request for each fix.
Do not create big pull requests, as long as you don't break anything features also can be
merged, even if they are not 100% finished.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via the badge on top.

Also take a look at our [Coding Conventions](https://github.com/xournalpp/xournalpp/wiki/Coding-conventions)

## FAQ
### Q: Secondary stylus button only works when there is no contact
This is due to a driver setting, which you can configure with `TPCButton` or `TabletPCButton` directive if using `Wacom` driver (but other drivers might have this setting too).

Here's a `/usr/share/X11/xorg.conf.d/30-wacom.conf` example snippet:
```
Section "InputClass"
	Identifier "Wacom tablets class"
	MatchProduct "Wacom"
	MatchDevicePath "/dev/input/event*"
	MatchIsTablet "on"
	Driver "wacom"
	Option "TabletPCButton" "on"
	Option "TPCButton" "on"
EndSection
```
If you are using `libinput` as driver, take a look at [#940](https://github.com/xournalpp/xournalpp/issues/940) and the ticket over at libinput which adresses this issue (https://gitlab.freedesktop.org/libinput/libinput/issues/193)

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
