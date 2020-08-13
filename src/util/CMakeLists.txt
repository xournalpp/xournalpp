# Util is pretty small so GLOB_RECURSE is sufficient
unset (util_SOURCES)
file (GLOB_RECURSE util_SOURCES *.cpp *.h)

add_library (util STATIC ${util_SOURCES})
add_dependencies(util std::filesystem)
target_link_libraries(util PUBLIC std::filesystem cxx17)
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
target_include_directories(util PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        #$<INSTALL_INTERFACE:>  # <prefix>/include/mylib
        )
