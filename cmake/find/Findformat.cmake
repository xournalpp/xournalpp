cmake_minimum_required(VERSION 3.14)

enable_language(CXX)

option(XOJ_USE_FMTLIB "Use fmtlib" OFF)
include(xoj_check_cxx_definition)
xoj_check_cxx_definition("__cpp_lib_format >= 202110L" "format" _XOJ_HAS_INCLUDE_FORMAT)

if(_XOJ_HAS_INCLUDE_FORMAT AND NOT XOJ_USE_FMTLIB)
    message(STATUS "Found <format>")
    set(FORMAT_LIB "std::format")
else()
    message(STATUS "Could not find <format>, fetch {fmt}")
    include(get_cpm)
    CPMAddPackage("gh:fmtlib/fmt#7.1.3")
    set(FORMAT_LIB "{fmt}")
endif()
set(format_FOUND TRUE)
