###
# Xournal++
#
# Static definitions for Xournal++
#
# @author Xournal++ Team
# https://github.com/xournalpp/xournalpp
#
# @license GNU GPLv2 or later
###

include(CMakePushCheckState)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)

cmake_push_check_state()

set(CMAKE_REQUIRED_QUIET ${})

# All of our tests require C++17 or later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Normalize and check the component list we were given
set(want_components ${CXX17_FIND_COMPONENTS})
set(extra_components ${CXX17_FIND_COMPONENTS})

# Warn on any unrecognized components
if (extra_components)
    list(REMOVE_ITEM extra_components optional filesystem map)
    foreach (component IN LISTS extra_components)
        message(WARNING "Extraneous find_package component for Filesystem: ${component}")
    endforeach ()
endif ()

if ("optional" IN_LIST want_components)
    check_include_file_cxx("optional" CXX_OPTIONAL_FOUND)
    if (CXX17_FIND_REQUIRED AND NOT CXX_OPTIONAL_FOUND)
        message(FATAL_ERROR "optional header not found")
    endif ()

endif ()
if ("filesystem" IN_LIST want_components)
    if (CXX17_FIND_REQUIRED)
        find_package(Filesystem REQUIRED COMPONENTS Final Experimental Boost)
    else (CXX17_FIND_REQUIRED)
        find_package(Filesystem COMPONENTS Final Experimental Boost)
    endif (CXX17_FIND_REQUIRED)
endif ()
if ("map" IN_LIST want_components)
    string(CONFIGURE [[
        #include <map>

        int main()
        {
        	std::map<int, int> id_to_name{{1, 1}, {2, 2}};
        	std::map<int, int> important{{5, 5}};
        	important.insert(id_to_name.extract(2));
        }

    ]] code @ONLY)

    # Try to compile a simple filesystem program without any linker flags
    check_cxx_source_compiles("${code}" CXX_17_MAP_FOUND)
    if (CXX17_FIND_REQUIRED AND NOT CXX_17_MAP_FOUND)
        message(FATAL_ERROR "map splicing is not supported but required")
    endif ()
endif ()

if (TARGET cxx17)
    # This module has already been processed. Don't do it again.
    return()
endif ()

add_library(cxx17 INTERFACE)
target_compile_features(cxx17 INTERFACE cxx_std_17)
set(_found TRUE)

cmake_pop_check_state()

set(CXX17_FOUND ${_found} CACHE INTERNAL "")

