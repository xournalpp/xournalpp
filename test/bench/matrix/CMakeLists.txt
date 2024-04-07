cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Release CACHE BOOL "" FORCE)

# create project
project(MyProject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add executable
add_executable(bench_matrix main.cpp MatrixV1.cpp)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.5.2
  OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)

if(benchmark_ADDED)
  # enable c++11 to avoid compilation errors
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
endif()

# link dependencies
target_link_libraries(bench_matrix benchmark::benchmark)