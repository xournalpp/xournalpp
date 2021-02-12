#include "NamedColor.h"

#include <sstream>

#include "util/StringUtils.h"

NamedColor::NamedColor():
        paletteIndex{0}, isPaletteColor{false}, name{"Custom Color"}, color{Color(0u)}, colorU16{ColorU16{}} {}

NamedColor::NamedColor(const Color& color): paletteIndex{0}, isPaletteColor{false}, name{"Custom Color"} {
    this->color = color;
    this->colorU16 = Util::GdkRGBA_to_ColorU16(Util::argb_to_GdkRGBA(color));
};

std::istream& operator>>(std::istream& str, NamedColor& namedColor) {
    std::string line;
    NamedColor tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        if (iss >> tmp.colorU16.red >> tmp.colorU16.blue >> tmp.colorU16.green && std::getline(iss, tmp.name)) {
            tmp.name = StringUtils::trim(tmp.name);
            tmp.color = Util::colorU16_to_argb(tmp.colorU16);
            tmp.isPaletteColor = true;
            /* OK: All read operations worked */
            namedColor = std::move(tmp);
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
