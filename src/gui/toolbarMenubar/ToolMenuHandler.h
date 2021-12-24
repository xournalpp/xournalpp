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

#include <gtk/gtk.h>

#include "control/Actions.h"

#include "ColorToolItem.h"
#include "MenuItem.h"

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

class ToolMenuHandler {
public:
    ToolMenuHandler(Control* control, GladeGui* gui, GtkWindow* parent);
    virtual ~ToolMenuHandler();

public:
    void deleteAllToolbarItems();
    void freeDynamicToolbarItems();
    static void unloadToolbar(GtkWidget* toolbar);

    void load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal);

    void registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group = GROUP_NOGROUP);

    void initToolItems();  // default one
    AbstractToolItem* addToolItemsfromName(string name);
    void connectSignalsOfToolItems();  // function to call after the composition of toolItems vector

    void setUndoDescription(const string& description);
    void setRedoDescription(const string& description);

    SpinPageAdapter* getPageSpinner();
    void setPageText(const string& text);

    void setFontButtonFont(XojFont& font);
    XojFont getFontButtonFont();

    void showFontSelectionDlg();

    void setTmpDisabled(bool disabled);

    void removeColorToolItem(AbstractToolItem* it);
    void addColorToolItem(AbstractToolItem* it);

    ToolbarModel* getModel();

    vector<AbstractToolItem*>* getToolItems();

    bool isColorInUse(Color color);

    void disableAudioPlaybackButtons();

    void enableAudioPlaybackButtons();

    void setAudioPlaybackPaused(bool paused);

private:
    AbstractToolItem* addToolItem(AbstractToolItem* it);

    static void signalConnectCallback(GtkBuilder* builder, GObject* object, const gchar* signalName,
                                      const gchar* handlerName, GObject* connectObject, GConnectFlags flags,
                                      ToolMenuHandler* self);
    ToolButton* initPenToolItem();
    ToolButton* initEraserToolItem();

private:
    vector<ColorToolItem*> toolbarColorItems;
    GtkWindow* parent = nullptr;

    vector<AbstractToolItem*> toolItems;
    vector<MenuItem*> menuItems;

    vector<ToolButton*> undoButton;
    vector<ToolButton*> redoButton;

    vector<ToolButton*> audioPausePlaybackButton;
    vector<ToolButton*> audioStopPlaybackButton;
    vector<ToolButton*> audioSeekBackwardsButton;
    vector<ToolButton*> audioSeekForwardsButton;

    vector<ToolPageSpinner*> toolPageSpinner;
    vector<ToolPageLayer*> toolPageLayer;

    vector<FontButton*> fontButton;

    Control* control = nullptr;
    ActionHandler* listener = nullptr;
    ZoomControl* zoom = nullptr;
    GladeGui* gui = nullptr;
    ToolHandler* toolHandler = nullptr;

    ToolbarModel* tbModel = nullptr;

    PageTypeMenu* newPageType = nullptr;
    PageBackgroundChangeController* pageBackgroundChangeController = nullptr;
};
