#include "i18n.h"

PlaceholderString::PlaceholderString(const char* text)
 : text(text)
{
}

PlaceholderString::~PlaceholderString()
{

}

PlaceholderString& PlaceholderString::operator%(uint64_t value)
{
	return *this;
}

PlaceholderString& PlaceholderString::operator%(int value)
{
	return *this;
}

PlaceholderString& PlaceholderString::operator%(string value)
{
	return *this;
}

string PlaceholderString::str()
{

	return "";
}

const char* PlaceholderString::c_str()
{
	return "";
}

std::ostream &operator<<(std::ostream &os, PlaceholderString &ps) {
    return os << ps.str();
}
