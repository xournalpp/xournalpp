#include "Sidebar.h"
#include "indextree/SidebarIndexPage.h"
#include "previews/SidebarPreviews.h"
#include "../../model/XojPage.h"
#include "../../model/Document.h"
#include "../../control/Control.h"
#include "../../control/PdfCache.h"
#include "../GladeGui.h"

#include <string.h>

#include <config.h>
#include <glib/gi18n-lib.h>

Sidebar::Sidebar(GladeGui * gui, Control * control) {
	XOJ_INIT_TYPE(Sidebar);

	this->control = control;
	this->pages = NULL;
	this->comboBox = GTK_COMBO_BOX(gui->get("cbSelectSidebar"));
	this->buttonCloseSidebar = gui->get("buttonCloseSidebar");
	this->visiblePage = NULL;

	GtkWidget * sidebar = gui->get("sidebarContents");

	this->initPages(sidebar);

	gtk_widget_set_visible(GTK_WIDGET(sidebar), control->getSettings()->isSidebarVisible());
}

void Sidebar::initPages(GtkWidget * sidebar) {
	XOJ_CHECK_TYPE(Sidebar);

	addPage(new SidebarIndexPage(this->control));
	addPage(new SidebarPreviews(this->control));

	// Init combobox with names

	GtkListStore * store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_combo_box_set_model(this->comboBox, GTK_TREE_MODEL(store));
	g_object_unref(store);

	GtkCellRenderer * cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->comboBox), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->comboBox), cell, "text", 0, NULL);

	int selected = -1;
	GtkWidget * selectedWidget = NULL;

	int i = 0;
	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;

		gtk_combo_box_append_text(comboBox, p->getName());

		if (selectedWidget == NULL) {
			selectedWidget = p->getWidget();
		}

		if (p->hasData() && selected == -1) {
			// first entry with data
			selected = i;
			selectedWidget = p->getWidget();
		}

		// Add widget to sidebar
		gtk_box_pack_start(GTK_BOX(sidebar), p->getWidget(), TRUE, TRUE, 0);

		i++;
	}

	if (selected < 0) {
		selected = 0;
	}

	gtk_widget_show(selectedWidget);
	this->visiblePage = selectedWidget;

	gtk_combo_box_set_active(this->comboBox, selected);

	g_signal_connect(this->comboBox, "changed", G_CALLBACK(cbChangedCallback), this);
}

void Sidebar::addPage(AbstractSidebarPage * page) {
	XOJ_CHECK_TYPE(Sidebar);

	this->pages = g_list_append(this->pages, page);
}

Sidebar::~Sidebar() {
	XOJ_CHECK_TYPE(Sidebar);

	this->control = NULL;

	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;
		delete p;
	}

	g_list_free(this->pages);
	this->pages = NULL;

	XOJ_RELEASE_TYPE(Sidebar);
}

void Sidebar::selectPageNr(int page, int pdfPage) {
	XOJ_CHECK_TYPE(Sidebar);

	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;
		p->selectPageNr(page, pdfPage);
	}
}

void Sidebar::cbChangedCallback(GtkComboBox * widget, Sidebar * sidebar) {
	XOJ_CHECK_TYPE_OBJ(sidebar, Sidebar);

	int selected = gtk_combo_box_get_active(widget);

	gtk_widget_hide(sidebar->visiblePage);

	GList * entry = g_list_nth(sidebar->pages, selected);
	if (entry) {
		AbstractSidebarPage * page = (AbstractSidebarPage *) entry->data;
		sidebar->visiblePage = page->getWidget();
		gtk_widget_show(sidebar->visiblePage);
	}
}

void Sidebar::setTmpDisabled(bool disabled) {
	XOJ_CHECK_TYPE(Sidebar);

	gtk_widget_set_sensitive(this->buttonCloseSidebar, !disabled);

	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;
		p->setTmpDisabled(disabled);
	}

	gdk_display_sync(gdk_display_get_default());
}

Control * Sidebar::getControl() {
	XOJ_CHECK_TYPE(Sidebar);

	return this->control;
}

