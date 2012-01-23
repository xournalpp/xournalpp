dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Xournalpp, 0.9.1)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl librsvg-2.0 is not really needed by Xournal++, but if you don't have it crashes while loading the SVG icons (Problem with Gentoo)
pkg_modules="gtk+-2.0 >= 2.18.3 poppler-glib >= 0.5.4 poppler >= 0.5.4 libglade-2.0 >= 2.6.4 gthread-2.0 >= 2.4.0 librsvg-2.0 >= 2.14.0"

AC_MSG_CHECKING([whether Python support is requested])
AC_ARG_ENABLE([python],
	AS_HELP_STRING([--enable-python],[Enable python support]),
	[enable_python=$enableval have_python=$enableval],
	[enable_python=autodetect have_python=no])
AC_MSG_RESULT([$enable_python])

AC_MSG_CHECKING([whether Overlay Scrollbar is enabled])

AC_ARG_ENABLE([os],
	AS_HELP_STRING([--enable-os],[Enable Overlay Scrollbar support]),
	[enable_os=$enableval enable_os=$enableval],
	[enable_os=autodetect enable_os=no])
AC_MSG_RESULT([$enable_os])

if test "x$enable_os" != "xno"; then
	pkg_modules="$pkg_modules overlay-scrollbar-0.1 >= 0.1.12"
fi

PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)


dnl FIXME not working!!
dnl if test "x$have_python" != "xno"; then
dnl  	AM_CHECK_PYTHON_HEADERS([],[have_python=no])
dnl fi

if test "x$have_python" != "xno"; then
	PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
	PYTHON_LIBS="-lpython$PYTHON_VERSION"

	if test "$os_win32" = yes; then
		PYTHON_LIBS=`echo $PYTHON_LIBS | sed 's/\.//g'`
		PYTHON_LIB_LOC="-I$PY_EXEC_PREFIX/libs"
		PYTHON_CFLAGS="-I$PY_EXEC_PREFIX/include"
		PYTHON_EXTRA_LIBS=
	else
		PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
		PYTHON_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config"
		PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
		PYTHON_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config/Makefile"
		PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
		PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
		PYTHON_EXTRA_LIBS="$PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"
	fi

	AC_SUBST([PYTHON_LIBS])
	AC_SUBST([PYTHON_LIB_LOC])
	AC_SUBST([PYTHON_CFLAGS])
	AC_SUBST([PYTHON_EXTRA_LIBS])

	PACKAGE_CFLAGS="$PACKAGE_CFLAGS $PYTHON_CFLAGS"
	PACKAGE_LIBS="$PACKAGE_LIBS $PYTHON_LIBS $PYTHON_LIB_LOC $PYTHON_CFLAGS $PYTHON_EXTRA_LIBS"
fi

if test "x$have_python" != "xyes"; then
	if test "x$enable_python" = "xyes"; then
		AC_MSG_ERROR([Python not found])
	elif test "x$enable_python" = "xautodetect"; then
		enable_python=no
		AC_MSG_WARN([Python not found, disabling python support])
	fi
fi

if test "x$have_python" != "xno" -a "x$enable_python" != "xno"; then
	enable_python=yes
	AC_DEFINE([ENABLE_PYTHON],[1],[Define to compile with python support])
fi

AM_CONDITIONAL([ENABLE_PYTHON],[test "x$enable_python" = "xyes"])
AM_CONDITIONAL([ENABLE_OS],[test "x$enable_os" = "xyes"])


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

AC_CONFIG_FILES(Makefile src/Makefile src/util/Makefile po/Makefile.in ui/Makefile ui/pixmaps/Makefile src/xoj-preview-extractor/Makefile)
AC_OUTPUT

bash `dirname $0`/src/pdf/popplerdirect/workaround/workaround.sh

echo "

Configuration:

	Compiler:                  ${CC}
	Python Plugins Support:    $enable_python
	Overlay Scrollbar enabled: $enable_os
"


