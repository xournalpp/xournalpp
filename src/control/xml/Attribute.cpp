#include "Attribute.h"

#include <utility>

XMLAttribute::XMLAttribute(string name): name(std::move(name)) {}

XMLAttribute::~XMLAttribute() = default;

auto XMLAttribute::getName() -> string { return name; }
