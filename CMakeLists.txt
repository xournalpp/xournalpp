cmake_minimum_required (VERSION 2.8.8)

project ("Xournal++" CXX C)
set (PROJECT_VERSION "1.0.1")
set (PROJECT_PACKAGE "xournalpp")

set (PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set (PROJECT_URL "https://github.com/xournalpp/xournalpp")

include (FindPkgConfig)
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Git repo info
include (GitRepo)

## Libraries ##

macro (add_includes_ldflags LDFLAGS INCLUDES)
    set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${LDFLAGS})
    set (xournalpp_INCLUDE_DIRS ${xournalpp_INCLUDE_DIRS} ${INCLUDES})
endmacro (add_includes_ldflags LDFLAGS INCLUDES)

# Boost (we start with boost, because if it'll be built it needs some linker tags)
option (BUILD_BOOST "Build boost libraries into Xournal++" OFF)
if (BUILD_BOOST)

  set (BOOST_PREFIX "${CMAKE_BINARY_DIR}/boost-prefix")

  include (ExternalProject)
  ExternalProject_Add (boost
    SVN_REPOSITORY http://svn.boost.org/svn/boost/branches/release
    LOG_DOWNLOAD 1

    CONFIGURE_COMMAND cd "${BOOST_PREFIX}/src/boost" && ./bootstrap.sh
      --prefix="${CMAKE_BINARY_DIR}/boost-prefix"

    BUILD_COMMAND cd "${BOOST_PREFIX}/src/boost" && ./b2
      --with-filesystem --with-iostreams --with-locale --with-thread --with-system
      "--prefix=${BOOST_PREFIX}"
      "--build-dir=${BOOST_PREFIX}/src/boost-build"
      "--stagedir=${BOOST_PREFIX}"
      threading=multi link=static

    INSTALL_COMMAND ""
  )

  # ICU – needed for boost::locale
  find_package (ICU COMPONENTS uc i18n)

  link_directories ("${BOOST_PREFIX}/lib/")
  set (Boost_LIBRARIES
    boost_filesystem
    boost_locale
    boost_iostreams
    boost_thread
    boost_system
    ${ICU_LIBRARIES}
  )
  set (Boost_INCLUDE_DIRS "${BOOST_PREFIX}/src/boost")

else (BUILD_BOOST)

  set (Boost_USE_MULTITHREADED ON)
  find_package (Boost 1.54 COMPONENTS system filesystem locale iostreams thread)
  if (Boost_VERSION VERSION_LESS 1.54)
    message (FATAL_ERROR "Boost 1.54 or newer not found – you should enable BUILD_BOOST cmake variable")
  endif (Boost_VERSION VERSION_LESS 1.54)

endif (BUILD_BOOST)
add_includes_ldflags ("${Boost_LIBRARIES}" "${Boost_INCLUDE_DIRS}")

# GTK+/Gnome
pkg_check_modules (Gnome REQUIRED "gtk+-2.0 >= 2.18.3" "libglade-2.0 >= 2.6.4" "glib-2.0 >= 2.32.0" "gthread-2.0 >= 2.4.0")
add_includes_ldflags ("${Gnome_LDFLAGS}" "${Gnome_INCLUDE_DIRS}")

# Glibmm
option (EXT_GLIBMM "Download external Glibmm library" OFF)
if (EXT_GLIBMM)
  include (ExternalDeb)
  ExternalDeb_Add (Glibmm "http://ftp.de.debian.org/debian/pool/main/g/glibmm2.4/libglibmm-2.4-dev_2.42.0-1_%ARCH%.deb")
  set (Glibmm_LDFLAGS ${Glibmm_SLIBRARIES})
else (EXT_GLIBMM)
  pkg_check_modules (Glibmm REQUIRED "glibmm-2.4 >= 2.38.0")
endif (EXT_GLIBMM)
add_includes_ldflags ("${Glibmm_LDFLAGS}" "${Glibmm_INCLUDE_DIRS}")

# Poppler
find_package (Poppler)
if (NOT POPPLER_FOUND)
  message (FATAL_ERROR "Poppler not found")
