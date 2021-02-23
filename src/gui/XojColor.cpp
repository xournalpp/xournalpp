#include "XojColor.h"

#include <utility>

XojColor::XojColor(Color color, std::string name): color(color), name(std::move(name)) {}

auto XojColor::getColor() const -> Color { return this->color; }

auto XojColor::getName() const -> string { return this->name; }
