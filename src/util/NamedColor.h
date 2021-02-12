#pragma once
#include <string>

#include <util/Color.h>

struct NamedColor {
    size_t paletteIndex;
    std::string name;
    ColorU16 color;

    friend std::istream& operator>>(std::istream& str, NamedColor& data);
    void swap(NamedColor& other);
};
