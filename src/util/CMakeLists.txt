# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB_RECURSE util_SOURCES *.cpp)

add_library (util STATIC ${util_SOURCES})
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
