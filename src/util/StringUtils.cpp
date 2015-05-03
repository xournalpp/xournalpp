#include "StringUtils.h"

void StringUtils::replace_all_chars(string& input, const std::vector<replace_pair> replaces)
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

/**
 * String tokenizer
 */
StringTokenizer::StringTokenizer(const string s, char token, bool returnToken)
{
	XOJ_INIT_TYPE(StringTokenizer);

	this->str = const_cast<char*> (s.c_str());
	this->token = token;
	this->tokenStr[0] = token;
	this->tokenStr[1] = 0;
	this->returnToken = returnToken;
	this->lastWasToken = false;
	this->x = 0;
	this->len = s.length();
}

StringTokenizer::~StringTokenizer()
{
	XOJ_CHECK_TYPE(StringTokenizer);

	//g_free(this->str);
	this->str = NULL;

	XOJ_RELEASE_TYPE(StringTokenizer);
}

const char* StringTokenizer::next()
{
	XOJ_CHECK_TYPE(StringTokenizer);

	if (this->x == -1)
	{
		return NULL;
	}

	if (this->lastWasToken)
	{
		this->lastWasToken = false;
		return this->tokenStr;
	}

	const char* tmp = this->str + x;

	for (; x < this->len; x++)
	{
		if (this->str[x] == this->token)
		{
			this->str[x] = 0;
			if (this->returnToken)
			{
				this->lastWasToken = true;
			}
			x++;
			return tmp;
		}
	}
	this->x = -1;

	return tmp;
}
