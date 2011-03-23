#include "TextAttribute.h"
#include "../../util/String.h"

TextAttribute::TextAttribute(const char * name, const char * value) :
	Attribute(name) {
	XOJ_INIT_TYPE(TextAttribute);

	this->value = g_strdup(value);
}

TextAttribute::~TextAttribute() {
	XOJ_CHECK_TYPE(TextAttribute);

	g_free(this->value);
	this->value = NULL;

	XOJ_RELEASE_TYPE(TextAttribute);
}

void TextAttribute::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(TextAttribute);

	String v = this->value;
	out->write(v.replace("\"", "&quot;"));
}
