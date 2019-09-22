#include "Attribute.h"

XMLAttribute::XMLAttribute(string name)
 : name(name)
{
}

XMLAttribute::~XMLAttribute()
{
}

string XMLAttribute::getName()
{
	return name;
}
