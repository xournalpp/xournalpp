#include "util/serializing/BinObjectEncoding.h"

#include <glib.h>  // for g_string_append_len

#include "util/safe_casts.h"  // for as_signed

BinObjectEncoding::BinObjectEncoding() = default;

BinObjectEncoding::~BinObjectEncoding() = default;

void BinObjectEncoding::addData(const void* data, size_t len) {
    g_string_append_len(this->data, static_cast<const char*>(data), as_signed(len));
}
