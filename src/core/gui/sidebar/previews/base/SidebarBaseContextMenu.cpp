#include "gui/sidebar/previews/base/SidebarBaseContextMenu.h"

#include <glib-object.h>  // for g_object_unref, g_signal_handler_disconnect

SidebarBaseContextMenu::SidebarBaseContextMenu(GtkWidget* contextMenu): contextMenu(contextMenu) {}

SidebarBaseContextMenu::~SidebarBaseContextMenu() {
    for (const auto& [widget, handlerId, _]: this->contextMenuSignals) {
        if (g_signal_handler_is_connected(widget, handlerId)) {
            g_signal_handler_disconnect(widget, handlerId);
        }
        g_object_unref(widget);
    }
}


void SidebarBaseContextMenu::open() {
    gtk_menu_popup(GTK_MENU(this->contextMenu), nullptr, nullptr, nullptr, nullptr, 3, gtk_get_current_event_time());
}
