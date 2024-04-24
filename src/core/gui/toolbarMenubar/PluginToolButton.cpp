#include "PluginToolButton.h"

#include "config-features.h"  // for ENABLE_PLUGINS

#ifdef ENABLE_PLUGINS

#include <utility>  // for move

#include "plugin/Plugin.h"  // for ToolbarButtonEntry
#include "util/glib_casts.h"


PluginToolButton::PluginToolButton(ToolbarButtonEntry* t):
        AbstractToolItem(std::move(t->toolbarId), Category::PLUGINS), t(t) {}

PluginToolButton::~PluginToolButton() = default;

auto PluginToolButton::createItem(ToolbarSide) -> Widgetry {
    xoj::util::WidgetSPtr item(gtk_button_new(), xoj::util::adopt);
    GtkWidget* btn = item.get();

    gtk_button_set_icon_name(GTK_BUTTON(btn), t->iconName.c_str());
    gtk_widget_set_tooltip_text(btn, t->description.c_str());

    g_signal_connect(btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* te = static_cast<ToolbarButtonEntry*>(d);
                         te->plugin->executeToolbarButton(te);
                     }),
                     this->t);

    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_button_new();

        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_button_set_child(GTK_BUTTON(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));

        g_signal_connect(proxy, "activate", G_CALLBACK(+[](GtkButton*, gpointer d) {
                             auto* te = static_cast<ToolbarButtonEntry*>(d);
                             te->plugin->executeToolbarButton(te);
                         }),
                         this->t);

        return proxy;
    };
    return {std::move(item), xoj::util::WidgetSPtr(createProxy(), xoj::util::adopt)};
}

auto PluginToolButton::getToolDisplayName() const -> std::string { return this->t->description; }

auto PluginToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(t->iconName.c_str());
}

#endif /* ENABLE_PLUGINS */
