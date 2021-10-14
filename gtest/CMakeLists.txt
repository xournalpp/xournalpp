# CMake file for building the gtest suite

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
add_dependencies (test-units xournalpp-core)
target_link_libraries (test-units ${xournalpp_LDFLAGS} ${PopplerGlib_LDFLAGS} std::filesystem gtest_main)
gtest_discover_tests(test-units)



