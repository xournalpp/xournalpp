/*
 * Xournal++
 *
 * String utilities
 *
 * @author MarPiRK
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XournalType.h"

#include <boost/algorithm/string.hpp>
namespace ba = boost::algorithm;
#include <boost/format.hpp>

#include <string>
using std::string;
#include <vector>
#include <utility>

typedef std::pair<char, string> replace_pair;

class StringUtils
{
public:
	static string toLowerCase(string input);
	static void replace_all_chars(string& input, const std::vector<replace_pair> replaces);
};

class StringTokenizer
{
public:
	StringTokenizer(const string s, char token, bool returnToken = false);
	virtual ~StringTokenizer();

	const char* next();

private:
	XOJ_TYPE_ATTRIB;

	char* str;
	int x;
	int len;
	char token;
	char tokenStr[2];
	bool returnToken;
	bool lastWasToken;
};
