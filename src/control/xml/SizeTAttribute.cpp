#include "SizeTAttribute.h"

SizeTAttribute::SizeTAttribute(const char* name, size_t value) : XMLAttribute(name)
{
	XOJ_INIT_TYPE(SizeTAttribute);

	this->value = value;
}

SizeTAttribute::~SizeTAttribute()
{
	XOJ_RELEASE_TYPE(SizeTAttribute);
}

void SizeTAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(SizeTAttribute);

	char* str = g_strdup_printf("%ull", value);
	out->write(str);
	g_free(str);
}
