# GTEST testing framework

## How to add a new test

Add a new `.cpp` file under the respective folder within `test/unit_tests` analogous to the hierarchy in `src`.
If the directory already exists, this test will be automatically discovered by gtest (see `gtest_discover_tests` in `CMakeLists.txt`).
However, **you still need to call** `touch test/CMakeLists.txt` as otherwise the `GLOB` recursive lookup for test files will not be triggered!!!

If the directory does not yet exist you need to create a new test program (see below section).

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

This distinction between subdirectories is done to reduce build & test time when working on only one part of the project.
Right now this is only implemented for the `unit-tests-util` target as the dependencies in the `src` file are not yet disentangled sufficiently.

This way of narrowing down the build scope also allows to test for the intended boundaries of parts of the application.
For example, if anyone will implement code in `src/util` which includes parts from outside `src/util` the CI will fail as the test cannot even be built.
This is a feature not a bug of this approach, as it makes the architectural impact of changes testable to some degree.

## How to migrate existing CPPUnit Test code

It's mostly:

* remove all code, apart from test functions, helper functions and includes (e.g class structure)
* replace `void <name of testcase>() {`  
  with  `TEST(<NAME of your TestSuite>, <name of testcase>) {`
* replace `CPPUNIT_ASSERT_EQUAL`  
  with `EXPECT_EQ`

## How to add a new test program

First why: This makes sense either if you are adding unit tests for a whole new part of the application which is not covered yet, or if you're intending to write something that is not really a unit test.
Otherwise, adding to tests to the existent tests is way simpler and preferred as  `gtest_discover_tests` does all the book keeping for you then.

* create a new folder for your test program
* add the definition of your test program to `test/CMakeLists.txt`
* add the test program to all jobs in `azure-pipelines/continuous-integration.yml`

For further pointers see the official [Quickstart Cmake Guide](http://google.github.io/googletest/quickstart-cmake.html).

## Problems when building tests

If you added a file to an existing unit-test directory and the build step fails the following might have happened.

 1. Your test includes a file from a different corresponding folder in `src`
     - This is a good indication that your test is not really a unit test of the specific part of the application. 
       E.g. If you're including `model/Stroke.h` from a unit test in `unit-tests/util` then it is questionable whether this is a real unit test for `util`.
     - In this case you probably want to either move you're whole test file or at least some parts to a different part of the testing directory.
       If there is no fitting place, then you might actually need to create a new test program, but this is really not as much work as it sounds (see above).
 2. Changes you did in the code violated a component boundary of the architecture of the application.
     - E.g. If in a `util/String.h` file you include `model/Stroke.h` it is questionable if the `String.h` code is really a util code or is rather something specific to `model`
     - In this case refactoring your code change is probably the best idea. The simplest solution is to just move the relevant file in a different part of the source code (of course as long as this is also semantically reasonable).

## Problems running `make test`

If CMake is generating UNIX Makefiles and `make test` fails with  the error `Unable to find executable: unit_test_util_NOT_BUILT`, make sure that:
 1. `make all-unit-tests` has been run (to compile the test executable)
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
