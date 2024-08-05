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
#include <utility>
#include <vector>  // for vector

#include <gio/gio.h>
#include <glib-object.h>  // for GObject, GConnectFlags
#include <glib.h>         // for gchar
#include <gtk/gtk.h>      // for GtkWidget, GtkWindow, GtkBuilder

#include "gui/IconNameHelper.h"  // for IconNameHelper
#include "util/raii/GObjectSPtr.h"

#include "ToolbarSide.h"

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
struct Palette;
class StylePopoverFactory;
class ToolbarBox;

static constexpr auto TOOLITEM_ID_PROPERTY = "xopp-toolitem-id";

class ToolMenuHandler {
public:
    ToolMenuHandler(Control* control, GladeGui* gui);
    virtual ~ToolMenuHandler();

    void populate(const GladeSearchpath* gladeSearchPath);

public:
    void freeDynamicToolbarItems();
    static void unloadToolbar(ToolbarBox* toolbar);

    /**
     * @brief Load the toolbar.ini file
     * This file persists the customized toolbars and is loaded upon starting the application.
     *
     * @param d Data Object representing the selected toolbars (e.g Portrait)
     * @param toolbar reference to the toolbar
     */
    void load(const ToolbarData* d, ToolbarBox& toolbar);

    /**
     * @brief Update all ColorToolItems based on palette
     *
     * @param palette
     */
    void updateColorToolItems(const Palette& palette);

    void initToolItems();
    void addPluginItem(ToolbarButtonEntry* t);

    void setPageInfo(size_t currentPage, size_t pageCount, size_t pdfpage);

    [[maybe_unused]] void removeColorToolItem(AbstractToolItem* it);
    void addColorToolItem(std::unique_ptr<ColorToolItem> it);

    ToolbarModel* getModel();

    const std::vector<std::unique_ptr<AbstractToolItem>>& getToolItems() const;
    const std::vector<std::unique_ptr<ColorToolItem>>& getColorToolItems() const;

    /// @return .first is the toolbar widget, .second is its proxy for the overflow menu
    std::pair<xoj::util::WidgetSPtr, xoj::util::WidgetSPtr> createItem(const char* id, ToolbarSide side) const;

    xoj::util::WidgetSPtr createIcon(const char* id) const;

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
