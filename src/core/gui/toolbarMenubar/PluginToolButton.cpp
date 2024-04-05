#include "PluginToolButton.h"

#include "config-features.h"  // for ENABLE_PLUGINS

#ifdef ENABLE_PLUGINS

#include <utility>  // for move

#include "plugin/Plugin.h"  // for ToolbarButtonEntry
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"


PluginToolButton::PluginToolButton(ToolbarButtonEntry* t): AbstractToolItem(std::move(t->toolbarId)), t(t) {}

PluginToolButton::~PluginToolButton() = default;

auto PluginToolButton::createItem(bool) -> xoj::util::WidgetSPtr {
    xoj::util::WidgetSPtr item(gtk_button_new(), xoj::util::adopt);
    gtk_widget_set_can_focus(item.get(), false);  // todo(gtk4) not necessary anymore

    GtkButton* btn = GTK_BUTTON(item.get());
    gtk_button_set_relief(btn, GTK_RELIEF_NONE);
    gtk_button_set_icon_name(btn, t->iconName.c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(btn), t->description.c_str());

    // Connect signal
    g_signal_connect(item.get(), "clicked", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                         auto* te = static_cast<ToolbarButtonEntry*>(d);
                         te->plugin->executeToolbarButton(te);
                     }),
                     this->t);

    return item;
}

auto PluginToolButton::getToolDisplayName() const -> std::string { return this->t->description; }

auto PluginToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(t->iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

#endif /* ENABLE_PLUGINS */
