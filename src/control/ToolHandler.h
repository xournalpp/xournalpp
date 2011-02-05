/*
 * Xournal++
 *
 * Handles Tools
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLHANDLER_H__
#define __TOOLHANDLER_H__

#include "Tool.h"
#include "../util/Arrayiterator.h"
#include "settings/Settings.h"

class ToolListener {
public:
	virtual void toolColorChanged() = 0;
	virtual void setCustomColorSelected() = 0;
	virtual void toolSizeChanged() = 0;
	virtual void toolChanged() = 0;
	virtual void eraserTypeChanged() = 0;
};

class ToolHandler {
public:
	ToolHandler(ToolListener * listener, Settings * settings);
	virtual ~ToolHandler();

	void setColor(int color);
	int getColor();
	GdkColor getGdkColor();

	void setRuler(bool ruler);
	bool isRuler();

	void setShapeRecognizer(bool reco);
	bool isShapeRecognizer();

	void setColorFound();

	ToolSize getSize();
	void setSize(ToolSize size);
	double getThikness();

	ToolSize getPenSize();
	ToolSize getEraserSize();
	ToolSize getHilighterSize();
	void setPenSize(ToolSize size);
	void setEraserSize(ToolSize size);
	void setHilighterSize(ToolSize size);


	void selectTool(ToolType type);
	ToolType getToolType();

	/**
	 * Do not call this Method
	 * Call Control::setEraserType
	 */
	void _setEraserType(EraserType eraserType);
	EraserType getEraserType();

	bool isEnableColor();
	bool isEnableSize();
	bool isEnableRuler();
	bool isEnableShapreRecognizer();

	void saveSettings();
	void loadSettings();

	void copyCurrentConfig();
	void restoreLastConfig();

	ArrayIterator<Tool *> iterator();

	void setSelectionEditTools(bool setColor, bool setSize);

	const double * getToolThikness(ToolType type);
protected:
	void initTools();

private:
	Tool * tools[TOOL_COUNT];
	Tool * current;
	Tool * lastSelectedTool;

	EraserType eraserType;

	bool colorFound;
	ToolListener * listener;

	Settings * settings;
};

#endif /* __TOOLHANDLER_H__ */
