cmake_minimum_required(VERSION 3.11)
cmake_policy(VERSION 3.11)

## Project related Variables
## Also update changelog in debian folder!
project(xournalpp
        VERSION 1.1.0
        DESCRIPTION "Xournal++ - Open source hand note-taking program"
        HOMEPAGE_URL "https://xournalpp.github.io/"
        LANGUAGES CXX C)
# according to https://cmake.org/cmake/help/latest/command/project.html fix suffix
set(VERSION_SUFFIX "+dev")
set(PROJECT_VERSION "${PROJECT_VERSION}${VERSION_SUFFIX}")
set(xournalpp_VERSION ${PROJECT_VERSION})
set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})

## CMAKE_Variables
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find" "${PROJECT_SOURCE_DIR}/cmake/include")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # enable clang tidy to use correct includes
set_directory_properties(PROPERTIES EP_BASE "${PROJECT_BINARY_DIR}/external")

add_definitions(-D_USE_MATH_DEFINES)

# package version
include(Version)
core_find_git_rev(RELEASE_IDENTIFIER)
string(TIMESTAMP PACKAGE_TIMESTAMP "%Y%m%d.%H%M" UTC)

include(TargetArch)
target_architecture(PACKAGE_ARCH)

configure_file(cmake/VERSION.in VERSION)

configure_file(
        cmake/postinst.in
        cmake/postinst
        @ONLY
)

include(FindPkgConfig)

set(PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")

# Git repo info
include(GitRepo)

################################
# CMake os-fixups
# this section should decrease in size for newer CMake versions.
################################

################################
# Project compiler flags
################################
set(xournalpp_CXX_FLAGS ${CMAKE_CXX_FLAGS})

if (MSVC)
    set(xournalpp_CXX_FLAGS ${xournalpp_CXX_FLAGS} -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif ()

## Libraries ##

macro(add_includes_ldflags LDFLAGS INCLUDES)
    list(APPEND xournalpp_external_deps "${LDFLAGS}")
    include_directories(SYSTEM ${INCLUDES})
endmacro(add_includes_ldflags LDFLAGS INCLUDES)

##############################
# c++17 support
##############################
find_package(CXX17 REQUIRED COMPONENTS optional)
find_package(Filesystem REQUIRED COMPONENTS ghc Final Experimental)

##############################
# stacktrace library
# Todo use platform independent stacktrace library
# Currently this works only for linux systems
##############################
if (NOT WIN32)
    find_package(Backtrace REQUIRED)
endif ()
add_library(gnu_stacktrace INTERFACE)
target_include_directories(gnu_stacktrace INTERFACE $<$<BOOL:${Backtrace_FOUND}>:${Backtrace_INCLUDE_DIRS}>)
target_link_libraries(gnu_stacktrace INTERFACE $<$<BOOL:${Backtrace_FOUND}>:${Backtrace_LIBRARIES}>)
target_link_options(gnu_stacktrace INTERFACE $<$<AND:$<NOT:$<PLATFORM_ID:Windows>>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>>:-rdynamic>)
add_library(xoj::stacktrace ALIAS gnu_stacktrace)

##############################
# other externals
##############################

set(pkg-module-spec "")
list(APPEND pkg-module-spec "glib-2.0 >= 2.32.0")
list(APPEND pkg-module-spec "gtk+-3.0 >= 3.18.9")
list(APPEND pkg-module-spec "poppler-glib >= 0.41.0")
list(APPEND pkg-module-spec "gthread-2.0 >= 2.4.0")
list(APPEND pkg-module-spec "libxml-2.0 >= 2.0.0")
list(APPEND pkg-module-spec "libzip >= 1.0.1")
list(APPEND pkg-module-spec "portaudiocpp >= 12")
list(APPEND pkg-module-spec "sndfile >= 1.0.25")

pkg_check_modules(Lua IMPORTED_TARGET "lua5.3")
if (NOT Lua_FOUND)
    pkg_check_modules(Lua IMPORTED_TARGET "lua >= 5.3")
endif ()

pkg_check_modules(ExternalModules REQUIRED IMPORTED_TARGET ${pkg-module-spec})
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
if (APPLE)
    pkg_check_modules(MacIntegration IMPORTED_TARGET "gtk-mac-integration")
elseif (NOT WIN32)
    find_package(Backtrace REQUIRED)
endif ()

add_library(external_modules INTERFACE)
target_link_libraries(external_modules INTERFACE
        PkgConfig::ExternalModules
        $<$<TARGET_EXISTS:PkgConfig::MacIntegration>:PkgConfig::MacIntegration>
        $<$<TARGET_EXISTS:PkgConfig::Lua>:PkgConfig::Lua>
        ZLIB::ZLIB
        Threads::Threads
        xoj::stacktrace
        $<$<PLATFORM_ID:Linux>:-lX11 -lXi>

        )
target_compile_definitions(external_modules INTERFACE
        GLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_32
        GDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18
        $<$<TARGET_EXISTS:PkgConfig::MacIntegration>:MAC_INTEGRATION>
        $<$<TARGET_EXISTS:PkgConfig::Lua>:ENABLE_PLUGINS>
        )
target_compile_options(external_modules INTERFACE
        $<$<AND:$<PLATFORM_ID:Windows>,$<CXX_COMPILER_ID:GNU>>:-rdynamic>
        )
add_library(xoj::external_modules ALIAS external_modules)

# CppUnit Todo move to test
option(ENABLE_CPPUNIT "Build CppUnit test instead of xournalpp application" OFF)
if (ENABLE_CPPUNIT)
endif (ENABLE_CPPUNIT)

if (Lua_FOUND)
    message("Enable Xournal++ Plugins")
    set(ENABLE_PLUGINS "true")
endif ()

unset(add_includes_ldflags)

#[[#
# DO NOT INCLUDE LIBRARIES WITH pkg_check_modules AFTER THIS LINE!!!
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Fix linker flag handling for MacOS builds where frameworks are separated from their flag
    string(REPLACE "-framework;" "-framework " xournalpp_external_deps "${xournalpp_external_deps}")
endif ()]]

