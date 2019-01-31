cmake_minimum_required (VERSION 2.8.8)

project ("Xournal++" CXX C)

## Also update changelog in debian folder!
set (PROJECT_VERSION "1.0.7")
set (PROJECT_PACKAGE "xournalpp")
set (PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set (PROJECT_URL "https://github.com/xournalpp/xournalpp")

set (CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake/find"
    "${PROJECT_SOURCE_DIR}/cmake/include"
)

# package version
include (Version)
core_find_git_rev(RELEASE_IDENTIFIER)
string(TIMESTAMP PACKAGE_TIMESTAMP "%Y%m%d.%H%M" UTC)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}~git${PACKAGE_TIMESTAMP}-${RELEASE_IDENTIFIER}-${DISTRO_CODENAME})

configure_file (
    cmake/postinst.in
    cmake/postinst
    @ONLY
)

include (FindPkgConfig)

set (PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")

# Git repo info
include (GitRepo)

## C++11 ##
include (C++11)
CheckCXX11 (FATAL_ERROR)

## os specific library ending ##
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LIB_ENDING "dylib")
else ()
    set(LIB_ENDING "so")
endif ()

if (WIN32)
    set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} "-mwindows")
endif ()

## For touch workaround, may need to be disabled for a Wayland Build
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} "-lX11 -lXi")
endif ()

## Libraries ##

macro (add_includes_ldflags LDFLAGS INCLUDES)
    set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${LDFLAGS})
    set (xournalpp_INCLUDE_DIRS ${xournalpp_INCLUDE_DIRS} ${INCLUDES})
endmacro (add_includes_ldflags LDFLAGS INCLUDES)

# GTK+
pkg_check_modules (GTK REQUIRED "gtk+-3.0 >= 3.18.9")
add_includes_ldflags ("${GTK_LDFLAGS}" "${GTK_INCLUDE_DIRS}")

# GLIB
pkg_check_modules (Glib REQUIRED "glib-2.0 >= 2.32.0")
add_includes_ldflags ("${Glib_LDFLAGS}" "${Glib_INCLUDE_DIRS}")

# GThread
pkg_check_modules (GThread REQUIRED "gthread-2.0 >= 2.4.0")
add_includes_ldflags ("${GThread_LDFLAGS}" "${GThread_INCLUDE_DIRS}")

# LibXML
pkg_check_modules (Libxml REQUIRED "libxml-2.0 >= 2.0.0")
add_includes_ldflags ("${Libxml_LDFLAGS}" "${Libxml_INCLUDE_DIRS}")

# Poppler
pkg_check_modules (PopplerGlib REQUIRED "poppler-glib >= 0.41.0")
add_includes_ldflags ("${PopplerGlib_LDFLAGS}" "${PopplerGlib_INCLUDE_DIRS}")
set(POPPLER_INCLUDE_DIR, "${PopplerGlib_INCLUDE_DIRS}")

# zlib
find_package (ZLIB REQUIRED)
add_includes_ldflags ("${ZLIB_LIBRARIES}" "${ZLIB_INCLUDE_DIRS}")

# pthreads
find_package (Threads REQUIRED)
set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${CMAKE_THREAD_LIBS_INIT})

# portaudio
pkg_check_modules(PORTAUDIOCPP REQUIRED portaudiocpp)
add_includes_ldflags ("${PORTAUDIOCPP_LDFLAGS}" "${PORTAUDIOCPP_INCLUDE_DIRS}")

# SOX
pkg_check_modules(SOX REQUIRED sox)
add_includes_ldflags ("${SOX_LDFLAGS}" "${SOX_INCLUDE_DIRS}")

## Additional features ##

# CppUnit
option (ENABLE_CPPUNIT "Build CppUnit test instead of xournalpp application" OFF)
if (ENABLE_CPPUNIT)
  pkg_check_modules (CppUnit REQUIRED "cppunit >= 1.12-0")
  enable_testing ()
endif (ENABLE_CPPUNIT)

