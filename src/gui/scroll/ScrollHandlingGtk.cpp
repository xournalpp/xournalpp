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
    
	// after a page has been inserted the layout size must be updated immediately,
    // otherwise it comes down to a race deciding if scrolling happens normally or not
    if (gtk_adjustment_get_upper(getHorizontal()) < width) {
        gtk_adjustment_set_upper(getHorizontal(), width);
    }
    if (gtk_adjustment_get_upper(getVertical()) < height) {
        gtk_adjustment_set_upper(getVertical(), height);
    }
	gtk_widget_queue_resize(xournal);
}

int ScrollHandlingGtk::getPreferredWidth()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	return layout->getMinimalWidth();
}

int ScrollHandlingGtk::getPreferredHeight()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	return layout->getMinimalHeight();
}

void ScrollHandlingGtk::translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2)
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	// Nothing to do here - all done by GTK
}

void ScrollHandlingGtk::translate(double& x, double& y)
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	// Nothing to do here - all done by GTK
}

bool ScrollHandlingGtk::fullRepaint()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	return false;
}

void ScrollHandlingGtk::scrollChanged()
{
	XOJ_CHECK_TYPE(ScrollHandlingGtk);

	// Nothing to do here - all done by GTK
}


