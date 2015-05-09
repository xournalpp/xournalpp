# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB_RECURSE util_SOURCES *.cpp)

add_library (util STATIC ${util_SOURCES})
add_dependencies (util poppler)

# Simple C++11 check
target_compile_features (util PRIVATE cxx_variadic_templates)
