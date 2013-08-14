# - Try to find the  Fontconfig
# Once done this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_LIBRARIES - Link these to use FONTCONFIG
#  FONTCONFIG_DEFINITIONS - Compiler switches required for using FONTCONFIG

# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)

  # in cache already
  set(FONTCONFIG_FOUND TRUE)

else (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)

  if (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    include(UsePkgConfig)

    pkgconfig(fontconfig _FONTCONFIGIncDir _FONTCONFIGLinkDir _FONTCONFIGLinkFlags _FONTCONFIGCflags)

    set(FONTCONFIG_DEFINITIONS ${_FONTCONFIGCflags} CACHE INTERNAL "The compilation flags for fontconfig")
  endif (NOT WIN32)

  find_path(FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h
    PATHS
    ${_FONTCONFIGIncDir}
    /usr/X11/include
  )

  find_library(FONTCONFIG_LIBRARIES NAMES fontconfig
    PATHS
    ${_FONTCONFIGLinkDir}
  )

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Fontconfig DEFAULT_MSG FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR )
  
  mark_as_advanced(FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR)

endif (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)