configure_file(
        src/config-features.h.in
        src/config-features.h
        ESCAPE_QUOTES @ONLY
)
configure_file(
        src/filesystem.h.in
        src/filesystem.h
)

## I18n ##
add_subdirectory(po)

## Configuration headers and development options ##

# Development options
option(DEV_CALL_LOG "Call log" OFF)

# Debug options
option(DEBUG_INPUT "Input debugging, e.g. eraser events etc" OFF)
option(DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS "Input debugging, print all motion events" OFF)
option(DEBUG_INPUT_GDK_PRINT_EVENTS "Input debugging, print all GDK events" OFF)
option(DEBUG_RECOGNIZER "Shape recognizer debug: output score etc" OFF)
option(DEBUG_SHEDULER "Scheduler debug: show jobs etc" OFF)
option(DEBUG_SHOW_ELEMENT_BOUNDS "Draw a surrounding border to all elements" OFF)
option(DEBUG_SHOW_REPAINT_BOUNDS "Draw a border around all repaint rects" OFF)
option(DEBUG_SHOW_PAINT_BOUNDS "Draw a border around all painted rects" OFF)
mark_as_advanced(FORCE
        DEBUG_INPUT DEBUG_RECOGNIZER DEBUG_SHEDULER DEBUG_SHOW_ELEMENT_BOUNDS DEBUG_SHOW_REPAINT_BOUNDS DEBUG_SHOW_PAINT_BOUNDS
        )

# Advanced development config
set(DEV_TOOLBAR_CONFIG "toolbar.ini" CACHE STRING "Toolbar config file name")
set(DEV_SETTINGS_XML_FILE "settings.xml" CACHE STRING "Settings file name")
set(DEV_PRINT_CONFIG_FILE "print-config.ini" CACHE STRING "Print config file name")
set(DEV_METADATA_FILE "metadata.ini" CACHE STRING "Metadata file name")
set(DEV_METADATA_MAX_ITEMS 50 CACHE STRING "Maximal amount of metadata elements")
set(DEV_ERRORLOG_DIR "errorlogs" CACHE STRING "Directory where errorlogfiles will be placed")
set(DEV_FILE_FORMAT_VERSION 4 CACHE STRING "File format version" FORCE)

option(DEV_ENABLE_GCOV "Build with gcov support" OFF) # Enabel gcov support – expanded in src/
option(DEV_CHECK_GTK3_COMPAT "Adds a few compiler flags to check basic GTK3 upgradeability support (still compiles for GTK2!)")
if (DEV_CHECK_GTK3_COMPAT)
    add_definitions(-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE)
endif ()

mark_as_advanced(FORCE
        DEV_TOOLBAR_CONFIG DEV_SETTINGS_XML_FILE DEV_PRINT_CONFIG_FILE DEV_METADATA_FILE DEV_METADATA_MAX_ITEMS
        DEV_ENABLE_GCOV DEV_CHECK_GTK3_COMPAT
        )

configure_file(
        src/config.h.in
        src/config.h
        ESCAPE_QUOTES @ONLY
)

configure_file(
        src/config-debug.h.in
        src/config-debug.h
        ESCAPE_QUOTES @ONLY
)

configure_file(
        src/config-dev.h.in
        src/config-dev.h
        ESCAPE_QUOTES @ONLY
)

configure_file(
        src/config-paths.h.in
        src/config-paths.h
        ESCAPE_QUOTES @ONLY
)

option(xournalpp_LINT "enable lint (clang-tidy) target" OFF)
if (xournalpp_LINT)
    include(clang-tidy)
endif ()

## Source building ##
add_subdirectory(src)

## Final targets and installing ##

# Install resources
install(DIRECTORY ui
        DESTINATION "share/xournalpp"
        COMPONENT xournalpp
        )
install(DIRECTORY plugins
        DESTINATION "share/xournalpp"
        COMPONENT xournalpp
        )
