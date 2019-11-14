#include "XojColor.h"

#include <utility>

XojColor::XojColor(int color, string name)
 : color(color)
 , name(std::move(name))
{
}

XojColor::~XojColor() = default;

auto XojColor::getColor() -> int
{
	return this->color;
}

auto XojColor::getName() -> string
{
	return this->name;
}
