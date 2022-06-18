#include "DoubleAttribute.h"

#include <string>  // for allocator, string

#include <glib.h>  // for g_ascii_formatd, G_ASCII_DTOSTR_B...

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream
#include "util/Util.h"              // for PRECISION_FORMAT_STRING

DoubleAttribute::DoubleAttribute(const char* name, double value): XMLAttribute(name) { this->value = value; }

DoubleAttribute::~DoubleAttribute() = default;

void DoubleAttribute::writeOut(OutputStream* out) {
    char str[G_ASCII_DTOSTR_BUF_SIZE];
    // g_ascii_ version uses C locale always.
    g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, value);
    out->write(str);
}
