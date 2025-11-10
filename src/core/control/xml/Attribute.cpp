#include "Attribute.h"

#include <utility>

XMLAttribute::XMLAttribute(std::u8string name): name(std::move(name)) {}

XMLAttribute::~XMLAttribute() = default;

auto XMLAttribute::getName() -> std::u8string { return name; }
