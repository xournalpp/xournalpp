#include "Attribute.h"

#include <utility>

XMLAttribute::XMLAttribute(std::string name): name(std::move(name)) {}

XMLAttribute::~XMLAttribute() = default;

auto XMLAttribute::getName() -> std::string { return name; }
