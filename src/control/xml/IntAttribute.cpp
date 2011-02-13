#include "IntAttribute.h"

IntAttribute::IntAttribute(const char * name, int value) :
	Attribute(name) {
	this->value = value;
}
IntAttribute::~IntAttribute() {
}

void IntAttribute::writeOut(OutputStream * out) {
	char * str = g_strdup_printf("%i", value);
	out->write(str);
	g_free(str);
}
