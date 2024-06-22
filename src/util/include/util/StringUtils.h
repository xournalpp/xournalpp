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

class StringUtils {
public:
    static std::string toLowerCase(const std::string& input);
    static void replaceAllChars(std::string& input, const std::vector<replace_pair>& replaces);
    static std::vector<std::string> split(const std::string& input, char delimiter);
    static bool startsWith(std::string_view str, std::string_view start);
    static bool endsWith(std::string_view str, std::string_view end);
    static std::string ltrim(std::string str);
    static std::string rtrim(std::string str);
    static std::string trim(std::string str);
    static bool iequals(const std::string& a, const std::string& b);
    static bool isNumber(const std::string& input);
};
