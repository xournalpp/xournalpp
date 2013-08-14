This workaround is for Poppler installations without CairoOutputDev.h.

It's not really a good workaround. If there is no CairoOutputDev.h we use
the header files from here, which may not match the current installed Poppler version.

Andreas Butti


I have altered this workaround for this branch a little. Xournalpp is now packaged with
the complete package for poppler-0.12.3, and workaround.sh will compile it and link it
statically. The problem was that poppler versions > 0.18 have a very different API and
a lot of the code will need to be changed to keep up. Note that this is a poor workaround
and will eventually have to be fixed.
