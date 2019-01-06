#include "StringUtils.h"
#include <sstream> // std::istringstream

#include <glib.h>
#include <string>

string StringUtils::toLowerCase(string input)
{	
	return string(g_utf8_strdown(input.c_str(), input.size()));
}

void StringUtils::replaceAllChars(string& input, const std::vector<replace_pair> replaces)
{
	string out;
	bool found = false;
	for (char c : input)
	{
		for (replace_pair p : replaces)
		{
			if (c == p.first)
			{
				out += p.second;
				found = true;
				break;
			}
		}
		if (!found)
		{
			out += c;
		}
		found = false;
	}
	input = out;
}

vector<string> StringUtils::split(string input, char delimiter)
{
	vector<string> tokens;
	string token;
	std::istringstream tokenStream(input);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

bool StringUtils::startsWith(string str, string start)
{
	return str.compare(0, start.length(), start) == 0;
}

bool StringUtils::endsWith(string str, string end)
{
	if (end.size() > str.size())
	{
		return false;
	}

	return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

const std::string TRIM_CHARS = "\t\n\v\f\r ";

std::string StringUtils::ltrim(std::string str)
{
    str.erase(0, str.find_first_not_of(TRIM_CHARS));
    return str;
}

std::string StringUtils::rtrim(std::string str)
{
    str.erase(str.find_last_not_of(TRIM_CHARS) + 1);
    return str;
}

std::string StringUtils::trim(std::string str)
{
    return ltrim(rtrim(str));
}

bool StringUtils::iequals(string a, string b)
{
	gchar* ca = g_utf8_casefold (a.c_str(), a.size());
	gchar* cb = g_utf8_casefold (b.c_str(), b.size());
	int result = strcmp(ca, cb);
	g_free(ca);
	g_free(cb);


	return result == 0;
}


