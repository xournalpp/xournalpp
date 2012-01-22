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
	this->tbSelectPage = GTK_TOOLBAR(gui->get("tbSelectSidebarPage"));
	this->buttonCloseSidebar = gui->get("buttonCloseSidebar");
	this->visiblePage = NULL;

	GtkWidget * sidebar = gui->get("sidebarContents");

	gtk_widget_set_size_request(sidebar, control->getSettings()->getSidebarWidth(), 100);

	this->initPages(sidebar, gui);

	gtk_widget_set_visible(GTK_WIDGET(sidebar), control->getSettings()->isSidebarVisible());

	registerListener(control);
}

void Sidebar::initPages(GtkWidget * sidebar, GladeGui * gui) {
	XOJ_CHECK_TYPE(Sidebar);

	addPage(new SidebarIndexPage(this->control));
	addPage(new SidebarPreviews(this->control));

	// Init toolbar with icons

	int i = 0;
	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;
		GtkToolItem * it = gtk_toggle_tool_button_new();
		p->tabButton = it;

		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), gui->loadIcon(p->getIconName()));
		g_signal_connect(it, "clicked", G_CALLBACK(&buttonClicked), new SidebarPageButton(this, i, p));
		gtk_tool_item_set_tooltip_text(it, p->getName());
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), p->getName());

		gtk_toolbar_insert(tbSelectPage, it, -1);

		// Add widget to sidebar
		gtk_box_pack_start(GTK_BOX(sidebar), p->getWidget(), TRUE, TRUE, 0);

		i++;
	}

	gtk_widget_show_all(GTK_WIDGET(this->tbSelectPage));

	updateEnableDisableButtons();
}

void Sidebar::buttonClicked(GtkToolButton * toolbutton, SidebarPageButton * buttonData) {
	XOJ_CHECK_TYPE_OBJ(buttonData->sidebar, Sidebar);

	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton))) {
		if (buttonData->sidebar->visiblePage != buttonData->page->getWidget()) {
			buttonData->sidebar->setSelectedPage(buttonData->index);
		}
	} else if (buttonData->sidebar->visiblePage == buttonData->page->getWidget()) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
	}
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

void Sidebar::setSelectedPage(int page) {
	int i = 0;

	AbstractSidebarPage * currentPage = (AbstractSidebarPage *)g_list_nth(this->pages, page)->data;
	this->visiblePage = currentPage->getWidget();

	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;

		if(page == i) {
			gtk_widget_show(p->getWidget());
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), true);
		} else {
			gtk_widget_hide(p->getWidget());
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), false);
		}

		i++;
	}
}

void Sidebar::updateEnableDisableButtons() {
	int i = 0;
	int selected = -1;

	for (GList * l = this->pages; l != NULL; l = l->next) {
		AbstractSidebarPage * p = (AbstractSidebarPage *) l->data;

		gtk_widget_set_sensitive(GTK_WIDGET(p->tabButton), p->hasData());

		if(p->hasData() && selected == -1) {
			selected = i;
		}

		i++;
	}

	setSelectedPage(selected);
}

void Sidebar::setTmpDisabled(bool disabled) {
	XOJ_CHECK_TYPE(Sidebar);

	gtk_widget_set_sensitive(this->buttonCloseSidebar, !disabled);
	gtk_widget_set_sensitive(GTK_WIDGET(this->tbSelectPage), !disabled);

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

void Sidebar::documentChanged(DocumentChangeType type) {
	XOJ_CHECK_TYPE(Sidebar);

	if(type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_PDF_BOOKMARKS) {
		updateEnableDisableButtons();
	}
}

void Sidebar::pageSizeChanged(int page) {}
void Sidebar::pageChanged(int page) {}
void Sidebar::pageInserted(int page) {}
void Sidebar::pageDeleted(int page) {}
void Sidebar::pageSelected(int page) {}


SidebarPageButton::SidebarPageButton(Sidebar * sidebar, int index, AbstractSidebarPage * page) {
	this->sidebar = sidebar;
	this->index = index;
	this->page = page;
}

