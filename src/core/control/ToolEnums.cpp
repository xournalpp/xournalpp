#include "ToolEnums.h"

#include <algorithm>  // for find, remove_if

#include "model/StrokeStyle.h"
#include "util/Assert.h"


auto toolSizeFromString(const std::string& size) -> ToolSize {
    auto it = std::find(toolSizeNames.begin(), toolSizeNames.end(), size);
    if (it != toolSizeNames.end()) {
        return static_cast<ToolSize>(std::distance(toolSizeNames.begin(), it));
    }
    return TOOL_SIZE_NONE;
}


auto drawingTypeFromString(const std::string& type) -> DrawingType {
    auto it = std::find(drawingTypeNames.begin(), drawingTypeNames.end(), type);
    if (it != drawingTypeNames.end()) {
        return static_cast<DrawingType>(std::distance(drawingTypeNames.begin(), it));
    }
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


auto toolTypeFromString(const std::string& type) -> ToolType {
    auto it = std::find(toolNames.begin(), toolNames.end(), type);
    if (it != toolNames.end()) {
        return static_cast<ToolType>(std::distance(toolNames.begin(), it));
    }
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


auto opacityFeatureFromString(const std::string& feature) -> OpacityFeature {
    auto it = std::find(opacityFeatureNames.begin(), opacityFeatureNames.end(), feature);
    if (it != opacityFeatureNames.end()) {
        return static_cast<OpacityFeature>(std::distance(opacityFeatureNames.begin(), it));
    }
    return OPACITY_NONE;
}


auto eraserTypeFromString(const std::string& type) -> EraserType {
    auto it = std::find(eraserTypeNames.begin(), eraserTypeNames.end(), type);
    if (it != eraserTypeNames.end()) {
        return static_cast<EraserType>(std::distance(eraserTypeNames.begin(), it));
    }
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

auto strokeTypeFromString(const std::string& type) -> StrokeType {
    auto it = std::find(strokeTypeNames.begin(), strokeTypeNames.end(), type);
    if (it != strokeTypeNames.end()) {
        return static_cast<StrokeType>(std::distance(strokeTypeNames.begin(), it));
    }
    return STROKE_TYPE_NONE;
}

bool xoj::tool::isPdfSelectionTool(ToolType toolType) {
    return toolType == TOOL_SELECT_PDF_TEXT_LINEAR || toolType == TOOL_SELECT_PDF_TEXT_RECT;
}

static constexpr auto makeToolCaps() {
    std::array<std::underlying_type_t<ToolCapabilities>, TOOL_COUNT> caps;

    caps[TOOL_PEN - TOOL_PEN] = TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE |
                                TOOL_CAP_ELLIPSE | TOOL_CAP_ARROW | TOOL_CAP_DOUBLE_ARROW | TOOL_CAP_SPLINE |
                                TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL | TOOL_CAP_LINE_STYLE;
    caps[TOOL_ERASER - TOOL_PEN] = TOOL_CAP_SIZE;
    caps[TOOL_HIGHLIGHTER - TOOL_PEN] = TOOL_CAP_COLOR | TOOL_CAP_SIZE | TOOL_CAP_RULER | TOOL_CAP_RECTANGLE |
                                        TOOL_CAP_ELLIPSE | TOOL_CAP_ARROW | TOOL_CAP_DOUBLE_ARROW | TOOL_CAP_SPLINE |
                                        TOOL_CAP_RECOGNIZER | TOOL_CAP_FILL;
    caps[TOOL_TEXT - TOOL_PEN] = TOOL_CAP_COLOR;
    caps[TOOL_IMAGE - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_RECT - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_REGION - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_MULTILAYER_RECT - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_MULTILAYER_REGION - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_OBJECT - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_VERTICAL_SPACE - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_HAND - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_PLAY_OBJECT - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_RECT - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_ELLIPSE - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_ARROW - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_DOUBLE_ARROW - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_COORDINATE_SYSTEM - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_DRAW_SPLINE - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_FLOATING_TOOLBOX - TOOL_PEN] = TOOL_CAP_NONE;
    caps[TOOL_SELECT_PDF_TEXT_LINEAR - TOOL_PEN] = TOOL_CAP_COLOR;
    caps[TOOL_SELECT_PDF_TEXT_RECT - TOOL_PEN] = TOOL_CAP_COLOR;
    caps[TOOL_LASER_POINTER_PEN - TOOL_PEN] = TOOL_CAP_COLOR | TOOL_CAP_SIZE;
    caps[TOOL_LASER_POINTER_HIGHLIGHTER - TOOL_PEN] = TOOL_CAP_COLOR | TOOL_CAP_SIZE;

    return caps;
}

auto xoj::tool::typeToCapabilities(ToolType type) -> ToolCapabilities {
    static constexpr auto caps = makeToolCaps();
    xoj_assert(type >= TOOL_PEN && type - TOOL_PEN < TOOL_COUNT);
    return static_cast<ToolCapabilities>(caps[type - TOOL_PEN]);
}
