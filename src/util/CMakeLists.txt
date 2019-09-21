# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB_RECURSE util_SOURCES *.cpp)

add_library (util STATIC ${util_SOURCES})
find_package(CXX17 REQUIRED)
target_link_libraries(util PRIVATE cxx17)
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
