/*
 * Xournal++
 *
 * Context menu layers sidebars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>
#include <memory>

#include <gtk/gtk.h>

#include "gui/sidebar/previews/base/SidebarBaseContextMenu.h"
#include "gui/sidebar/previews/base/SidebarToolbar.h"

class SidebarLayersContextMenu: public SidebarBaseContextMenu {
public:
    SidebarLayersContextMenu(GladeGui* gui, SidebarToolbar* toolbar);

    void setActionsSensitivity(SidebarActions actions) override;

private:
    GtkWidget* contextMenuMoveUp = nullptr;
    GtkWidget* contextMenuMoveDown = nullptr;
    GtkWidget* contextMenuMergeDown = nullptr;
    GtkWidget* contextMenuDuplicate = nullptr;
    GtkWidget* contextMenuDelete = nullptr;
};
