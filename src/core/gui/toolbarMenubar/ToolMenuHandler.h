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

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <vector>   // for vector

#include <glib-object.h>  // for GObject, GConnectFlags
#include <glib.h>         // for gchar
#include <gtk/gtk.h>      // for GtkWidget, GtkWindow, GtkBuilder

#include "enums/ActionGroup.enum.h"  // for GROUP_NOGROUP, ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType
#include "gui/IconNameHelper.h"      // for IconNameHelper
#include "util/Color.h"              // for Color

class AbstractToolItem;
class FontButton;
class GladeGui;
class GladeSearchpath;
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
class ActionHandler;
class ColorToolItem;
class MenuItem;
struct ToolbarButtonEntry;

class ToolMenuHandler {
public:
    ToolMenuHandler(Control* control, GladeGui* gui);
    virtual ~ToolMenuHandler();

    void populate(const GladeSearchpath* gladeSearchPath);

public:
    void freeDynamicToolbarItems();
    static void unloadToolbar(GtkWidget* toolbar);

    /**
     * @brief Load the toolbar.ini file
     * This file persists the customized toolbars and is loaded upon starting the application.
     *
     * @param d Data Object representing the selected toolbars (e.g Portrait)
     * @param toolbar reference to the widget representing the toolbar
     * @param toolbarName toolbarName which should be read from the file
     * @param horizontal whether the toolbar is horizontal
     */
    void load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal);

    void registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group = GROUP_NOGROUP);

    void initToolItems();
    void addPluginItem(ToolbarButtonEntry* t);

    void setUndoDescription(const std::string& description);
    void setRedoDescription(const std::string& description);

    SpinPageAdapter* getPageSpinner();
    void setPageInfo(size_t pagecount, size_t pdfpage = 0);

    void setFontButtonFont(const XojFont& font);
    XojFont getFontButtonFont();

    void showFontSelectionDlg();

    void setTmpDisabled(bool disabled);

    [[maybe_unused]] void removeColorToolItem(AbstractToolItem* it);
    void addColorToolItem(AbstractToolItem* it);

    ToolbarModel* getModel();

    std::vector<AbstractToolItem*>* getToolItems();
    const std::vector<ColorToolItem*>& getColorToolItems() const;

    Control* getControl();

    [[maybe_unused]] bool isColorInUse(Color color);

    void disableAudioPlaybackButtons();

    void enableAudioPlaybackButtons();

    void setAudioPlaybackPaused(bool paused);
    std::string iconName(const char* icon);

private:
    void addToolItem(AbstractToolItem* it);

    static void signalConnectCallback(GtkBuilder* builder, GObject* object, const gchar* signalName,
                                      const gchar* handlerName, GObject* connectObject, GConnectFlags flags,
                                      ToolMenuHandler* self);
    void initPenToolItem();
    void initEraserToolItem();

private:
    std::vector<ColorToolItem*> toolbarColorItems;
    GtkWindow* parent = nullptr;

    std::vector<AbstractToolItem*> toolItems;
    std::vector<MenuItem*> menuItems;

    ToolButton* undoButton = nullptr;
    ToolButton* redoButton = nullptr;

    ToolButton* audioPausePlaybackButton = nullptr;
    ToolButton* audioStopPlaybackButton = nullptr;
    ToolButton* audioSeekBackwardsButton = nullptr;
    ToolButton* audioSeekForwardsButton = nullptr;

    ToolPageSpinner* toolPageSpinner = nullptr;
    ToolPageLayer* toolPageLayer = nullptr;
    FontButton* fontButton = nullptr;

    Control* control = nullptr;
    ActionHandler* listener = nullptr;
    ZoomControl* zoom = nullptr;
    GladeGui* gui = nullptr;
    ToolHandler* toolHandler = nullptr;

    std::unique_ptr<ToolbarModel> tbModel;

    PageTypeMenu* newPageType = nullptr;
    PageBackgroundChangeController* pageBackgroundChangeController = nullptr;
    IconNameHelper iconNameHelper;
};
