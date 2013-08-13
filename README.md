XOURNALPP
=====================

Description
---------------------

Xournalpp is a modified and extended Xournal. New features include:
	support for image insertion
	a better eraser
	reduced memory usage

Hopefully you enjoy it! Just build with

```bash
./configure
make
```

The binary will be in the src/ subdirectory.

```bash
make install
```

should also work.


Development
---------------------

The buildsystem directory contained php and xml files that are used to properly
format the Makefiles used to build and compile this software. These needed to be
updated accordingly. I removed it because I felt it was a hassle. Adding source 
files, I also found the po/ directory to contain useful as well as the Makefile.am 
in the src directory. I could not get the php buildsystem to work the way I wanted. 
I have a feeling automake is the way to go for this.

