#include "SidebarToolbar.h"

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "gui/GladeGui.h"  // for GladeGui

SidebarToolbar::SidebarToolbar(SidebarToolbarActionListener* listener, GladeGui* gui): listener(listener) {
    this->btUp = GTK_BUTTON(gui->get("btUp"));
    this->btDown = GTK_BUTTON(gui->get("btDown"));
    this->btCopy = GTK_BUTTON(gui->get("btCopy"));
    this->btDelete = GTK_BUTTON(gui->get("btDelete"));

    g_signal_connect(this->btUp, "clicked", G_CALLBACK(&btUpClicked), this);
    g_signal_connect(this->btDown, "clicked", G_CALLBACK(&btDownClicked), this);
    g_signal_connect(this->btCopy, "clicked", G_CALLBACK(&btCopyClicked), this);
    g_signal_connect(this->btDelete, "clicked", G_CALLBACK(&btDeleteClicked), this);
}

SidebarToolbar::~SidebarToolbar() = default;

void SidebarToolbar::runAction(SidebarActions actions) { this->listener->actionPerformed(actions); }

void SidebarToolbar::btUpClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar) {
    toolbar->runAction(SIDEBAR_ACTION_MOVE_UP);
}

void SidebarToolbar::btDownClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar) {
    toolbar->runAction(SIDEBAR_ACTION_MOVE_DOWN);
}

void SidebarToolbar::btCopyClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar) {
    toolbar->runAction(SIDEBAR_ACTION_COPY);
}

void SidebarToolbar::btDeleteClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar) {
    toolbar->runAction(SIDEBAR_ACTION_DELETE);
}

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
