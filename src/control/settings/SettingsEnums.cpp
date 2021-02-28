#include "SettingsEnums.h"

auto stylusCursorTypeFromString(const string& stylusCursorTypeStr) -> StylusCursorType {
    if (stylusCursorTypeStr == "none") {
        return STYLUS_CURSOR_NONE;
    }
    if (stylusCursorTypeStr == "dot") {
        return STYLUS_CURSOR_DOT;
    }
    if (stylusCursorTypeStr == "big") {
        return STYLUS_CURSOR_BIG;
    }
    g_warning("Settings::Unknown stylus cursor type: %s\n", stylusCursorTypeStr.c_str());
    return STYLUS_CURSOR_DOT;
}

auto iconStyleFromString(const std::string& s) -> std::optional<IconStyle> {
    if (s == "Color")
        return {IconStyle::Color};
    else if (s == "Mono")
        return {IconStyle::Mono};

    return {};
}

auto iconStyleToString(IconStyle sty) -> std::string {
    switch (sty) {
        case IconStyle::Color:
            return "Color";
        case IconStyle::Mono:
            return "Mono";
    }
    g_assert(false);
    return "";
}
