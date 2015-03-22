# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_boost_locale.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_LOCALE
#
# DESCRIPTION
#
#   Test for System library from the Boost C++ libraries. The macro requires
#   a preceding call to AX_BOOST_BASE. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_LOCALE_LIB)
#
#   And sets:
#
#     HAVE_BOOST_LOCALE
#
# LICENSE
#
#   Copyright (c) 2012 Xiyue Deng <manphiz@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_BOOST_LOCALE],
[
	AC_ARG_WITH([boost-locale],
	AS_HELP_STRING([--with-boost-locale@<:@=special-lib@:>@],
                   [use the Locale library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-locale=boost_locale-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_locale_lib=""
        else
		    want_boost="yes"
		ax_boost_user_locale_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
        AC_REQUIRE([AC_CANONICAL_BUILD])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::Locale library is available,
					   ax_cv_boost_locale,
        [AC_LANG_PUSH([C++])
			 CXXFLAGS_SAVE=$CXXFLAGS

			 AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/locale.hpp>]],
                                   [[boost::locale::generator gen;
                                   std::locale::global(gen(""));]])],
                   ax_cv_boost_locale=yes, ax_cv_boost_locale=no)
			 CXXFLAGS=$CXXFLAGS_SAVE
             AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_locale" = "xyes"; then
			AC_SUBST(BOOST_CPPFLAGS)

			AC_DEFINE(HAVE_BOOST_LOCALE,,[define if the Boost::Locale library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`

			LDFLAGS_SAVE=$LDFLAGS
            if test "x$ax_boost_user_locale_lib" = "x"; then
                for libextension in `ls $BOOSTLIBDIR/libboost_locale*.so* $BOOSTLIBDIR/libboost_locale*.dylib* $BOOSTLIBDIR/libboost_locale*.a* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_locale.*\)\.so.*$;\1;' -e 's;^lib\(boost_locale.*\)\.dylib.*$;\1;' -e 's;^lib\(boost_locale.*\)\.a.*$;\1;'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_LOCALE_LIB="-l$ax_lib"; AC_SUBST(BOOST_LOCALE_LIB) link_locale="yes"; break],
                                 [link_locale="no"])
				done
                if test "x$link_locale" != "xyes"; then
                for libextension in `ls $BOOSTLIBDIR/boost_locale*.dll* $BOOSTLIBDIR/boost_locale*.a* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^\(boost_locale.*\)\.dll.*$;\1;' -e 's;^\(boost_locale.*\)\.a.*$;\1;'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_LOCALE_LIB="-l$ax_lib"; AC_SUBST(BOOST_LOCALE_LIB) link_locale="yes"; break],
                                 [link_locale="no"])
				done
                fi

            else
               for ax_lib in $ax_boost_user_locale_lib boost_locale-$ax_boost_user_locale_lib; do
				      AC_CHECK_LIB($ax_lib, exit,
                                   [BOOST_LOCALE_LIB="-l$ax_lib"; AC_SUBST(BOOST_LOCALE_LIB) link_locale="yes"; break],
                                   [link_locale="no"])
                  done

            fi
            if test "x$ax_lib" = "x"; then
                AC_MSG_ERROR(Could not find a version of the library!)
            fi
			if test "x$link_locale" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
	LDFLAGS="$LDFLAGS_SAVED"
	fi
])
