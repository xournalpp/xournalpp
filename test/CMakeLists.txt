cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

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

# Download and Build GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
# Prevent reloading if already downloaed
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Prevent GoogleTest from being installed with xournalpp
# This is not strictly necessary as add_subdirectory is called on ./test with
# EXCLUDE_FROM_ALL, but better safe than sorry
option(INSTALL_GTEST "Enable installation of googletest." OFF)
FetchContent_MakeAvailable(googletest)

# Load configure file including constants and helper Macros
configure_file (
    config-test.h.in
    config-test.h
    ESCAPE_QUOTES @ONLY
)

###############################################################################
# Define test-units
###############################################################################

# Get all unit test source files
file (GLOB_RECURSE test-units-sources
  unit_tests/*.cpp
)

# Define test-units target
add_executable (test-units EXCLUDE_FROM_ALL ${test-units-sources})
target_link_libraries (test-units xoj::core xoj::util std::filesystem gtest_main)
target_include_directories(test-units PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Discover and Register Tests
###############################################################################
# MacOs needs a bit longer to find the tests on azure
# Hence, set the timeout to 30s (default 5s)
include(GoogleTest)
gtest_discover_tests(test-units DISCOVERY_TIMEOUT 30)
