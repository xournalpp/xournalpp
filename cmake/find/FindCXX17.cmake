
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
if (CXX17_FIND_COMPONENTS STREQUAL "")
#Nothing to do here
endif ()

# Warn on any unrecognized components
if (NOT extra_components STREQUAL "")
    set(extra_components ${want_components})
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

add_library(cxx17 INTERFACE IMPORTED)
target_compile_features(cxx17 INTERFACE cxx_std_17)
set(_found TRUE)

cmake_pop_check_state()

set(CXX17_FOUND ${_found} CACHE BOOL "TRUE if we can compile and link a program using cxx17" FORCE)

