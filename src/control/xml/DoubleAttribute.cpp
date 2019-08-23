#include "DoubleAttribute.h"
#include "Util.h"

DoubleAttribute::DoubleAttribute(const char* name, double value)
 : XMLAttribute(name)
{
	XOJ_INIT_TYPE(DoubleAttribute);

	this->value = value;
}

DoubleAttribute::~DoubleAttribute()
{
	XOJ_RELEASE_TYPE(DoubleAttribute);
}

void DoubleAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(DoubleAttribute);

	char str[G_ASCII_DTOSTR_BUF_SIZE];
	// g_ascii_ version uses C locale always.
	g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, value);
	out->write(str);
}
