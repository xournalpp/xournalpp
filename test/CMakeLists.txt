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

# XojPreviewExtractor
add_executable (test-xojPreviewExtractor $<TARGET_OBJECTS:xournalpp-core> $<TARGET_OBJECTS:xournalpp-test-base>
    util/XojPreviewExtractorTest.cpp
)
add_dependencies (test-xojPreviewExtractor xournalpp-core xournalpp-test-base util)
target_link_libraries (test-xojPreviewExtractor ${xournalpp_LDFLAGS} ${CppUnit_LDFLAGS})

# LoadHandler
add_executable (test-loadHandler $<TARGET_OBJECTS:xournalpp-core> $<TARGET_OBJECTS:xournalpp-test-base>
    control/LoadHandlerTest.cpp
)
add_dependencies (test-loadHandler xournalpp-core xournalpp-test-base util)
target_link_libraries (test-loadHandler ${xournalpp_LDFLAGS} ${CppUnit_LDFLAGS})

## CTest ##
add_test (XojPreviewExtractor test-xojPreviewExtractor)
add_test (LoadHandler test-loadHandler)
