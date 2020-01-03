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
using std::string;
#include <vector>
using std::vector;

typedef std::pair<char, string> replace_pair;

class StringUtils {
public:
    static string toLowerCase(const string& input);
    static void replaceAllChars(string& input, const std::vector<replace_pair>& replaces);
    static vector<string> split(const string& input, char delimiter);
    static bool startsWith(const string& str, const string& start);
    static bool endsWith(const string& str, const string& end);
    static string ltrim(string str);
    static string rtrim(string str);
    static string trim(string str);
    static bool iequals(const string& a, const string& b);
};
