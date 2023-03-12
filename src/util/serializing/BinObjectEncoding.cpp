#include "util/serializing/BinObjectEncoding.h"

#include <string>

BinObjectEncoding::BinObjectEncoding() = default;

BinObjectEncoding::~BinObjectEncoding() = default;

void BinObjectEncoding::addData(const void* data, int len) {
    this->data.append(std::string((const char *)data, (const char *)data + len));
}
