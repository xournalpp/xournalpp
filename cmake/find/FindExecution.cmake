###
# Xournal++
#
# Configure for std::execution implementations
#
# @author Xournal++ Team
# https://github.com/xournalpp/xournalpp
#
# @license GNU GPLv2 or later
###

if(TARGET std::execution)
    return()
endif()

include(CheckCXXSymbolExists)

add_library(execution_lib INTERFACE IMPORTED)

# TODO(cpp20) replace `ciso646` by `version`
check_cxx_symbol_exists(__GLIBCXX__ ciso646 GLIBCXX)
if(GLIBCXX) # Using libstdc++
    find_package(TBB REQUIRED)  # for libstdc++ <execution>
    target_link_libraries(execution_lib INTERFACE TBB::tbb)
endif()
check_cxx_symbol_exists(_LIBCPP_VERSION ciso646 LIBCPP)
if(LIBCPP) # Using libc++
    # c++17 execution policies are still in the experimental lib (at least in clang 20)
    target_compile_options(execution_lib INTERFACE "-fexperimental-library")
    target_link_options(execution_lib INTERFACE "-fexperimental-library")
endif()

block(PROPAGATE _STD_EXECUTION_COMPILES)
set(code [[
    #include <algorithm>
    #include <execution>
    #include <numeric>
    #include <array>

    int main() {
        std::array<int, 20> ar;
        std::generate(std::execution::par, ar.begin(), ar.end(), [n = 0] () mutable { return n++; });
        return std::reduce(std::execution::par, ar.begin(), ar.end());
    }
]])
set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};execution_lib")
check_cxx_source_compiles("${code}" _STD_EXECUTION_COMPILES)
endblock()

if (_STD_EXECUTION_COMPILES)
    message(STATUS "Found std::execution")
    add_library(std::execution ALIAS execution_lib)
    set(_found TRUE)
    set(STD_EXECUTION_FOUND ${_found} CACHE INTERNAL "")
else()
    message(FATAL_ERROR "Cannot compile simple program using std::execution")
endif()

