/*
 * Xournal++
 *
 * Abstract base class for sidebar context menus
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <tuple>   // for tuple
#include <vector>  // for vector

#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget

#include "gui/sidebar/previews/base/SidebarToolbar.h"  // for SidebarActions

class SidebarBaseContextMenu {
public:
    SidebarBaseContextMenu(GtkWidget* contextMenu);
    virtual ~SidebarBaseContextMenu();

    SidebarBaseContextMenu(SidebarBaseContextMenu&&) = delete;
    SidebarBaseContextMenu(const SidebarBaseContextMenu&) = delete;
    SidebarBaseContextMenu& operator=(SidebarBaseContextMenu&&) = delete;
    SidebarBaseContextMenu& operator=(const SidebarBaseContextMenu&) = delete;

    /**
     * Sets the sensitivity of the actions in the menu according to the actions
     * specified by the passed `actions`.
     *
     * If the context menu had widgets (entries) for the actions A, B, C, D
     * and this was passed the actions value (A|C) then it would sensitize
     * (make clickable) the widgets for the actions A and C, and desensitize the
     * others.
     */
    virtual void setActionsSensitivity(SidebarActions actions) = 0;

    /**
     * Opens the context menu.
     */
    virtual void open();

protected:
    /**
     * The context menu to display on right click.
     */
    GtkWidget* const contextMenu = nullptr;

    /**
     * The data passed to the menu item callbacks.
     */
    struct ContextMenuData {
        SidebarToolbar* toolbar;
        SidebarActions actions;
    };

    /**
     * The signals connected to the context menu items. This must be kept track
     * of so the data can be deallocated safely.
     */
    std::vector<std::tuple<GtkWidget*, gulong, std::unique_ptr<ContextMenuData>>> contextMenuSignals;

private:
};
