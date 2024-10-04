#pragma once

// define XOJ_USE_STD_FORMAT if std::format should be forced

#if !defined(XOJ_USE_STD_FORMAT) && __has_include(<format>)
#include <format>  // IWYU pragma: export
#if defined(__cpp_lib_format) && __cpp_lib_format >= 202110L
#define XOJ_USE_STD_FORMAT = 1
#endif
#endif

#if !defined(XOJ_USE_STD_FORMAT)
#include <fmt/format.h>  // IWYU pragma: export
#else                    // Include all the symbols from std::format into the fmt namespace
namespace fmt {
using std::basic_format_arg;
using std::basic_format_args;
using std::basic_format_context;
using std::basic_format_parse_context;
using std::basic_format_string;
using std::format;
using std::format_args;
using std::format_context;
using std::format_error;
using std::format_parse_context;
using std::format_string;
using std::format_to;
using std::format_to_n;
using std::formatted_size;
using std::formatter;
using std::make_format_args;
using std::make_wformat_args;
using std::vformat;
using std::vformat_to;
using std::visit_format_arg;
using std::wformat_args;
using std::wformat_context;
using std::wformat_parse_context;
using std::wformat_string;
}  // namespace fmt
#endif
