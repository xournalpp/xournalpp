cmake_minimum_required(VERSION 3.15)

####################################
# Project related Variables
# Also update changelog in debian folder!
####################################

project("Xournal++"
		VERSION 1.2.0
		DESCRIPTION "Xournal++ - Open source hand note-taking program"
		HOMEPAGE_URL "https://github.com/xournalpp/xournalpp"
		LANGUAGES CXX C)

#Bypass CMAKE-Version requirements (only numbers in the Version)
set(VERSION_SUFFIX "+dev")
set(PROJECT_VERSION "${PROJECT_VERSION}${VERSION_SUFFIX}")
set(Xournal++_VERSION ${PROJECT_VERSION})
set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})

set(PROJECT_PACKAGE "xournalpp")

####################################
# CMake Globs
####################################

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find" "${PROJECT_SOURCE_DIR}/cmake/include")
set(CMAKE_GENERATOR_DIR ${CMAKE_BINARY_DIR}/generated)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-D_USE_MATH_DEFINES)

# package version
include(Version)
core_find_git_rev(RELEASE_IDENTIFIER)
string(TIMESTAMP PACKAGE_TIMESTAMP "%Y%m%d.%H%M" UTC)

include(TargetArch)
target_architecture(PACKAGE_ARCH)

configure_file(cmake/VERSION.in VERSION)

configure_file (
    cmake/postinst.in
    cmake/postinst
    @ONLY
)

include (FindPkgConfig)

set(PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")

# Git repo info
include(GitRepo)

################################
# CMake os-fixups
# this section should decrease in size for newer CMake versions.
################################
include(TargetSystem)

################################
# External dependencies
################################

find_package(CXX17 REQUIRED COMPONENTS optional filesystem)
find_package(Threads REQUIRED)
#Todo (fabian): find_package(GTest REQUIRED)
find_package(Lager CONFIG)
find_package(Zug CONFIG)
#find_package(ZLIB REQUIRED)

# GLIB, GTK+, GThread
# libxml Poppler libzip portaudio sndfile

add_definitions(-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_32)
add_definitions(-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18)
pkg_check_modules(xournalpp_deps REQUIRED IMPORTED_TARGET GLOBAL
		"glib-2.0 >= 2.32.0"
		"gtk+-3.0 >= 3.18.9"
		"gthread-2.0 >= 2.4.0"
		#"libxml-2.0 >= 2.0.0"
		#"poppler-glib >= 0.41.0"
		#"libzip >= 1.0.1"
		#"portaudiocpp >= 12"
		#"sndfile >= 1.0.25"
		)

################################
# Runtime linked
# Todo (fabian): Add portaudiocpp, Lua
################################

################################
# Internal dependencies
# Xournalpp targets
################################
## I18n Translations ##
add_subdirectory(po)

## ui files ##
include(glib-compile-resources)
glib_compile_resources(Xournal++UI ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# enforce strong guarantees
add_definitions(-DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE)
include(clang-tidy)

set(xournalpp_CXX_FLAGS ${CMAKE_CXX_FLAGS})

## Source building ##
add_subdirectory(base)

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

# Install desktop shortcuts for Linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message("Installing desktop files")

	# Install icons
	install(FILES ui/pixmaps/com.github.xournalpp.xournalpp.svg
			DESTINATION share/icons/hicolor/scalable/apps)

	# Symlink are not easy to use with CMake, therefor simple install a copy...
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
configure_file (
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
")

option(CMAKE_DEBUG_INCLUDES_LDFLAGS "List include dirs and ldflags for xournalpp target" OFF)
mark_as_advanced(FORCE CMAKE_DEBUG_INCLUDES_LDFLAGS)
if (CMAKE_DEBUG_INCLUDES_LDFLAGS)
	message("Include directories: ${xournalpp_INCLUDE_DIRS}")
	message("LDFLAGS/LIBRARIES: ${xournalpp_LDFLAGS}")
endif (CMAKE_DEBUG_INCLUDES_LDFLAGS)


# Packaging options
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}~git${PACKAGE_TIMESTAMP}-${RELEASE_IDENTIFIER}-${DISTRO_CODENAME})
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
set(CPACK_OUTPUT_FILE_PREFIX packages)
set(CPACK_PACKAGE_NAME "${PROJECT_PACKAGE}")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME "xournalpp-${PROJECT_VERSION}-${DISTRO_NAME}-${DISTRO_CODENAME}-${PACKAGE_ARCH}")

# .deb package options
set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${PACKAGE_URL})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreas Butti <andreasbutti@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
		"libglib2.0-0 (>= 2.32), libgtk-3-0 (>= 3.18), libpoppler-glib8 (>= 0.41.0), libxml2 (>= 2.0.0), libportaudiocpp0 (>= 12), libsndfile1 (>= 1.0.25), liblua5.3-0, libzip4 (>= 1.0.1), zlib1g")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "texlive-base, texlive-latex-extra")  # Latex tool
# Use debian's arch scheme; we only care about x86/amd64 for now but feel free to add more
if (${PACKAGE_ARCH} STREQUAL "x86_64")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif ()

if (NOT DEFINED CPACK_GENERATOR)
	set(CPACK_GENERATOR "TGZ")
endif ()

include(CPack)
