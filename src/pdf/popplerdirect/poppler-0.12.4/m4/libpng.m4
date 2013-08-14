dnl Based on Xpdf configure.in and evince configure.ac
dnl Based on kde acinclude.m4.in, LGPL Licensed

AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    echo "configure: __oline__: $i/$j" >&AC_FD_CC
    if test -r "$i/$j"; then
      echo "taking that" >&AC_FD_CC
      $3=$i
      break 2
    fi
  done
done
])

AC_DEFUN([FIND_PNG_HELPER],
[
AC_MSG_CHECKING([for libpng])
AC_CACHE_VAL(ac_cv_lib_png,
[
ac_save_LIBS="$LIBS"
LIBS="$all_libraries $USER_LDFLAGS -lpng -lm"
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes $USER_INCLUDES"
AC_TRY_LINK(
[
#ifdef __cplusplus
extern "C" {
#endif
void png_access_version_number();
#ifdef __cplusplus
}
#endif
],
[png_access_version_number();],
            eval "ac_cv_lib_png=-lpng",
            eval "ac_cv_lib_png=no")
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
])

if eval "test ! \"`echo $ac_cv_lib_png`\" = no"; then
  enable_libpng=yes
  LIBPNG_LIBS="$ac_cv_lib_png"
  AC_MSG_RESULT($ac_cv_lib_png)
else
  AC_MSG_RESULT(no)
  $1
fi
])


AC_DEFUN([POPPLER_FIND_PNG],
[
dnl first look for libraries
FIND_PNG_HELPER(
   FIND_PNG_HELPER(normal, [],
    [
       LIBPNG_LIBS=
    ]
   )
)

dnl then search the headers (can't use simply AC_TRY_xxx, as png.h
dnl requires system dependent includes loaded before it)
png_incdirs="`eval echo $includedir` /usr/include /usr/local/include "
AC_FIND_FILE(png.h, $png_incdirs, png_incdir)
test "x$png_incdir" = xNO && png_incdir=

dnl if headers _and_ libraries are missing, this is no error, and we
dnl continue with a warning (the user will get no png support)
dnl if only one is missing, it means a configuration error, but we still
dnl only warn
if test -n "$png_incdir" && test -n "$LIBPNG_LIBS" ; then
  AC_DEFINE_UNQUOTED(ENABLE_LIBPNG, 1, [Define if you have libpng])
else
  if test -n "$png_incdir" || test -n "$LIBPNG_LIBS" ; then
    AC_MSG_WARN([
There is an installation error in png support. You seem to have only one of
either the headers _or_ the libraries installed. You may need to either provide
correct --with-extra-... options, or the development package of libpng. You
can get a source package of libpng from http://www.libpng.org/pub/png/libpng.html
Disabling PNG support.
])
  else
    AC_MSG_WARN([libpng not found. disable PNG support.])
  fi
  png_incdir=
  enable_libpng=no
  LIBPNG_LIBS=
fi

AC_SUBST(LIBPNG_LIBS)
])
