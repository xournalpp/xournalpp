/*
 * Xournal++
 *
 * Handles Tools
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Tool.h"
#include "settings/Settings.h"

#include <Arrayiterator.h>
#include <XournalType.h>

class LastSelectedTool;

class ToolListener
{
public:
	virtual void toolColorChanged(bool userSelection) = 0;
	virtual void setCustomColorSelected() = 0;
	virtual void toolSizeChanged() = 0;
	virtual void toolFillChanged() = 0;
	virtual void toolLineStyleChanged() = 0;
	virtual void toolChanged() = 0;

	virtual ~ToolListener();
};

class ActionHandler;

class ToolHandler
{
public:
	ToolHandler(ToolListener* listener, ActionHandler* actionHandler, Settings* settings);
	virtual ~ToolHandler();

	/**
	 * Select the color for the tool
	 *
	 * @param color Color
	 * @param userSelection
	 * 			true if the user selected the color
	 * 			false if the color is selected by a tool change
	 * 			and therefore should not be applied to a selection
	 */
	void setColor(int color, bool userSelection);
	int getColor();

	/**
	 * @return -1 if fill is disabled, else the fill alpha value
	 */
	int getFill();

	DrawingType getDrawingType();
	void setDrawingType(DrawingType drawingType);

	const LineStyle& getLineStyle();

	void setColorFound();

	ToolSize getSize();
	void setSize(ToolSize size);
	double getThickness();

	void setLineStyle(const LineStyle& style);

	ToolSize getPenSize();
	ToolSize getEraserSize();
	ToolSize getHilighterSize();
	void setPenSize(ToolSize size);
	void setEraserSize(ToolSize size);
	void setHilighterSize(ToolSize size);

	void setPenFillEnabled(bool fill, bool fireEvent = true);
	bool getPenFillEnabled();
	void setPenFill(int alpha);
	int getPenFill();

	void setHilighterFillEnabled(bool fill, bool fireEvent = true);
	bool getHilighterFillEnabled();
	void setHilighterFill(int alpha);
	int getHilighterFill();

	void selectTool(ToolType type, bool fireToolChanged = true);
	ToolType getToolType();
	void fireToolChanged();

	Tool& getTool(ToolType type);

	void setEraserType(EraserType eraserType);
	EraserType getEraserType();
	void eraserTypeChanged();

	bool hasCapability(ToolCapabilities cap);

	void saveSettings();
	void loadSettings();

	void copyCurrentConfig();
	void restoreLastConfig();

	ArrayIterator<Tool*> iterator();

	/**
	 * Change the selection tools capabilities, depending on the selected elements
	 */
	void setSelectionEditTools(bool setColor, bool setSize, bool setFill);

	const double* getToolThickness(ToolType type);

	/**
	 * Returns whether the current tool will create an element that may only reside on a single page even when the pointer moves to another
	 * @return
	 */
	bool isSinglePageTool();

protected:
	void initTools();

private:
	Tool* tools[TOOL_COUNT] = { 0 };
	Tool* current = nullptr;

	/**
	 * Last selected tool, reference with color values etc.
	 */
	LastSelectedTool* lastSelectedTool = nullptr;

	EraserType eraserType = ERASER_TYPE_DEFAULT;

	/**
	 * If a color is selected, it may be in the list,
	 * so its a "predefined" color for us, but may it is
	 * not in the list, so its a "custom" color for us
	 */
	bool colorFound = false;

	ToolListener* listener = nullptr;

	ActionHandler* actionHandler = nullptr;

	Settings* settings = nullptr;
};
