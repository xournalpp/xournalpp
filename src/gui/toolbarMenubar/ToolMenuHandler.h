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

#include "ColorToolItem.h"
#include "MenuItem.h"
#include "control/Actions.h"

#include <gtk/gtk.h>

class AbstractToolItem;
typedef std::vector<AbstractToolItem*> AbstractToolItemVector;

class FontButton;
class GladeGui;
class ToolbarData;
class ToolbarModel;
class ToolButton;
class ToolHandler;
class ToolPageLayer;
class ToolPageSpinner;
class PageTypeMenu;
class SpinPageAdapter;
class XojFont;
class ZoomControl;
class Control;
class PageBackgroundChangeController;

class ToolMenuHandler
{
public:
	ToolMenuHandler(Control* control, GladeGui* gui, GtkWindow* parent);
	virtual ~ToolMenuHandler();

public:
	void freeDynamicToolbarItems();
	void unloadToolbar(GtkWidget* tBunload);

	void load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal);

	void registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group = GROUP_NOGROUP);

	void initToolItems();

	void setUndoDescription(string description);
	void setRedoDescription(string description);

	SpinPageAdapter* getPageSpinner();
	void setPageText(string text);

	int getSelectedLayer();
	void setLayerCount(int count, int selected);

	void setFontButtonFont(XojFont& font);
	XojFont getFontButtonFont();

	void showFontSelectionDlg();

	void setTmpDisabled(bool disabled);

	void removeColorToolItem(AbstractToolItem* it);
	void addColorToolItem(AbstractToolItem* it);

	ToolbarModel* getModel();

	AbstractToolItemVector* getToolItems();

	bool isColorInUse(int color);

private:
	void addToolItem(AbstractToolItem* it);

	void initEraserToolItem();

private:
	XOJ_TYPE_ATTRIB;

	std::vector<ColorToolItem*> toolbarColorItems;
	GtkWindow* parent;

	AbstractToolItemVector toolItems;
	std::vector<MenuItem*> menuItems;

	ToolButton* undoButton;
	ToolButton* redoButton;

	ToolPageSpinner* toolPageSpinner;
	ToolPageLayer* toolPageLayer;
	FontButton* fontButton;

	ActionHandler* listener;
	ZoomControl* zoom;
	GladeGui* gui;
	ToolHandler* toolHandler;

	ToolbarModel* tbModel;

	PageTypeMenu* newPageType;
	PageBackgroundChangeController* pageBackgroundChangeController;
};
