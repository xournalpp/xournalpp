cmake_minimum_required (VERSION 3.1)

project ("Xournal++" LANGUAGES CXX C)
set (PROJECT_VERSION "1.0.1")
set (PROJECT_PACKAGE "xournalpp")

set (PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set (PROJECT_URL "https://github.com/xournalpp/xournalpp")

include (FindPkgConfig)
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Git repo info
include (GitRepo)

## Libraries ##

pkg_check_modules (Gnome REQUIRED "gtk+-2.0 >= 2.18.3" "libglade-2.0 >= 2.6.4" "glib-2.0 >= 2.32.0" "gthread-2.0 >= 2.4.0")
pkg_check_modules (Glibmm REQUIRED "glibmm-2.4 >= 2.24.0")

# librsvg-2.0 is not really needed by Xournal++, but if you don't have it, it crashes while loading the SVG icons
# We also are compiling with libpoppler so we require a number of extra packages
pkg_check_modules (OtherLibs REQUIRED "librsvg-2.0 >= 2.14.0" zlib fontconfig lcms)

set (Boost_USE_MULTITHREADED ON)
find_package (Boost 1.54 REQUIRED COMPONENTS system filesystem locale iostreams thread)
# TODO add possibility to build boost with Xournal++

# JPEG
option (ENABLE_LIBOPENJPEG "Use libopenjpeg instead of builtin jpeg2000 decoder" ON)
if (ENABLE_LIBOPENJPEG)
  find_package (OpenJPEG REQUIRED)
endif (ENABLE_LIBOPENJPEG)

find_package (JPEG)
if (NOT JPEG_FOUND)
  message (WARNING "JPEG loader will not be built (JPEG header file not found)")
endif (NOT JPEG_FOUND)

set (PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")

## Additional features ##

# CppUnit
option (ENABLE_CPPUNIT "Build CppUnit test instead of xournalpp application" OFF)
if (ENABLE_CPPUNIT)
  pkg_check_modules (CppUnit REQUIRED "cppunit >= 1.13-0")
endif (ENABLE_CPPUNIT)

# Overlay scrollbar
option (ENABLE_OS "Overlay Scrollbar support" OFF)
if (ENABLE_OS)
  pkg_check_modules (os REQUIRED "overlay-scrollbar-0.1 >= 0.1.12")
endif (ENABLE_OS)

# Mathtex
option (ENABLE_MATHTEX "Mathtex support" OFF)

# Unstable features

option (UNSTABLE_LAYERS_SIDEBAR "Layers sidebar (unstable)" OFF)

configure_file (
  src/config-features.h.in
  src/config-features.h
  ESCAPE_QUOTES @ONLY
)

## I18n ##
add_subdirectory (po)

## Configuration headers ##

# Development options
option (DEV_MEMORY_CHECKING "Memory checking" ON)
option (DEV_MEMORY_LEAK_CHECKING "Memory leak checking" ON)
option (DEV_CALL_LOG "Call log" OFF)

# Debug options
option (DEBUG_INPUT "Input debugging, e.g. eraser events etc" OFF)
option (DEBUG_RECOGNIZER "Shape recognizer debug: output score etc" OFF)
option (DEBUG_SHEDULER "Scheduler debug: show jobs etc" OFF)
option (DEBUG_SHOW_ELEMENT_BOUNDS "Draw a surrounding border to all elements" OFF)
option (DEBUG_SHOW_REPAINT_BOUNDS "Draw a border around all repaint rects" OFF)
option (DEBUG_SHOW_PAINT_BOUNDS "Draw a border around all painted rects" OFF)
mark_as_advanced (FORCE
  DEBUG_INPUT DEBUG_RECOGNIZER DEBUG_SHEDULER DEBUG_SHOW_ELEMENT_BOUNDS DEBUG_SHOW_REPAINT_BOUNDS DEBUG_SHOW_PAINT_BOUNDS
)

# Advanced development config
set (DEV_CONFIG_DIR ".xournalpp" CACHE STRING "Xournal++ config dir, relative to user's home dir")
set (DEV_TOOLBAR_CONFIG "toolbar.ini" CACHE STRING "Toolbar config file name")
set (DEV_SETTINGS_XML_FILE "settings.xml" CACHE STRING "Settings file name")
set (DEV_PRINT_CONFIG_FILE "print-config.ini" CACHE STRING "Print config file name")
set (DEV_METADATA_FILE "metadata.ini" CACHE STRING "Metadata file name")
set (DEV_METADATA_MAX_ITEMS 50 CACHE STRING "Maximal amount of metadata elements")
mark_as_advanced (FORCE
  DEV_CONFIG_DIR DEV_TOOLBAR_CONFIG DEV_SETTINGS_XML_FILE DEV_PRINT_CONFIG_FILE DEV_METADATA_FILE DEV_METADATA_MAX_ITEMS
)

configure_file (
  src/config.h.in
  src/config.h
  ESCAPE_QUOTES @ONLY
)

configure_file (
  src/config-debug.h.in
  src/config-debug.h
  ESCAPE_QUOTES @ONLY
)

configure_file (
  src/config-dev.h.in
  src/config-dev.h
  ESCAPE_QUOTES @ONLY
)

## Source building ##
add_subdirectory (src)

## Final targets and installing ##

# Install resources
install (DIRECTORY ui
  DESTINATION share/xournalpp
  COMPONENT xournalpp
)

# Uninstall target
configure_file (
  cmake/cmake_uninstall.cmake.in
  cmake/cmake_uninstall.cmake
  IMMEDIATE @ONLY)

add_custom_target (uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake
  DEPENDS desktop-uninstall

  COMMENT "Uninstall entire xournalpp"
)

# Install desktop target
configure_file (desktop/desktop_install.sh.in desktop/desktop_install.sh @ONLY)

add_custom_target (desktop-install-xournalpp
  COMMAND desktop/desktop_install.sh install

  COMMENT "Install xournalpp desktop files"
)

add_custom_target (desktop-install
  DEPENDS desktop-install-xournalpp desktop-install-thumbnailer
)

# Uninstall desktop target
add_custom_target (desktop-uninstall-xournalpp
  COMMAND desktop/desktop_install.sh uninstall

  COMMENT "Uninstall xournalpp desktop files"
)

add_custom_target (desktop-uninstall
  DEPENDS desktop-uninstall-xournalpp desktop-uninstall-thumbnailer
)

message ("

Configuration:
	Compiler:                  ${CMAKE_CXX_COMPILER}
	Overlay Scrollbar enabled: ${ENABLE_OS}
	Mathtex enabled:           ${ENABLE_MATHTEX}
	CppUnit enabled:           ${ENABLE_CPPUNIT}

Unstable features:
	Layers sidebar:            ${UNSTABLE_LAYERS_SIDEBAR}
")
