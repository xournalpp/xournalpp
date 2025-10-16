#include "DoubleArrayAttribute.h"

#include <algorithm>  // for for_each
#include <iterator>   // for begin, end
#include <string>     // for allocator, string
#include <utility>    // for move

#include <glib.h>  // for g_ascii_formatd, G_ASCII_DTOSTR_B...

#include "control/xml/Attribute.h"  // for XMLAttribute
#include "util/OutputStream.h"      // for OutputStream
#include "util/Util.h"              // for PRECISION_FORMAT_STRING
#include <charconv>

DoubleArrayAttribute::DoubleArrayAttribute(const char* name, std::vector<double>&& values):
        XMLAttribute(name), values(std::move(values)) {}

DoubleArrayAttribute::~DoubleArrayAttribute() = default;

void DoubleArrayAttribute::writeOut(OutputStream* out) {
    if (!this->values.empty()) {

        std::array<char, G_ASCII_DTOSTR_BUF_SIZE> text;
        auto result = std::to_chars(text.data(), text.data() + G_ASCII_DTOSTR_BUF_SIZE, this->values[0], std::chars_format::general, 8);

        
        if (result.ec == std::errc{}) {
            *result.ptr = '\0';  // terminatore
        }

        // g_ascii_ version uses C locale always.

        //g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, this->values[0]);

        out->write(text.data());

        std::for_each(std::begin(this->values) + 1, std::end(this->values), [&](auto& x) {

            std::array<char, G_ASCII_DTOSTR_BUF_SIZE> text;
            auto result = std::to_chars(text.data(), text.data() + G_ASCII_DTOSTR_BUF_SIZE, x, std::chars_format::general, 8);

            
            if (result.ec == std::errc{}) {
                *result.ptr = '\0';  // terminatore
            }

            //g_ascii_formatd(str, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, x);
            out->write(" ");
            out->write(text.data());
        });
    }
}
