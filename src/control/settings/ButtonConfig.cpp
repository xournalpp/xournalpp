#include "ButtonConfig.h"

#include "control/ToolHandler.h"

ButtonConfig::ButtonConfig(ToolType action, int color, ToolSize size, DrawingType drawingType, EraserType eraserMode)
{
	XOJ_INIT_TYPE(ButtonConfig);

	this->action = action;
	this->color = color;
	this->size = size;
	this->drawingType = drawingType;
	this->eraserMode = eraserMode;
	this->disableDrawing = false;
}

ButtonConfig::~ButtonConfig()
{
	XOJ_RELEASE_TYPE(ButtonConfig);
}

bool ButtonConfig::getDisableDrawing()
{
	XOJ_CHECK_TYPE(ButtonConfig);

	return this->disableDrawing;
}

DrawingType ButtonConfig::getDrawingType()
{
	XOJ_CHECK_TYPE(ButtonConfig);

	return this->drawingType;
}

ToolType ButtonConfig::getAction()
{
	XOJ_CHECK_TYPE(ButtonConfig);

	return this->action;
}

void ButtonConfig::acceptActions(ToolHandler* toolHandler)
{
	XOJ_CHECK_TYPE(ButtonConfig);

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
