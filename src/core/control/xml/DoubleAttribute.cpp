#include "DoubleAttribute.h"

#include <string>  // for allocator, string

#include <glib.h>  // for g_ascii_formatd, G_ASCII_DTOSTR_B...

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream
#include "util/Util.h"              // for PRECISION_FORMAT_STRING
#include <charconv>

DoubleAttribute::DoubleAttribute(const char* name, double value): XMLAttribute(name) { this->value = value; }

DoubleAttribute::~DoubleAttribute() = default;

void DoubleAttribute::writeOut(OutputStream* out) {
    // g_ascii_ version uses C locale always.

    std::array<char, G_ASCII_DTOSTR_BUF_SIZE> text;
    auto result = std::to_chars(text.data(), text.data() + G_ASCII_DTOSTR_BUF_SIZE, value, std::chars_format::general, 8);
    
    if (result.ec == std::errc{}) {
        *result.ptr = '\0';  // terminatore
    }

    //g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, value);
    out->write(text.data());
}
