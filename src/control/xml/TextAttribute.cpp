#include "TextAttribute.h"

#include <utility>

#include "StringUtils.h"

TextAttribute::TextAttribute(std::string name, std::string value):
        XMLAttribute(std::move(name)), value(std::move(value)) {}

TextAttribute::~TextAttribute() = default;

void TextAttribute::writeOut(OutputStream* out) {
    std::string v = this->value;
    StringUtils::replaceAllChars(v, {
                                            replace_pair('&', "&amp;"),
                                            replace_pair('\"', "&quot;"),
                                            replace_pair('<', "&lt;"),
                                            replace_pair('>', "&gt;"),
                                            replace_pair('\n', "&#13;"),
                                    });
    out->write(v);
}
