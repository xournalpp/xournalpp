#include "ButtonConfig.h"

#include "control/Tool.h"         // for Tool
#include "control/ToolHandler.h"  // for ToolHandler

ButtonConfig::ButtonConfig(ToolType action, Color color, ToolSize size, DrawingType drawingType, EraserType eraserMode,
                           StrokeType strokeType):
        action(action),
        color(color),
        size(size),
        eraserMode(eraserMode),
        drawingType(drawingType),
        strokeType(strokeType),
        disableDrawing(false) {}

ButtonConfig::~ButtonConfig() = default;

auto ButtonConfig::getDisableDrawing() const -> bool { return this->disableDrawing; }

auto ButtonConfig::getDrawingType() const -> DrawingType { return this->drawingType; }

auto ButtonConfig::getAction() const -> ToolType { return this->action; }

void ButtonConfig::initButton(ToolHandler* toolHandler, Button button) const {
    if (this->action == TOOL_NONE) {
        return;
    }

    toolHandler->resetButtonTool(this->action, button);
    const Tool& t = toolHandler->getTool(this->action);

    if (t.hasCapability(TOOL_CAP_SIZE) && this->size != TOOL_SIZE_NONE) {
        toolHandler->setButtonSize(this->size, button);
    }
    if (t.hasCapability(TOOL_CAP_COLOR)) {
        toolHandler->setButtonColor(this->color, button);
    }
    if (t.hasCapability(TOOL_CAP_LINE_STYLE) && this->strokeType != STROKE_TYPE_NONE) {
        toolHandler->setButtonStrokeType(this->strokeType, button);
    }

    // No TOOL_CAP_BLA for that
    if ((this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) &&
        this->drawingType != DRAWING_TYPE_DONT_CHANGE) {
        toolHandler->setButtonDrawingType(this->drawingType, button);
    }
    if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE) {
        toolHandler->setButtonEraserType(this->eraserMode, button);
    }
}

void ButtonConfig::applyConfigToToolbarTool(ToolHandler* toolHandler) const {
    if (this->action == TOOL_NONE) {
        return;
    }
    toolHandler->selectTool(this->action);
    const Tool* t = toolHandler->getActiveTool();

    if (t->hasCapability(TOOL_CAP_SIZE) && this->size != TOOL_SIZE_NONE) {
        toolHandler->setSize(this->size);
    }
    if (t->hasCapability(TOOL_CAP_COLOR)) {
        toolHandler->setColor(this->color, false);
    }
    if (t->hasCapability(TOOL_CAP_LINE_STYLE) && this->strokeType != STROKE_TYPE_NONE) {
        toolHandler->setLineStyle(strokeTypeToLineStyle(this->strokeType));
    }

    // No TOOL_CAP_BLA for that
    if ((this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) &&
        this->drawingType != DRAWING_TYPE_DONT_CHANGE) {
        toolHandler->setDrawingType(this->drawingType);
    }
    if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE) {
        toolHandler->setEraserType(this->eraserMode);
    }
}

auto ButtonConfig::applyNoChangeSettings(ToolHandler* toolHandler, Button button) const -> bool {
    if (this->action == TOOL_NONE) {
        return false;
    }
    Tool const& t = toolHandler->getTool(this->action);

    if (t.hasCapability(TOOL_CAP_SIZE) && this->size == TOOL_SIZE_NONE) {
        toolHandler->setButtonSize(t.getSize(), button);
    }
    if (t.hasCapability(TOOL_CAP_LINE_STYLE) && this->strokeType == STROKE_TYPE_NONE) {
        toolHandler->setButtonStrokeType(t.getLineStyle(), button);
    }

    // No TOOL_CAP_BLA for that
    if ((this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) &&
        this->drawingType == DRAWING_TYPE_DONT_CHANGE) {
        toolHandler->setButtonDrawingType(t.getDrawingType(), button);
    }
    if (this->action == TOOL_ERASER && this->eraserMode == ERASER_TYPE_NONE) {
        toolHandler->setButtonEraserType(t.getEraserType(), button);
    }

    return true;
}
