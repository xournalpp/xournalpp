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
# Define unit tests for util
###############################################################################

# Get all unit test source files
file (GLOB_RECURSE UNIT_TESTS_UTIL_SOURCES_RECURSE
  unit_tests/util/*.cpp
)

# Define unit-tests-util target
add_executable (unit-tests-util EXCLUDE_FROM_ALL 
    $<TARGET_OBJECTS:util>
    ${UNIT_TESTS_UTIL_SOURCES_RECURSE}
)
# NOTE: It is intentional that only util is a dependency
# There is no good reason why a util function should depend on other code in the project,
# with the sole exception of other util code maybe.
# Furthermore, this improves build and test time significantly.
add_dependencies (unit-tests-util util)
target_link_libraries (unit-tests-util ${xournalpp_LDFLAGS} std::filesystem gtest_main)

# Add gtest to include directories
# This enables usage of 
#     #include <config-test.h>
target_include_directories(unit-tests-util PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Define unit tests for model
###############################################################################

# Get all unit test source files
file (GLOB_RECURSE UNIT_TESTS_MODEL_SOURCES_RECURSE
  unit_tests/model/*.cpp
)

# Define unit-tests-model target
add_executable (unit-tests-model EXCLUDE_FROM_ALL 
    $<TARGET_OBJECTS:xournalpp-core>
    ${UNIT_TESTS_MODEL_SOURCES_RECURSE}
)
add_dependencies (unit-tests-model xournalpp-core)
target_link_libraries (unit-tests-model ${xournalpp_LDFLAGS} std::filesystem gtest_main)

# Add gtest to include directories
# This enables usage of 
#     #include <config-test.h>
target_include_directories(unit-tests-model PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Define unit tests for control
###############################################################################

# Get all unit test source files
file (GLOB_RECURSE UNIT_TESTS_CONTROL_SOURCES_RECURSE
  unit_tests/control/*.cpp
)

# Define unit-tests-control target
add_executable (unit-tests-control EXCLUDE_FROM_ALL 
    $<TARGET_OBJECTS:xournalpp-core>
    ${UNIT_TESTS_CONTROL_SOURCES_RECURSE}
)
add_dependencies (unit-tests-control xournalpp-core)
target_link_libraries (unit-tests-control ${xournalpp_LDFLAGS} std::filesystem gtest_main)

# Add gtest to include directories
# This enables usage of 
#     #include <config-test.h>
target_include_directories(unit-tests-control PRIVATE "${PROJECT_BINARY_DIR}/test")

###############################################################################
# Register Tests
###############################################################################
# Convenience target to build all unit-tests
add_custom_target(all-unit-tests)
add_dependencies(all-unit-tests 
      unit-tests-util 
      unit-tests-model 
      unit-tests-control
)
# MacOs needs a bit longer to find the tests on azure
# Hence, set the timeout to 30s (default 5s)
gtest_discover_tests(unit-tests-util DISCOVERY_TIMEOUT 30)
gtest_discover_tests(unit-tests-model DISCOVERY_TIMEOUT 30)
gtest_discover_tests(unit-tests-control DISCOVERY_TIMEOUT 30)



