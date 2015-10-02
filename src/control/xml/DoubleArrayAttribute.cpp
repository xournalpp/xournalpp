#include "DoubleArrayAttribute.h"

DoubleArrayAttribute::DoubleArrayAttribute(const char* name, double* values, int count) : XMLAttribute(name)
{
	XOJ_INIT_TYPE(DoubleArrayAttribute);

	this->values = values;
	this->count = count;
}

DoubleArrayAttribute::~DoubleArrayAttribute()
{
	XOJ_CHECK_TYPE(DoubleArrayAttribute);

	delete values;

	XOJ_RELEASE_TYPE(DoubleArrayAttribute);
}

void DoubleArrayAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(DoubleArrayAttribute);

	if (this->count > 0)
	{
		char* str = g_strdup_printf("%0.2lf", this->values[0]);
		out->write(str);
		g_free(str);
	}

	for (int i = 1; i < this->count; i++)
	{
		char* str = g_strdup_printf(" %0.2lf", this->values[i]);
		out->write(str);
		g_free(str);
	}
}
