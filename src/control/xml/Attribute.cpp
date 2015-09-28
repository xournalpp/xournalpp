#include "Attribute.h"

XMLAttribute::XMLAttribute(const char* name)
{
	XOJ_INIT_TYPE(XMLAttribute);

	this->name = g_strdup(name);
}

XMLAttribute::~XMLAttribute()
{
	XOJ_CHECK_TYPE(XMLAttribute);

	g_free(this->name);
	this->name = NULL;

	XOJ_RELEASE_TYPE(XMLAttribute);
}

const char* XMLAttribute::getName()
{
	XOJ_CHECK_TYPE(XMLAttribute);

	return name;
}
