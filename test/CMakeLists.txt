## Additional config ##

option (TEST_CHECK_SPEED "Show speed benchmarks of tests" OFF)

configure_file (
    config-test.h.in
    config-test.h
    ESCAPE_QUOTES @ONLY
)

## Building ##

include_directories (
    "${PROJECT_BINARY_DIR}/test"
    "${PROJECT_SOURCE_DIR}/test"
    ${CppUnit_INCLUDE_DIRS}
)

# Test base
set (xournalpp-test_SOURCES
    TestMain.cpp
)
if (TEST_CHECK_SPEED)
    set (xournalpp-test_SOURCES ${xournalpp-test_SOURCES} ${PROJECT_SOURCE_DIR}/test/SpeedTest.cpp)
endif ()
add_library (xournalpp-test-base OBJECT
    ${xournalpp-test_SOURCES}
)

## ------------------------

# These dirs are xournalpp only so it's safe to add then recursively
file (GLOB_RECURSE util_sources_SOURCES_RECURSE
  util/*.cpp
)

# Util Test
add_executable (test-util $<TARGET_OBJECTS:xournalpp-core> $<TARGET_OBJECTS:xournalpp-test-base>
    ${util_sources_SOURCES_RECURSE}
)
add_dependencies (test-util xournalpp-core xournalpp-test-base util)
target_link_libraries (test-util ${xournalpp_LDFLAGS} ${CppUnit_LDFLAGS} std::filesystem)

## ------------------------

# LoadHandler
add_executable (test-loadHandler $<TARGET_OBJECTS:xournalpp-core> $<TARGET_OBJECTS:xournalpp-test-base>
    control/LoadHandlerTest.cpp
)
add_dependencies (test-loadHandler xournalpp-core xournalpp-test-base util)
target_link_libraries (test-loadHandler ${xournalpp_LDFLAGS} ${CppUnit_LDFLAGS} std::filesystem)

## CTest ##
add_test (util test-util)
add_test (LoadHandler test-loadHandler)




