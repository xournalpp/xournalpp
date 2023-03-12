#include "util/serializing/ObjectEncoding.h"

#include <string>

ObjectEncoding::ObjectEncoding() = default;

ObjectEncoding::~ObjectEncoding() = default;

void ObjectEncoding::addStr(const char* str) { this->data.append(std::string(str)); }

auto ObjectEncoding::getData() -> std::string {
    std::string res = this->data;
    this->data.resize(0);
    return res;
}
