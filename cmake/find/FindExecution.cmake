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

include(CheckCXXSymbolExists)

add_library(execution_lib INTERFACE)

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
endif()

add_library(std::execution ALIAS execution_lib)

set(_found TRUE)

set(STD_EXECUTION_FOUND ${_found} CACHE INTERNAL "")
