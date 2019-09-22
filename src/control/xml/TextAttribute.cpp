#include "TextAttribute.h"
#include <StringUtils.h>

TextAttribute::TextAttribute(string name, string value)
 : XMLAttribute(name),
   value(value)
{
}

TextAttribute::~TextAttribute()
{
}

void TextAttribute::writeOut(OutputStream* out)
{
	string v = this->value;
	StringUtils::replaceAllChars(v, {
		replace_pair('&', "&amp;"),
		replace_pair('\"', "&quot;"),
		replace_pair('<', "&lt;"),
		replace_pair('>', "&gt;"),
	});
	out->write(v);
}
