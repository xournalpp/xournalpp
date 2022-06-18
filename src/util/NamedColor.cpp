#include "util/NamedColor.h"

#include <cstdint>    // for uint16_t, uint8_t
#include <sstream>    // for istringstream, basic_istream, basic_i...
#include <stdexcept>  // for invalid_argument
#include <utility>    // for move

#include "util/StringUtils.h"   // for StringUtils
#include "util/serdesstream.h"  // for serdes_stream


NamedColor::NamedColor():
        paletteIndex{0}, name{"Custom Color"}, colorU16{ColorU16{}}, color{Color(0u)}, isPaletteColor{false} {}

NamedColor::NamedColor(size_t paletteIndex):
        paletteIndex{paletteIndex},
        name{"Fallback Color"},
        colorU16{ColorU16{}},
        color{Color(0u)},
        isPaletteColor{true} {}

NamedColor::NamedColor(Color color):
        paletteIndex{0},
        name{"Custom Color"},
        colorU16(Util::argb_to_ColorU16(color)),
        color(color),
        isPaletteColor{false} {};

auto operator>>(std::istream& str, NamedColor& namedColor) -> std::istream& {
    std::string line;
    NamedColor tmp;
    if (std::getline(str, line)) {
        auto iss = serdes_stream<std::istringstream>(line);
        uint16_t r{}, g{}, b{};
        if (iss >> r >> g >> b && std::getline(iss, tmp.name)) {
            if (r > 255 || g > 255 || b > 255) {
                throw std::invalid_argument("RGB values bigger than 255 are not supported.");
            }
            tmp.color = Color(uint8_t(r), uint8_t(g), uint8_t(b));
            tmp.name = StringUtils::trim(tmp.name);
            tmp.colorU16 = Util::argb_to_ColorU16(tmp.color);
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

auto NamedColor::getName() const -> std::string const& { return name; };