# Mac integration
pkg_check_modules (MacIntegration "gtk-mac-integration")
if (MacIntegration_FOUND)
	message("Enable Mac integration")
	add_includes_ldflags ("${MacIntegration_LDFLAGS}" "${MacIntegration_INCLUDE_DIRS}")
	add_definitions(-DMAC_INTEGRATION)
endif ()

configure_file (
  src/config-features.h.in
  src/config-features.h
  ESCAPE_QUOTES @ONLY
)

## I18n ##
add_subdirectory (po)

## Configuration headers and developement options ##

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
set (DEV_ERRORLOG_DIR "errorlogs" CACHE STRING "Directory where errorlogfiles will be placed")

option (DEV_ENABLE_GCOV "Build with gcov support" OFF) # Enabel gcov support – expanded in src/
option (DEV_CHECK_GTK3_COMPAT "Adds a few compiler flags to check basic GTK3 upgradeability support (still compiles for GTK2!)")
if (DEV_CHECK_GTK3_COMPAT)
    add_definitions (-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE)
endif ()

mark_as_advanced (FORCE
  DEV_CONFIG_DIR DEV_TOOLBAR_CONFIG DEV_SETTINGS_XML_FILE DEV_PRINT_CONFIG_FILE DEV_METADATA_FILE DEV_METADATA_MAX_ITEMS
  DEV_ENABLE_GCOV DEV_CHECK_GTK3_COMPAT
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

configure_file (
  src/config-paths.h.in
  src/config-paths.h
  ESCAPE_QUOTES @ONLY
)

## Source building ##
add_subdirectory (src)

## Final targets and installing ##

# Install resources
install (DIRECTORY ui
  DESTINATION "share/xournalpp"
  COMPONENT xournalpp
)

# Install desktop shortcuts for Linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message ("Installing desktop files")
	# Install desktop entry
	#install(FILES data/albert.desktop DESTINATION /share/applications )

	# Install icons
	install(FILES ui/pixmaps/xournalpp.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)

	# Symlink are not easy to use with CMake, therefor simple install a copy...
	install(FILES ui/pixmaps/application-x-xopp.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)
	install(FILES ui/pixmaps/application-x-xopt.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)
	install(FILES ui/pixmaps/application-x-xojpp.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)

	install(FILES ui/pixmaps/gnome-mime-application-x-xopp.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)
	install(FILES ui/pixmaps/gnome-mime-application-x-xopt.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)

	install(FILES desktop/xournalpp.xml
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)
	install(FILES desktop/xournalpp.desktop
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
	install(FILES desktop/x-xojpp.desktop
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mimelnk/application)
	install(FILES desktop/x-xopp.desktop
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mimelnk/application)
	install(FILES desktop/x-xopt.desktop
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mimelnk/application)

	install(FILES desktop/xournalpp.thumbnailer
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thumbnailers)
	install(PROGRAMS utility/usr/local/bin/xopp-recording.sh
			DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)

	install(CODE "execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cmake/postinst configure)")

endif ()

# Uninstall target
configure_file (
  cmake/cmake_uninstall.cmake.in
  cmake/cmake_uninstall.cmake
  @ONLY
)

add_custom_target (uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake

  COMMENT "Uninstall entire Xournal++"
)

message ("
Configuration:
	Compiler:                   ${CMAKE_CXX_COMPILER}
	CppUnit enabled:            ${ENABLE_CPPUNIT}
")

option (CMAKE_DEBUG_INCLUDES_LDFLAGS "List include dirs and ldflags for xournalpp target" OFF)
mark_as_advanced (FORCE CMAKE_DEBUG_INCLUDES_LDFLAGS)
if (CMAKE_DEBUG_INCLUDES_LDFLAGS)
    message ("Include directories: ${xournalpp_INCLUDE_DIRS}")
    message ("LDFLAGS/LIBRARIES: ${xournalpp_LDFLAGS}")
endif (CMAKE_DEBUG_INCLUDES_LDFLAGS)
