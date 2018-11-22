#include "TextAttribute.h"
#include <StringUtils.h>

TextAttribute::TextAttribute(string name, string value)
 : XMLAttribute(name),
   value(value)
{
	XOJ_INIT_TYPE(TextAttribute);
}

TextAttribute::~TextAttribute()
{
	XOJ_RELEASE_TYPE(TextAttribute);
}

void TextAttribute::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(TextAttribute);

	string v = this->value;
	StringUtils::replace_all_chars(v, {
		replace_pair('&', "&amp;"),
		replace_pair('\"', "&quot;"),
		replace_pair('<', "&lt;"),
		replace_pair('>', "&gt;"),
	});
	out->write(v);
}
