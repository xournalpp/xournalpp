#include "TextAttribute.h"

#include <utility>  // for move

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream
#include "util/StringUtils.h"       // for replace_pair, StringUtils

TextAttribute::TextAttribute(std::u8string name, std::u8string value):
        XMLAttribute(std::move(name)), value(std::move(value)) {}

TextAttribute::~TextAttribute() = default;

void TextAttribute::writeOut(OutputStream* out) {
    // Todo: perform entity replacement with utf-8 string, avoiding a copy
    auto v = std::string(this->value.begin(), this->value.end());
    StringUtils::replaceAllChars(v, {
                                            replace_pair('&', "&amp;"),
                                            replace_pair('\"', "&quot;"),
                                            replace_pair('<', "&lt;"),
                                            replace_pair('>', "&gt;"),
                                            replace_pair('\n', "&#10;"),
                                            replace_pair('\r', "&#13;"),
                                    });
    out->write(v);
}
