# CMake file for building the gtest suite

if(MACOSX)
    message(STATUS "MACOSX discoverd: Adding ${CMAKE_INSTALL_PREFIX}/lib as @rpath")
    # This is needed as gtest_discover_tests() results in the test executables
    # being called during make (right after linking).
    # If the @rpath is not set at that point this will fail with "dyld: Library not loaded"
    # or similar.
    # By default @rpath would only be set with make install.
    # Hence it needs to be set during build already.
    # In case this affects installs on MACOSX in a negative way 
    #
    #     set(CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    #
    # could also be used instead of the following lines.
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

# Load configure file including constants and helper Macros
configure_file (
    config-test.h.in
    config-test.h
    ESCAPE_QUOTES @ONLY
)
include(GoogleTest)


###############################################################################
# Define test-units
###############################################################################

# Get all unit test source files
file (GLOB_RECURSE unit_sources_SOURCES_RECURSE
  unit_tests/*.cpp
)

# Define test-units target
add_executable (test-units EXCLUDE_FROM_ALL 
    $<TARGET_OBJECTS:xournalpp-core>
    ${unit_sources_SOURCES_RECURSE}
)
add_dependencies (test-units xournalpp-core)
target_link_libraries (test-units ${xournalpp_LDFLAGS} std::filesystem gtest_main)

# Add gtest to include directories
# This enables usage of 
#     #include <config-test.h>
target_include_directories(test-units PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Register Tests
###############################################################################
# MacOs needs a bit longer to find the tests on azure
# Hence, set the timeout to 30s (default 5s)
gtest_discover_tests(test-units DISCOVERY_TIMEOUT 30)



