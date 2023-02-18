#include "ToolEnums.h"

auto toolSizeToString(ToolSize size) -> std::string {
    switch (size) {
        case TOOL_SIZE_NONE:
            return "none";
        case TOOL_SIZE_VERY_FINE:
            return "veryThin";
        case TOOL_SIZE_FINE:
            return "thin";
        case TOOL_SIZE_MEDIUM:
            return "medium";
        case TOOL_SIZE_THICK:
            return "thick";
        case TOOL_SIZE_VERY_THICK:
            return "veryThick";
        default:
            return "";
    }
}

auto toolSizeFromString(const std::string& size) -> ToolSize {
    if (size == "veryThin") {
        return TOOL_SIZE_VERY_FINE;
    }
    if (size == "thin") {
        return TOOL_SIZE_FINE;
    }
    if (size == "medium") {
        return TOOL_SIZE_MEDIUM;
    }
    if (size == "thick") {
        return TOOL_SIZE_THICK;
    }
    if (size == "veryThick") {
        return TOOL_SIZE_VERY_THICK;
    }
    return TOOL_SIZE_NONE;
}

auto drawingTypeToString(DrawingType type) -> std::string {
    switch (type) {
        case DRAWING_TYPE_DONT_CHANGE:
            return "dontChange";
        case DRAWING_TYPE_DEFAULT:
            return "default";
        case DRAWING_TYPE_LINE:
            return "line";
        case DRAWING_TYPE_RECTANGLE:
            return "rectangle";
        case DRAWING_TYPE_ELLIPSE:
            return "ellipse";
        case DRAWING_TYPE_ARROW:
            return "arrow";
        case DRAWING_TYPE_DOUBLE_ARROW:
            return "doubleArrow";
        case DRAWING_TYPE_STROKE_RECOGNIZER:
            return "strokeRecognizer";
        case DRAWING_TYPE_COORDINATE_SYSTEM:
            return "drawCoordinateSystem";
        case DRAWING_TYPE_SPLINE:
            return "spline";
        default:
            return "";
    }
}

auto drawingTypeFromString(const std::string& type) -> DrawingType {
    if (type == "dontChange") {
        return DRAWING_TYPE_DONT_CHANGE;
    }
    if (type == "line") {
        return DRAWING_TYPE_LINE;
    }
    if (type == "rectangle") {
        return DRAWING_TYPE_RECTANGLE;
    }
    if (type == "ellipse") {
        return DRAWING_TYPE_ELLIPSE;
    }
    if (type == "arrow") {
        return DRAWING_TYPE_ARROW;
    }
    if (type == "doubleArrow") {
        return DRAWING_TYPE_DOUBLE_ARROW;
    }
    if (type == "strokeRecognizer") {
        return DRAWING_TYPE_STROKE_RECOGNIZER;
    }
    if (type == "drawCoordinateSystem") {
        return DRAWING_TYPE_COORDINATE_SYSTEM;
    }
    if (type == "spline") {
        return DRAWING_TYPE_SPLINE;
    }
    return DRAWING_TYPE_DEFAULT;
}

auto isSelectToolType(ToolType type) -> bool {
    return type == TOOL_SELECT_RECT
            || type == TOOL_SELECT_REGION
            || type == TOOL_SELECT_MULTILAYER_RECT
            || type == TOOL_SELECT_MULTILAYER_REGION
            || type == TOOL_SELECT_OBJECT;
}

