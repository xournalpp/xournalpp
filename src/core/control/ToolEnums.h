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

#include "model/LineStyle.h"

#define FOR_TOOLSIZE(DO)                  \
    DO(TOOL_SIZE_VERY_FINE, "veryThin")   \
    DO(TOOL_SIZE_FINE, "thin")            \
    DO(TOOL_SIZE_MEDIUM, "medium")        \
    DO(TOOL_SIZE_THICK, "thick")          \
    DO(TOOL_SIZE_VERY_THICK, "veryThick") \
    DO(TOOL_SIZE_NONE, "none")

#define DEFINE_ENUMERATION(name, str, ...) name,

enum ToolSize { FOR_TOOLSIZE(DEFINE_ENUMERATION) };
std::string toolSizeToString(ToolSize size);
ToolSize toolSizeFromString(const std::string& size);

#define FOR_DRAWINGTYPE(DO)                                    \
    DO(DRAWING_TYPE_DONT_CHANGE, "dontChange")                 \
    DO(DRAWING_TYPE_DEFAULT, "default")                        \
    DO(DRAWING_TYPE_LINE, "line")                              \
    DO(DRAWING_TYPE_RECTANGLE, "rectangle")                    \
    DO(DRAWING_TYPE_ELLIPSE, "ellipse")                        \
    DO(DRAWING_TYPE_ARROW, "arrow")                            \
    DO(DRAWING_TYPE_DOUBLE_ARROW, "doubleArrow")               \
    DO(DRAWING_TYPE_COORDINATE_SYSTEM, "drawCoordinateSystem") \
    DO(DRAWING_TYPE_SHAPE_RECOGNIZER, "strokeRecognizer")      \
    DO(DRAWING_TYPE_SPLINE, "spline")

enum DrawingType { FOR_DRAWINGTYPE(DEFINE_ENUMERATION) };
std::string drawingTypeToString(DrawingType type);
DrawingType drawingTypeFromString(const std::string& type);


// The numbers must agree with the action's targets in ui/mainmenubar.xml
#define FOR_TOOLTYPE(DO)                                           \
    DO(TOOL_NONE, "none", 0)                                       \
    DO(TOOL_PEN, "pen", 1)                                         \
    DO(TOOL_ERASER, "eraser", 2)                                   \
    DO(TOOL_HIGHLIGHTER, "highlighter", 3)                         \
    DO(TOOL_TEXT, "text", 4)                                       \
    DO(TOOL_IMAGE, "image", 5)                                     \
    DO(TOOL_SELECT_RECT, "selectRect", 6)                          \
    DO(TOOL_SELECT_REGION, "selectRegion", 7)                      \
    DO(TOOL_SELECT_MULTILAYER_RECT, "selectMultiLayerRect", 8)     \
    DO(TOOL_SELECT_MULTILAYER_REGION, "selectMultiLayerRegion", 9) \
    DO(TOOL_SELECT_OBJECT, "selectObject", 10)                     \
    DO(TOOL_PLAY_OBJECT, "playObject", 11)                         \
    DO(TOOL_VERTICAL_SPACE, "verticalSpace", 12)                   \
    DO(TOOL_HAND, "hand", 13)                                      \
    DO(TOOL_DRAW_RECT, "drawRect", 14)                             \
    DO(TOOL_DRAW_ELLIPSE, "drawEllipse", 15)                       \
    DO(TOOL_DRAW_ARROW, "drawArrow", 16)                           \
    DO(TOOL_DRAW_DOUBLE_ARROW, "drawDoubleArrow", 17)              \
    DO(TOOL_DRAW_COORDINATE_SYSTEM, "drawCoordinateSystem", 18)    \
    DO(TOOL_FLOATING_TOOLBOX, "showFloatingToolbox", 19)           \
    DO(TOOL_DRAW_SPLINE, "drawSpline", 20)                         \
    DO(TOOL_SELECT_PDF_TEXT_LINEAR, "selectPdfTextLinear", 21)     \
    DO(TOOL_SELECT_PDF_TEXT_RECT, "selectPdfTextRect", 22)         \
    DO(TOOL_LASER_POINTER_PEN, "laserPointerPen", 23)              \
    DO(TOOL_LASER_POINTER_HIGHLIGHTER, "laserPointerHighlighter", 24)

#define ASSIGN_ENUM_VALUES(name, str, ind) name = ind,
enum ToolType { FOR_TOOLTYPE(ASSIGN_ENUM_VALUES) TOOL_END_ENTRY };
auto isSelectToolType(ToolType type) -> bool;
auto isSelectToolTypeSingleLayer(ToolType type) -> bool;

/**
 * @brief Whether or not the tool needs the selection to be cleared when selected
 */
auto requiresClearedSelection(ToolType type) -> bool;

// The count of tools
#define TOOL_COUNT (TOOL_END_ENTRY - 1)

std::string toolTypeToString(ToolType type);
ToolType toolTypeFromString(const std::string& type);

#define FOR_OPACITYFEATURE(DO)                             \
    DO(OPACITY_NONE, "none")                               \
    DO(OPACITY_FILL_PEN, "opacityFillPen")                 \
    DO(OPACITY_FILL_HIGHLIGHTER, "opacityFillHighlighter") \
    DO(OPACITY_SELECT_PDF_TEXT_MARKER, "opacitySelectPdfTextMarker")

enum OpacityFeature { FOR_OPACITYFEATURE(DEFINE_ENUMERATION) };
std::string opacityFeatureToString(OpacityFeature feature);
OpacityFeature opacityFeatureFromString(const std::string& feature);

#define FOR_ERASERTYPE(DO)               \
    DO(ERASER_TYPE_NONE, "none")         \
    DO(ERASER_TYPE_DEFAULT, "default")   \
    DO(ERASER_TYPE_WHITEOUT, "whiteout") \
    DO(ERASER_TYPE_DELETE_STROKE, "deleteStroke")

enum EraserType { FOR_ERASERTYPE(DEFINE_ENUMERATION) };
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

#define FOR_STROKETYPE(DO)                \
    DO(STROKE_TYPE_NONE, "none")          \
    DO(STROKE_TYPE_STANDARD, "standard")  \
    DO(STROKE_TYPE_DASHED, "dashed")      \
    DO(STROKE_TYPE_DASHDOTTED, "dashdot") \
    DO(STROKE_TYPE_DOTTED, "dot")

enum StrokeType { FOR_STROKETYPE(DEFINE_ENUMERATION) };
auto strokeTypeFromString(const std::string& type) -> StrokeType;
auto strokeTypeToLineStyle(StrokeType type) -> LineStyle;
auto strokeTypeToString(StrokeType type) -> std::string;

namespace xoj::tool {
/// \return Whether the provided tool is used for selecting objects on a PDF.
bool isPdfSelectionTool(ToolType toolType);
}  // namespace xoj::tool
