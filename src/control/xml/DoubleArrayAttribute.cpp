#include "DoubleArrayAttribute.h"

DoubleArrayAttribute::DoubleArrayAttribute(const char * name, double * values, int count) :
	Attribute(name) {
	this->values = values;
	this->count = count;
}

DoubleArrayAttribute::~DoubleArrayAttribute() {
	delete values;
}

void DoubleArrayAttribute::writeOut(OutputStream * out) {
	if (this->count > 0) {
		char * str = g_strdup_printf("%0.2lf", this->values[0]);
		out->write(str);
		g_free(str);
	}

	for (int i = 1; i < this->count; i++) {
		char * str = g_strdup_printf(" %0.2lf", this->values[i]);
		out->write(str);
		g_free(str);
	}
}
