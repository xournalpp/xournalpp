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

class StringUtils
{
public:
	static string toLowerCase(string input);
	static void replaceAllChars(string& input, const std::vector<replace_pair> replaces);
	static vector<string> split(string input, char delimiter);
	static bool startsWith(string str, string start);
};
