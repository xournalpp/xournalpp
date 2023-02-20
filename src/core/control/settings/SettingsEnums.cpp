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

auto eraserVisibilityFromString(const std::string& eraserVisibility) -> EraserVisibility {
    if (eraserVisibility == "never") {
        return ERASER_VISIBILITY_NEVER;
    }
    if (eraserVisibility == "always") {
        return ERASER_VISIBILITY_ALWAYS;
    }
    if (eraserVisibility == "hover") {
        return ERASER_VISIBILITY_HOVER;
    }
    if (eraserVisibility == "touch") {
        return ERASER_VISIBILITY_TOUCH;
    }
    g_warning("Settings::Unknown eraser visibility: %s\n", eraserVisibility.c_str());
    return ERASER_VISIBILITY_ALWAYS;
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

auto emptyLastPageAppendFromString(const std::string& str) -> EmptyLastPageAppendType {
    if (str == "disabled") {
        return EmptyLastPageAppendType::Disabled;
    }
    if (str == "onDrawOfLastPage") {
        return EmptyLastPageAppendType::OnDrawOfLastPage;
    }
    if (str == "onScrollOfLastPage") {
        return EmptyLastPageAppendType::OnScrollToEndOfLastPage;
    }

    g_warning("Settings::Unknown empty last page append type: %s\n", str.c_str());
    return EmptyLastPageAppendType::Disabled;
}
