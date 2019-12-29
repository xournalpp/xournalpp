#include "DoubleArrayAttribute.h"

#include <algorithm>

#include "Util.h"

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
