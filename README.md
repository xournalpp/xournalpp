# Xournal++

[![Build Status](https://dev.azure.com/xournalpp/xournalpp/_apis/build/status/CI?branchName=master)](https://dev.azure.com/xournalpp/xournalpp/_build/latest?definitionId=1&branchName=master)
[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

<table border="0px" ><tr><td width = 600px>

<img src="readme/main.png" width=550px% title="Xournal++ Screenshot on Linux"/>

</td><td>

## Shout out - Translators Needed!

Recently we revisited the settings dialog to improve the feeling and usability.
While doing that we also added better descriptions, for which we require
new translations.

Partial translations, which need to be updated:

- Czech
- Polish
- Chinese

Full translations for all languages not mentioned previously **except**:

- English
- German
- Italian

If you would like to help us improve the localization of Xournal++ take a look at [our Crowdin project](https://crowdin.com/project/xournalpp). If you are interested in translating a new language, contact us on [Gitter](https://gitter.im/xournalpp/xournalpp) or create a new issue and we will unlock the language on Crowdin.

**Thanks in advance!**

</td></tr></table>

## Features

Xournal++ is a hand note taking software written in C++ with the target of flexibility, functionality and speed.
Stroke recognizer and other parts are based on Xournal Code, which you can find at [sourceforge](http://sourceforge.net/projects/xournal/)

Xournal++ features:

- Support for pen pressure, e.g. Wacom Tablet
- Support for annotating PDFs
- Fill shape functionality
- PDF Export (with and without paper style)
- PNG Export (with and without transparent background)
- Allow to map different tools / colors etc. to stylus buttons / mouse buttons
- Sidebar with Page Previews with advanced page sorting, PDF Bookmarks and Layers (can be individually hidden, editing layer can be selected)
- enhanced support for image insertion
- Eraser with multiple configurations
- Significantly reduced memory usage and code to detect memory leaks compared to Xournal
- LaTeX support (requires a working LaTeX install)
- bug reporting, auto-save, and auto backup tools
- Customizable toolbar, with multiple configurations, e.g. to optimize toolbar for portrait / landscape
- Page Template definitions
- Shape drawing (line, arrow, circle, rect, splines)
- Shape resizing and rotation
- Rotation snapping every 15 degrees
- Rect snapping to grid
- Audio recording and playback alongside with handwritten notes
- Multi Language Support, Like English, German (Deutsch), Italian (Italiano)...
- Plugins using LUA Scripting

## Mobile & web app

Since mid 2020, there is a Flutter-written mobile app for **Android**, **Chrome OS** and **iOS** (in coming) as well as a **web app** available. Even though it is not perfectly stable nor every of Xournal++'s features is supported yet, you may check it out and open your Xournal++ notebooks on your mobile devices. You can get in touch in it's [separate repository on GitLab](https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile).

[Get it on Google Play](https://play.google.com/store/apps/details?id=online.xournal.mobile)

The web app is available at [xournal.online](https://xournal.online).

_Why is the iOS app not published yet?_

According to the Apple App Store guidelines, it is prohibited to publish unstable or beta apps. Hence we wait until Xournal++ Mobile works more stable and offers more complete feature compatibility to Xournal++.

<table>
<tr>
<td>

## Linux

<img src="readme/main.png" width=100% title="Xournal++ Screenshot on Linux"/>

</td><td>

## Windows 10

<img src="readme/main-win.png" width=100% title="Xournal++ Screenshot on Win10"/>

</td></tr><tr><td>

## macOS High Sierra

<img src="readme/main-mac.png" width=100% title="Xournal++ Screenshot on macOS"/>

</td><td>

## Xournal++ Mobile

<img src="https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile/-/raw/master/fastlane/metadata/android/en_US/images/tenInchScreenshots/03.png" width=100% title="Xournal++ Mobile Screenshot on Chromium OS"/>

</td></tr><tr><td>

## Toolbar / Page Background / Layer

Multiple page background, easy selectable on the toolbar
<img src="readme/background.png" width=100% title="Xournal++ Screenshot"/>

</td><td>

## Layer sidebar and advance Layer selection.

<img src="readme/layer.png" width=100% title="Xournal++ Screenshot"/>

</td></tr><tr><td>

## Multiple predefined and fully customizable Toolbar.

<img src="readme/toolbar.png" width=100% title="Xournal++ Screenshot"/>

</td></tr></table>

## User Manual and FAQ

For general usage, consult the [User
Manual](https://github.com/xournalpp/xournalpp/wiki/User-Manual). Answers to
some common questions can be found in the
[FAQ](https://github.com/xournalpp/xournalpp/wiki/Frequently-Asked-Questions-&-Problem-Solving).

## Experimental Features:

Sometimes a feature is added that might not be rock solid, or the developers aren't sure it is useful.
Try these out and give us some feedback.

Here are a few under development that you can play with now.

- <img src="readme/floatingtoolboxmbmenu.png"  title="Xournal++ Screenshot"/> Assign a mouse button or stylus button to bring up a toolbox of toolbars right under the cursor. You can also modify what is in the toolbox through the usual View->Toolbars->Customize although **it won't appear unless you've assigned a button in preferences: mouse or stylus** ( or selected a toolbar configuration that uses it).

  - This is an experimental feature because not everything you can put in the toolbox behaves. So be aware.

    <img src="readme/floatingtoolbox.png" width=25% />

* Keep your eyes out for other experimental features in preferences as seen here:

  DrawingTools: When drawing a box, circle etc simulate ctrl or shift modifiers by the initial direction you move the mouse.

  Action on Tool Tap: Allow a brief tap on the screen to bring up the floating toolbox and/or select an object. May work with pen and highlighter only.

   <img src="readme/moreexperimentals.png" width=50% />

## Installing

The official releases of Xournal++ can be found on the
[Releases](https://github.com/xournalpp/xournalpp/releases) page. We provide
binaries for Debian (Buster), Ubuntu (16.04), MacOS (10.13 and newer), and
Windows. For other Linux distributions (or older/newer ones), we also provide an
AppImage that is binary compatible with any distribution released around or
after Ubuntu 16.04. For installing Xournal++ Mobile on handheld devices, please check out [Xournal++ Mobile's instructions](https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile#try-it-out)

**A note for Ubuntu/Debian users**: The official binaries that we provide are
only compatible with the _specific version of Debian or Ubuntu_ indicated by the
file name. For example, if you are on Ubuntu 20.04, the binary whose name
contains `Ubuntu-xenial` is _only_ compatible with Ubuntu 18.04. If your system
is not one of the specific Debian or Ubuntu versions that are supported by the
official binaries, we recommend you use either the PPA, the Flatpak, or the
AppImage.

There is also an _unstable_, [automated nightly
release](https://github.com/xournalpp/xournalpp/releases/tag/nightly) that
includes the very latest features and bug fixes.

With the help of the community, Xournal++ is also available on official repositories
of some popular Linux distros and platforms.

### Ubuntu and derivatives

#### Stable PPA
The latest stable version is available via the following [_unofficial_ PPA](https://github.com/xournalpp/xournalpp/issues/1013#issuecomment-692656810):

```bash
sudo add-apt-repository ppa:apandada1/xournalpp-stable
sudo apt update
sudo apt install xournalpp
```

#### Unstable PPA
An _unstable_, nightly release is available for Ubuntu-based distributions via the following PPA:

```bash
sudo add-apt-repository ppa:andreasbutti/xournalpp-master
sudo apt update
sudo apt install xournalpp
```

This PPA is provided by the Xournal++ team. While it has the latest features and
bug fixes, it has also not been tested thoroughly and may break periodically (we
try our best not to break things, though).

### Fedora

The [released version of
xournalpp](https://src.fedoraproject.org/rpms/xournalpp) is available in the
[main repository](https://bodhi.fedoraproject.org/updates/?packages=xournalpp)
via _Software_ application or the following command:

```bash
sudo dnf install xournalpp
```

or

```bash
pkcon install xournalpp
```

The bleeding edge packages synced to xournalpp git master on a daily basis are available from [COPR luya/xournalpp](https://copr.fedorainfracloud.org/coprs/luya/xournalpp/).
[![Copr build status](https://copr.fedorainfracloud.org/coprs/luya/xournalpp/package/xournalpp/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/luya/xournalpp/package/xournalpp/)

### openSUSE

On openSUSE Tumbleweed, the released version of Xournal++ is available from the
main repository:

```bash
sudo zypper in xournalpp
```

For openSUSE Leap 15.0 and earlier, use the install link from
[X11:Utilities](https://software.opensuse.org//download.html?project=X11%3AUtilities&package=xournalpp).

For all versions of openSUSE, bleeding edge packages synced to xournalpp git
master on a weekly basis are available from
[home:badshah400:Staging](https://software.opensuse.org//download.html?project=home%3Abadshah400%3AStaging&package=xournalpp).

### Arch Linux

The latest stable release is available [in the [extra]
repository](https://www.archlinux.org/packages/?q=xournalpp).

To build the latest state of the master branch yourself, use [this AUR
package](https://aur.archlinux.org/packages/xournalpp-git/).

### Solus

The latest stable release is available in the main repository:

```bash
sudo eopkg it xournalpp
```

### Flatpak

The Xournal++ team officially supports a [FlatHub
release](https://flathub.org/apps/details/com.github.xournalpp.xournalpp), which
can be installed with

```bash
flatpak install flathub com.github.xournalpp.xournalpp
```

Note that for Xournal++ to work properly, you must have at least one GTK theme
and one icon theme installed on Flatpak. To enable LaTeX support, you will also
need to install the TeX Live extension:

```bash
flatpak install flathub org.freedesktop.Sdk.Extension.texlive
```

The Flatpak manifest can be found at the [Xournal++ Flatpak packaging
repository](https://github.com/flathub/com.github.xournalpp.xournalpp), and all
Flatpak-related packaging issues should be reported there.

### Android and Chrome OS

Android is supported by Xournal++ Mobile. It can be downloaded either on the [Tags page](https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile/-/tags) or [from Google Play](https://play.google.com/store/apps/details?id=online.xournal.mobile).

### iOS

Unfortunately, the iOS app is not published yet in the Apple App Store. See [here](#mobile--web-app) to learn, why. Anyway, in the [Building section](#building) you can learn how to build an early preview.

### Windows

Official Windows releases are provided on the [Releases
page](https://github.com/xournalpp/xournalpp/releases).

**Notes:**

- Currently, only WinTab drivers are supported. This is due to a limitation with
  the underlying library that we use, GTK.
- There is a GTK bug that prevents stylus input from working correctly. Please start
  Xournal++, touch with the stylus, quit Xournal++ and start again. Then stylus
  input will be working, until you restart Windows. See
  [#659](https://github.com/xournalpp/xournalpp/issues/659).

### Mac OS X

Mac OS X releases are provided on the [Releases
page](https://github.com/xournalpp/xournalpp/releases).

**Notes:**

- There have been compatibility problems with Mac OS X Catalina regarding both
  file permissions and stylus support
  ([#1772](https://github.com/xournalpp/xournalpp/issues/1772) and
  [#1757](https://github.com/xournalpp/xournalpp/issues/1757)). Unfortunately,
  we don't have the resources to adequately support Catalina at this time. Help
  would be appreciated!
- Xournal++ will be delivered with a patched GTK. Else pressure sensitivity will not work on Mac
  [#569](https://github.com/xournalpp/xournalpp/issues/569).

## Building

[Linux Build](readme/LinuxBuild.md)

[Mac Build](readme/MacBuild.md)

[Windows Build](readme/WindowsBuild.md)

[Android Build](https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile#getting-started)

[iOS Build](https://gitlab.com/TheOneWithTheBraid/xournalpp_mobile#getting-started)

## File format

The file format _.xopp is an XML which is .gz compressed. PDFs are not embedded into the file, so if the PDF is deleted, the background is lost. _.xopp is basically the same file format as _.xoj, which is used by Xournal. Therefor Xournal++ reads _.xoj files, and can also export _.xoj. On exporting to _.xoj all Xournal++ specific Extension are lost, like additional Background types.

\*.xopp can theoretically be read by Xournal, as long as you do not use any new feature, Xournal does not open files at all if there are new attributes or unknown values, because of this Xournal++ will add the extension .xopp to all saved files.

All new files will be saved as _.xopp, if an _.xoj file is opened which was created by Xournal, the Save-As dialog will be displayed on save. If the \*.xoj file was by Xournal++ created, Xournal++ overwrite the file on save, and does not change the extension.

**We are currently introducing a new file format that can efficiently store attached PDF files and other attachments internally. We will still allow for attachments that are linked to external files. Please refer to [#937](https://github.com/xournalpp/xournalpp/issues/937) for futher details.**

## Development

For developing new features, write a Ticket, so others know what you are doing.
For development create a fork, and use the master as base. Create a Pull request for each fix.
Do not create big pull requests, as long as you don't break anything features also can be
merged, even if they are not 100% finished.

See [GitHub:xournalpp](http://github.com/xournalpp/xournalpp) for current development. You can also join
our Gitter channel via the badge on top.

Also take a look at our [Coding Conventions](https://github.com/xournalpp/xournalpp/wiki/Coding-conventions)

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
