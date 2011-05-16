/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef EDITABLETOOLBAR_H_
#define EDITABLETOOLBAR_H_

#include <gtk/gtk.h>

#include "../../control/Actions.h"
#include <String.h>
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

class ToolMenuHandler {
public:
	ToolMenuHandler(ActionHandler * listener, ZoomControl * zoom, GladeGui * gui, ToolHandler * toolHandler);
	virtual ~ToolMenuHandler();

public:
	void freeDynamicToolbarItems();
	void unloadToolbar(GtkWidget * tBunload);

	void load(ToolbarData * d, GtkWidget * toolbar, const char * toolbarName, bool horizontal);

	void registerMenupoint(GtkWidget * widget, ActionType type);
	void registerMenupoint(GtkWidget * widget, ActionType type, ActionGroup group);

	void initToolItems();

	void setUndoDescription(String description);
	void setRedoDescription(String description);

	GtkWidget * getPageSpinner();
	void setPageText(String text);

	int getSelectedLayer();
	void setLayerCount(int count, int selected);

	void setFontButtonFont(XojFont & font);
	XojFont getFontButtonFont();


	void setTmpDisabled(bool disabled);

	ToolbarModel * getModel();

	ListIterator<AbstractToolItem *> getToolItems();

	void startEditMode();
	void endEditMode();
private:
	void addToolItem(AbstractToolItem * it);

	void initEraserToolItem();

private:
	XOJ_TYPE_ATTRIB;

	GList * toolbarColorItems;

	GList * toolItems;
	GList * menuItems;

	ToolButton * undoButton;
	ToolButton * redoButton;

	ToolPageSpinner * toolPageSpinner;
	ToolPageLayer * toolPageLayer;
	FontButton * fontButton;

	ActionHandler * listener;
	ZoomControl * zoom;
	GladeGui * gui;
	ToolHandler * toolHandler;

	ToolbarModel * tbModel;
};

#endif /* EDITABLETOOLBAR_H_ */
