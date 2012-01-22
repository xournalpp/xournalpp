#include "SidebarToolbar.h"
#include "../../../control/Control.h"

SidebarToolbar::SidebarToolbar(Control * control) {
	XOJ_INIT_TYPE(SidebarToolbar);

	this->control = control;
	this->toolbar = GTK_TOOLBAR(gtk_toolbar_new());

	gtk_toolbar_set_icon_size(this->toolbar, GTK_ICON_SIZE_SMALL_TOOLBAR);

	g_object_ref(this->toolbar);

	this->btUp = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_toolbar_insert(this->toolbar, this->btUp, -1);
	g_signal_connect(this->btUp, "clicked", G_CALLBACK(&btUpClicked), this);

	this->btDown = gtk_tool_button_new_from_stock(GTK_STOCK_GO_DOWN);
	gtk_toolbar_insert(this->toolbar, this->btDown, -1);
	g_signal_connect(this->btDown, "clicked", G_CALLBACK(&btDownClicked), this);

	this->btCopy = gtk_tool_button_new_from_stock(GTK_STOCK_COPY);
	gtk_toolbar_insert(this->toolbar, this->btCopy, -1);
	g_signal_connect(this->btCopy, "clicked", G_CALLBACK(&btCopyClicked), this);

	this->btDelete = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(this->toolbar, this->btDelete, -1);
	g_signal_connect(this->btDelete, "clicked", G_CALLBACK(&btDeleteClicked), this);

	gtk_widget_show_all(GTK_WIDGET(this->toolbar));
}

SidebarToolbar::~SidebarToolbar() {
	XOJ_RELEASE_TYPE(SidebarToolbar);

	gtk_widget_unref(GTK_WIDGET(this->toolbar));
	this->toolbar = NULL;

	this->control = NULL;
}

void SidebarToolbar::btUpClicked(GtkToolButton * toolbutton, SidebarToolbar * toolbar) {
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document * doc = toolbar->control->getDocument();
	doc->lock();

	int page = doc->indexOf(toolbar->currentPage);
	if (page != -1) {
		doc->deletePage(page);
		doc->insertPage(toolbar->currentPage, page - 1);
	}
	doc->unlock();

	toolbar->control->firePageDeleted(page);
	toolbar->control->firePageInserted(page - 1);
	toolbar->control->firePageSelected(page - 1);
}

void SidebarToolbar::btDownClicked(GtkToolButton * toolbutton, SidebarToolbar * toolbar) {
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document * doc = toolbar->control->getDocument();
	doc->lock();

	int page = doc->indexOf(toolbar->currentPage);
	if (page != -1) {
		doc->deletePage(page);
		doc->insertPage(toolbar->currentPage, page + 1);
	}
	doc->unlock();

	toolbar->control->firePageDeleted(page);
	toolbar->control->firePageInserted(page + 1);
	toolbar->control->firePageSelected(page + 1);
}

void SidebarToolbar::btCopyClicked(GtkToolButton * toolbutton, SidebarToolbar * toolbar) {
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document * doc = toolbar->control->getDocument();
	doc->lock();

	int page = doc->indexOf(toolbar->currentPage);

	PageRef newPage = toolbar->currentPage.clone();
	doc->insertPage(newPage, page + 1);

	doc->unlock();

	toolbar->control->firePageInserted(page + 1);
	toolbar->control->firePageSelected(page + 1);

}

void SidebarToolbar::btDeleteClicked(GtkToolButton * toolbutton, SidebarToolbar * toolbar) {
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	toolbar->control->deletePage();
}

void SidebarToolbar::setButtonEnabled(bool enableUp, bool enableDown, bool enableCopy, bool enableDelete, PageRef currentPage) {
	XOJ_CHECK_TYPE(SidebarToolbar);

	gtk_widget_set_sensitive(GTK_WIDGET(this->btUp), enableUp);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDown), enableDown);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btCopy), enableCopy);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDelete), enableDelete);

	this->currentPage = currentPage;
}

GtkWidget * SidebarToolbar::getWidget() {
	XOJ_CHECK_TYPE(SidebarToolbar);

	return GTK_WIDGET(this->toolbar);
}
