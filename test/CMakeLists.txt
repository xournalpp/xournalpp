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

unset (xournalpp-test_SOURCES)
file (GLOB_RECURSE xournalpp-test_SOURCES
  *.cpp
)

if (NOT TEST_CHECK_SPEED)
  list (REMOVE_ITEM xournalpp-test_SOURCES ${PROJECT_SOURCE_DIR}/test/SpeedTest.cpp) # it's not needed then
endif (NOT TEST_CHECK_SPEED)

add_executable (xournalpp-test
  $<TARGET_OBJECTS:xournalpp-core>
  ${xournalpp-test_SOURCES}
)

add_dependencies (xournalpp-test
  xournalpp-core
  poppler
  util
)

target_link_libraries (xournalpp-test
  ${xournalpp_LDFLAGS}
  ${CppUnit_LDFLAGS}
)

## CTest ##
add_test (cpp-unit xournalpp-test)
