/*
 * Xournal++
 *
 * Enum definition used for tools
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string


/**
 * @brief Buttons that can have a configuration attached to them
 * Used for ButtonConfig and ToolHandler
 *
 */
enum Button {
    BUTTON_ERASER,
    BUTTON_MOUSE_MIDDLE,
    BUTTON_MOUSE_RIGHT,
    BUTTON_TOUCH,
    BUTTON_DEFAULT,
    BUTTON_STYLUS_ONE,
    BUTTON_STYLUS_TWO,
    BUTTON_COUNT
};

enum AttributeType {
    ATTRIBUTE_TYPE_NONE,
    ATTRIBUTE_TYPE_STRING,
    ATTRIBUTE_TYPE_INT,
    ATTRIBUTE_TYPE_DOUBLE,
    ATTRIBUTE_TYPE_INT_HEX,
    ATTRIBUTE_TYPE_BOOLEAN,
};

// use this as a bit flag
enum ScrollbarHideType {
    SCROLLBAR_HIDE_NONE = 0,
    SCROLLBAR_HIDE_HORIZONTAL = 1 << 1,
    SCROLLBAR_HIDE_VERTICAL = 1 << 2,
    SCROLLBAR_HIDE_BOTH = SCROLLBAR_HIDE_HORIZONTAL | SCROLLBAR_HIDE_VERTICAL
};

enum class EmptyLastPageAppendType {
    Disabled = 0,
    OnDrawOfLastPage = 1,
    OnScrollToEndOfLastPage = 2,
};

/**
 * The user-selectable device types
 */
enum class InputDeviceTypeOption {
    Disabled = 0,
    Mouse = 1,
    Pen = 2,
    Eraser = 3,
    Touchscreen = 4,
    MouseKeyboardCombo = 5,
};

enum StylusCursorType {
    STYLUS_CURSOR_NONE = 0,
    STYLUS_CURSOR_DOT = 1,
    STYLUS_CURSOR_BIG = 2,
    STYLUS_CURSOR_ARROW = 3,
};

enum EraserVisibility {
    ERASER_VISIBILITY_NEVER = 0,
    ERASER_VISIBILITY_ALWAYS = 1,
    ERASER_VISIBILITY_HOVER = 2,
    ERASER_VISIBILITY_TOUCH = 3,
};

enum IconTheme {
    ICON_THEME_COLOR = 0,
    ICON_THEME_LUCIDE = 1,
};

/**
 * The user-selectable Page Preview Decoration style
 */
enum class SidebarNumberingStyle {
    /* No page numbers are displayed */
    NONE = 0,
    /* Page numbers are displayed below previews */
    NUMBER_BELOW_PREVIEW = 1,
    /* Page numbers are displayed in a circle in the lower right corner */
    NUMBER_WITH_CIRCULAR_BACKGROUND = 2,
    /* Page numbers are displayed in a square in the lower right corner */
    NUMBER_WITH_SQUARE_BACKGROUND = 3,

    /* MIN, MAX and Default values. Need to be updated when new style is added. */
    MIN = SidebarNumberingStyle::NONE,
    MAX = SidebarNumberingStyle::NUMBER_WITH_SQUARE_BACKGROUND,
    DEFAULT = SidebarNumberingStyle::NUMBER_BELOW_PREVIEW
};

constexpr auto buttonToString(Button button) -> const char* {
    switch (button) {
        case BUTTON_ERASER:
            return "eraser";
        case BUTTON_MOUSE_MIDDLE:
            return "middle";
        case BUTTON_MOUSE_RIGHT:
            return "right";
        case BUTTON_TOUCH:
            return "touch";
        case BUTTON_DEFAULT:
            return "default";
        case BUTTON_STYLUS_ONE:
            return "stylus";
        case BUTTON_STYLUS_TWO:
            return "stylus2";
        default:
            return "unknown";
    }
}

constexpr auto stylusCursorTypeToString(StylusCursorType stylusCursorType) -> const char* {
    switch (stylusCursorType) {
        case STYLUS_CURSOR_NONE:
            return "none";
        case STYLUS_CURSOR_DOT:
            return "dot";
        case STYLUS_CURSOR_BIG:
            return "big";
        case STYLUS_CURSOR_ARROW:
            return "arrow";
        default:
            return "unknown";
    }
}

constexpr auto eraserVisibilityToString(EraserVisibility eraserVisibility) -> const char* {
    switch (eraserVisibility) {
        case ERASER_VISIBILITY_NEVER:
            return "never";
        case ERASER_VISIBILITY_ALWAYS:
            return "always";
        case ERASER_VISIBILITY_HOVER:
            return "hover";
        case ERASER_VISIBILITY_TOUCH:
            return "touch";
        default:
            return "unknown";
    }
}

constexpr auto iconThemeToString(IconTheme iconTheme) -> const char* {
    switch (iconTheme) {
        case ICON_THEME_COLOR:
            return "iconsColor";
        case ICON_THEME_LUCIDE:
            return "iconsLucide";
        default:
            return "unknown";
    }
}

constexpr auto emptyLastPageAppendToString(EmptyLastPageAppendType appendType) -> const char* {
    switch (appendType) {
        case EmptyLastPageAppendType::Disabled:
            return "disabled";
        case EmptyLastPageAppendType::OnDrawOfLastPage:
            return "onDrawOfLastPage";
        case EmptyLastPageAppendType::OnScrollToEndOfLastPage:
            return "onScrollOfLastPage";
        default:
            return "unknown";
    }
}

StylusCursorType stylusCursorTypeFromString(const std::string& stylusCursorTypeStr);
EraserVisibility eraserVisibilityFromString(const std::string& eraserVisibilityStr);
IconTheme iconThemeFromString(const std::string& iconThemeStr);
EmptyLastPageAppendType emptyLastPageAppendFromString(const std::string& str);
