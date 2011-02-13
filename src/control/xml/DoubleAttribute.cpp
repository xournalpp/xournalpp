#include "DoubleAttribute.h"

DoubleAttribute::DoubleAttribute(const char * name, double value) :
	Attribute(name) {
	this->value = value;
}

DoubleAttribute::~DoubleAttribute() {
}

void DoubleAttribute::writeOut(OutputStream * out) {
	char * str = g_strdup_printf("%0.2lf", value);
	out->write(str);
	g_free(str);
}
