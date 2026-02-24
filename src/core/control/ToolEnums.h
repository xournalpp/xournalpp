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

#include <array>
#include <string>  // for string
#include <cstdint>

#include "model/LineStyle.h"

enum ToolSize {
    TOOL_SIZE_VERY_FINE = 0,
    TOOL_SIZE_FINE,
    TOOL_SIZE_MEDIUM,
    TOOL_SIZE_THICK,
    TOOL_SIZE_VERY_THICK,
    // None has to be at the end, because this enum is used as memory offset
    TOOL_SIZE_NONE
};

static constexpr std::array<std::string_view, 6> toolSizeNames{"veryThin", "thin",      "medium",
                                                               "thick",    "veryThick", "none"};

static constexpr std::string_view toolSizeToString(ToolSize size) {
    return toolSizeNames.at(static_cast<size_t>(size));
}
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
    DRAWING_TYPE_SHAPE_RECOGNIZER,
    DRAWING_TYPE_SPLINE,
    DRAWING_TYPE_EXP,
    DRAWING_TYPE_GAUSS,
    DRAWING_TYPE_POLY,
    DRAWING_TYPE_POLYNEG,
    DRAWING_TYPE_SINUS
};
static constexpr std::array<std::string_view, 15> drawingTypeNames{
        "dontChange",           "default",          "line",  "rectangle", "ellipse", "arrow", "doubleArrow",
        "drawCoordinateSystem", "strokeRecognizer", "spline", "exp", "gauss", "poly", "polyneg", "sinus"};

static constexpr std::string_view drawingTypeToString(DrawingType type) {
    return drawingTypeNames.at(static_cast<size_t>(type));
}
DrawingType drawingTypeFromString(const std::string& type);

// The numbers must agree with the action's targets in ui/mainmenubar.xml
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
    TOOL_DRAW_EXP = 25,
    TOOL_DRAW_GAUSS = 26,
    TOOL_DRAW_POLY = 27,
    TOOL_DRAW_POLYNEG = 28,
    TOOL_DRAW_SINUS = 29,
    TOOL_SELECT_PDF_TEXT_LINEAR = 21,
    TOOL_SELECT_PDF_TEXT_RECT = 22,
    TOOL_LASER_POINTER_PEN = 23,
    TOOL_LASER_POINTER_HIGHLIGHTER = 24,

    TOOL_END_ENTRY
};
static constexpr std::array<std::string_view, 30> toolNames{"none",
                                                            "pen",
                                                            "eraser",
                                                            "highlighter",
                                                            "text",
                                                            "image",
                                                            "selectRect",
                                                            "selectRegion",
                                                            "selectMultiLayerRect",
                                                            "selectMultiLayerRegion",
                                                            "selectObject",
                                                            "playObject",
                                                            "verticalSpace",
                                                            "hand",
                                                            "drawRect",
                                                            "drawEllipse",
                                                            "drawArrow",
                                                            "drawDoubleArrow",
                                                            "drawCoordinateSystem",
                                                            "showFloatingToolbox",
                                                            "drawSpline",
                                                            "drawExp",
                                                            "drawGauss",
                                                            "drawPoly",
                                                            "drawPolyNeg",
                                                            "drawSinus",
                                                            "selectPdfTextLinear",
                                                            "selectPdfTextRect",
                                                            "laserPointerPen",
                                                            "laserPointerHighlighter"};

auto isSelectToolType(ToolType type) -> bool;
auto isSelectToolTypeSingleLayer(ToolType type) -> bool;

/**
 * @brief Whether or not the tool needs the selection to be cleared when selected
 */
auto requiresClearedSelection(ToolType type) -> bool;

// The count of tools
#define TOOL_COUNT (TOOL_END_ENTRY - 1)

static constexpr std::string_view toolTypeToString(ToolType type) { return toolNames.at(static_cast<size_t>(type)); }
ToolType toolTypeFromString(const std::string& type);


enum OpacityFeature {
    OPACITY_NONE,
    OPACITY_FILL_PEN,
    OPACITY_FILL_HIGHLIGHTER,
    OPACITY_SELECT_PDF_TEXT_MARKER,
};
static constexpr std::array<std::string_view, 4> opacityFeatureNames{"none", "opacityFillPen", "opacityFillHighlighter",
                                                                     "opacitySelectPdfTextMarker"};

static constexpr std::string_view opacityFeatureToString(OpacityFeature feature) {
    return opacityFeatureNames.at(static_cast<size_t>(feature));
}
OpacityFeature opacityFeatureFromString(const std::string& feature);

enum EraserType { ERASER_TYPE_NONE = 0, ERASER_TYPE_DEFAULT, ERASER_TYPE_WHITEOUT, ERASER_TYPE_DELETE_STROKE };
static constexpr std::array<std::string_view, 4> eraserTypeNames{"none", "default", "whiteout", "deleteStroke"};

static constexpr std::string_view eraserTypeToString(EraserType type) {
    return eraserTypeNames.at(static_cast<size_t>(type));
}
EraserType eraserTypeFromString(const std::string& type);


enum ToolCapabilities : uint32_t {
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
    TOOL_CAP_EXP = 1 << 13,
    TOOL_CAP_GAUSS = 1 << 14,
    TOOL_CAP_POLY = 1 << 15,
    TOOL_CAP_POLYNEG = 1 << 16,
    TOOL_CAP_SINUS = 1 << 17,
    TOOL_CAP_LINE_STYLE = 1 << 12
};

enum StrokeType {
    STROKE_TYPE_NONE = 0,
    STROKE_TYPE_STANDARD = 1,
    STROKE_TYPE_DASHED = 2,
    STROKE_TYPE_DASHDOTTED = 3,
    STROKE_TYPE_DOTTED = 4
};

static constexpr std::array<std::string_view, 5> strokeTypeNames{"none", "standard", "dashed", "dashdot", "dot"};

auto strokeTypeFromString(const std::string& type) -> StrokeType;
auto strokeTypeToLineStyle(StrokeType type) -> LineStyle;
static constexpr auto strokeTypeToString(StrokeType type) -> std::string_view {
    return strokeTypeNames.at(static_cast<size_t>(type));
}

namespace xoj::tool {
/// \return Whether the provided tool is used for selecting objects on a PDF.
bool isPdfSelectionTool(ToolType toolType);
}  // namespace xoj::tool
