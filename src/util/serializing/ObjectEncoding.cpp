#include "util/serializing/ObjectEncoding.h"

#include <string>

ObjectEncoding::ObjectEncoding() = default;

ObjectEncoding::~ObjectEncoding() = default;

void ObjectEncoding::addStr(const std::string_view str) { this->addData(str.data(), str.length()); }

auto ObjectEncoding::stealData() -> std::vector<std::byte> { return std::move(this->data); }
