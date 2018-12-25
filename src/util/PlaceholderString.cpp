#include "PlaceholderString.h"

#include <glib.h>

/**
 * Base class for Formatting
 */
class PlaceholderElement {
public:
	virtual ~PlaceholderElement()
	{
	}

public:
	virtual string format(string format) = 0;
};

/**
 * Format String
 */
class PlaceholderElementString : public PlaceholderElement{
public:
	PlaceholderElementString(string text)
	 : text(text)
	{
	}

public:
	string format(string format)
	{
		return text;
	}

private:
	string text;
};


/**
 * Format int
 */
class PlaceholderElementInt : public PlaceholderElement{
public:
	PlaceholderElementInt(int64_t value)
	 : value(value)
	{
	}

public:
	string format(string format)
	{
		return std::to_string(value);
	}

private:
	int64_t value;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

PlaceholderString::PlaceholderString(string text)
 : text(text)
{
}

PlaceholderString::~PlaceholderString()
{
	for (PlaceholderElement* e : data)
	{
		delete e;
	}

	data.clear();
}

PlaceholderString& PlaceholderString::operator%(int64_t value)
{
	data.push_back(new PlaceholderElementInt(value));
	return *this;
}

PlaceholderString& PlaceholderString::operator%(string value)
{
	data.push_back(new PlaceholderElementString(value));
	return *this;
}

string PlaceholderString::formatPart(string format)
{
	string formatDef;

	std::size_t comma = format.find(',');
	if (comma != string::npos)
	{
		formatDef = format.substr(comma + 1);
		format = format.substr(0, comma);
	}

	int index;
	try
	{
		index = std::stoi(format);
	}
	catch (const std::exception& e)
	{
		g_error("Could not parse «%s» as int, error: %s", format.c_str(), e.what());
		return "{?}";
	}

	// Placeholder index starting at 1, vector at 0
	index--;

	if (index < 0 || index >= (int)data.size())
	{
		string notFound = "{";
		notFound += std::to_string(index + 1);
		notFound += "}";
		return notFound;
	}

	PlaceholderElement* pe = data[index];

	return pe->format(formatDef);
}

void PlaceholderString::process()
{
	if (processed != "")
	{
		// Already processed
		return;
	}

	bool openBracket = false;
	string formatString;

	// Should work, also for UTF-8
	for (int i = 0; i < (int)text.length(); i++)
	{
		char c = text.at(i);

		if (c == '{' && openBracket && formatString.length() == 0)
		{
			openBracket = false;
			processed += '{';
			continue;
		}

		if (c == '}')
		{
			processed += formatPart(formatString);

			openBracket = false;
			formatString = "";
			continue;
		}

		if (openBracket)
		{
			formatString += c;
			continue;
		}

		if (c == '{')
		{
			openBracket = true;
			continue;
		}

		processed += c;
	}
}

string PlaceholderString::str()
{
	process();

	return processed;
}

const char* PlaceholderString::c_str()
{
	process();

	return processed.c_str();
}

std::ostream &operator<<(std::ostream &os, PlaceholderString &ps) {
    return os << ps.str();
}
