#include "NamedColor.h"

#include <sstream>

#include <util/StringUtils.h>

NamedColor::NamedColor():
        paletteIndex{0}, isPaletteColor{false}, name{"Custom Color"}, color{Color(0u)}, colorU16{ColorU16{}} {}

NamedColor::NamedColor(const Color& color): paletteIndex{0}, isPaletteColor{false}, name{"Custom Color"} {
    this->color = color;
    this->colorU16 = Util::GdkRGBA_to_ColorU16(Util::argb_to_GdkRGBA(color));
};

NamedColor& NamedColor::operator=(const NamedColor& other) {
    if (this == &other)
        return *this;
    paletteIndex = other.paletteIndex;
    isPaletteColor = other.isPaletteColor;
    name = other.name;
    color = other.color;
    colorU16 = other.colorU16;
    return *this;
}

NamedColor::NamedColor(NamedColor&& other) noexcept:
        paletteIndex{other.paletteIndex},
        isPaletteColor{other.isPaletteColor},
        name{other.name},
        color{other.color},
        colorU16{other.colorU16} {}

NamedColor& NamedColor::operator=(NamedColor&& other) noexcept {
    if (this == &other)
        return *this;
    paletteIndex = other.paletteIndex;
    isPaletteColor = other.isPaletteColor;
    name = other.name;
    color = other.color;
    colorU16 = other.colorU16;
    return *this;
};


NamedColor::NamedColor(const NamedColor& other):
        paletteIndex{other.paletteIndex}, isPaletteColor{other.isPaletteColor}, name{other.name} {
    this->color = other.color;
    this->colorU16 = other.colorU16;
}

NamedColor::~NamedColor(){};

std::istream& operator>>(std::istream& str, NamedColor& namedColor) {
    std::string line;
    NamedColor tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        if (iss >> tmp.colorU16.red >> tmp.colorU16.blue >> tmp.colorU16.green && std::getline(iss, tmp.name)) {
            tmp.name = StringUtils::trim(tmp.name);
            tmp.color = Util::colorU16_to_rgb(tmp.colorU16);
            tmp.isPaletteColor = true;
            /* OK: All read operations worked */
            namedColor.swap(tmp);  // C++03 as this answer was written a long time ago.
        } else {
            // One operation failed.
            // So set the state on the main stream
            // to indicate failure.
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}

ColorU16 NamedColor::getColorU16() const { return colorU16; }

Color NamedColor::getColor() const { return color; }

auto NamedColor::getIndex() const -> size_t { return paletteIndex; };

auto NamedColor::getName() const -> std::string { return name; };

void NamedColor::swap(NamedColor& other)  // C++03 as this answer was written a long time ago.
{
    std::swap(colorU16, other.colorU16);
    std::swap(color, other.color);
    std::swap(name, other.name);
    std::swap(paletteIndex, other.paletteIndex);
    std::swap(isPaletteColor, other.isPaletteColor);
}
