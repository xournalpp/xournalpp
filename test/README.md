# GTEST testing framework

## How to add a new test

Add a new `.cpp` file somewhere within `test/unit_tests`.
This test will be automatically discovered by gtest (see `gtest_discover_tests` in `CMakeLists.txt`).
However, **you still need to call** `touch test/CMakeLists.txt` as otherwise the `GLOB` recursive lookup for test files will not be triggered!!!

A basic example for a test file would be 

```cpp
#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
```
as taken from the official [docs](http://google.github.io/googletest/quickstart-cmake.html).

As all `test/unit_tests` are built with a dependency on `xournalpp-core` you can include any file from `src` as you would in the main code.

## How to migrate existing CPPUnit Test code

It's mostly:

* remove all code, apart from test functions, helper functions and includes (e.g class structure)
* replace `void <name of testcase>() {`  
  with  `TEST(<NAME of your TestSuite>, <name of testcase>) {`
* replace `CPPUNIT_ASSERT_EQUAL`  
  with `EXPECT_EQ`

## How to add a new test program

First why: This would make sense in case you have a very different testing usecase than with unit-tests and you might want to collect code coverage separately. Otherwise, adding to unit-tests is way simpler and preferred as  `gtest_discover_tests` does all the book keeping for you then.

* create a new folder for your test program
* add the definition of your test program to `test/CMakeLists.txt`
* add the test program to all jobs in `azure-pipelines/continuous-integration.yml`

For further pointers see the official [Quickstart Cmake Guide](http://google.github.io/googletest/quickstart-cmake.html).

## Problems running `make test`

If CMake is generating UNIX Makefiles and `make test` fails with  the error `Unable to find executable: test-units_NOT_BUILT`, make sure that:
 1. `make test-units` has been run (to compile the test executable)
 2. The project was configured with `-DENABLE_GTEST=ON` (e.g. by configuring with `cmake .. -DENABLE_GTEST=ON`)

 The fact that 1. is not handled automatically seems to be a known limitation of CMake (see [issue](https://gitlab.kitware.com/cmake/cmake/-/issues/8774) and [stackoverflow](https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests)).
 One possible workaround is to "build & run" the `test-units`executable directly.
 This way the tests are always built before they are executed.

## Problems Building on MacOSX

If you get a linking error of the sort,

```
[317/317] Linking CXX executable test/test-units
FAILED: test/test-units test/test-units[1]_tests.cmake 

...

dyld: Library not loaded: @rpath/libpoppler-glib.8.dylib
  Referenced from: /Users/runner/work/1/s/build/test/test-units
  Reason: image not found
```

then you have some fun `@rpath` problem.
The `test/CMakeLists.txt` already contains a solution for this, so that is a good start for debugging if something related to MacOSX build failures would come up.

Apart from that resources I found useful:
 * [Understanding RPATH (with CMake)](https://dev.my-gate.net/2021/08/04/understanding-rpath-with-cmake/)
 * [RPATH handling](https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling)

## Further Reference

* [GoogleTest User’s Guide](http://google.github.io/googletest/)
* [CPPUnit project page](http://cppunit.sourceforge.net/doc/cvs/group___assertions.html) (for migration)
