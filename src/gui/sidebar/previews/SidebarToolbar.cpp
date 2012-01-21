#include "SidebarToolbar.h"

SidebarToolbar::SidebarToolbar() {
	XOJ_INIT_TYPE(SidebarToolbar);

	this->toolbar = GTK_TOOLBAR(gtk_toolbar_new());

	GtkToolItem * it;

	it = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_toolbar_insert(this->toolbar, it, 0);

	it = gtk_tool_button_new_from_stock(GTK_STOCK_GO_DOWN);
	gtk_toolbar_insert(this->toolbar, it, 0);

	it = gtk_tool_button_new_from_stock(GTK_STOCK_COPY);
	gtk_toolbar_insert(this->toolbar, it, 0);

	it = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(this->toolbar, it, 0);

	gtk_widget_show_all(GTK_WIDGET(this->toolbar));
}

SidebarToolbar::~SidebarToolbar() {
	XOJ_RELEASE_TYPE(SidebarToolbar);

	gtk_widget_unref(GTK_WIDGET(this->toolbar));
	this->toolbar = NULL;
}

GtkWidget * SidebarToolbar::getWidget() {
	XOJ_CHECK_TYPE(SidebarToolbar);

	return GTK_WIDGET(this->toolbar);
}
