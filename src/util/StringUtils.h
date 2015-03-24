/*
 * Xournal++
 *
 * Reference String which is automatically deleted
 *
 * @author MarPiRK
 * http://marpirk.net
 *
 * @license GNU GPLv3
 */

#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

#include <glib.h>
#include <string>
#include <vector>
#include <utility>
#include <boost/locale/format.hpp>
#include <boost/algorithm/string.hpp>

#include <XournalType.h>

namespace bl = boost::locale;
namespace ba = boost::algorithm;

using std::string;

#define CONCAT StringUtils::concat

typedef std::pair<char, string> replace_pair;

class StringUtils
{
public:
	static string format(const char* format, ...);

private:

	static void addToString(string& str) { };

	template<typename T, typename... Args>
	static void addToString(string& str, const T& a_value, Args... a_args)
	{
		str += a_value;
		addToString(str, a_args...);
	}
public:

	template<typename... Args>
	static string concat(Args... a_args)
	{
		string s("");
		addToString(s, a_args...);
		return s;
	}

public:
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

#endif /* __STRINGUTILS_H__ */