auto toolTypeToString(ToolType type) -> std::string {
    switch (type) {
        case TOOL_NONE:
            return "none";
        case TOOL_PEN:
            return "pen";
        case TOOL_ERASER:
            return "eraser";
        case TOOL_HIGHLIGHTER:
            return "highlighter";
        case TOOL_TEXT:
            return "text";
        case TOOL_IMAGE:
            return "image";
        case TOOL_SELECT_RECT:
            return "selectRect";
        case TOOL_SELECT_REGION:
            return "selectRegion";
        case TOOL_SELECT_MULTILAYER_RECT:
            return "selectMultiLayerRect";
        case TOOL_SELECT_MULTILAYER_REGION:
            return "selectMultiLayerRegion";
        case TOOL_SELECT_OBJECT:
            return "selectObject";
        case TOOL_PLAY_OBJECT:
            return "playObject";
        case TOOL_VERTICAL_SPACE:
            return "verticalSpace";
        case TOOL_HAND:
            return "hand";
        case TOOL_DRAW_RECT:
            return "drawRect";
        case TOOL_DRAW_ELLIPSE:
            return "drawEllipse";
        case TOOL_DRAW_ARROW:
            return "drawArrow";
        case TOOL_DRAW_DOUBLE_ARROW:
            return "drawDoubleArrow";
        case TOOL_DRAW_COORDINATE_SYSTEM:
            return "drawCoordinateSystem";
        case TOOL_DRAW_SPLINE:
            return "drawSpline";
        case TOOL_FLOATING_TOOLBOX:
            return "showFloatingToolbox";
        case TOOL_SELECT_PDF_TEXT_LINEAR:
            return "selectPdfTextLinear";
        case TOOL_SELECT_PDF_TEXT_RECT:
            return "selectPdfTextRect";
        default:
            return "";
    }
}

auto toolTypeFromString(const std::string& type) -> ToolType {
    if (type == "pen") {
        return TOOL_PEN;
    }
    if (type == "eraser") {
        return TOOL_ERASER;
    }
    // recognize previous spelling of Highlighter, V1.0.19 (Dec 2020) and earlier
    if (type == "highlighter" || type == "hilighter") {
        return TOOL_HIGHLIGHTER;
    }
    if (type == "text") {
        return TOOL_TEXT;
    }
    if (type == "image") {
        return TOOL_IMAGE;
    }
    if (type == "selectRect") {
        return TOOL_SELECT_RECT;
    }
    if (type == "selectRegion") {
        return TOOL_SELECT_REGION;
    }
    if (type == "selectMultiLayerRect") {
        return TOOL_SELECT_MULTILAYER_RECT;
    }
    if (type == "selectMultiLayerRegion") {
        return TOOL_SELECT_MULTILAYER_REGION;
    }
    if (type == "selectObject") {
        return TOOL_SELECT_OBJECT;
    }
    if (type == "playObject") {
        return TOOL_PLAY_OBJECT;
    }
    if (type == "verticalSpace") {
        return TOOL_VERTICAL_SPACE;
    }
    if (type == "hand") {
        return TOOL_HAND;
    }
    if (type == "drawRect") {
        return TOOL_DRAW_RECT;
    }
    // recognize previous spelling of Ellipse, V1.1.0+dev (Jan 2021) and earlier
    if (type == "drawEllipse" || type == "drawCircle") {
        return TOOL_DRAW_ELLIPSE;
    }
    if (type == "drawArrow") {
        return TOOL_DRAW_ARROW;
    }
    if (type == "drawDoubleArrow") {
        return TOOL_DRAW_DOUBLE_ARROW;
    }
    if (type == "drawCoordinateSystem") {
        return TOOL_DRAW_COORDINATE_SYSTEM;
    }
    if (type == "drawSpline") {
        return TOOL_DRAW_SPLINE;
    }
    if (type == "showFloatingToolbox") {
        return TOOL_FLOATING_TOOLBOX;
    }
    if (type == "selectPdfTextLinear") {
        return TOOL_SELECT_PDF_TEXT_LINEAR;
    }
    if (type == "selectPdfTextRect") {
        return TOOL_SELECT_PDF_TEXT_RECT;
    }
    return TOOL_NONE;
}

auto eraserTypeToString(EraserType type) -> std::string {
    switch (type) {
        case ERASER_TYPE_NONE:
            return "none";
        case ERASER_TYPE_DEFAULT:
            return "default";
        case ERASER_TYPE_WHITEOUT:
            return "whiteout";
        case ERASER_TYPE_DELETE_STROKE:
            return "deleteStroke";
        default:
            return "";
    }
}

auto eraserTypeFromString(const std::string& type) -> EraserType {
    if (type == "default") {
        return ERASER_TYPE_DEFAULT;
    }
    if (type == "whiteout") {
        return ERASER_TYPE_WHITEOUT;
    }
    if (type == "deleteStroke") {
        return ERASER_TYPE_DELETE_STROKE;
    }
    return ERASER_TYPE_NONE;
}


bool xoj::tool::isPdfSelectionTool(ToolType toolType) {
    return toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT;
}
