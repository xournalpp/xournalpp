#include "ScrollHandlingGtk.h"

#include "gui/Layout.h"


ScrollHandlingGtk::ScrollHandlingGtk(GtkScrollable* scrollable)
 : ScrollHandling(gtk_scrollable_get_hadjustment(scrollable), gtk_scrollable_get_vadjustment(scrollable))
{
	XOJ_INIT_TYPE(ScrollHandlingGtk);
}

ScrollHandlingGtk::~ScrollHandlingGtk()
{
	XOJ_RELEASE_TYPE(ScrollHandlingGtk);
}

void ScrollHandlingGtk::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	gtk_widget_queue_resize(xournal);
}

int ScrollHandlingGtk::getPrefferedWidth()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	return layout->getLayoutWidth();;
}

int ScrollHandlingGtk::getPrefferedHeight()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	return layout->getLayoutHeight();
}

