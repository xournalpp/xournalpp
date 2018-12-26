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
#include <XournalType.h>

class GladeGui;
class PopupMenuButton;
class LayerController;

class ToolPageLayer : public AbstractToolItem
{
public:
	ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, string id, ActionType type);
	virtual ~ToolPageLayer();

public:
	virtual string getToolDisplayName();

protected:
	GtkWidget* createSpecialMenuEntry(string name);
	void createSeparator();

	/**
	 * Add special button to the top of the menu
	 */
	void addSpecialButtonTop();

	/**
	 * Rebuild the Menu
	 */
	void updateMenu();

	virtual GtkToolItem* newItem();
	virtual GtkWidget* getNewToolIcon();

private:
	XOJ_TYPE_ATTRIB;

	LayerController* lc;
	GladeGui* gui;

	GtkWidget* layerLabel;
	GtkWidget* layerButton;
	GtkWidget* menu;

	PopupMenuButton* popupMenuButton;
	int menuY;
};
