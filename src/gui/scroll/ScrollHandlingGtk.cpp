#include "ScrollHandlingGtk.h"

#include "gui/Layout.h"


ScrollHandlingGtk::ScrollHandlingGtk(GtkScrollable* scrollable)
 : ScrollHandling(gtk_scrollable_get_hadjustment(scrollable), gtk_scrollable_get_vadjustment(scrollable))
{
}

ScrollHandlingGtk::~ScrollHandlingGtk()
{
}

void ScrollHandlingGtk::setLayoutSize(int width, int height)
{
	gtk_widget_queue_resize(xournal);
}

int ScrollHandlingGtk::getPreferredWidth()
{
	return layout->getMinimalWidth();
}

int ScrollHandlingGtk::getPreferredHeight()
{
	return layout->getMinimalHeight();
}

void ScrollHandlingGtk::translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2)
{
	// Nothing to do here - all done by GTK
}

void ScrollHandlingGtk::translate(double& x, double& y)
{
	// Nothing to do here - all done by GTK
}

bool ScrollHandlingGtk::fullRepaint()
{
	return false;
}

void ScrollHandlingGtk::scrollChanged()
{
	// Nothing to do here - all done by GTK
}


