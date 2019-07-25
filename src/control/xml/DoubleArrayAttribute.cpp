#include "DoubleArrayAttribute.h"
#include "Util.h"

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
	values = NULL;

	XOJ_RELEASE_TYPE(DoubleArrayAttribute);
}

void DoubleArrayAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(DoubleArrayAttribute);

	if (this->count > 0)
	{
		char str[G_ASCII_DTOSTR_BUF_SIZE];
		// g_ascii_ version uses C locale always.
		g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, this->values[0]);
		out->write(str);

		for (int i = 1; i < this->count; i++)
		{
			// g_ascii_ version uses C locale always.
			g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, this->values[i]);
			out->write(" ");
			out->write(str);
		}
	}
}
