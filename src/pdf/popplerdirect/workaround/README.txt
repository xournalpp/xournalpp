This workaround is for Poppler installations without CairoOutputDev.h.

It's not really a good workaround. If there is no CairoOutputDev.h we use
the header files from here, which may not match the current installed Poppler version.

Andreas Butti