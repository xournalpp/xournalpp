#include "util/Color.h"

#include <algorithm>  // for max, min
#include <cassert>    // for assert
#include <cstdio>     // for snprintf
#include <sstream>    // for operator<<, stringstream, basic_ostream, char_t...

float Util::as_grayscale_color(Color color) {
    GdkRGBA components = rgb_to_GdkRGBA(color);
    float componentAvg = static_cast<float>(components.red + components.green + components.blue) / 3.0f;

    return componentAvg;
}

float Util::get_color_contrast(Color color1, Color color2) {
    float grayscale1 = as_grayscale_color(color1);
    float grayscale2 = as_grayscale_color(color2);

    return std::max(grayscale1, grayscale2) - std::min(grayscale1, grayscale2);
}

auto Util::rgb_to_hex_string(Color rgb) -> std::string {
    char resultHex[7];

    // 06: Disregard alpha channel and pad with zeroes to a length of 6.
    assert(std::snprintf(resultHex, 7, "%06x", uint32_t(rgb) & 0xffffff) > 0);

    std::stringstream result;
    result << "#" << resultHex;
    return result.str();
}
