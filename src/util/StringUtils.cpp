#include "util/StringUtils.h"

#include <cstring>
#include <sstream>  // std::istringstream
#include <string>
#include <string_view>
#include <utility>

#include <glib.h>

#include "util/safe_casts.h"  // for as_signed

using std::string;
using std::vector;

auto StringUtils::toLowerCase(const string& input) -> string {
    char* lower = g_utf8_strdown(input.c_str(), as_signed(input.size()));
    string lowerStr = lower;
    g_free(lower);
    return lowerStr;
}

void StringUtils::replaceAllChars(string& input, const std::vector<replace_pair>& replaces) {
    string out;
    bool found = false;
    for (char c: input) {
        for (const replace_pair& p: replaces) {
            if (c == p.first) {
                out += p.second;
                found = true;
                break;
            }
        }
        if (!found) {
            out += c;
        }
        found = false;
    }
    input = out;
}

auto StringUtils::split(const string& input, char delimiter) -> vector<string> {
    vector<string> tokens;
    string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

auto StringUtils::startsWith(std::string_view str, std::string_view start) -> bool {
    return str.compare(0, start.length(), start) == 0;
}

auto StringUtils::endsWith(std::string_view str, std::string_view end) -> bool {
    if (end.size() > str.size()) {
        return false;
    }

    return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

const std::string TRIM_CHARS = "\t\n\v\f\r ";

auto StringUtils::ltrim(std::string str) -> std::string {
    str.erase(0, str.find_first_not_of(TRIM_CHARS));
    return str;
}

auto StringUtils::rtrim(std::string str) -> std::string {
    str.erase(str.find_last_not_of(TRIM_CHARS) + 1);
    return str;
}

auto StringUtils::trim(std::string str) -> std::string { return ltrim(rtrim(std::move(str))); }

auto StringUtils::iequals(const string& a, const string& b) -> bool {
    gchar* ca = g_utf8_casefold(a.c_str(), as_signed(a.size()));
    gchar* cb = g_utf8_casefold(b.c_str(), as_signed(b.size()));
    int result = strcmp(ca, cb);
    g_free(ca);
    g_free(cb);


    return result == 0;
}

auto StringUtils::ellipsize(std::string_view sv, std::size_t max_width) -> std::string {
    constexpr std::string_view ELLIPSIS_STR = "...";
    xoj_assert(max_width > ELLIPSIS_STR.size());
    const auto length = g_utf8_strlen(sv.data(), as_signed(sv.size()));

    if (length <= as_signed(max_width)) {
        return std::string{sv};
    }

    const auto bytes_kept = static_cast<std::size_t>(
            g_utf8_offset_to_pointer(sv.data(), as_signed(max_width - ELLIPSIS_STR.size())) - sv.data());
    std::string str;
    str.reserve(bytes_kept + ELLIPSIS_STR.size());
    str.append(sv.data(), bytes_kept);
    str.append(ELLIPSIS_STR);
    return str;
}

auto StringUtils::markup_escape(std::string_view sv) -> std::string {
    auto escaped = g_markup_escape_text(sv.data(), as_signed(sv.size()));
    std::string str{escaped};
    g_free(escaped);
    return str;
}