endif (NOT POPPLER_FOUND)
add_includes_ldflags ("${POPPLER_LIBRARIES}" "${POPPLER_INCLUDE_DIRS}")

# zlib
find_package (ZLIB REQUIRED)    # zlib
add_includes_ldflags ("${ZLIB_LIBRARIES}" "${ZLIB_INCLUDE_DIRS}")

# pthreads
find_package (Threads REQUIRED) # pthreads
set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${CMAKE_THREAD_LIBS_INIT})

# JPEG

option (ENABLE_LIBOPENJPEG "Use libopenjpeg instead of builtin jpeg2000 decoder" ON)
if (ENABLE_LIBOPENJPEG)
  find_package (OpenJPEG REQUIRED)
  add_includes_ldflags ("${OPENJPEG_LIBRARIES};-lopenjpeg" "${OPENJPEG_INCLUDE_DIRS}") # -lopenjpeg added as fallback
endif (ENABLE_LIBOPENJPEG)

find_package (JPEG)
add_includes_ldflags ("${JPEG_LIBRARIES}" "${JPEG_INCLUDE_DIRS}")
if (NOT JPEG_FOUND)
  message (WARNING "JPEG loader will not be built (JPEG header file not found)")
endif (NOT JPEG_FOUND)

set (PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")

## Additional features ##

# CppUnit
option (ENABLE_CPPUNIT "Build CppUnit test instead of xournalpp application" OFF)
if (ENABLE_CPPUNIT)
  pkg_check_modules (CppUnit REQUIRED "cppunit >= 1.12-0")
  enable_testing ()
endif (ENABLE_CPPUNIT)

# Overlay scrollbar
option (ENABLE_OS "Overlay Scrollbar support" OFF)
if (ENABLE_OS)
  pkg_check_modules (OS REQUIRED "overlay-scrollbar-0.2")
  add_includes_ldflags ("${OS_LDFLAGS}" "${OS_INCLUDE_DIRS}")
endif (ENABLE_OS)

# Mathtex
option (ENABLE_MATHTEX "Mathtex support" OFF)
if (ENABLE_MATHTEX AND WIN32)
    message (FATAL_ERROR "Mathtex is not supported on Windows for now")
endif (ENABLE_MATHTEX AND WIN32)

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

option (DEV_ENABLE_GCOV "Build with gcov support" OFF) # Enabel gcov support – expanded in src/

mark_as_advanced (FORCE
  DEV_CONFIG_DIR DEV_TOOLBAR_CONFIG DEV_SETTINGS_XML_FILE DEV_PRINT_CONFIG_FILE DEV_METADATA_FILE DEV_METADATA_MAX_ITEMS
  DEV_ENABLE_GCOV
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

## C++11 ##

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else(COMPILER_SUPPORTS_CXX11)
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif(COMPILER_SUPPORTS_CXX11)

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
  IMMEDIATE @ONLY
)

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
	Compiler:                   ${CMAKE_CXX_COMPILER}
	Overlay Scrollbar enabled:  ${ENABLE_OS}
	Mathtex enabled:            ${ENABLE_MATHTEX}
	CppUnit enabled:            ${ENABLE_CPPUNIT}

Unstable features:
	Layers sidebar:             ${UNSTABLE_LAYERS_SIDEBAR}

Static libraries:
	Boost:                      ${BUILD_BOOST}
	Glibmm:                     ${EXT_GLIBMM}
")

option (CMAKE_DEBUG_INCLUDES_LDFLAGS "List include dirs and ldflags for xournalpp target" OFF)
mark_as_advanced (FORCE CMAKE_DEBUG_INCLUDES_LDFLAGS)
if (CMAKE_DEBUG_INCLUDES_LDFLAGS)
    message ("Include directories: ${xournalpp_INCLUDE_DIRS}")
    message ("LDFLAGS/LIBRARIES: ${xournalpp_LDFLAGS}")
endif (CMAKE_DEBUG_INCLUDES_LDFLAGS)
