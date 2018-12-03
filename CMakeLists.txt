cmake_minimum_required (VERSION 2.8.8)

project ("Xournal++" CXX C)
set (PROJECT_VERSION "1.0.2")
set (PROJECT_PACKAGE "xournalpp")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Xournal++: Notetaking software designed around a tablet")
string(TIMESTAMP CPACK_PACKAGE_VERSION "nightly-build-%Y-%m-%dT%H%M%SZ")
set(CPACK_GENERATOR "DEB"  CACHE STRING "Alternativelly generate RPM. Needs rpmbuild.")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Xournal++ Team")

include(CPack)

set (PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set (PROJECT_URL "https://github.com/xournalpp/xournalpp")

include (FindPkgConfig)
set (CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake/find"
    "${PROJECT_SOURCE_DIR}/cmake/include"
)

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

## Libraries ##

macro (add_includes_ldflags LDFLAGS INCLUDES)
    set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${LDFLAGS})
    set (xournalpp_INCLUDE_DIRS ${xournalpp_INCLUDE_DIRS} ${INCLUDES})
endmacro (add_includes_ldflags LDFLAGS INCLUDES)

# Boost (we start with boost, because if it'll be built it needs some linker tags)
option (BUILD_BOOST "Build boost libraries into Xournal++" OFF)
if (BUILD_BOOST)

  set (BOOST_PREFIX "${CMAKE_BINARY_DIR}/boost-prefix")
  if (COMPILER_CXX11_FLAG) # add flags only when needed
    set (BOOST_CXXFLAGS cxxflags=${COMPILER_CXX11_FLAG})
  endif ()

  include (ExternalProject)
  ExternalProject_Add (boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    PREFIX "${BOOST_PREFIX}"

    CONFIGURE_COMMAND cd "${BOOST_PREFIX}/src/boost" && ./bootstrap.sh --prefix="${BOOST_PREFIX}"

    BUILD_COMMAND cd "${BOOST_PREFIX}/src/boost" && ./b2
      ${BOOST_CXXFLAGS}
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
    message (FATAL_ERROR "Boost 1.54 or newer not found – you should enable BUILD_BOOST CMake flag")
  endif (Boost_VERSION VERSION_LESS 1.54)

endif (BUILD_BOOST)
add_includes_ldflags ("${Boost_LIBRARIES}" "${Boost_INCLUDE_DIRS}")

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

find_program(LSB_RELEASE_EXEC lsb_release)
execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
  OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_program(LSB_RELEASE_EXEC lsb_release)
execute_process(COMMAND ${LSB_RELEASE_EXEC} -rs
  OUTPUT_VARIABLE LSB_RELEASE_NUMBER_SHORT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Poppler
option (BUILD_POPPLER "Build Poppler from git or other source" DEFAULT_BUILD_POPPLER)

## Ubuntu 16.04 need a poppler build
if (LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu" AND LSB_RELEASE_NUMBER_SHORT STREQUAL "16.04")
  set(BUILD_POPPLER ON)
  message ("Automatically set BUILD_POPPLER ON on Ubuntu 16.04")
endif()
## Mac OS X too
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(BUILD_POPPLER ON)
  message ("Automatically set BUILD_POPPLER ON on Mac OS")
endif ()

set (POPPLER_MIN_VER "0.58")
set (POPPLER_MAX_VER  "0.68")
set (POPPLER_BUILD_FROM "git" CACHE STRING "Source to build poppler from: git (default), tarball, sourcefolder")
set (POPPLER_GIT_VER "0.61.1" CACHE STRING "Version of Poppler to build")
set (POPPLER_SRC_DIR "/path/to/poppler/source" CACHE STRING "Directory with poppler source")
set (POPPLER_SRC_TARXZ "/path/to/poppler.tar.xz" CACHE STRING "Tarball file location")

if (BUILD_POPPLER)
  # For documentation see https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
  set(POPPLER_LIB_DIR ${CMAKE_INSTALL_PREFIX}/bin/xournalpplib)
  # RPATH needs to be set *here*, it's set later in the script, it is not used!
  SET(CMAKE_INSTALL_RPATH "${POPPLER_LIB_DIR}")

  include (ExternalProject)
  set (POPPLER_PREFIX "${CMAKE_BINARY_DIR}/poppler-prefix")
  set (POPPLER_CMAKE_ARGS "-DBUILD_QT5_TESTS=OFF -DENABLE_UTILS=OFF -DENABLE_QT5=OFF -DENABLE_QT4=OFF -DBUILD_QT4_TESTS=OFF")
  if (POPPLER_BUILD_FROM STREQUAL "git")
    ExternalProject_Add (poppler
        GIT_REPOSITORY "git://git.freedesktop.org/git/poppler/poppler"
        GIT_TAG "poppler-${POPPLER_GIT_VER}"
        PREFIX "${POPPLER_PREFIX}"

        CMAKE_ARGS ${POPPLER_CMAKE_ARGS}
        INSTALL_COMMAND ""
    )
  elseif (POPPLER_BUILD_FROM STREQUAL "sourcefolder")
    ExternalProject_Add (poppler
        SOURCE_DIR "${POPPLER_SRC_DIR}"
        PREFIX "${POPPLER_PREFIX}"

        CMAKE_ARGS ${POPPLER_CMAKE_ARGS}
        INSTALL_COMMAND ""
    )
  elseif (POPPLER_BUILD_FROM STREQUAL "tarball")
	ExternalProject_Add (poppler
        URL "${POPPLER_SRC_TARXZ}"
        PREFIX "${POPPLER_PREFIX}"

        CMAKE_ARGS ${POPPLER_CMAKE_ARGS}
		INSTALL_COMMAND ""
    )
  else ()
    message (FATAL_ERROR "Source to build poppler from must be: git (default), sourcefolder or tarball")
  endif()
  
  # Additional packages required by Poppler
  find_package (OpenJPEG REQUIRED)
  find_package (JPEG REQUIRED)
  pkg_check_modules (PopplerLibs REQUIRED "librsvg-2.0 >= 2.14.0" fontconfig)

  pkg_check_modules (lcms2 lcms2)
  if (lcms2_FOUND)
    set (lcms_VERSION 2)
  else ()
    pkg_check_modules (lcms1 lcms)
    if (lcms1_FOUND)
      set (lcms_VERSION 1)
    else ()
      message (FATAL_ERROR "Neither first nor second version of lcms (needed to link with poppler) was found!")
    endif ()
  endif ()
  
  set (lcms_FOUND ON)
  set (lcms_LDFLAGS "${lcms${lcms_VERSION}_LDFLAGS}")
  set (lcms_INCLUDE_DIRS "${lcms${lcms_VERSION}_INCLUDE_DIRS}")
  
  link_directories (
    "${POPPLER_PREFIX}/src/poppler-build"
    "${POPPLER_PREFIX}/src/poppler-build/glib"
    "${POPPLER_PREFIX}/src/poppler-build/cpp"
    "${POPPLER_PREFIX}/src/poppler-build/utils"
  )
  set (POPPLER_LIBRARIES
    "libpoppler.${LIB_ENDING}"
    "libpoppler-cpp.${LIB_ENDING}"
    "libpoppler-glib.${LIB_ENDING}"

    ${OPENJPEG_LIBRARIES}
# Comment out, seems to be a problem on Debian...
# Do we need to detect something to enable / disable?
#    -lopenjpeg
    ${JPEG_LIBRARIES}
    ${PopplerLibs_LDFLAGS}
    ${lcms_LDFLAGS}
  )
  # -lopenjpeg added as fallback
  
  set (POPPLER_INCLUDE_DIRS
    "${POPPLER_PREFIX}/src/poppler-build"
    "${POPPLER_PREFIX}/src/poppler-build/poppler"
    "${POPPLER_PREFIX}/src/poppler-build/glib"
    "${POPPLER_PREFIX}/src/poppler-build/cpp"

    "${POPPLER_PREFIX}/src/poppler"
    
    ${OPENJPEG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ${PopplerLibs_INCLUDE_DIRS}
    ${lcms_INCLUDE_DIRS}
  )

else ()

  find_package (Poppler)
  if (NOT POPPLER_FOUND)
    message (FATAL_ERROR "Poppler not found – you should enable BUILD_POPPLER CMake flag")
  elseif (POPPLER_VERSION VERSION_LESS ${POPPLER_MIN_VER})
    message (FATAL_ERROR "Poppler too old - you should enable BUILD_POPPLER CMake flag")
  elseif (POPPLER_VERSION VERSION_GREATER ${POPPLER_MAX_VER})
    message (FATAL_ERROR "Poppler too new, Xournalpp has not been adapted jet - you should enable BUILD_POPPLER CMake flag")
  endif (NOT POPPLER_FOUND)

endif ()
add_includes_ldflags ("${POPPLER_LIBRARIES}" "${POPPLER_INCLUDE_DIRS}")

# zlib
find_package (ZLIB REQUIRED)    # zlib
add_includes_ldflags ("${ZLIB_LIBRARIES}" "${ZLIB_INCLUDE_DIRS}")

# pthreads
find_package (Threads REQUIRED) # pthreads
set (xournalpp_LDFLAGS ${xournalpp_LDFLAGS} ${CMAKE_THREAD_LIBS_INIT})

## Additional features ##

# CppUnit
option (ENABLE_CPPUNIT "Build CppUnit test instead of xournalpp application" OFF)
if (ENABLE_CPPUNIT)
  pkg_check_modules (CppUnit REQUIRED "cppunit >= 1.12-0")
  enable_testing ()
endif (ENABLE_CPPUNIT)

# Mathtex
option (ENABLE_MATHTEX "Mathtex support" ON)
if (ENABLE_MATHTEX AND WIN32)
    message (FATAL_ERROR "Mathtex is not supported on Windows for now")
endif (ENABLE_MATHTEX AND WIN32)

# Unstable features

option (UNSTABLE_LAYERS_SIDEBAR "Layers sidebar (unstable)" OFF)
option (UNSTABLE_HIGHDPI_FIXES "HighDPI Fixes (unstable)" OFF)

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
	install(FILES ui/pixmaps/application-x-xoj.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)

	install(FILES ui/pixmaps/gnome-mime-application-x-xopp.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)
	install(FILES ui/pixmaps/application-x-xopt.svg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/mimetypes/)

	install(FILES desktop/xournal.xml
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)
	install(FILES desktop/xournalpp.desktop
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
	install(FILES desktop/x-xoj.desktop
			DESTINATION /usr/share/mimelnk/application)
	install(FILES desktop/x-xopp.desktop
			DESTINATION /usr/share/mimelnk/application)
	
	install(FILES desktop/xournalpp.thumbnailer
			DESTINATION /usr/share/thumbnailers)
	install(FILES utility/usr/local/bin/xopp-recording.sh
			DESTINATION /usr/local/bin/
			PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)


	install(CODE "
		execute_process(COMMAND update-desktop-database)
		execute_process(COMMAND update-mime-database ${CMAKE_INSTALL_PREFIX}/share/mime)
		execute_process(COMMAND gtk-update-icon-cache -f -t ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor)"
	  )

endif ()

if (BUILD_POPPLER)
	# Manual install Poppler libs
	# RPATH is set before, so they will be loaded
	install(CODE "MESSAGE(\"Install Poppler workaround to ${CMAKE_INSTALL_PREFIX}\")")

	install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX})

	install(
	  CODE "file(GLOB POPPLER_LIBS LIST_DIRECTORIES false \"\${CMAKE_CURRENT_BINARY_DIR}/poppler-prefix/src/poppler-build/libpoppler.so.*\")"
	  CODE "file(INSTALL \${POPPLER_LIBS} DESTINATION ${POPPLER_LIB_DIR})"
	)

	install(
	  CODE "file(GLOB POPPLER_GLIB_LIBS LIST_DIRECTORIES false \"\${CMAKE_CURRENT_BINARY_DIR}/poppler-prefix/src/poppler-build/glib/libpoppler-glib.so.*\")"
	  CODE "file(INSTALL \${POPPLER_GLIB_LIBS} DESTINATION ${POPPLER_LIB_DIR})"
	)
endif()

# Uninstall target
configure_file (
  cmake/cmake_uninstall.cmake.in
  cmake/cmake_uninstall.cmake
  IMMEDIATE @ONLY
)

add_custom_target (uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake

  COMMENT "Uninstall entire Xournal++"
)

message ("
Configuration:
	Compiler:                   ${CMAKE_CXX_COMPILER}
	Mathtex enabled:            ${ENABLE_MATHTEX}
	CppUnit enabled:            ${ENABLE_CPPUNIT}

Unstable features:
	Layers sidebar:             ${UNSTABLE_LAYERS_SIDEBAR}
	HighDPI Fixes:              ${UNSTABLE_HIGHDPI_FIXES}

Static libraries:
	Poppler:                    ${BUILD_POPPLER}
	Boost:                      ${BUILD_BOOST}
")

option (CMAKE_DEBUG_INCLUDES_LDFLAGS "List include dirs and ldflags for xournalpp target" OFF)
mark_as_advanced (FORCE CMAKE_DEBUG_INCLUDES_LDFLAGS)
if (CMAKE_DEBUG_INCLUDES_LDFLAGS)
    message ("Include directories: ${xournalpp_INCLUDE_DIRS}")
    message ("LDFLAGS/LIBRARIES: ${xournalpp_LDFLAGS}")
endif (CMAKE_DEBUG_INCLUDES_LDFLAGS)
