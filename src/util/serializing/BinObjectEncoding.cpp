#include "BinObjectEncoding.h"

BinObjectEncoding::BinObjectEncoding() = default;

BinObjectEncoding::~BinObjectEncoding() = default;

void BinObjectEncoding::addData(const void* data, int len) {
    g_string_append_len(this->data, static_cast<const char*>(data), len);
}
