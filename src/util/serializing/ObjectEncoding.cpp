#include "util/serializing/ObjectEncoding.h"

#include <string>

ObjectEncoding::ObjectEncoding() = default;

ObjectEncoding::~ObjectEncoding() = default;

void ObjectEncoding::addStr(const std::string_view &str) { this->addData(str.data(), str.length()); }

auto ObjectEncoding::getData() -> std::vector<std::byte> const& {
    return std::move(this->data);
}
