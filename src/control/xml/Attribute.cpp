#include "Attribute.h"

Attribute::Attribute(const char * name) {
	this->name = g_strdup(name);
}

Attribute::~Attribute() {
	g_free(this->name);
	this->name = NULL;
}

const char * Attribute::getName() {
	CHECK_MEMORY(this);
	return name;
}
