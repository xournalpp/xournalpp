#include "TextAttribute.h"
#include <StringUtils.h>

TextAttribute::TextAttribute(const char* name, const char* value) :
Attribute(name)
{
	XOJ_INIT_TYPE(TextAttribute);

	this->value = g_strdup(value);
}

TextAttribute::~TextAttribute()
{
	XOJ_CHECK_TYPE(TextAttribute);

	g_free(this->value);
	this->value = NULL;

	XOJ_RELEASE_TYPE(TextAttribute);
}

void TextAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(TextAttribute);

	string v(this->value);
	StringUtils::replace_all_chars(v,{
		replace_pair('&', "&amp;"),
		replace_pair('\"', "&quot;"),
		replace_pair('<', "&lt;"),
		replace_pair('>', "&gt;"),
	});
	out->write(v);
}
