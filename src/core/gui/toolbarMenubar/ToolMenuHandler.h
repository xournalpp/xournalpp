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

#include <cstddef>   // for size_t
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string
#include <vector>    // for vector

#include <gio/gio.h>
#include <glib-object.h>  // for GObject, GConnectFlags
#include <glib.h>         // for gchar
#include <gtk/gtk.h>      // for GtkWidget, GtkWindow, GtkBuilder

#include "gui/IconNameHelper.h"  // for IconNameHelper
#include "util/raii/GObjectSPtr.h"

class AbstractToolItem;
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
class ZoomControl;
class Control;
class PageBackgroundChangeController;
class ColorToolItem;
struct ToolbarButtonEntry;
class PageTypeSelectionPopover;
class PageType;
class StylePopoverFactory;

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

    void initToolItems();
    void addPluginItem(ToolbarButtonEntry* t);

    void setPageInfo(size_t currentPage, size_t pageCount, size_t pdfpage);

    [[maybe_unused]] void removeColorToolItem(AbstractToolItem* it);
    void addColorToolItem(std::unique_ptr<ColorToolItem> it);

    ToolbarModel* getModel();

    const std::vector<std::unique_ptr<AbstractToolItem>>& getToolItems() const;
    const std::vector<std::unique_ptr<ColorToolItem>>& getColorToolItems() const;

    Control* getControl();

    void hideAudioMenuItems();

    std::string iconName(const char* icon);

    void setDefaultNewPageType(const std::optional<PageType>& pt);

private:
    template <class tool_item, class... Args>
    tool_item& emplaceItem(Args&&... args);

    void initPenToolItem();
    void initEraserToolItem();

private:
    std::vector<std::unique_ptr<ColorToolItem>> toolbarColorItems;
    GtkWindow* parent = nullptr;

    std::vector<std::unique_ptr<AbstractToolItem>> toolItems;

    ToolPageSpinner* toolPageSpinner = nullptr;

    Control* control = nullptr;
    ZoomControl* zoom = nullptr;
    GladeGui* gui = nullptr;
    ToolHandler* toolHandler = nullptr;

    std::unique_ptr<ToolbarModel> tbModel;

    PageTypeMenu* newPageType = nullptr;
    PageBackgroundChangeController* pageBackgroundChangeController = nullptr;
    IconNameHelper iconNameHelper;

    std::unique_ptr<PageTypeSelectionPopover> pageTypeSelectionPopup;
    std::unique_ptr<StylePopoverFactory> penLineStylePopover;
    std::unique_ptr<StylePopoverFactory> eraserTypePopover;

    xoj::util::GObjectSPtr<GSimpleAction> gAction;
};
