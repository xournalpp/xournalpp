# - try to find Cairo
# Once done this will define
#
#  CAIRO_FOUND - system has Cairo
#  CAIRO_CFLAGS - the Cairo CFlags
#  CAIRO_LIBRARIES - Link these to use Cairo
#
# Copyright (c) 2007, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT WIN32)
  include(UsePkgConfig)

  pkgconfig(cairo _LibCairoIncDir _LibCairoLinkDir _CairoLinkFlags _CairoCflags)
  set (CAIRO_FOUND FALSE)
  if (_LibCairoIncDir)

    if (CAIRO_VERSION)

      exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=${CAIRO_VERSION} cairo RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull)
      if(_return_VALUE STREQUAL "0")
        set (CAIRO_CFLAGS ${_CairoCflags})
        set (CAIRO_LIBRARIES ${_CairoLinkFlags})
      endif(_return_VALUE STREQUAL "0")

    else (CAIRO_VERSION)
      set (CAIRO_CFLAGS ${_CairoCflags})
      set (CAIRO_LIBRARIES ${_CairoLinkFlags})
    endif (CAIRO_VERSION)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Cairo DEFAULT_MSG CAIRO_LIBRARIES CAIRO_CFLAGS)

  endif (_LibCairoIncDir)

endif(NOT WIN32)

mark_as_advanced(
  CAIRO_CFLAGS
  CAIRO_LIBRARIES
)

