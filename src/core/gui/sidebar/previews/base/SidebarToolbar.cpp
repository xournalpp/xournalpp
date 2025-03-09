#include "SidebarToolbar.h"

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "gui/GladeGui.h"  // for GladeGui
#include "util/glib_casts.h"

template <SidebarActions a>
void cb(GtkButton*, SidebarToolbar* toolbar) {
    toolbar->runAction(a);
}

SidebarToolbar::SidebarToolbar(SidebarToolbarActionListener* listener, GladeGui* gui): listener(listener) {
    this->btUp = GTK_BUTTON(gui->get("btUp"));
    this->btDown = GTK_BUTTON(gui->get("btDown"));
    this->btCopy = GTK_BUTTON(gui->get("btCopy"));
    this->btDelete = GTK_BUTTON(gui->get("btDelete"));

    g_signal_connect(this->btUp, "clicked", xoj::util::wrap_for_g_callback_v<cb<SIDEBAR_ACTION_MOVE_UP>>, this);
    g_signal_connect(this->btDown, "clicked", xoj::util::wrap_for_g_callback_v<cb<SIDEBAR_ACTION_MOVE_DOWN>>, this);
    g_signal_connect(this->btCopy, "clicked", xoj::util::wrap_for_g_callback_v<cb<SIDEBAR_ACTION_COPY>>, this);
    g_signal_connect(this->btDelete, "clicked", xoj::util::wrap_for_g_callback_v<cb<SIDEBAR_ACTION_DELETE>>, this);
}

SidebarToolbar::~SidebarToolbar() = default;

void SidebarToolbar::runAction(SidebarActions actions) { this->listener->actionPerformed(actions); }

void SidebarToolbar::setHidden(bool hidden) {
    gtk_widget_set_visible(GTK_WIDGET(this->btUp), !hidden);
    gtk_widget_set_visible(GTK_WIDGET(this->btDown), !hidden);
    gtk_widget_set_visible(GTK_WIDGET(this->btCopy), !hidden);
    gtk_widget_set_visible(GTK_WIDGET(this->btDelete), !hidden);
}

void SidebarToolbar::setButtonEnabled(SidebarActions enabledActions) {
    gtk_widget_set_sensitive(GTK_WIDGET(this->btUp), enabledActions & SIDEBAR_ACTION_MOVE_UP);
    gtk_widget_set_sensitive(GTK_WIDGET(this->btDown), enabledActions & SIDEBAR_ACTION_MOVE_DOWN);
    gtk_widget_set_sensitive(GTK_WIDGET(this->btCopy), enabledActions & SIDEBAR_ACTION_COPY);
    gtk_widget_set_sensitive(GTK_WIDGET(this->btDelete), enabledActions & SIDEBAR_ACTION_DELETE);
}

void SidebarToolbar::setButtonTooltips(const std::string& tipUp, const std::string& tipDown, const std::string& tipCopy,
                                       const std::string& tipDelete) {
    gtk_widget_set_tooltip_text(GTK_WIDGET(this->btUp), tipUp.c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(this->btDown), tipDown.c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(this->btCopy), tipCopy.c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(this->btDelete), tipDelete.c_str());
}

SidebarToolbarActionListener::~SidebarToolbarActionListener() = default;

void SidebarToolbarActionListener::actionPerformed(SidebarActions action) {}
