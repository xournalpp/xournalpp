#include "util/serializing/BinObjectEncoding.h"

#include <algorithm>
#include <vector>

BinObjectEncoding::BinObjectEncoding() = default;

BinObjectEncoding::~BinObjectEncoding() = default;

void BinObjectEncoding::addData(const void* data, size_t len) {
    const std::byte* start = reinterpret_cast<const std::byte*>(data);
    const std::byte* end = start + len;
    this->data.reserve(this->data.size() + len);
    std::transform(start, end, std::back_inserter(this->data), [](const std::byte b) { return b; });
}
