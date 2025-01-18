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

auto ButtonConfig::getDrawingType() -> DrawingType { return this->drawingType; }

auto ButtonConfig::getAction() -> ToolType { return this->action; }

void ButtonConfig::initButton(ToolHandler* toolHandler, Button button) {
    if (this->action == TOOL_NONE) {
        return;
    }

    toolHandler->resetButtonTool(this->action, button);

    if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER || this->action == TOOL_ERASER) {
        if (this->size != TOOL_SIZE_NONE) {
            toolHandler->setButtonSize(this->size, button);
        }
        if (this->action == TOOL_PEN)
            if (this->strokeType != STROKE_TYPE_NONE)
                toolHandler->setButtonStrokeType(this->strokeType, button);
        if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) {
            toolHandler->setButtonColor(this->color, button);
            if (this->drawingType != DRAWING_TYPE_DONT_CHANGE)
                toolHandler->setButtonDrawingType(this->drawingType, button);
        }
        if (this->action == TOOL_ERASER) {
            if (this->eraserMode != ERASER_TYPE_NONE)
                toolHandler->setButtonEraserType(this->eraserMode, button);
        }
    }
}

void ButtonConfig::applyConfigToToolbarTool(ToolHandler* toolHandler) {
    if (this->action == TOOL_NONE) {
        return;
    }
    toolHandler->selectTool(this->action);

    if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER || this->action == TOOL_ERASER) {
        if (this->size != TOOL_SIZE_NONE) {
            toolHandler->setSize(this->size);
        }
        if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) {
            toolHandler->setColor(this->color, false);
            if (this->drawingType != DRAWING_TYPE_DONT_CHANGE)
                toolHandler->setDrawingType(this->drawingType);
        }
        if (this->action == TOOL_ERASER) {
            if (this->eraserMode != ERASER_TYPE_NONE)
                toolHandler->setEraserType(this->eraserMode);
        }
        if (this->action == TOOL_PEN && this->strokeType != STROKE_TYPE_NONE) {
            toolHandler->setLineStyle(strokeTypeToLineStyle(this->strokeType));
        }
    }
}

auto ButtonConfig::applyNoChangeSettings(ToolHandler* toolHandler, Button button) -> bool {
    if (this->action == TOOL_NONE) {
        return false;
    }
    if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER || this->action == TOOL_ERASER) {
        Tool const& correspondingTool = toolHandler->getTool(this->action);

        if (this->size == TOOL_SIZE_NONE) {
            toolHandler->setButtonSize(correspondingTool.getSize(), button);
        }
        if (this->action == TOOL_PEN || this->action == TOOL_HIGHLIGHTER) {
            if (this->drawingType == DRAWING_TYPE_DONT_CHANGE)
                toolHandler->setButtonDrawingType(correspondingTool.getDrawingType(), button);
        }
        if (this->action == TOOL_ERASER) {
            if (this->eraserMode == ERASER_TYPE_NONE)
                toolHandler->setButtonEraserType(correspondingTool.getEraserType(), button);
        }
        if (this->action == TOOL_PEN && this->strokeType == STROKE_TYPE_NONE)
            toolHandler->setButtonStrokeType(correspondingTool.getLineStyle(), button);
    }

    return true;
}
