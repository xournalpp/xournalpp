/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractToolItem.h"
#include "control/ToolHandler.h"

#include <XournalType.h>

class ColorSelectImage;

class ColorToolItem : public AbstractToolItem
{
public:
	ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, int color,
			bool selektor = false);
	virtual ~ColorToolItem();

public:
	virtual void actionSelected(ActionGroup group, ActionType action);
	void enableColor(int color);
	bool colorEqualsMoreOreLess(int color);
	virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);

	virtual string getToolDisplayName();
	virtual GtkWidget* getNewToolIcon();

	virtual string getId();

	int getColor();

	/**
	 * Enable / Disable the tool item
	 */
	virtual void enable(bool enabled);

protected:
	virtual GtkToolItem* newItem();
	void updateName();
	bool isSelector();

	/**
	 * Free the allocated icons
	 */
	void freeIcons();

	/**
	 * Show colochooser to select a custom color
	 */
	void showColorchooser();

private:
	/**
	 * Color
	 */
	int color;

	/**
	 * Name of the Color
	 */
	string name;

	/**
	 * Icon to display
	 */
	ColorSelectImage* icon = nullptr;

	/**
	 * Switch to pen if the color icon is pressed
	 */
	bool switchToPen = false;

	GtkWindow* parent = nullptr;
	ToolHandler* toolHandler = nullptr;

	static bool inUpdate;
};
