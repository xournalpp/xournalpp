#include "SizeTAttribute.h"

SizeTAttribute::SizeTAttribute(const char* name, size_t value) : XMLAttribute(name)
{
	this->value = value;
}

SizeTAttribute::~SizeTAttribute()
{
}

void SizeTAttribute::writeOut(OutputStream* out)
{
	char* str = g_strdup_printf("%ull", value);
	out->write(str);
	g_free(str);
}
