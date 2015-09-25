# - Try to find Poppler and Poppler-glib
# Once done this will define
#
#  POPPLER_FOUND - system has Poppler
#  POPPLER_INCLUDE_DIRS - the include directories for Poppler and Poppler-glib headers
#  POPPLER_LIBRARIES - link these to use Poppler and Poppler-glib

if (POPPLER_LIBRARIES)
   # in cache already
   SET(POPPLER_FIND_QUIETLY TRUE)
endif (POPPLER_LIBRARIES)

find_package(PkgConfig)

if (PKG_CONFIG_FOUND)
    pkg_check_modules(GLIB_PKG glib-2.0)
    find_path(GTK2_GLIBCONFIG_INCLUDE_DIR
        NAMES glibconfig.h
        HINTS ${GLIB_PKG_INCLUDE_DIRS}
    )
endif (PKG_CONFIG_FOUND)

find_package(GTK2)

pkg_check_modules(POPPLER_PKG poppler poppler-glib)

# Paths

find_path(POPPLER_GLIB_INCLUDE_DIR
    NAMES poppler.h
    HINTS ${POPPLER_PKG_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES glib
)
if (NOT POPPLER_GLIB_INCLUDE_DIR)
    message(STATUS "Could not find poppler-glib include dir")
    set(POPPLER_FOUND FALSE)
endif (NOT POPPLER_GLIB_INCLUDE_DIR)

find_path(POPPLER_INCLUDE_DIR
    NAMES poppler-config.h
    HINTS ${POPPLER_PKG_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include
)
if (NOT POPPLER_INCLUDE_DIR)
    message(STATUS "Could not find poppler include dir")
    set(POPPLER_FOUND FALSE)
endif (NOT POPPLER_INCLUDE_DIR)

# Libraries

find_library(POPPLER_LIBRARY
    NAMES poppler libpoppler
    HINTS ${POPPLER_PKG_LIBRARY_DIRS}
    PATHS /usr/lib /usr/local/lib
)
if (NOT POPPLER_LIBRARY)
    message(STATUS "Could not find libpoppler")
    set(POPPLER_FOUND FALSE)
endif (NOT POPPLER_LIBRARY)

find_library(POPPLER_GLIB_LIBRARY
    NAMES poppler-glib libpoppler-glib
    HINTS ${POPPLER_PKG_LIBRARY_DIRS}
    PATHS /usr/lib /usr/local/lib
)
if (NOT POPPLER_GLIB_LIBRARY)
    message(STATUS "Could not find libpoppler-glib")
    set(POPPLER_FOUND FALSE)
endif (NOT POPPLER_GLIB_LIBRARY)

# Version

file (STRINGS "${POPPLER_INCLUDE_DIR}/poppler-config.h" POPPLER_CONFIG_H_CONTENTS REGEX "#define POPPLER_VERSION ")
string (REGEX MATCH "([0-9]+).([0-9]+).([0-9]+)" POPPLER_VERSION "${POPPLER_CONFIG_H_CONTENTS}")
unset(POPPLER_CONFIG_H_CONTENTS)


set(POPPLER_INCLUDE_DIRS ${GTK2_INCLUDE_DIRS} ${POPPLER_INCLUDE_DIR} ${POPPLER_GLIB_INCLUDE_DIR})
set(POPPLER_LIBRARIES ${GTK2_LIBRARIES} ${POPPLER_LIBRARY} ${POPPLER_GLIB_LIBRARY})

if (POPPLER_INCLUDE_DIRS AND POPPLER_LIBRARIES)
    set(POPPLER_FOUND TRUE)
else (POPPLER_INCLUDE_DIRS AND POPPLER_LIBRARIES)
    set(POPPLER_FOUND FALSE)
endif (POPPLER_INCLUDE_DIRS AND POPPLER_LIBRARIES)
