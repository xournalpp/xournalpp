#include "DoubleAttribute.h"

DoubleAttribute::DoubleAttribute(const char * name, double value) :
	Attribute(name) {
	XOJ_INIT_TYPE(DoubleAttribute);

	this->value = value;
}

DoubleAttribute::~DoubleAttribute() {
	XOJ_RELEASE_TYPE(DoubleAttribute);
}

void DoubleAttribute::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(DoubleAttribute);

	char * str = g_strdup_printf("%0.2lf", value);
	out->write(str);
	g_free(str);
}
