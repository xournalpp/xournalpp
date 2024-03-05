#include "control/xojfile/XmlParserHelper.h"

#include <array>
#include <cstdint>
#include <exception>
#include <ios>
#include <istream>
#include <optional>
#include <ostream>
#include <string>

#include <glib.h>

#include "model/LineStyle.h"
#include "model/StrokeStyle.h"
#include "util/Color.h"


// template specializations

template <>
auto XmlParserHelper::getAttrib<std::string>(const std::string& name,
                                             const AttributeMap& attributeMap) -> std::optional<std::string> {
    auto it = attributeMap.find(name);
    if (it != attributeMap.end()) {
        return it->second;
    } else {
        return {};
    }
}

template <>
auto XmlParserHelper::getAttribMandatory<std::string>(const std::string& name, const AttributeMap& attributeMap,
                                                      const std::string& defaultValue, bool warn) -> std::string {
    auto it = attributeMap.find(name);
    if (it != attributeMap.end()) {
        return it->second;
    } else {
        if (warn) {
            g_warning("XML parser: Mandatory attribute \"%s\" not found. Using default value \"%s\"", name.c_str(),
                      defaultValue.c_str());
        }
        return defaultValue;
    }
}


// custom attribute parsing functions

auto XmlParserHelper::getAttribColorMandatory(const AttributeMap& attributeMap, const Color& defaultValue,
                                              bool bg) -> Color {
    const auto optColorStr = getAttrib<std::string>("color", attributeMap);

    if (optColorStr) {
        std::optional<Color> optColor;
        if (bg) {
            optColor = parseBgColor(*optColorStr);
            if (optColor) {
                return *optColor;
            }
        }
        optColor = parseColorCode(*optColorStr);
        if (optColor) {
            return *optColor;
        }
        optColor = parsePredefinedColor(*optColorStr);
        if (optColor) {
            return *optColor;
        }

        // Nothing worked: fall back to default value
        g_warning("XML parser: Unkown color \"%s\" found. Using default value \"%s\"", optColorStr->c_str(),
                  Util::rgb_to_hex_string(defaultValue).c_str());
        return defaultValue;
    } else {
        g_warning("XML parser: Mandatory attribute \"color\" not found. Using default value \"%s\"",
                  Util::rgb_to_hex_string(defaultValue).c_str());
        return defaultValue;
    }
}

struct PredefinedColor {
    const char* name = nullptr;
    Color color{};
};

constexpr std::array<PredefinedColor, 5> BACKGROUND_COLORS = {{{"blue", Colors::xopp_paleturqoise},
                                                               {"pink", Colors::xopp_pink},
                                                               {"green", Colors::xopp_aquamarine},
                                                               {"orange", Colors::xopp_lightsalmon},
                                                               {"yellow", Colors::xopp_khaki}}};

auto XmlParserHelper::parseBgColor(const std::string& str) -> std::optional<Color> {
    for (const auto& i: BACKGROUND_COLORS) {
        if (str == i.name) {
            return i.color;
        }
    }

    // color not found in predefined background colors
    return {};
}

auto XmlParserHelper::parseColorCode(const std::string& str) -> std::optional<Color> {
    if ((!str.empty()) && (str[0] == '#')) {
        uint32_t color{};
        try {
            color = static_cast<uint32_t>(std::stoul(str.substr(1), nullptr, 16));
        } catch (const std::exception& e) {
            g_warning("XML parser: Unknown color code \"%s\".\nMessage: %s", str.c_str(), e.what());
            return {};
        }
        // discards alpha for now
        return Color((color >> 8U) | (color << 24U));  // constructor takes AARRGGBB byte order instead of RRGGBBAA
    } else {
        // not a color code
        return {};
    }
}

constexpr std::array<PredefinedColor, 11> PREDEFINED_COLORS = {{{"black", Colors::black},
                                                                {"blue", Colors::xopp_royalblue},
                                                                {"red", Colors::red},
                                                                {"green", Colors::green},
                                                                {"gray", Colors::gray},
                                                                {"lightblue", Colors::xopp_deepskyblue},
                                                                {"lightgreen", Colors::lime},
                                                                {"magenta", Colors::magenta},
                                                                {"orange", Colors::xopp_darkorange},
                                                                {"yellow", Colors::yellow},
                                                                {"white", Colors::white}}};

auto XmlParserHelper::parsePredefinedColor(const std::string& str) -> std::optional<Color> {
    for (const auto& i: PREDEFINED_COLORS) {
        if (str == i.name) {
            return i.color;
        }
    }

    g_warning("XML parser: Color \"%s\" unknown (not defined in default color list)", str.c_str());
    return {};
}


auto XmlParserHelper::decodeBase64(const char* base64data) -> std::string {
    size_t binaryBufferLen = 0;
    guchar* binaryBuffer = g_base64_decode(base64data, &binaryBufferLen);
    auto str = std::string(reinterpret_cast<char*>(binaryBuffer), binaryBufferLen);
    g_free(binaryBuffer);
    return str;
}


// stream operator overloads

// XmlParserHelper::Domain
auto operator<<(std::ostream& stream, const XmlParserHelper::Domain domain) -> std::ostream& {
    switch (domain) {
        case XmlParserHelper::Domain::ABOSLUTE:
            stream << "absolute";
            break;
        case XmlParserHelper::Domain::ATTACH:
            stream << "attach";
            break;
        case XmlParserHelper::Domain::CLONE:
            stream << "clone";
            break;
        default:
            // invalid domain
            stream.setstate(std::ios::failbit);
            break;
    }
    return stream;
}

auto operator>>(std::istream& stream, XmlParserHelper::Domain& domain) -> std::istream& {
    std::string str;
    stream >> str;
    if (str == "absolute") {
        domain = XmlParserHelper::Domain::ABOSLUTE;
    } else if (str == "attach") {
        domain = XmlParserHelper::Domain::ATTACH;
    } else if (str == "clone") {
        domain = XmlParserHelper::Domain::CLONE;
    } else {
        // invalid input
        stream.setstate(std::ios::failbit);
    }
    return stream;
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