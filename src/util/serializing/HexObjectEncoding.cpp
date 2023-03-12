#include "util/serializing/HexObjectEncoding.h"

#include <sstream>
#include <string>
#include <iomanip>

HexObjectEncoding::HexObjectEncoding() = default;

HexObjectEncoding::~HexObjectEncoding() = default;

void HexObjectEncoding::addData(const void* data, int len) {
    std::ostringstream os;
    os << std::hex;

    for (int i = 0; i < len; ++i) {
        os << std::setw(2) << std::setfill('0') << *((const char*)data + i); 
    }

    this->data.append(os.str());
}
