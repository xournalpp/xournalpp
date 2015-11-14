XOURNAL++
=====================

[![Join the chat at https://gitter.im/xournalpp/xournalpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/xournalpp/xournalpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Description
---------------------

Xournal++ is not Xournal! 
It is a ground-up rewrite of Xournal in a different language (C++ instead of C).
The purpose was to create a more flexible application that significantly extends the functionality of Xournal.
At the moment, Xournal is very stable software while Xournal++ is not.
If you want stability, you might be more interested in the original Xournal project, which you can find at
<a href="http://sourceforge.net/projects/xournal/">sourceforge</a> or, for some in-development features of Xournal, at
<a href="https://github.com/dmgerman/xournal">github.com/dmgerman/xournal</a>.

New features in Xournal++ include:

* enhanced support for image insertion
* a better eraser
* significantly reduced memory usage and code to detect memory leaks
* LaTeX support (requires a working LaTeX install and ```--enable-mathtex``` flag when configuring)
* advanced page sorting (a sidebar, page up/down, etc.)
* bug reporting, autosave, and auto backup tools

Hopefully you enjoy it! 


At the moment compiling Xournal++ from source is tricky business.
You should therefore first visit the wiki in order to see if there is a known working guide for your system:
<a href="https://github.com/xournalpp/xournalpp/wiki/">Xournal++ Wiki</a>.
There will be some binaries appearing in the near future, so ideally you can make use of those as well.
They'll be at <a href="https://github.com/xournalpp/xournalpp/releases">Xournal++ releases</a>.

If at a loss, you can try to build with
```bash
autoreconf
./configure --enable-mathtex
make
```
from the root directory (xournalpp/).
If you have libpoppler > 0.16, you will want to statically compile
against our own included libpoppler, which involves first checking
out the addpoppler branch with Git: ```git checkout addpoppler```,
followed by the above compilation.

The binary executable will be in the src/ subdirectory.

```bash
make install
```
will then install it in your system path (along with, provided everything works,
the mathtex-xournalpp executable for latex support).

Often problems with this method arise, especially due to autotools.
Some general strategies are to reconfigure autotools by running ```autoreconf```
in the xournalpp/ directory, before a ```./configure && make```.


Development
---------------------

The buildsystem directory contains php and xml files that are used to 
format the Makefiles used to build and compile this software. These needed to be
updated accordingly. 
I could not get the php buildsystem to work so the instructions above were what
worked for me.

By far the most hacked together part of development is the use of an internal
libpoppler package to compile static libpoppler.a files, and build them into the
final xournalpp executable. We had to do this because the libpoppler-dev install
on many systems just didn't include the headers we were used to access the PDF api.
Fortunately even though compile time is greatly increased, and source code size is large,
the final executable is still very reasonably sized, and it is much more portable in this way.

See <a href="http://github.com/xournalpp/xournalpp">Github:xournalpp</a> for current development.
