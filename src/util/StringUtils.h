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

#include <string>
#include <vector>

typedef std::pair<char, std::string> replace_pair;

class StringUtils {
public:
    static std::string toLowerCase(const std::string& input);
    static void replaceAllChars(std::string& input, const std::vector<replace_pair>& replaces);
    static std::vector<std::string> split(const std::string& input, char delimiter);
    static bool startsWith(const std::string& str, const std::string& start);
    static bool endsWith(const std::string& str, const std::string& end);
    static std::string ltrim(std::string str);
    static std::string rtrim(std::string str);
    static std::string trim(std::string str);
    static bool iequals(const std::string& a, const std::string& b);
};
