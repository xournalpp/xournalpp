dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Xournalpp, 1.0.2)
AM_CONFIG_HEADER(config.h)

dnl backwards compatibility for autoconf <= 2.64
dnl PACKAGE_URL should be the fifth argument of AC_INIT
m4_define([AC_PACKAGE_URL], [https://github.com/xournalpp/xournalpp])
AC_DEFINE(PACKAGE_URL, "AC_PACKAGE_URL", [Package URL])
AC_SUBST(PACKAGE_URL, AC_PACKAGE_URL)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_LIBTOOL

AC_LANG(C++)
AX_CXX_COMPILE_STDCXX_11

AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl librsvg-2.0 is not really needed by Xournal++, but if you dont have it crashes while loading the SVG icons (Problem with Gentoo)
#We also are compiling with libpoppler so we require a number of extra packages
pkg_modules="gtk+-2.0 >= 2.18.3 libglade-2.0 >= 2.6.4 glib-2.0 >= 2.32.0 gtkmm-2.4 >= 2.24.0 gthread-2.0 >= 2.4.0 librsvg-2.0 >= 2.14.0 zlib fontconfig lcms"

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


AC_MSG_CHECKING([whether Mathtex is to be enabled])
AC_ARG_ENABLE([mathtex],
             AS_HELP_STRING([--enable-mathtex],[Enable Mathtex]),
             [enable_mathtex=$enableval enable_mathtex=$enableval],
             [enable_mathtex=autodetect enable_mathtex=no])
AC_MSG_RESULT([$enable_mathtex])



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
    AM_PATH_PYTHON([2.4])

	PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
	PYTHON_LIBS="-lpython$PYTHON_VERSION"

	if test "$os_win32" = yes; then
		PYTHON_LIBS=`echo $PYTHON_LIBS | sed 's/\.//g'`
		PYTHON_LIB_LOC="-I$PY_EXEC_PREFIX/libs"
		PYTHON_CFLAGS="-I$PY_EXEC_PREFIX/include"
		PYTHON_EXTRA_LIBS=
	else
		PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
		PYTHON_CONFIGDIR=`$PYTHON-config --configdir`
		PYTHON_LIB_LOC="-L$PYTHON_CONFIGDIR"
		PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
		PYTHON_MAKEFILE="$PYTHON_CONFIGDIR/Makefile"
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

dnl Test for libopenjpeg
AC_ARG_ENABLE(libopenjpeg,
	      AC_HELP_STRING([--disable-libopenjpeg],
	                     [Will not build against libopenjpeg.]),
              enable_libopenjpeg=$enableval,
              enable_libopenjpeg="try")
if test x$enable_libopenjpeg = xyes; then
  AC_CHECK_LIB([openjpeg], [opj_cio_open],,
	       AC_MSG_ERROR("*** libopenjpeg library not found ***"))
  AC_CHECK_HEADERS([openjpeg.h],,
		   AC_MSG_ERROR("*** libopenjpeg headers not found ***"))
elif test x$enable_libopenjpeg = xtry; then
  AC_CHECK_LIB([openjpeg], [opj_cio_open],
               [enable_libopenjpeg="yes"],
	       [enable_libopenjpeg="no"])
  AC_CHECK_HEADERS([openjpeg.h],,
		   [enable_libopenjpeg="no"])
fi

if test x$enable_libopenjpeg = xyes; then
  LIBOPENJPEG_LIBS="-lopenjpeg"
  AC_SUBST(LIBOPENJPEG_LIBS)
dnl  AC_DEFINE(ENABLE_LIBOPENJPEG)
fi

dnl AM_CONDITIONAL(BUILD_LIBOPENJPEG, test x$enable_libopenjpeg = xyes)
dnl AH_TEMPLATE([ENABLE_LIBOPENJPEG],
dnl 	    [Use libopenjpeg instead of builtin jpeg2000 decoder.])

dnl boost
BOOST_REQUIRE([1.54])
BOOST_SYSTEM([mt])
BOOST_FILESYSTEM([mt])
BOOST_LOCALE([mt])
BOOST_IOSTREAMS([mt])
BOOST_FORMAT([mt])

dnl libjpeg
AC_ARG_WITH(libjpeg,
            [AC_HELP_STRING([--without-libjpeg],
                            [disable JPEG])])
if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
  AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
               jpeg_ok=yes,
               jpeg_ok=no
               AC_MSG_WARN(*** JPEG will not be built (JPEG library not found) ***))
  if test "$jpeg_ok" = yes; then
    AC_MSG_CHECKING([for jpeglib.h])
    AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <jpeglib.h>],
        jpeg_ok=yes,
        jpeg_ok=no)
    AC_MSG_RESULT($jpeg_ok)
    if test "$jpeg_ok" = yes; then
      LIBJPEG='-ljpeg'
    else
        AC_MSG_WARN(*** JPEG loader will not be built (JPEG header file not found) ***)
    fi
  fi

  if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
     AC_MSG_ERROR([
*** Checks for JPEG failed. You can build without it by passing
*** --without-libjpeg to configure])
  else
    AC_DEFINE(HAVE_LIBJPEG, [1], [Have JPEG library])
  fi
fi

AM_CONDITIONAL(HAVE_LIBJPEG, test ! -z "$LIBJPEG")
AC_SUBST(LIBJPEG)

PACKAGE_LIBS="$PACKAGE_LIBS $LIBJPEG $LIBOPENJPEG_LIBS"


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


dnl Compile with mathtex
if test "x$enable_mathtex" != "xno"; then
	AC_DEFINE([ENABLE_MATHTEX],[1],[Define to compile with mathtex support])
fi
AM_CONDITIONAL([ENABLE_MATHTEX],[test "x$enable_mathtex" = "xyes"])

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

AC_CONFIG_FILES(Makefile
		src/Makefile
		src/mathtex/Makefile
		src/util/Makefile
		po/Makefile.in
		ui/Makefile
		ui/pixmaps/Makefile
		src/xoj-preview-extractor/Makefile)

AC_CONFIG_FILES(src/pdf/popplerdirect/workaround/workaround.sh,
		[chmod +x src/pdf/popplerdirect/workaround/workaround.sh])

AC_OUTPUT

echo "

Configuration:

	Compiler:                  ${CC}
	Python Plugins Support:    $enable_python
	Overlay Scrollbar enabled: $enable_os
	Mathtex enabled:	   $enable_mathtex
"


