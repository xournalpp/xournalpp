#include "PluginToolButton.h"

#include "config-features.h"  // for ENABLE_PLUGINS

#ifdef ENABLE_PLUGINS

#include <utility>  // for move

#include "plugin/Plugin.h"  // for ToolbarButtonEntry
#include "util/gtk4_helper.h"


PluginToolButton::PluginToolButton(ToolbarButtonEntry* t): AbstractToolItem(std::move(t->toolbarId)), t(t) {}

PluginToolButton::~PluginToolButton() = default;

GtkToolItem* PluginToolButton::createItem(bool) {
    GtkButton* btn = GTK_BUTTON(gtk_button_new());
    gtk_button_set_relief(btn, GTK_RELIEF_NONE);
    gtk_button_set_icon_name(btn, t->iconName.c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(btn), t->description.c_str());

    // Connect signal
    g_signal_connect(btn, "clicked",
                     G_CALLBACK(+[](GtkWidget*, ToolbarButtonEntry* te) { te->plugin->executeToolbarButton(te); }),
                     this->t);


    item = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(btn));

    return this->item;
}
GtkToolItem* PluginToolButton::newItem() { return nullptr; }

auto PluginToolButton::getToolDisplayName() const -> std::string { return this->t->description; }

auto PluginToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(t->iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

#endif /* ENABLE_PLUGINS */
