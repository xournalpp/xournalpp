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

#include <string>
#include <vector>

#include "XournalType.h"

enum Buttons {
    BUTTON_ERASER,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_TOUCH,
    BUTTON_DEFAULT,
    BUTTON_STYLUS,
    BUTTON_STYLUS2,
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
};

constexpr auto buttonToString(Buttons button) -> const char* {
    switch (button) {
        case BUTTON_ERASER:
            return "eraser";
        case BUTTON_MIDDLE:
            return "middle";
        case BUTTON_RIGHT:
            return "right";
        case BUTTON_TOUCH:
            return "touch";
        case BUTTON_DEFAULT:
            return "default";
        case BUTTON_STYLUS:
            return "stylus";
        case BUTTON_STYLUS2:
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
        default:
            return "unknown";
    }
}

StylusCursorType stylusCursorTypeFromString(const string& stylusCursorTypeStr);
