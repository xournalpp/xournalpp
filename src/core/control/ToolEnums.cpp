#include "ToolEnums.h"

#include "model/StrokeStyle.h"

#define HANDLE_CASE(name, str, ...) \
    case name:                      \
        return str;
#define HANDLE_IF_SIZE(name, str, ...) \
    if (size == str) {                 \
        return name;                   \
    }
#define HANDLE_IF_TYPE(name, str, ...) \
    if (type == str) {                 \
        return name;                   \
    }
#define HANDLE_IF_FEATURE(name, str, ...) \
    if (feature == str) {                 \
        return name;                      \
    }

auto toolSizeToString(ToolSize size) -> std::string {
    switch (size) {
        FOR_TOOLSIZE(HANDLE_CASE)
        default:
            return "";
    }
}

auto toolSizeFromString(const std::string& size) -> ToolSize {
    FOR_TOOLSIZE(HANDLE_IF_SIZE)
    return TOOL_SIZE_NONE;
}

auto drawingTypeToString(DrawingType type) -> std::string {
    switch (type) {
        FOR_DRAWINGTYPE(HANDLE_CASE)
        default:
            return "";
    }
}

auto drawingTypeFromString(const std::string& type) -> DrawingType {
    FOR_DRAWINGTYPE(HANDLE_IF_TYPE)
    return DRAWING_TYPE_DEFAULT;
}

auto isSelectToolType(ToolType type) -> bool {
    return type == TOOL_SELECT_RECT || type == TOOL_SELECT_REGION || type == TOOL_SELECT_MULTILAYER_RECT ||
           type == TOOL_SELECT_MULTILAYER_REGION || type == TOOL_SELECT_OBJECT;
}

auto isSelectToolTypeSingleLayer(ToolType type) -> bool {
    return type == TOOL_SELECT_RECT || type == TOOL_SELECT_REGION || type == TOOL_SELECT_OBJECT;
}

auto requiresClearedSelection(ToolType type) -> bool {
    return type == TOOL_PEN || type == TOOL_HIGHLIGHTER || type == TOOL_ERASER || type == TOOL_TEXT ||
           type == TOOL_IMAGE || type == TOOL_SELECT_PDF_TEXT_RECT || type == TOOL_SELECT_PDF_TEXT_RECT ||
           type == TOOL_VERTICAL_SPACE;
}

auto toolTypeToString(ToolType type) -> std::string {
    switch (type) {
        FOR_TOOLTYPE(HANDLE_CASE)
        default:
            return "";
    }
}

auto toolTypeFromString(const std::string& type) -> ToolType {
    FOR_TOOLTYPE(HANDLE_IF_TYPE)
    // recognize previous spelling of Highlighter, V1.0.19 (Dec 2020) and earlier
    if (type == "hilighter") {
        return TOOL_HIGHLIGHTER;
    }
    // recognize previous spelling of Ellipse, V1.1.0+dev (Jan 2021) and earlier
    if (type == "drawCircle") {
        return TOOL_DRAW_ELLIPSE;
    }
    return TOOL_NONE;
}

auto opacityFeatureToString(OpacityFeature feature) -> std::string {
    switch (feature) {
        FOR_OPACITYFEATURE(HANDLE_CASE)
        default:
            return "";
    }
}

auto opacityFeatureFromString(const std::string& feature) -> OpacityFeature {
    FOR_OPACITYFEATURE(HANDLE_IF_FEATURE)
    return OPACITY_NONE;
}

auto eraserTypeToString(EraserType type) -> std::string {
    switch (type) {
        FOR_ERASERTYPE(HANDLE_CASE)
        default:
            return "";
    }
}

auto eraserTypeFromString(const std::string& type) -> EraserType {
    FOR_ERASERTYPE(HANDLE_IF_TYPE)
    return ERASER_TYPE_NONE;
}

auto strokeTypeToLineStyle(StrokeType type) -> LineStyle {
    switch (type) {
        case STROKE_TYPE_STANDARD:
            return {};
        case STROKE_TYPE_DASHED:
            return StrokeStyle::parseStyle("dash");
        case STROKE_TYPE_DASHDOTTED:
            return StrokeStyle::parseStyle("dashdot");
        case STROKE_TYPE_DOTTED:
            return StrokeStyle::parseStyle("dot");
        default:
            return {};
    }
}

auto strokeTypeToString(StrokeType type) -> std::string {
    switch (type) {
        FOR_STROKETYPE(HANDLE_CASE)
        default:
            return "";
    }
}
auto strokeTypeFromString(const std::string& type) -> StrokeType {
    FOR_STROKETYPE(HANDLE_IF_TYPE)
    return STROKE_TYPE_NONE;
}

bool xoj::tool::isPdfSelectionTool(ToolType toolType) {
    return toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT;
}
