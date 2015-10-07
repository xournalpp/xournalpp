#include "AbstractSidebarPage.h"

AbstractSidebarPage::AbstractSidebarPage(Control* control)
{
	XOJ_INIT_TYPE(AbstractSidebarPage);

	this->control = control;
	this->tabButton = NULL;
}

AbstractSidebarPage::~AbstractSidebarPage()
{
	XOJ_RELEASE_TYPE(AbstractSidebarPage);

	this->control = NULL;
}

bool AbstractSidebarPage::selectPageNr(size_t page, size_t pdfPage)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);

	// TODO LOW PRIO why not pageSelected?

	return false;
}

void AbstractSidebarPage::documentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);
}

void AbstractSidebarPage::pageSizeChanged(size_t page)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);
}

void AbstractSidebarPage::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);
}

void AbstractSidebarPage::pageInserted(size_t page)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);
}

void AbstractSidebarPage::pageDeleted(size_t page)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);

}

void AbstractSidebarPage::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);
}

Control* AbstractSidebarPage::getControl()
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);

	return this->control;
}

void AbstractSidebarPage::setTmpDisabled(bool disabled)
{
	XOJ_CHECK_TYPE(AbstractSidebarPage);

	GdkCursor* cursor = NULL;

	if (disabled)
	{
		cursor = gdk_cursor_new(GDK_WATCH);
	}

	if (gtk_widget_get_window(this->getWidget()))
	{
		gdk_window_set_cursor(gtk_widget_get_window(this->getWidget()), cursor);
	}

	gtk_widget_set_sensitive(this->getWidget(), !disabled);


	if (cursor)
	{
		gdk_cursor_unref(cursor);
	}
}
