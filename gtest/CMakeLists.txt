# CMake file for building the gtest suite

if(MACOSX)
    message(STATUS "MACOSX discoverd: Adding ${CMAKE_INSTALL_PREFIX}/lib as @rpath")
    # This is needed as gtest_discover_tests() results in the test executables
    # being called during make (right after linking).
    # If the @rpath is not set at that point this will fail with "dyld: Library not loaded"
    # or similar.
    # By default @rpath would only be set with make install.
    # Hence it needs to be set during build already.
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()
# Load configure file including constants and helper Macros
configure_file (
    config-test.h.in
    config-test.h
    ESCAPE_QUOTES @ONLY
)

# build
include_directories (
    "${PROJECT_BINARY_DIR}/gtest"
    "${PROJECT_SOURCE_DIR}/gtest"
)
include(GoogleTest)


## utils

# Get all unit tests
file (GLOB_RECURSE unit_sources_SOURCES_RECURSE
  unit_tests/*.cpp
)

# Build all unit tests
add_executable (test-units $<TARGET_OBJECTS:xournalpp-core>
    ${unit_sources_SOURCES_RECURSE}
)

message(STATUS "Test: LDFLAGS: ${xournalpp_LDFLAGS}")
add_dependencies (test-units xournalpp-core)
target_link_libraries (test-units ${xournalpp_LDFLAGS} std::filesystem gtest_main)
gtest_discover_tests(test-units)



