#include "Attribute.h"

XMLAttribute::XMLAttribute(string name)
 : name(name)
{
	XOJ_INIT_TYPE(XMLAttribute);
}

XMLAttribute::~XMLAttribute()
{
	XOJ_RELEASE_TYPE(XMLAttribute);
}

string XMLAttribute::getName()
{
	XOJ_CHECK_TYPE(XMLAttribute);

	return name;
}