install(DIRECTORY resources
        DESTINATION "share/xournalpp"
        COMPONENT xournalpp
        )

# Install desktop shortcuts for Linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Installing desktop files")

    # Install icons
    install(FILES ui/pixmaps/com.github.xournalpp.xournalpp.svg
            DESTINATION share/icons/hicolor/scalable/apps)
    # Symlink are not easy to use with CMake, therefore simple install a copy...
    install(FILES ui/pixmaps/application-x-xopp.svg
            DESTINATION share/icons/hicolor/scalable/mimetypes/)
    install(FILES ui/pixmaps/application-x-xopt.svg
            DESTINATION share/icons/hicolor/scalable/mimetypes/)
    install(FILES ui/pixmaps/application-x-xojpp.svg
            DESTINATION share/icons/hicolor/scalable/mimetypes/)
    install(FILES ui/pixmaps/gnome-mime-application-x-xopp.svg
            DESTINATION share/icons/hicolor/scalable/mimetypes/)
    install(FILES ui/pixmaps/gnome-mime-application-x-xopt.svg
            DESTINATION share/icons/hicolor/scalable/mimetypes/)
    install(FILES desktop/com.github.xournalpp.xournalpp.xml
            DESTINATION share/mime/packages)
    install(FILES desktop/com.github.xournalpp.xournalpp.desktop
            DESTINATION share/applications)
    install(FILES desktop/x-xojpp.desktop
            DESTINATION share/mimelnk/application)
    install(FILES desktop/x-xopp.desktop
            DESTINATION share/mimelnk/application)
    install(FILES desktop/x-xopt.desktop
            DESTINATION share/mimelnk/application)
    install(FILES desktop/com.github.xournalpp.xournalpp.thumbnailer
            DESTINATION share/thumbnailers)
    install(FILES desktop/com.github.xournalpp.xournalpp.appdata.xml
            DESTINATION share/metainfo)
endif ()

# Uninstall target
configure_file(
        cmake/cmake_uninstall.cmake.in
        cmake/cmake_uninstall.cmake
        @ONLY
)

add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake

        COMMENT "Uninstall entire Xournal++"
        )

message("
Configuration:
	Compiler:                   ${CMAKE_CXX_COMPILER}
	CppUnit enabled:            ${ENABLE_CPPUNIT}
	Filesystem library:         ${CXX_FILESYSTEM_NAMESPACE}
")

option(CMAKE_DEBUG_INCLUDES_LDFLAGS "List include dirs and ldflags for xournalpp target" OFF)
mark_as_advanced(FORCE CMAKE_DEBUG_INCLUDES_LDFLAGS)
if (CMAKE_DEBUG_INCLUDES_LDFLAGS)
    message("Include directories: ${xournalpp_INCLUDE_DIRS}")
    message("LDFLAGS/LIBRARIES: ${xournalpp_external_deps}")
endif (CMAKE_DEBUG_INCLUDES_LDFLAGS)

##############################
# Packaging options
# !!! only override defaults if necessary !!!
##############################

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}~git${PACKAGE_TIMESTAMP}-${RELEASE_IDENTIFIER}-${DISTRO_CODENAME})
set(CPACK_OUTPUT_FILE_PREFIX packages)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/debian/package_description")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Xournal++")
set(CPACK_PACKAGE_FILE_NAME "xournalpp-${PROJECT_VERSION}-${DISTRO_NAME}-${DISTRO_CODENAME}-${PACKAGE_ARCH}")
set(CPACK_PACKAGE_CONTACT "Andreas Butti <andreasbutti@gmail.com>")

# .deb package options
set(CPACK_DEBIAN_PACKAGE_RELEASE 2)
set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libglib2.0-0 (>= 2.32), libgtk-3-0 (>= 3.18), libpoppler-glib8 (>= 0.41.0), libxml2 (>= 2.0.0), libportaudiocpp0 (>= 12), libsndfile1 (>= 1.0.25), liblua5.3-0, libzip4 (>= 1.0.1) | libzip5, zlib1g, libc6")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "texlive-base, texlive-latex-extra")  # Latex tool
# Use debian's arch scheme; we only care about x86/amd64 for now but feel free to add more
if (${PACKAGE_ARCH} STREQUAL "x86_64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif ()

if (CPACK_GENERATOR STREQUAL "DEB")
    message("Preparing documentation for DEB package")
    add_custom_target(package_docummentation ALL)

    #Compress changelog and save it as share/doc/xournalpp/changelog.Debian.gz
    add_custom_command(TARGET package_docummentation PRE_BUILD
            COMMAND gzip -c -9 -n "${PROJECT_SOURCE_DIR}/debian/changelog" > "changelog.Debian.gz" VERBATIM)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.Debian.gz" DESTINATION "share/doc/xournalpp/")
endif ()

if (NOT DEFINED CPACK_GENERATOR)
    set(CPACK_GENERATOR "TGZ")
endif ()

include(CPack)
