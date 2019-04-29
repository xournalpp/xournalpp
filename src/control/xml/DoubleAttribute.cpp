#include "DoubleAttribute.h"

DoubleAttribute::DoubleAttribute(const char* name, double value) : XMLAttribute(name)
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
	g_ascii_dtostr( str, G_ASCII_DTOSTR_BUF_SIZE, value);	//  g_ascii_ version uses C locale always.
	out->write(str);

}
