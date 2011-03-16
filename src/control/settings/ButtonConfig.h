/*
 * Xournal++
 *
 * Configuration for Mouse Buttons, Eraser and default configuration
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BUTTONCONFIG_H__
#define __BUTTONCONFIG_H__

#include "../../util/String.h"
#include "../Tool.h"

enum DrawingType {
	DRAWING_TYPE_DONT_CHANGE, DRAWING_TYPE_RULER, DRAWING_TYPE_STROKE_RECOGNIZER, DRAWING_TYPE_NONE
};

class ToolHandler;

class ButtonConfig {
public:
	ButtonConfig(ToolType action, int color, ToolSize size, DrawingType drawingType, EraserType eraserMode);

public:
	void acceptActions(ToolHandler * toolHandler);
	ToolType getAction();
	bool getDisableDrawing();

private:
	ToolType action;
	int color;
	ToolSize size;
	EraserType eraserMode;
	DrawingType drawingType;

	bool disableDrawing;

public:
	String device;

	friend class Settings;
	friend class ButtonConfigGui;
};
#endif /* __BUTTONCONFIG_H__ */
