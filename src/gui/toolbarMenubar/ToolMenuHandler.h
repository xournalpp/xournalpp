/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef EDITABLETOOLBAR_H_
#define EDITABLETOOLBAR_H_

#include <gtk/gtk.h>

#include "../../control/Actions.h"
#include <StringUtils.h>
#include <ListIterator.h>


class AbstractToolItem;
class ToolButton;
class ToolPageSpinner;
class ToolPageLayer;
class FontButton;
class ToolbarData;
class ZoomControl;
class ToolHandler;
class XojFont;
class GladeGui;
class ToolbarModel;
class SpinPageAdapter;

class ToolMenuHandler
{
public:
	ToolMenuHandler(ActionHandler* listener, ZoomControl* zoom, GladeGui* gui,
					ToolHandler* toolHandler, GtkWindow* parent);
	virtual ~ToolMenuHandler();

public:
	void freeDynamicToolbarItems();
	void unloadToolbar(GtkWidget* tBunload);

	void load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName,
			  bool horizontal);

	void registerMenupoint(GtkWidget* widget, ActionType type);
	void registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group);

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

	ListIterator<AbstractToolItem*> getToolItems();

	bool isColorInUse(int color);

private:
	void addToolItem(AbstractToolItem* it);

	void initEraserToolItem();

private:
	XOJ_TYPE_ATTRIB;

	GList* toolbarColorItems;
	GtkWindow* parent;

	GList* toolItems;
	GList* menuItems;

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
};

#endif /* EDITABLETOOLBAR_H_ */
