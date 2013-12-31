#include "Attribute.h"

Attribute::Attribute(const char* name)
{
	XOJ_INIT_TYPE(Attribute);

	this->name = g_strdup(name);
}

Attribute::~Attribute()
{
	XOJ_CHECK_TYPE(Attribute);

	g_free(this->name);
	this->name = NULL;

	XOJ_RELEASE_TYPE(Attribute);
}

const char* Attribute::getName()
{
	XOJ_CHECK_TYPE(Attribute);

	return name;
}
