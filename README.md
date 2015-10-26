# Xournal++

[![Build Status](https://travis-ci.org/xournalpp/xournalpp.svg?branch=string_new)](https://travis-ci.org/xournalpp/xournalpp)

If you would like to contribute in Xournal++ development you can join our Slack group at [xournalpp.slack.com](https://xournalpp.slack.com).

If you prefer not to register on Slack, you can always use our public IRC channel (which is integrated with Slack):

**Server:** irc.geekshed.net:6667

**Channel:** #xournalpp

Online IRC client: http://www.geekshed.net/chat/


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

Basic steps are:
````bash
git clone http://github.com/xournalpp/xournalpp.git
cd xournalpp
mkdir build
cd build
cmake ..
make
````

If you'd like to enable mathtex support you should add `-DENABLE_MATHTEX=ON` to cmake command or use `cmake-gui ..`
to see graphically all available options.

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

For now branches aren't organized too well, but (currently) the most of development happens in `string_new`.

See [Github:xournalpp](http://github.com/xournalpp/xournalpp) for current development (you can invite yourself
to this group following link on top of this page).

See our [Trello page](https://trello.com/xournalpp) for current roadmap and future development ideas.
