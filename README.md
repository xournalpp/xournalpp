XOURNALPP
=====================

Description
---------------------

Xournalpp is a modified and extended Xournal. New features include:

* support for image insertion
* a better eraser
* reduced memory usage
* LaTeX support (requires a working LaTeX install and ```--enable-mathtex``` flag)
* advanced page sorting

Hopefully you enjoy it! Just build with

```bash
./configure
make
```

The binary will be in the src/ subdirectory.

```bash
make install
```

will then install it in your system path.
For more details on installation, visit the wiki at <a href="http://github.com/xournalpp/xournalpp/wiki">Xournalpp Wiki</a>.


Development
---------------------

The buildsystem directory contains php and xml files that are used to 
format the Makefiles used to build and compile this software. These needed to be
updated accordingly. 
I could not get the php buildsystem to work so the instructions above were what
worked for me.

See <a href="http://github.com/xournalpp/xournalpp">Github:xournalpp</a> for current development.
