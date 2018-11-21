#include "Tool.h"

Tool::Tool(string name, ToolType type, int color, bool enableColor, bool enableSize, bool enableRuler,
		   bool enableRectangle, bool enableCircle, bool enableArrow, bool enableShapreRecognizer,
		   double* thickness)
{
	XOJ_INIT_TYPE(Tool);

	this->name = name;
	this->type = type;
	this->thickness = thickness;

	this->enableColor = enableColor;
	this->enableSize = enableSize;
	this->enableShapeRecognizer = enableShapreRecognizer;
	this->enableRuler = enableRuler;
	this->enableRectangle = enableRectangle;
	this->enableCircle = enableCircle;
	this->enableArrow = enableArrow;

	this->drawingType = DRAWING_TYPE_DEFAULT;

	this->color = color;
	this->size = TOOL_SIZE_MEDIUM;
}

Tool::~Tool()
{
	XOJ_CHECK_TYPE(Tool);

	delete[] this->thickness;
	this->thickness = NULL;

	XOJ_RELEASE_TYPE(Tool);
}

string Tool::getName()
{
	XOJ_CHECK_TYPE(Tool);

	return this->name;
}

int Tool::getColor()
{
	XOJ_CHECK_TYPE(Tool);

	return this->color;
}

void Tool::setColor(int color)
{
	XOJ_CHECK_TYPE(Tool);

	this->color = color;
}

ToolSize Tool::getSize()
{
	XOJ_CHECK_TYPE(Tool);

	return this->size;
}

void Tool::setSize(ToolSize size)
{
	XOJ_CHECK_TYPE(Tool);

	this->size = size;
}

bool Tool::isEnableColor()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableColor;
}

bool Tool::isEnableSize()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableSize;
}

bool Tool::isEnableRuler()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableRuler;
}

bool Tool::isEnableRectangle()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableRectangle;
}

bool Tool::isEnableCircle()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableCircle;
}

bool Tool::isEnableArrow()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableArrow;
}

bool Tool::isEnableShapeRecognizer()
{
	XOJ_CHECK_TYPE(Tool);

	return this->enableShapeRecognizer;
}

DrawingType Tool::getDrawingType()
{
	XOJ_CHECK_TYPE(Tool);

	return this->drawingType;
}

void Tool::setDrawingType(DrawingType drawingType)
{
	XOJ_CHECK_TYPE(Tool);

	this->drawingType = drawingType;
}

double Tool::getThickness(ToolSize size)
{
	return this->thickness[size - TOOL_SIZE_VERY_FINE];
}

string toolTypeToString(ToolType type)
{
	switch (type)
	{
	case TOOL_NONE:				return "none";
	case TOOL_PEN:				return "pen";
	case TOOL_ERASER:			return "eraser";
	case TOOL_HILIGHTER:		return "hilighter";
	case TOOL_TEXT:				return "text";
	case TOOL_IMAGE:			return "image";
	case TOOL_SELECT_RECT:		return "selectRect";
	case TOOL_SELECT_REGION:	return "selectRegion";
	case TOOL_SELECT_OBJECT:	return "selectObject";
	case TOOL_PLAY_OBJECT:		return "PlayObject";
	case TOOL_VERTICAL_SPACE:	return "verticalSpace";
	case TOOL_HAND:				return "hand";
	case TOOL_DRAW_RECT:		return "drawRect";
	case TOOL_DRAW_CIRCLE:		return "drawCircle";
	case TOOL_DRAW_ARROW:		return "drawArrow";
	default:					return "";
	}
}

ToolType toolTypeFromString(string type)
{
	if (type == "none")					return TOOL_NONE;
	else if (type == "pen")				return TOOL_PEN;
	else if (type == "eraser")			return TOOL_ERASER;
	else if (type == "hilighter")		return TOOL_HILIGHTER;
	else if (type == "image")			return TOOL_IMAGE;
	else if (type == "selectRect")		return TOOL_SELECT_RECT;
	else if (type == "selectRegion")	return TOOL_SELECT_REGION;
	else if (type == "selectObject")	return TOOL_SELECT_OBJECT;
	else if (type == "playObject")		return TOOL_PLAY_OBJECT;
	else if (type == "verticalSpace")	return TOOL_VERTICAL_SPACE;
	else if (type == "hand")			return TOOL_HAND;
	else if (type == "drawRect")		return TOOL_DRAW_RECT;
	else if (type == "drawCircle")		return TOOL_DRAW_CIRCLE;
	else if (type == "drawArrow")		return TOOL_DRAW_ARROW;
	else								return TOOL_NONE;
}

string toolSizeToString(ToolSize size)
{
	switch (size)
	{
	case TOOL_SIZE_NONE:		return "none";
	case TOOL_SIZE_VERY_FINE:	return "veryThin";
	case TOOL_SIZE_FINE:		return "thin";
	case TOOL_SIZE_MEDIUM:		return "medium";
	case TOOL_SIZE_THICK:		return "thick";
	case TOOL_SIZE_VERY_THICK:	return "veryThick";
	default:					return "";
	}
}

ToolSize toolSizeFromString(string size)
{
	if (size == "veryThin")			return TOOL_SIZE_VERY_FINE;
	else if (size == "thin")		return TOOL_SIZE_FINE;
	else if (size == "medium")		return TOOL_SIZE_MEDIUM;
	else if (size == "thick")		return TOOL_SIZE_THICK;
	else if (size == "veryThick")	return TOOL_SIZE_VERY_THICK;
	else if (size == "none")		return TOOL_SIZE_NONE;
	else							return TOOL_SIZE_NONE;
}

string eraserTypeToString(EraserType type)
{
	switch (type)
	{
	case ERASER_TYPE_NONE:			return "none";
	case ERASER_TYPE_DEFAULT:		return "default";
	case ERASER_TYPE_WHITEOUT:		return "whiteout";
	case ERASER_TYPE_DELETE_STROKE:	return "deleteStroke";
	default:						return "";
	}
}

EraserType eraserTypeFromString(string type)
{
	if (type == "none")					return ERASER_TYPE_NONE;
	else if (type == "default")			return ERASER_TYPE_DEFAULT;
	else if (type == "whiteout")		return ERASER_TYPE_WHITEOUT;
	else if (type == "deleteStroke")	return ERASER_TYPE_DELETE_STROKE;
	else								return ERASER_TYPE_NONE;
}

