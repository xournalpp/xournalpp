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


enum ToolSize {
    TOOL_SIZE_VERY_FINE = 0,
    TOOL_SIZE_FINE,
    TOOL_SIZE_MEDIUM,
    TOOL_SIZE_THICK,
    TOOL_SIZE_VERY_THICK,
    // None has to be at the end, because this enum is used as memory offset
    TOOL_SIZE_NONE
};
string toolSizeToString(ToolSize size);
ToolSize toolSizeFromString(const string& size);


enum DrawingType {
    /**
     * For config entry, don't change value
     */
    DRAWING_TYPE_DONT_CHANGE = 0,

    /**
     * Default drawing, nothing special
     */
    DRAWING_TYPE_DEFAULT,
    DRAWING_TYPE_LINE,
    DRAWING_TYPE_RECTANGLE,
    DRAWING_TYPE_ELLIPSE,
    DRAWING_TYPE_ARROW,
    DRAWING_TYPE_COORDINATE_SYSTEM,
    DRAWING_TYPE_STROKE_RECOGNIZER,
    DRAWING_TYPE_SPLINE
};
string drawingTypeToString(DrawingType type);
DrawingType drawingTypeFromString(const string& type);


// Has to be in the same order as in Action.h: ActionType!
enum ToolType {
    TOOL_NONE = 0,

    // First valid tool, often used starting Index 0
    TOOL_PEN = 1,
    TOOL_ERASER = 2,
    TOOL_HIGHLIGHTER = 3,
    TOOL_TEXT = 4,
    TOOL_IMAGE = 5,
    TOOL_SELECT_RECT = 6,
    TOOL_SELECT_REGION = 7,
    TOOL_SELECT_OBJECT = 8,
    TOOL_PLAY_OBJECT = 9,
    TOOL_VERTICAL_SPACE = 10,
    TOOL_HAND = 11,
    TOOL_DRAW_RECT = 12,
    TOOL_DRAW_ELLIPSE = 13,
    TOOL_DRAW_ARROW = 14,
    TOOL_DRAW_COORDINATE_SYSTEM = 15,
    TOOL_FLOATING_TOOLBOX = 16,
    TOOL_DRAW_SPLINE = 17,

    TOOL_END_ENTRY
};

// The count of tools
#define TOOL_COUNT (TOOL_END_ENTRY - 1)

string toolTypeToString(ToolType type);
ToolType toolTypeFromString(const string& type);


enum EraserType { ERASER_TYPE_NONE = 0, ERASER_TYPE_DEFAULT, ERASER_TYPE_WHITEOUT, ERASER_TYPE_DELETE_STROKE };
string eraserTypeToString(EraserType type);
EraserType eraserTypeFromString(const string& type);


enum ToolCapabilities {
    TOOL_CAP_NONE = 0,
    TOOL_CAP_COLOR = 1 << 0,
    TOOL_CAP_SIZE = 1 << 1,
    TOOL_CAP_RULER = 1 << 2,
    TOOL_CAP_RECTANGLE = 1 << 3,
    TOOL_CAP_ELLIPSE = 1 << 4,
    TOOL_CAP_ARROW = 1 << 5,
    TOOL_CAP_RECOGNIZER = 1 << 6,
    TOOL_CAP_FILL = 1 << 7,
    TOOL_CAP_COORDINATE_SYSTEM = 1 << 8,
    TOOL_CAP_DASH_LINE = 1 << 9,
    TOOL_CAP_SPLINE = 1 << 10,
};
