# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB_RECURSE util_SOURCES *.cpp)

add_library (util STATIC ${util_SOURCES})

if (BUILD_BOOST)
  add_dependencies (util boost)
endif ()

if (BUILD_POPPLER)
  add_dependencies (util poppler)
endif ()
