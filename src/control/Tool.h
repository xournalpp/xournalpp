/*
 * Xournal++
 *
 * A tool
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOL_H__
#define __TOOL_H__

#include <String.h>
#include <XournalType.h>

#define TOOL_COUNT 10

// Has to be in the same order as in Action.h: ActionType!
enum ToolType {
	TOOL_NONE = 0,
	TOOL_PEN = 1,
	TOOL_ERASER,
	TOOL_HILIGHTER,
	TOOL_TEXT,
	TOOL_IMAGE,
	TOOL_SELECT_RECT,
	TOOL_SELECT_REGION,
	TOOL_SELECT_OBJECT,
	TOOL_VERTICAL_SPACE,
	TOOL_HAND
};

String toolTypeToString(ToolType type);
ToolType toolTypeFromString(String type);

enum ToolSize {
	TOOL_SIZE_VERY_FINE = 0,
	TOOL_SIZE_FINE,
	TOOL_SIZE_MEDIUM,
	TOOL_SIZE_THICK,
	TOOL_SIZE_VERY_THICK,
	// None has to be at the end, because this enum is used as memory offset
	TOOL_SIZE_NONE
};

String toolSizeToString(ToolSize size);
ToolSize toolSizeFromString(String size);

enum EraserType {
	ERASER_TYPE_NONE = 0,
	ERASER_TYPE_DEFAULT,
	ERASER_TYPE_WHITEOUT,
	ERASER_TYPE_DELETE_STROKE
};

String eraserTypeToString(EraserType type);
EraserType eraserTypeFromString(String type);

enum PageInsertType {
	PAGE_INSERT_TYPE_PLAIN = 1,
	PAGE_INSERT_TYPE_LINED,
	PAGE_INSERT_TYPE_RULED,
	PAGE_INSERT_TYPE_GRAPH,
	PAGE_INSERT_TYPE_COPY,
	PAGE_INSERT_TYPE_PDF_BACKGROUND
};

String pageInsertTypeToString(PageInsertType type);
PageInsertType pageInsertTypeFromString(String type);

class Tool {
public:
	Tool(String name, ToolType tool, int color, bool enableColor, bool enableSize, bool enableRuler, bool enableShapreRecognizer, double * thikness);
	virtual ~Tool();

	String getName();

	int getColor();
	void setColor(int color);

	ToolSize getSize();
	void setSize(ToolSize size);

	bool isShapeRecognizer();
	void setShapeRecognizer(bool enabled);
	bool isRuler();
	void setRuler(bool enabled);

	bool isEnableColor();
	bool isEnableSize();
	bool isEnableRuler();
	bool isEnableShapeRecognizer();

private:
	Tool(const Tool & t);
	void operator = (const Tool & t);

private:
	XOJ_TYPE_ATTRIB;

	String name;
	ToolType type;

	int color;
	ToolSize size;
	double * thikness;

	bool shapeRecognizer;
	bool ruler;

	bool enableColor;
	bool enableSize;
	bool enableRuler;
	bool enableShapeRecognizer;

	friend class ToolHandler;
};

#endif /* __TOOL_H__ */
