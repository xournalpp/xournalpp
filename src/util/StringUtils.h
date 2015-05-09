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
#include <boost/locale/format.hpp>
namespace bl = boost::locale;

#include <string>
using std::string;
#include <vector>
#include <utility>

#define CONCAT StringUtils::concat

typedef std::pair<char, string> replace_pair;

class StringUtils
{
private:
	static bl::format* slformat(bl::format* format)
	{
		return format;
	};
	
	template<typename Formattible, typename... Args>
	static bl::format* slformat(bl::format* format, const Formattible& object, Args... objects)
	{
		return slformat(&(*format % object), objects...);
	}
	
public:
	/**
	 * The same as bl::format(format) % obj1 % obj2 % ...
	 * 
     * @param format Format string
     * @param objects list of objects, which format format format string
     * @return string representation of bl::format
     */
	template<typename... Args>
	static string lformat(string format, Args... objects)
	{
		bl::format f(format);
		return slformat(&f, objects...)->str();
	}
	
private:
	static boost::format* sformat(boost::format* format)
	{
		return format;
	};
	
	template<typename Formattible, typename... Args>
	static boost::format* sformat(boost::format* format, const Formattible& object, Args... objects)
	{
		return sformat(&(*format % object), objects...);
	}
	
public:
	/**
	 * The same as boost::format(format) % obj1 % obj2 % ...
	 * 
     * @param format Format string
     * @param objects list of objects, which format format format string
     * @return string representation of boost::format
     */
	template<typename... Args>
	static string format(string format, Args... objects)
	{
		boost::format f(format);
		return sformat(&f, objects...)->str();
	}
	
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
