#include "SidebarToolbar.h"

SidebarToolbar::SidebarToolbar(SidebarToolbarActionListener* listener, GladeGui* gui)
 : listener(listener)
{
	this->btUp = GTK_BUTTON(gui->get("btUp"));
	this->btDown = GTK_BUTTON(gui->get("btDown"));
	this->btCopy = GTK_BUTTON(gui->get("btCopy"));
	this->btDelete = GTK_BUTTON(gui->get("btDelete"));

	g_signal_connect(this->btUp, "clicked", G_CALLBACK(&btUpClicked), this);
	g_signal_connect(this->btDown, "clicked", G_CALLBACK(&btDownClicked), this);
	g_signal_connect(this->btCopy, "clicked", G_CALLBACK(&btCopyClicked), this);
	g_signal_connect(this->btDelete, "clicked", G_CALLBACK(&btDeleteClicked), this);
}

SidebarToolbar::~SidebarToolbar()
{
}

void SidebarToolbar::runAction(SidebarActions actions)
{
	this->listener->actionPerformed(actions);
}

void SidebarToolbar::btUpClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	toolbar->runAction(SIDEBAR_ACTION_MOVE_UP);
}

void SidebarToolbar::btDownClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	toolbar->runAction(SIDEBAR_ACTION_MOVE_DOWN);
}

void SidebarToolbar::btCopyClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	toolbar->runAction(SIDEBAR_ACTION_COPY);
}

void SidebarToolbar::btDeleteClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	toolbar->runAction(SIDEBAR_ACTION_DELETE);
}

void SidebarToolbar::setHidden(bool hidden)
{
	gtk_widget_set_visible(GTK_WIDGET(this->btUp), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btDown), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btCopy), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btDelete), !hidden);
}

void SidebarToolbar::setButtonEnabled(SidebarActions enabledActions)
{
	gtk_widget_set_sensitive(GTK_WIDGET(this->btUp), enabledActions & SIDEBAR_ACTION_MOVE_UP);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDown), enabledActions & SIDEBAR_ACTION_MOVE_DOWN);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btCopy), enabledActions & SIDEBAR_ACTION_COPY);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDelete), enabledActions & SIDEBAR_ACTION_DELETE);
}


SidebarToolbarActionListener::~SidebarToolbarActionListener()
{
}

void SidebarToolbarActionListener::actionPerformed(SidebarActions action)
{
}

