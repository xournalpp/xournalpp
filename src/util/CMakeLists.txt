cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

# Util is pretty small so GLOB_RECURSE is sufficient
unset(util_SOURCES)
file(GLOB_RECURSE util_SOURCES *.cpp *.h)

add_library(util STATIC ${util_SOURCES})
add_dependencies(util std::filesystem)
target_link_libraries(util PUBLIC xoj::defaults xoj::external_modules std::filesystem cxx17)
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
target_include_directories(util PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
add_library(xoj::util ALIAS util)
