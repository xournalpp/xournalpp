#include "control/xojfile/XmlParserHelper.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <glib.h>

#include "control/xojfile/XmlAttrs.h"
#include "model/LineStyle.h"
#include "model/StrokeStyle.h"
#include "util/Assert.h"
#include "util/Color.h"
#include "util/Util.h"
#include "util/safe_casts.h"
#include "util/utf8_view.h"

#include "filesystem.h"


XmlParserHelper::AttributeMap::AttributeMap(const char** attributeNames, const char** attributeValues) {
    // Get array length, and verify that it is identical for names and values
    std::size_t attribute_count = 0;
    while (attributeNames[attribute_count] != nullptr) {
        xoj_assert(attributeValues[attribute_count] != nullptr);
        ++attribute_count;
    }
    xoj_assert(attributeValues[attribute_count] == nullptr);

    // Allocate space for string views
    this->names.resize(attribute_count);
    this->values.resize(attribute_count);

    // Reference strings
    for (size_t i = 0; i < attribute_count; ++i) {
        this->names[i] = std::string_view{attributeNames[i]};
        this->values[i] = std::string_view{attributeValues[i]};
    }
}

auto XmlParserHelper::AttributeMap::operator[](const std::string_view name) const -> std::optional<std::string_view> {
    auto it = std::find(this->names.cbegin(), this->names.cend(), name);
    if (it != this->names.end()) {
        // Name was found
        return this->values[as_unsigned(std::distance(this->names.cbegin(), it))];
    }

    // Name not found
    return std::nullopt;
}

// template specializations

template <>
auto XmlParserHelper::getAttrib<std::string_view>(std::string_view name, const AttributeMap& attributeMap)
        -> std::optional<std::string_view> {
    return attributeMap[name];
}

template <>
auto XmlParserHelper::getAttrib<fs::path>(std::string_view name, const AttributeMap& attributeMap)
        -> std::optional<fs::path> {
    const auto optSV = attributeMap[name];
    if (optSV) {
        return fs::path(*optSV | xoj::util::utf8);
    } else {
        return std::nullopt;
    }
}

template <>
auto XmlParserHelper::getAttrib<LineStyle>(std::string_view name, const AttributeMap& attributeMap)
        -> std::optional<LineStyle> {
    const auto optSV = attributeMap[name];
    if (optSV) {
        // With lots of efforts, we could avoid a copy here, but this attribute likely does
        // not show up often in regular files.
        return StrokeStyle::parseStyle(std::string{*optSV});
    } else {
        return std::nullopt;
    }
}


// custom attribute parsing functions

auto XmlParserHelper::getAttribColorMandatory(const AttributeMap& attributeMap, const Color& defaultValue, bool bg)
        -> Color {
    const auto optColorSV = getAttrib<std::string_view>(xoj::xml_attrs::COLOR_STR, attributeMap);

    if (optColorSV) {
        std::optional<Color> optColor;
        if (bg) {
            optColor = parseBgColor(*optColorSV);
            if (optColor) {
                return *optColor;
            }
        }
        optColor = parseColorCode(*optColorSV);
        if (optColor) {
            return *optColor;
        }
        optColor = parsePredefinedColor(*optColorSV);
        if (optColor) {
            return *optColor;
        }

        // Nothing worked: fall back to default value
        g_warning("XML parser: Unkown color \"" SV_FMT " \" found. Using default value \"%s\"", SV_ARG(*optColorSV),
                  Util::rgb_to_hex_string(defaultValue).c_str());
        return defaultValue;
    } else {
        g_warning(R"(XML parser: Mandatory attribute "color" not found. Using default value "%s")",
                  Util::rgb_to_hex_string(defaultValue).c_str());
        return defaultValue;
    }
}

struct PredefinedColor {
    std::string_view name{};
    Color color{};
};

using namespace std::literals::string_view_literals;
constexpr std::array<PredefinedColor, 5> BACKGROUND_COLORS = {{{"blue"sv, Colors::xopp_paleturqoise},
                                                               {"pink"sv, Colors::xopp_pink},
                                                               {"green"sv, Colors::xopp_aquamarine},
                                                               {"orange"sv, Colors::xopp_lightsalmon},
                                                               {"yellow"sv, Colors::xopp_khaki}}};

auto XmlParserHelper::parseBgColor(std::string_view sv) -> std::optional<Color> {
    for (const auto& i: BACKGROUND_COLORS) {
        if (sv == i.name) {
            return i.color;
        }
    }

    // color not found in predefined background colors
    return {};
}

auto XmlParserHelper::parseColorCode(std::string_view sv) -> std::optional<Color> {
    if ((!sv.empty()) && (sv[0] == '#')) {
        uint32_t color{};
        auto [ptr, ec] = std::from_chars(sv.begin() + 1, sv.end(), color, 16);
        if (ec != std::errc{} || ptr != sv.end()) {
            g_warning("XML parser: Unknown color code \"" SV_FMT "\".", SV_ARG(sv));
            return {};
        }
        // discard alpha for now
        return Color((color >> 8U) | (color << 24U));  // constructor takes AARRGGBB byte order instead of RRGGBBAA
    } else {
        // not a color code
        return {};
    }
}

constexpr std::array<PredefinedColor, 11> PREDEFINED_COLORS = {{{"black"sv, Colors::black},
                                                                {"blue"sv, Colors::xopp_royalblue},
                                                                {"red"sv, Colors::red},
                                                                {"green"sv, Colors::green},
                                                                {"gray"sv, Colors::gray},
                                                                {"lightblue"sv, Colors::xopp_deepskyblue},
                                                                {"lightgreen"sv, Colors::lime},
                                                                {"magenta"sv, Colors::magenta},
                                                                {"orange"sv, Colors::xopp_darkorange},
                                                                {"yellow"sv, Colors::yellow},
                                                                {"white"sv, Colors::white}}};

auto XmlParserHelper::parsePredefinedColor(std::string_view sv) -> std::optional<Color> {
    for (const auto& i: PREDEFINED_COLORS) {
        if (sv == i.name) {
            return i.color;
        }
    }

    g_warning("XML parser: Color \"" SV_FMT "\" unknown (not defined in default color list)", SV_ARG(sv));
    return {};
}


auto XmlParserHelper::decodeBase64(std::string_view base64data) -> std::string {
    // Worst case: 3 bytes per 4 chars (round up)
    const size_t maxDecodedSize = (base64data.size() / 4 + 1) * 3;
    std::string result;
    result.resize(maxDecodedSize);

    // g_base64_decode requires a null-terminated C-string. Use the step decoding
    // function instead and feed the whole string at once.
    gint state = 0;
    guint save = 0;
    const size_t actualSize = g_base64_decode_step(base64data.data(), base64data.size(),
                                                   reinterpret_cast<guchar*>(result.data()), &state, &save);
    result.resize(actualSize);
    result.shrink_to_fit();
    return result;
}

// LineStyle
auto operator<<(std::ostream& stream, const LineStyle& style) -> std::ostream& {
    stream << StrokeStyle::formatStyle(style);
    return stream;
}

auto operator>>(std::istream& stream, LineStyle& style) -> std::istream& {
    std::string str;
    stream >> str;
    style = StrokeStyle::parseStyle(str);
    return stream;
}
