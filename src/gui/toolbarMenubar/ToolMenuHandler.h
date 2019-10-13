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

	void setFontButtonFont(XojFont& font);
	XojFont getFontButtonFont();

	void showFontSelectionDlg();

	void setTmpDisabled(bool disabled);

	void removeColorToolItem(AbstractToolItem* it);
	void addColorToolItem(AbstractToolItem* it);

	ToolbarModel* getModel();

	vector<AbstractToolItem*>* getToolItems();

	bool isColorInUse(int color);

	void disableAudioPlaybackButtons();

	void enableAudioPlaybackButtons();

	void setAudioPlaybackPaused(bool paused);

private:
	void addToolItem(AbstractToolItem* it);

	static void signalConnectCallback(GtkBuilder* builder, GObject* object, const gchar* signalName,
				const gchar* handlerName, GObject* connectObject, GConnectFlags flags, ToolMenuHandler* self);
	void initPenToolItem();
	void initEraserToolItem();

private:
	vector<ColorToolItem*> toolbarColorItems;
	GtkWindow* parent = NULL;

	vector<AbstractToolItem*> toolItems;
	vector<MenuItem*> menuItems;

	ToolButton* undoButton = NULL;
	ToolButton* redoButton = NULL;

	ToolButton* audioPausePlaybackButton = NULL;
	ToolButton* audioStopPlaybackButton = NULL;

	ToolPageSpinner* toolPageSpinner = NULL;
	ToolPageLayer* toolPageLayer = NULL;
	FontButton* fontButton = NULL;

	Control* control = NULL;
	ActionHandler* listener = NULL;
	ZoomControl* zoom = NULL;
	GladeGui* gui = NULL;
	ToolHandler* toolHandler = NULL;

	ToolbarModel* tbModel = NULL;

	PageTypeMenu* newPageType = NULL;
	PageBackgroundChangeController* pageBackgroundChangeController = NULL;
};
