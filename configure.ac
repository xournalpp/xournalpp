dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Xournalpp, 0.1)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_HEADER_STDC

pkg_modules="gtk+-2.0 >= 2.18.3 poppler-glib >= 0.5.4 poppler >= 0.5.4 libglade-2.0 >= 2.6.4 gthread-2.0 >= 2.4.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

dnl Set PACKAGE DATA & DOC DIR
packagedatadir=share/${PACKAGE}
packagedocdir=doc/${PACKAGE}

dnl Subst PACKAGE_DATA_DIR.
NO_PREFIX_PACKAGE_DATA_DIR="${packagedatadir}"
AC_SUBST(NO_PREFIX_PACKAGE_DATA_DIR)
PACKAGE_DATA_DIR="${packageprefix}/${packagedatadir}"
AC_SUBST(PACKAGE_DATA_DIR)

IT_PROG_INTLTOOL([0.41.0])

GETTEXT_PACKAGE=xournalpp
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])

dnl Add the languages which your application supports here.
ALL_LINGUAS="`grep -v '^#' "$srcdir/po/LINGUAS" | tr '\n' ' '`"
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES(Makefile src/Makefile po/Makefile.in ui/Makefile ui/pixmaps/Makefile src/xoj-preview-extractor/Makefile)
AC_OUTPUT

