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
#include <Arrayiterator.h>
#include <XournalType.h>
#include "settings/Settings.h"

class ToolListener {
public:
	virtual void toolColorChanged() = 0;
	virtual void setCustomColorSelected() = 0;
	virtual void toolSizeChanged() = 0;
	virtual void toolChanged() = 0;
};

class ActionHandler;

class ToolHandler {
public:
	ToolHandler(ToolListener * listener, ActionHandler * actionHandler, Settings * settings);
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
	double getThickness();

	ToolSize getPenSize();
	ToolSize getEraserSize();
	ToolSize getHilighterSize();
	void setPenSize(ToolSize size);
	void setEraserSize(ToolSize size);
	void setHilighterSize(ToolSize size);


	void selectTool(ToolType type, bool fireToolChanged = true);
	ToolType getToolType();
	void fireToolChanged();

	void setEraserType(EraserType eraserType);
	EraserType getEraserType();
	void eraserTypeChanged();

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

	const double * getToolThickness(ToolType type);

protected:
	void initTools();

private:
	XOJ_TYPE_ATTRIB;

	Tool * tools[TOOL_COUNT];
	Tool * current;
	Tool * lastSelectedTool;

	EraserType eraserType;

	/**
	 * If a color is selected, it may be in the list,
	 * so its a "predefined" color for us, but may it is
	 * not in the list, so its a "custom" color for us
	 */
	bool colorFound;
	ToolListener * listener;
	ActionHandler * actionHandler;

	Settings * settings;
};

#endif /* __TOOLHANDLER_H__ */
