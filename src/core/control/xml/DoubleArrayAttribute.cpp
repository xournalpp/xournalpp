#include "DoubleArrayAttribute.h"

#include <algorithm>  // for for_each
#include <iterator>   // for begin, end
#include <string>     // for allocator, string
#include <utility>    // for move

#include <glib.h>  // for g_ascii_formatd, G_ASCII_DTOSTR_B...

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream
#include "util/Util.h"              // for PRECISION_FORMAT_STRING

DoubleArrayAttribute::DoubleArrayAttribute(const char* name, std::vector<double>&& values):
        XMLAttribute(name), values(std::move(values)) {}

DoubleArrayAttribute::~DoubleArrayAttribute() = default;

void DoubleArrayAttribute::writeOut(OutputStream* out) {
    if (!this->values.empty()) {
        char str[G_ASCII_DTOSTR_BUF_SIZE];
        // g_ascii_ version uses C locale always.
        g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, this->values[0]);
        out->write(str);

        std::for_each(std::begin(this->values) + 1, std::end(this->values), [&](auto& x) {
            g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, x);
            out->write(" ");
            out->write(str);
        });
    }
}
