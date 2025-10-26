/*
 * Xournal++
 *
 * String utilities
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>   // for string, basic_string
#include <utility>  // for pair
#include <vector>   // for vector

typedef std::pair<char, std::string> replace_pair;

namespace StringUtils {

std::string toLowerCase(const std::string& input);
void replaceAllChars(std::string& input, const std::vector<replace_pair>& replaces);
std::vector<std::string> split(const std::string& input, char delimiter);
bool startsWith(std::string_view str, std::string_view start);
bool endsWith(std::string_view str, std::string_view end);
std::string ltrim(std::string str);
std::string rtrim(std::string str);
std::string trim(std::string str);
bool iequals(const std::string& a, const std::string& b);
bool isNumber(const std::string& input);

/**
 * Wrapper around an std::string_view that only accepts compile-time instances
 * It can only be constructed from a litteral or other compile-time ressources
 */
class StaticStringView {
public:
    consteval StaticStringView(std::string_view v): view(v) {}
    constexpr operator std::string_view() const noexcept { return view; }

private:
    std::string_view view;
};

};  // namespace StringUtils

inline auto char_cast(std::u8string_view str) -> std::string_view {
    return {reinterpret_cast<const char*>(str.data()), str.size()};
}

inline auto char_cast(char8_t const* str) -> char const* { return reinterpret_cast<const char*>(str); }

// Helpers for C-style formatting of string views
// Usage: printf("Message " SV_FMT, SV_ARG(string_view))
#define SV_FMT "%.*s"
#define SV_ARG(sv) static_cast<int>((sv).size()), (sv).data()
