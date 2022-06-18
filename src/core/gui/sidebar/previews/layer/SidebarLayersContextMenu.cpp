#include "gui/sidebar/previews/layer/SidebarLayersContextMenu.h"

#include <map>      // for map, operator!=, _Rb_tree_const_iterator
#include <memory>   // for unique_ptr, allocator, make_unique
#include <string>   // for string, operator==, basic_string, operator<
#include <tuple>    // for tuple
#include <utility>  // for tuple_element<>::type, move
#include <vector>   // for vector

#include <glib-object.h>  // for g_object_ref, G_CALLBACK, g_signal_connect
#include <glib.h>         // for gulong, g_assert

#include "gui/GladeGui.h"  // for GladeGui

SidebarLayersContextMenu::SidebarLayersContextMenu(GladeGui* gui, SidebarToolbar* toolbar):
        SidebarBaseContextMenu(gui->get("sidebarPreviewLayersContextMenu")) {


    constexpr const char* sidebarPreviewLayerMoveUp = "sidebarPreviewLayerMoveUp";
    constexpr const char* sidebarPreviewLayerMoveDown = "sidebarPreviewLayerMoveDown";
    constexpr const char* sidebarPreviewMergeDown = "sidebarPreviewMergeDown";
    constexpr const char* sidebarPreviewLayerDuplicate = "sidebarPreviewLayerDuplicate";
    constexpr const char* sidebarPreviewLayerDelete = "sidebarPreviewLayerDelete";

    // Connect the context menu actions
    const std::map<std::string, SidebarActions> ctxMenuActions = {
            {sidebarPreviewLayerMoveUp, SIDEBAR_ACTION_MOVE_UP},
            {sidebarPreviewLayerMoveDown, SIDEBAR_ACTION_MOVE_DOWN},
            {sidebarPreviewMergeDown, SIDEBAR_ACTION_MERGE_DOWN},
            {sidebarPreviewLayerDuplicate, SIDEBAR_ACTION_COPY},
            {sidebarPreviewLayerDelete, SIDEBAR_ACTION_DELETE},
            // TODO(ja-he): add SIDEBAR_ACTION_NEW_BEFORE/AFTER if/when these are defined for layers
    };


    for (const auto& [name, action]: ctxMenuActions) {

        GtkWidget* const entry = gui->get(name);
        g_assert(entry != nullptr);

        // Create callbacks, store the entry
        using Data = ContextMenuData;
        auto userdata = std::make_unique<Data>(Data{toolbar, action});

        const auto callback =
                G_CALLBACK(+[](GtkMenuItem* item, Data* data) { data->toolbar->runAction(data->actions); });
        const gulong signalId = g_signal_connect(entry, "activate", callback, userdata.get());
        g_object_ref(entry);
        this->contextMenuSignals.emplace_back(entry, signalId, std::move(userdata));

        if (name == sidebarPreviewLayerMoveUp) {
            this->contextMenuMoveUp = entry;
        } else if (name == sidebarPreviewLayerMoveDown) {
            this->contextMenuMoveDown = entry;
        } else if (name == sidebarPreviewMergeDown) {
            this->contextMenuMergeDown = entry;
        } else if (name == sidebarPreviewLayerDuplicate) {
            this->contextMenuDuplicate = entry;
        } else if (name == sidebarPreviewLayerDelete) {
            this->contextMenuDelete = entry;
        }
    }
}

void SidebarLayersContextMenu::setActionsSensitivity(SidebarActions actions) {
    gtk_widget_set_sensitive(this->contextMenuMoveUp, actions & SIDEBAR_ACTION_MOVE_UP);
    gtk_widget_set_sensitive(this->contextMenuMoveDown, actions & SIDEBAR_ACTION_MOVE_DOWN);
    gtk_widget_set_sensitive(this->contextMenuMergeDown, actions & SIDEBAR_ACTION_MERGE_DOWN);
    gtk_widget_set_sensitive(this->contextMenuDuplicate, actions & SIDEBAR_ACTION_COPY);
    gtk_widget_set_sensitive(this->contextMenuDelete, actions & SIDEBAR_ACTION_DELETE);
}
