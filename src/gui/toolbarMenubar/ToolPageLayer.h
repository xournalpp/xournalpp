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
#include "control/layer/LayerCtrlListener.h"

#include <map>
using std::map;

class GladeGui;
class PopupMenuButton;
class LayerController;

class ToolPageLayer : public AbstractToolItem, public LayerCtrlListener
{
public:
	ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, string id, ActionType type);
	virtual ~ToolPageLayer();

public:
	virtual string getToolDisplayName();

	// LayerCtrlListener
public:
	virtual void rebuildLayerMenu();
	virtual void layerVisibilityChanged();

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

	/**
	 * Update selected layer, update visible layer
	 */
	void updateLayerData();

	virtual GtkToolItem* newItem();
	virtual GtkWidget* getNewToolIcon();

private:
	void createLayerMenuItem(string text, int layerId);
	void layerMenuClicked(GtkWidget* menu);
	void createLayerMenuItemShow(int layerId);
	void layerMenuShowClicked(GtkWidget* menu);

	void selectLayer(int layerId);

private:
	LayerController* lc = nullptr;
	GladeGui* gui = nullptr;

	GtkWidget* layerLabel = nullptr;
	GtkWidget* layerButton = nullptr;
	GtkWidget* menu = nullptr;

	map<int, GtkWidget*> layerItems;
	map<int, GtkWidget*> showLayerItems;

	PopupMenuButton* popupMenuButton = nullptr;
	int menuY = 0;

	/**
	 * Menu is currently updating - ignore events
	 */
	bool inMenuUpdate = false;
};
