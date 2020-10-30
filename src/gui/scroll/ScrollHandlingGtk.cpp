#include "ScrollHandlingGtk.h"

#include "gui/Layout.h"


ScrollHandlingGtk::ScrollHandlingGtk(GtkScrollable* scrollable):
        ScrollHandling(gtk_scrollable_get_hadjustment(scrollable), gtk_scrollable_get_vadjustment(scrollable)) {}

ScrollHandlingGtk::~ScrollHandlingGtk() = default;

void ScrollHandlingGtk::setLayoutSize(int width, int height) {
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

auto ScrollHandlingGtk::getPreferredWidth() -> int { return layout->getMinimalWidth(); }

auto ScrollHandlingGtk::getPreferredHeight() -> int { return layout->getMinimalHeight(); }

void ScrollHandlingGtk::translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2) {
    // Nothing to do here - all done by GTK
}

void ScrollHandlingGtk::translate(double& x, double& y) {
    // Nothing to do here - all done by GTK
}

auto ScrollHandlingGtk::fullRepaint() -> bool { return false; }

void ScrollHandlingGtk::scrollChanged() {
    // Nothing to do here - all done by GTK
}
