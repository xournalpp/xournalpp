## Additional config ##
enable_testing()
pkg_check_modules(CppUnit REQUIRED IMPORTED_TARGET "cppunit >= 1.12-0")
option(TEST_CHECK_SPEED "Show speed benchmarks of tests" OFF)

configure_file(
        config-test.h.in
        config-test.h
        ESCAPE_QUOTES @ONLY
)

## Building ##

set(test-include-dirs ${PROJECT_BINARY_DIR}/test ${PROJECT_SOURCE_DIR}/test)

file(GLOB_RECURSE xournalpp-test-util-sources util/*.cpp)

# Test base
list(APPEND xournalpp-test-base-sources TestMain.cpp)
if (TEST_CHECK_SPEED)
    list(APPEND xournalpp-test-base-sources SpeedTest.cpp)
endif ()

## ------------------------

# Util Test
add_executable(test-util ${xournalpp-test-base-sources} ${xournalpp-test-util-sources})
target_link_libraries(test-util PRIVATE PkgConfig::CppUnit xoj::core xoj::util)
target_compile_features(test-util PRIVATE cxx_std_17)
target_include_directories(test-util PRIVATE ${test-include-dirs})
## ------------------------

# LoadHandler
add_executable(test-loadHandler ${xournalpp-test-base-sources} control/LoadHandlerTest.cpp)
target_include_directories(test-loadHandler PRIVATE ${test-include-dirs})
target_link_libraries(test-loadHandler PRIVATE PkgConfig::CppUnit xoj::core xoj::util)
target_compile_features(test-loadHandler PRIVATE cxx_std_17)

## CTest ##
add_test(util test-util)
add_test(LoadHandler test-loadHandler)




