#include "SidebarToolbar.h"

#include "control/Control.h"
#include "undo/CopyUndoAction.h"
#include "undo/SwapUndoAction.h"


SidebarToolbar::SidebarToolbar(Control* control, GladeGui* gui)
{
	XOJ_INIT_TYPE(SidebarToolbar);

	this->control = control;

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
	XOJ_CHECK_TYPE(SidebarToolbar);

	this->control = NULL;
	XOJ_RELEASE_TYPE(SidebarToolbar);
}

void SidebarToolbar::btUpClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document* doc = toolbar->control->getDocument();
	doc->lock();

	size_t page = doc->indexOf(toolbar->currentPage);
	PageRef swappedPage = toolbar->currentPage;
	PageRef otherPage = doc->getPage(page - 1);
	if (page != size_t_npos)
	{
		doc->deletePage(page);
		doc->insertPage(toolbar->currentPage, page - 1);
	}
	doc->unlock();

	UndoRedoHandler* undo = toolbar->control->getUndoRedoHandler();
	undo->addUndoAction(new SwapUndoAction(page - 1, true, swappedPage, otherPage));

	toolbar->control->firePageDeleted(page);
	toolbar->control->firePageInserted(page - 1);
	toolbar->control->firePageSelected(page - 1);

	toolbar->control->getScrollHandler()->scrollToPage(page - 1);
}

void SidebarToolbar::btDownClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document* doc = toolbar->control->getDocument();
	PageRef swapped_page, other_page;
	doc->lock();

	size_t page = doc->indexOf(toolbar->currentPage);
	swapped_page = toolbar->currentPage;
	other_page = doc->getPage(page + 1);
	if (page != size_t_npos)
	{
		doc->deletePage(page);
		doc->insertPage(toolbar->currentPage, page + 1);
	}
	doc->unlock();

	UndoRedoHandler* undo = toolbar->control->getUndoRedoHandler();
	undo->addUndoAction(new SwapUndoAction(page, false, swapped_page, other_page));

	toolbar->control->firePageDeleted(page);
	toolbar->control->firePageInserted(page + 1);
	toolbar->control->firePageSelected(page + 1);

	toolbar->control->getScrollHandler()->scrollToPage(page + 1);
}

void SidebarToolbar::btCopyClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	Document* doc = toolbar->control->getDocument();
	doc->lock();

	int page = doc->indexOf(toolbar->currentPage);
	if (page < 0)
	{
		return;
	}

	PageRef newPage = toolbar->currentPage.clone();
	doc->insertPage(newPage, page + 1);

	doc->unlock();

	UndoRedoHandler* undo = toolbar->control->getUndoRedoHandler();
	undo->addUndoAction(new CopyUndoAction(newPage, page + 1));

	toolbar->control->firePageInserted(page + 1);
	toolbar->control->firePageSelected(page + 1);

	toolbar->control->getScrollHandler()->scrollToPage(page + 1);
}

void SidebarToolbar::btDeleteClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar)
{
	XOJ_CHECK_TYPE_OBJ(toolbar, SidebarToolbar);

	toolbar->control->deletePage();
}

void SidebarToolbar::setHidden(bool hidden)
{
	XOJ_CHECK_TYPE(SidebarToolbar);

	gtk_widget_set_visible(GTK_WIDGET(this->btUp), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btDown), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btCopy), !hidden);
	gtk_widget_set_visible(GTK_WIDGET(this->btDelete), !hidden);
}

void SidebarToolbar::setButtonEnabled(bool enableUp, bool enableDown, bool enableCopy, bool enableDelete, PageRef currentPage)
{
	XOJ_CHECK_TYPE(SidebarToolbar);

	gtk_widget_set_sensitive(GTK_WIDGET(this->btUp), enableUp);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDown), enableDown);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btCopy), enableCopy);
	gtk_widget_set_sensitive(GTK_WIDGET(this->btDelete), enableDelete);

	this->currentPage = currentPage;
}

