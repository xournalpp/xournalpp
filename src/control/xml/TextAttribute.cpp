#include "TextAttribute.h"
#include <StringUtils.h>

#include <utility>

TextAttribute::TextAttribute(string name, string value)
 : XMLAttribute(std::move(name))
 , value(std::move(value))
{
}

TextAttribute::~TextAttribute() = default;

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
