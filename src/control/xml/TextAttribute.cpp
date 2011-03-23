#include "TextAttribute.h"
#include "../../util/String.h"
// TODO: AA: type check

TextAttribute::TextAttribute(const char * name, const char * value) :
	Attribute(name) {
	this->value = g_strdup(value);
}

TextAttribute::~TextAttribute() {
	g_free(this->value);
	this->value = NULL;
}

void TextAttribute::writeOut(OutputStream * out) {
	String v = this->value;
	out->write(v.replace("\"", "&quot;"));
}
