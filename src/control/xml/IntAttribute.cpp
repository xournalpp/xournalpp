#include "IntAttribute.h"

IntAttribute::IntAttribute(const char* name, int value) : XMLAttribute(name)
{
	XOJ_INIT_TYPE(IntAttribute);

	this->value = value;
}

IntAttribute::~IntAttribute()
{
	XOJ_RELEASE_TYPE(IntAttribute);
}

void IntAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(IntAttribute);

	char* str = g_strdup_printf("%i", value);
	out->write(str);
	g_free(str);
}
