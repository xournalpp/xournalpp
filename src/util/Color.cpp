#include "util/Color.h"

#include <algorithm>  // for max, min
#include <cassert>    // for assert
#include <iomanip>    // for operator<<, setfill, setw
#include <sstream>    // for operator<<, stringstream, basic_ostream, char_t...

#include "util/serdesstream.h"  // for serdes_stream

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
    auto s = serdes_stream<std::ostringstream>();
    auto tmp_color = rgb;
    tmp_color.alpha = 0;
    s << "#" << std::hex << std::setfill('0') << std::setw(6) << std::right << tmp_color;

    return s.str();
}
