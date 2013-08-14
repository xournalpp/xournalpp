# POPPLER_FIND_QT(VARIABLE-PREFIX, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# Check whether the Qt libraries are available.  Adapted from
# OpenOffice.org configure.in
#
# --------------------------------------------------------------
AC_DEFUN([POPPLER_FIND_QT],
[
dnl Search paths for Qt 
if test "$build_cpu" != "x86_64" ; then
    qt_incdirs="$QTINC /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib/qt3/include /usr/lib/qt/include /usr/share/qt3/include"
    qt_libdirs="$QTLIB /usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt3/lib /usr/lib/qt/lib /usr/share/qt3/lib"
else
    qt_incdirs="$QTINC /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/lib64/qt3/include /usr/lib64/qt/include /usr/share/qt3/include /usr/lib/qt3/include /usr/lib/qt/include"
    qt_libdirs="$QTLIB /usr/local/qt/lib64 /usr/lib64/qt /usr/lib64 /usr/X11R6/lib64/X11/qt /usr/X11R6/lib64/qt /usr/lib64/qt3/lib64 /usr/lib64/qt/lib64 /usr/share/qt3/lib64 /usr/local/qt/lib /usr/lib/qt /usr/lib /usr/X11R6/lib/X11/qt /usr/X11R6/lib/qt /usr/lib/qt3/lib /usr/lib/qt/lib /usr/share/qt3/lib"
fi
if test -n "$QTDIR" ; then
    qt_incdirs="$QTDIR/include $qt_incdirs"
    if test "$build_cpu" != "x86_64" ; then
        qt_libdirs="$QTDIR/lib $qt_libdirs"
    else
        qt_libdirs="$QTDIR/lib64 $QTDIR/lib $qt_libdirs"
    fi
fi

dnl What to test
qt_test_include="qstyle.h"
qt_test_la_library="libqt-mt.la"
qt_test_library="libqt-mt.so"

dnl Check for Qt headers
AC_MSG_CHECKING([for Qt headers])
qt_incdir="no"
for it in $qt_incdirs ; do
    if test -r "$it/$qt_test_include" ; then
        qt_incdir="$it"
        break
    fi
done
AC_MSG_RESULT([$qt_incdir])

dnl Check for Qt libraries
AC_MSG_CHECKING([for Qt libraries])
qt_libdir="no"
for qt_check in $qt_libdirs ; do
    if test -r "$qt_check/$qt_test_la_library" ; then
        qt_libdir="$qt_check"
        break
    fi

    if test -r "$qt_check/$qt_test_library" ; then
        qt_libdir="$qt_check"
        break
    fi
done
AC_MSG_RESULT([$qt_libdir])

if test "x$qt_libdir" != "xno" ; then
    if test "x$qt_incdir" != "xno" ; then
        have_qt=yes
    fi
fi

if test "x$have_qt" = "xyes"; then
    AC_LANG_PUSH([C++])
    pthread_needed=no

    save_LDFLAGS=$LDFLAGS
    save_CXXFLAGS=$CXXFLAGS
    save_LIBS=$LIBS
    CXXFLAGS="$CXXFLAGS -I$qt_incdir"
    LIBS="$LIBS -L$qt_libdir -lqt-mt"
    AC_MSG_CHECKING([if Qt needs -pthread])
    AC_TRY_LINK([#include <qt.h>], [QString s;], [pthread_needed=no], [pthread_needed=yes])
    if test "x$pthread_needed" = "xyes"; then
        LDFLAGS="$LDFLAGS -pthread"
        AC_TRY_LINK([#include <qt.h>], [QString s;], [pthread_needed=yes], [pthread_needed=no])
    fi
    AC_MSG_RESULT([$pthread_needed])
    LDFLAGS=$save_LDFLAGS
    CXXFLAGS=$save_CXXFLAGS
    LIBS=$save_LIBS

    AC_LANG_POP

    qtpthread=''
    if test "x$pthread_needed" = "xyes"; then
        qtpthread="-pthread"
    fi

    $1[]_CXXFLAGS="-I$qt_incdir"
    $1[]_LIBS="$qtpthread -L$qt_libdir -lqt-mt"
    ifelse([$2], , :, [$2])
else
    ifelse([$3], , [AC_MSG_FAILURE(dnl
[Qt development libraries not found])],
  	   [$3])
fi
])


