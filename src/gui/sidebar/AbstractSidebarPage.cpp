#include "AbstractSidebarPage.h"

AbstractSidebarPage::AbstractSidebarPage(Control* control, SidebarToolbar* toolbar)
 : control(control),
   toolbar(toolbar)
{
}

AbstractSidebarPage::~AbstractSidebarPage()
{
	this->control = nullptr;
	this->toolbar = nullptr;
}

void AbstractSidebarPage::selectPageNr(size_t page, size_t pdfPage)
{
}

Control* AbstractSidebarPage::getControl()
{
	return this->control;
}

void AbstractSidebarPage::setTmpDisabled(bool disabled)
{
	GdkCursor* cursor = nullptr;
	if (disabled)
	{
		cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_WATCH);
	}

	if (gtk_widget_get_window(this->getWidget()))
	{
		gdk_window_set_cursor(gtk_widget_get_window(this->getWidget()), cursor);
	}

	gtk_widget_set_sensitive(this->getWidget(), !disabled);


	if (cursor)
	{
		g_object_unref(cursor);
	}
}
