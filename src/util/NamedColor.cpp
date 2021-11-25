#include "util/NamedColor.h"

#include <sstream>

#include "util/StringUtils.h"

NamedColor::NamedColor():
        paletteIndex{0}, name{"Custom Color"}, colorU16{ColorU16{}}, color{Color(0u)}, isPaletteColor{false} {}

NamedColor::NamedColor(const size_t& paletteIndex):
        paletteIndex{paletteIndex},
        name{"Fallback Color"},
        colorU16{ColorU16{}},
        color{Color(0u)},
        isPaletteColor{true} {}

NamedColor::NamedColor(const Color& color):
        paletteIndex{0},
        name{"Custom Color"},
        colorU16(Util::GdkRGBA_to_ColorU16(Util::argb_to_GdkRGBA(color))),
        color(color),
        isPaletteColor{false} {};

auto operator>>(std::istream& str, NamedColor& namedColor) -> std::istream& {
    std::string line;
    NamedColor tmp;
    if (std::getline(str, line)) {
        std::istringstream iss{line};
        /**
         * Some locales have a white space as a thousand separator, leading the following parsing to fail.
         * We avoid that by parsing in the classic locale.
         */
        iss.imbue(std::locale::classic());
        if (iss >> tmp.colorU16.red >> tmp.colorU16.green >> tmp.colorU16.blue && std::getline(iss, tmp.name)) {
            if (tmp.colorU16.red > 255 || tmp.colorU16.green > 255 || tmp.colorU16.blue > 255) {
                throw std::invalid_argument("RGB values bigger than 255 are not supported.");
            }
            tmp.name = StringUtils::trim(tmp.name);
            tmp.color = Util::colorU16_to_argb(tmp.colorU16);
            tmp.isPaletteColor = true;
            tmp.paletteIndex = namedColor.paletteIndex;
            // All read operations worked
            namedColor = std::move(tmp);
        } else {
            /*
             * One operation failed.
             * So set the state on the main stream
             * to indicate failure.
             */
            str.setstate(std::ios::failbit);
        }
    }
    return str;
}

auto NamedColor::getColorU16() const -> ColorU16 { return colorU16; }

auto NamedColor::getColor() const -> Color { return color; }

auto NamedColor::getIndex() const -> size_t { return paletteIndex; };

auto NamedColor::getName() const -> std::string { return name; };
