#include "ButtonConfig.h"

#include "control/ToolHandler.h"

ButtonConfig::ButtonConfig(ToolType action, int color, ToolSize size, DrawingType drawingType, EraserType eraserMode)
{
	this->action = action;
	this->color = color;
	this->size = size;
	this->drawingType = drawingType;
	this->eraserMode = eraserMode;
	this->disableDrawing = false;
}

ButtonConfig::~ButtonConfig()
{
}

bool ButtonConfig::getDisableDrawing()
{
	return this->disableDrawing;
}

DrawingType ButtonConfig::getDrawingType()
{
	return this->drawingType;
}

ToolType ButtonConfig::getAction()
{
	return this->action;
}

void ButtonConfig::acceptActions(ToolHandler* toolHandler)
{
	if (this->action == TOOL_NONE)
	{
		return;
	}

	toolHandler->selectTool(this->action, false);

	if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_ERASER)
	{

		if (this->drawingType != DRAWING_TYPE_DONT_CHANGE)
		{
			toolHandler->setDrawingType(this->drawingType);
		}

		if (this->size != TOOL_SIZE_NONE)
		{
			toolHandler->setSize(this->size);
		}
	}

	if (this->action == TOOL_PEN || this->action == TOOL_HILIGHTER || this->action == TOOL_TEXT)
	{
		toolHandler->setColor(this->color, false);
	}

	if (this->action == TOOL_ERASER && this->eraserMode != ERASER_TYPE_NONE)
	{
		toolHandler->setEraserType(this->eraserMode);
	}

	toolHandler->fireToolChanged();
}
