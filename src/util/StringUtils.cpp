#include "StringUtils.h"
#include <sstream> // std::istringstream

#include <glib.h>

string StringUtils::toLowerCase(string input)
{
	char* lower = g_utf8_strdown(input.c_str(), input.size());
	string strLower = lower;
	g_free(lower);

	return strLower;
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


