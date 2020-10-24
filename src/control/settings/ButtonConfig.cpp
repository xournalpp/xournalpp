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

void ButtonConfig::initActions(ToolHandler* toolHandler, ToolPointer toolpointer) {
    if (this->action == TOOL_NONE) {
        return;
    }

    bool button = !(toolpointer == ToolPointer::toolbar || toolpointer == ToolPointer::current);

    if (button)
        toolHandler->initButtonTool(toolpointer, this->action);
    else
        toolHandler->selectTool(this->action, false, toolpointer);

    if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER) {

        if (this->drawingType != DRAWING_TYPE_DONT_CHANGE) {
            toolHandler->setDrawingType(this->drawingType, toolpointer);
        }

        if (this->size != TOOL_SIZE_NONE) {
            toolHandler->setSize(this->size, toolpointer);
        }
        toolHandler->setColor(this->color, false, toolpointer);
    }
    if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE) {
        toolHandler->setEraserType(this->eraserMode);
    }
}

void ButtonConfig::acceptActions(ToolHandler* toolHandler, ToolPointer toolpointer) {
    if (this->action == TOOL_NONE) {
        return;
    }

    bool button = !(toolpointer == ToolPointer::toolbar || toolpointer == ToolPointer::current);

    if (button) {
        if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER) {

            if (this->drawingType == DRAWING_TYPE_DONT_CHANGE) {
                toolHandler->setDrawingType(toolHandler->getDrawingType(ToolPointer::toolbar), toolpointer);
            }

            if (this->size == TOOL_SIZE_NONE) {
                toolHandler->setSize(toolHandler->getSize(ToolPointer::toolbar), toolpointer);
            }
        }
        toolHandler->pointCurrentToolToButtonTool(toolpointer);
    }
    toolHandler->fireToolChanged();
}
