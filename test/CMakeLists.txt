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
    # The default GDK pixbuf loaders locations may be using relative paths.
    # These paths are wrong when running tests, so override them.
    set (TEST_GDK_PIXBUF_LOADER_CACHE "${CMAKE_CURRENT_BINARY_DIR}/loaders.cache")
    set (TEST_ENV_VARS "${TEST_ENV_VARS}" GDK_PIXBUF_MODULE_FILE="${TEST_GDK_PIXBUF_LOADER_CACHE}")
    add_custom_target (test-create-gdk-pixbuf-loader-cache
      COMMAND "${PROJECT_SOURCE_DIR}/mac-setup/setup-gdk-pixbuf.sh" "${TEST_GDK_PIXBUF_LOADER_CACHE}"
      BYPRODUCTS "${TEST_GDK_PIXBUF_LOADER_CACHE}"
      COMMENT "Create gdk-pixbuf loader cache for testing")
    add_test(create-gdk-pixbuf-loader-cache
      "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target test-create-gdk-pixbuf-loader-cache)
endif()

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Prevent GoogleTest from being installed with xournalpp
# This is not strictly necessary as add_subdirectory is called on ./test with
# EXCLUDE_FROM_ALL, but better safe than sorry
option(INSTALL_GTEST "Enable installation of googletest." OFF)

# Explicit flag to enable gtest download
option(DOWNLOAD_GTEST "Force download of googletest." OFF)

if (${DOWNLOAD_GTEST})
  message(STATUS "Downloading gtest...")
  # Download and build GoogleTest
  include(FetchContent)
  FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
  )
  # Prevent reloading if already downloaed
  set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
  FetchContent_MakeAvailable(googletest)
else ()
  # Use system GoogleTest
  find_package(GTest)
  if (NOT ${GTEST_FOUND})
    message(FATAL_ERROR
      "googletest not found. If you would like to download it automatically, add\n"
      "    -DDOWNLOAD_GTEST=on\n"
      "to the cmake command."
    )
  endif ()
endif ()

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
  main.cpp
)

# Define test-units target
add_executable (test-units EXCLUDE_FROM_ALL ${test-units-sources})
target_link_libraries (test-units xoj::core xoj::util std::filesystem gtest)
target_include_directories(test-units PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Discover and Register Tests
###############################################################################
# MacOs needs a bit longer to find the tests on azure
# Hence, set the timeout to 30s (default 5s)
include(GoogleTest)
gtest_discover_tests(test-units
  DISCOVERY_TIMEOUT 30
  PROPERTIES
    ENVIRONMENT "${TEST_ENV_VARS}"
  $<$<NOT:$<STREQUAL:${TEST_GDK_PIXBUF_LOADER_CACHE},"">>:
    DEPENDS create-gdk-pixbuf-loader-cache>
)
