cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)
# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24 and up
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
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
      URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
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
target_link_libraries (test-units xoj::core xoj::util gtest)
target_compile_features(test-units PRIVATE cxx_std_20)
target_include_directories(test-units PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Define test-gtk-integration
###############################################################################

# Get all test-gtk test source files
file (GLOB_RECURSE test-gtk-sources
        gtk_tests/*.cpp
        )

# Define test-gtk-integration target
add_executable (test-gtk-integration EXCLUDE_FROM_ALL ${test-gtk-sources})
target_link_libraries (test-gtk-integration xoj::core xoj::util gtest_main gtest)
target_compile_features(test-gtk-integration PRIVATE cxx_std_20)
target_include_directories(test-gtk-integration PRIVATE "${PROJECT_BINARY_DIR}/test")

# Use a test-only catalog so new strings do not depend on the Crowdin merge schedule.
set(test_locale_dir "${PROJECT_BINARY_DIR}/test/locale")
set(test_po_file "${PROJECT_SOURCE_DIR}/test/files/locale/scribble.po")
set(test_catalog "${test_locale_dir}/pt_BR/LC_MESSAGES/${PROJECT_NAME}.mo")
add_custom_command(
  OUTPUT "${test_catalog}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${test_locale_dir}/pt_BR/LC_MESSAGES"
  COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" --check --check-format
          -o "${test_catalog}" "${test_po_file}"
  DEPENDS "${test_po_file}"
  VERBATIM
)
add_custom_target(test-translations DEPENDS "${test_catalog}")
add_dependencies(test-gtk-integration test-translations)


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
gtest_discover_tests(test-gtk-integration
        DISCOVERY_TIMEOUT 30
        PROPERTIES
        ENVIRONMENT "${TEST_ENV_VARS}"
        $<$<NOT:$<STREQUAL:${TEST_GDK_PIXBUF_LOADER_CACHE},"">>:
        DEPENDS create-gdk-pixbuf-loader-cache>
)

###############################################################################
# Define benchmarks: gtest-like app with benchmark units - not added to CTest or the `test` target
###############################################################################

file (GLOB_RECURSE benchmarks-sources benchmarks/*.cpp)
add_executable (benchmarks EXCLUDE_FROM_ALL ${benchmarks-sources})
target_link_libraries (benchmarks xoj::core xoj::util gtest_main gtest)
target_include_directories(benchmarks PRIVATE "${PROJECT_BINARY_DIR}/test")
target_compile_options(benchmarks PRIVATE "-O2") # Always optimize for benchmarks
