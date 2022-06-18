#include "SettingsEnums.h"

#include <glib.h>  // for g_warning

auto stylusCursorTypeFromString(const std::string& stylusCursorTypeStr) -> StylusCursorType {
    if (stylusCursorTypeStr == "none") {
        return STYLUS_CURSOR_NONE;
    }
    if (stylusCursorTypeStr == "dot") {
        return STYLUS_CURSOR_DOT;
    }
    if (stylusCursorTypeStr == "big") {
        return STYLUS_CURSOR_BIG;
    }
    if (stylusCursorTypeStr == "arrow") {
        return STYLUS_CURSOR_ARROW;
    }
    g_warning("Settings::Unknown stylus cursor type: %s\n", stylusCursorTypeStr.c_str());
    return STYLUS_CURSOR_DOT;
}

auto iconThemeFromString(const std::string& iconThemeStr) -> IconTheme {
    if (iconThemeStr == "iconsColor") {
        return ICON_THEME_COLOR;
    }
    if (iconThemeStr == "iconsLucide") {
        return ICON_THEME_LUCIDE;
    }
    g_warning("Settings::Unknown icon theme: %s\n", iconThemeStr.c_str());
    return ICON_THEME_COLOR;
}
