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


enum ToolSize {
    TOOL_SIZE_VERY_FINE = 0,
    TOOL_SIZE_FINE,
    TOOL_SIZE_MEDIUM,
    TOOL_SIZE_THICK,
    TOOL_SIZE_VERY_THICK,
    // None has to be at the end, because this enum is used as memory offset
    TOOL_SIZE_NONE
};
std::string toolSizeToString(ToolSize size);
ToolSize toolSizeFromString(const std::string& size);


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
    DRAWING_TYPE_DOUBLE_ARROW,
    DRAWING_TYPE_COORDINATE_SYSTEM,
    DRAWING_TYPE_STROKE_RECOGNIZER,
    DRAWING_TYPE_SPLINE
};
std::string drawingTypeToString(DrawingType type);
DrawingType drawingTypeFromString(const std::string& type);


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
    TOOL_SELECT_MULTILAYER_RECT = 8,
    TOOL_SELECT_MULTILAYER_REGION = 9,
    TOOL_SELECT_OBJECT = 10,
    TOOL_PLAY_OBJECT = 11,
    TOOL_VERTICAL_SPACE = 12,
    TOOL_HAND = 13,
    TOOL_DRAW_RECT = 14,
    TOOL_DRAW_ELLIPSE = 15,
    TOOL_DRAW_ARROW = 16,
    TOOL_DRAW_DOUBLE_ARROW = 17,
    TOOL_DRAW_COORDINATE_SYSTEM = 18,
    TOOL_FLOATING_TOOLBOX = 19,
    TOOL_DRAW_SPLINE = 20,
    TOOL_SELECT_PDF_TEXT_LINEAR = 21,
    TOOL_SELECT_PDF_TEXT_RECT = 22,

    TOOL_END_ENTRY
};
auto isSelectToolType(ToolType type) -> bool;

// The count of tools
#define TOOL_COUNT (TOOL_END_ENTRY - 1)

std::string toolTypeToString(ToolType type);
ToolType toolTypeFromString(const std::string& type);


enum EraserType { ERASER_TYPE_NONE = 0, ERASER_TYPE_DEFAULT, ERASER_TYPE_WHITEOUT, ERASER_TYPE_DELETE_STROKE };
std::string eraserTypeToString(EraserType type);
EraserType eraserTypeFromString(const std::string& type);


enum ToolCapabilities : unsigned int {
    TOOL_CAP_NONE = 0,
    TOOL_CAP_COLOR = 1 << 0,
    TOOL_CAP_SIZE = 1 << 1,
    TOOL_CAP_RULER = 1 << 2,
    TOOL_CAP_RECTANGLE = 1 << 3,
    TOOL_CAP_ELLIPSE = 1 << 4,
    TOOL_CAP_ARROW = 1 << 5,
    TOOL_CAP_DOUBLE_ARROW = 1 << 6,
    TOOL_CAP_RECOGNIZER = 1 << 7,
    TOOL_CAP_FILL = 1 << 8,
    TOOL_CAP_COORDINATE_SYSTEM = 1 << 9,
    TOOL_CAP_DASH_LINE = 1 << 10,
    TOOL_CAP_SPLINE = 1 << 11,
    TOOL_CAP_LINE_STYLE = 1 << 12
};

namespace xoj::tool {
/// \return Whether the provided tool is used for selecting objects on a PDF.
bool isPdfSelectionTool(ToolType toolType);
}  // namespace xoj::tool
