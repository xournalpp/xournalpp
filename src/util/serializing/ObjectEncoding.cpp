#include "util/serializing/ObjectEncoding.h"

ObjectEncoding::ObjectEncoding() { this->data = g_string_new(""); }

ObjectEncoding::~ObjectEncoding() = default;

void ObjectEncoding::addStr(const char* str) const { g_string_append(this->data, str); }

auto ObjectEncoding::stealData() -> GString* {
    GString* str = this->data;
    this->data = nullptr;
    return str;
}
