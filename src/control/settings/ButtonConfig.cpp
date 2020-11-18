#include "ButtonConfig.h"

#include "control/ToolHandler.h"

ButtonConfig::ButtonConfig(ToolType action, Color color, ToolSize size, DrawingType drawingType,
                           EraserType eraserMode) {
    this->action = action;
    this->color = color;
    this->size = size;
    this->drawingType = drawingType;
    this->eraserMode = eraserMode;
    this->disableDrawing = false;
}

ButtonConfig::~ButtonConfig() = default;

auto ButtonConfig::getDisableDrawing() const -> bool { return this->disableDrawing; }

auto ButtonConfig::getDrawingType() -> DrawingType { return this->drawingType; }

auto ButtonConfig::getAction() -> ToolType { return this->action; }

void ButtonConfig::initButton(ToolHandler* toolHandler, Buttons button) {
    if (this->action == TOOL_NONE) {
        return;
    }

    toolHandler->initButtonTool(button, this->action);

    if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER) {

        if (this->drawingType != DRAWING_TYPE_DONT_CHANGE) {
            toolHandler->setButtonDrawingType(this->drawingType, button);
        }

        if (this->size != TOOL_SIZE_NONE) {
            toolHandler->setButtonSize(this->size, button);
        }
        toolHandler->setButtonColor(this->color, button);
    }
    if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE) {
        toolHandler->setButtonEraserType(this->eraserMode, button);
    }
}

void ButtonConfig::initActions(ToolHandler* toolHandler) {
    if (this->action == TOOL_NONE) {
        return;
    }
    toolHandler->selectTool(this->action, false);

    if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER) {

        if (this->drawingType != DRAWING_TYPE_DONT_CHANGE) {
            toolHandler->setDrawingType(this->drawingType);
        }

        if (this->size != TOOL_SIZE_NONE) {
            toolHandler->setSize(this->size);
        }
        toolHandler->setColor(this->color, false);
    }
    if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE) {
        toolHandler->setEraserType(this->eraserMode);
    }
}

bool ButtonConfig::acceptActions(ToolHandler* toolHandler, Buttons button) {
    if (this->action == TOOL_NONE) {
        return false;
    }
    if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER) {

        if (this->drawingType == DRAWING_TYPE_DONT_CHANGE) {
            toolHandler->setButtonDrawingType(toolHandler->getDrawingType(SelectedTool::toolbar), button);
        }

        if (this->size == TOOL_SIZE_NONE) {
            toolHandler->setButtonSize(toolHandler->getSize(SelectedTool::toolbar), button);
        }
    }
    if (this->action == TOOL_ERASER && this->eraserMode == ERASER_TYPE_NONE) {
        toolHandler->setButtonEraserType(toolHandler->getEraserType(SelectedTool::toolbar), button);
    }
    return true;
}
