/*
 * Xournal++
 *
 * A tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <StringUtils.h>
#include <XournalType.h>

// Has to be in the same order as in Action.h: ActionType!
enum ToolType
{
	TOOL_NONE = 0,

	// First valid tool, often used starting Index 0
	TOOL_PEN			= 1,
	TOOL_ERASER			= 2,
	TOOL_HILIGHTER		= 3,
	TOOL_TEXT			= 4,
	TOOL_IMAGE			= 5,
	TOOL_SELECT_RECT	= 6,
	TOOL_SELECT_REGION	= 7,
	TOOL_SELECT_OBJECT	= 8,
	TOOL_PLAY_OBJECT	= 9,
	TOOL_VERTICAL_SPACE = 10,
	TOOL_HAND			= 11,
	TOOL_DRAW_RECT		= 12,
	TOOL_DRAW_CIRCLE	= 13,
	TOOL_DRAW_ARROW		= 14,

	TOOL_END_ENTRY
};

// The count of tools
#define TOOL_COUNT (TOOL_END_ENTRY - 1)


string toolTypeToString(ToolType type);
ToolType toolTypeFromString(string type);

enum ToolSize
{
	TOOL_SIZE_VERY_FINE = 0,
	TOOL_SIZE_FINE,
	TOOL_SIZE_MEDIUM,
	TOOL_SIZE_THICK,
	TOOL_SIZE_VERY_THICK,
	// None has to be at the end, because this enum is used as memory offset
	TOOL_SIZE_NONE
};

string toolSizeToString(ToolSize size);
ToolSize toolSizeFromString(string size);

enum EraserType
{
	ERASER_TYPE_NONE = 0,
	ERASER_TYPE_DEFAULT,
	ERASER_TYPE_WHITEOUT,
	ERASER_TYPE_DELETE_STROKE
};

string eraserTypeToString(EraserType type);
EraserType eraserTypeFromString(string type);

enum PageInsertType
{
	PAGE_INSERT_TYPE_PLAIN = 1,
	PAGE_INSERT_TYPE_LINED,
	PAGE_INSERT_TYPE_RULED,
	PAGE_INSERT_TYPE_GRAPH,
	PAGE_INSERT_TYPE_COPY,
	PAGE_INSERT_TYPE_PDF_BACKGROUND
};

enum DrawingType
{
	/**
	 * For config entry, don't change value
	 */
	DRAWING_TYPE_DONT_CHANGE = 0,

	/**
	 * Default drawing, nothing special
	 */
	DRAWING_TYPE_DEFAULT,
	DRAWING_TYPE_RULER,
	DRAWING_TYPE_RECTANGLE,
	DRAWING_TYPE_CIRCLE,
	DRAWING_TYPE_ARROW,
	DRAWING_TYPE_STROKE_RECOGNIZER
};
string drawingTypeToString(DrawingType type);
DrawingType drawingTypeFromString(string type);

string pageInsertTypeToString(PageInsertType type);
PageInsertType pageInsertTypeFromString(string type);

class Tool
{
public:
	Tool(string name, ToolType tool, int color, bool enableColor, bool enableSize,
		 bool enableRuler, bool enableRectangle, bool enableCircle, bool enableArrow,
		 bool enableShapreRecognizer, double* thickness);
	virtual ~Tool();

	string getName();

	int getColor();
	void setColor(int color);

	ToolSize getSize();
	void setSize(ToolSize size);

	DrawingType getDrawingType();
	void setDrawingType(DrawingType drawingType);

	bool isEnableColor();
	bool isEnableSize();
	bool isEnableRuler();
	bool isEnableRectangle();
	bool isEnableCircle();
	bool isEnableArrow();
	bool isEnableShapeRecognizer();

	double getThickness(ToolSize size);

private:
	Tool(const Tool& t);
	void operator=(const Tool& t);

private:
	XOJ_TYPE_ATTRIB;

	string name;
	ToolType type;

	int color;
	ToolSize size;
	double* thickness;

	DrawingType drawingType;

	bool enableColor;
	bool enableSize;
	bool enableRuler;
	bool enableRectangle;
	bool enableCircle;
	bool enableArrow;
	bool enableShapeRecognizer;

	friend class ToolHandler;
};
