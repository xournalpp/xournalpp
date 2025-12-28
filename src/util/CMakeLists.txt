cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

# Util is pretty small so GLOB_RECURSE is sufficient
unset(util_SOURCES)
file(GLOB_RECURSE util_SOURCES *.cpp *.h)

add_library(util STATIC ${util_SOURCES})
target_link_libraries(util PUBLIC xoj::defaults xoj::external_modules)
target_compile_features(util PUBLIC ${PROJECT_CXX_FEATURES})
target_compile_features(util PUBLIC cxx_std_20)
target_include_directories(util PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
add_library(xoj::util ALIAS util)

# Check if std::filesystem::absolute needs to be wrapped.
# With libstdc++, there's a bug where it corrupts UNC paths.
if (WIN32)
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(__GLIBCXX__ version GLIBCXX)
    if(GLIBCXX)
        target_link_options(util PUBLIC
            # std::filesystem::absolute(std::filesystem::__cxx11::path const&)
            "-Wl,--wrap=_ZNSt10filesystem8absoluteERKNS_7__cxx114pathE"
            # std::filesystem::weakly_canonical(std::filesystem::__cxx11::path const&)
            "-Wl,--wrap=_ZNSt10filesystem16weakly_canonicalERKNS_7__cxx114pathE"
        )
        target_compile_definitions(util PUBLIC XOURNALPP_WRAP_STD_FS_ABSOLUTE)
    endif()
endif()
