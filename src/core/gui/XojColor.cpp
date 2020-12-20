#include "XojColor.h"

#include <utility>

XojColor::XojColor(Color color, string name): color(color), name(std::move(name)) {}

XojColor::~XojColor() = default;

auto XojColor::getColor() const -> Color { return this->color; }

auto XojColor::getName() -> string { return this->name; }
