cmake_minimum_required(VERSION 3.15)

####################################
# Project related Variables
# Also update changelog in debian folder!
####################################

project("Xournal++"
		VERSION 2.0.0
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
find_package(Lager REQUIRED CONFIG)
find_package(Zug REQUIRED CONFIG)
find_package(Boost REQUIRED)

# GLIB, GTK+, GThread

pkg_check_modules(xournalpp_deps REQUIRED IMPORTED_TARGET GLOBAL
		"glib-2.0 >= 2.32.0"
		"gtk+-3.0 >= 3.22.0"
		"gthread-2.0 >= 2.4.0"
		)

add_library(xournalpp_deff INTERFACE)
target_compile_definitions(xournalpp_deff
		INTERFACE -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_32
		INTERFACE -DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_22
		INTERFACE -DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE)
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

include(clang-tidy)

set(xournalpp_CXX_FLAGS ${CMAKE_CXX_FLAGS})

## Source building ##
add_subdirectory(base)