#include "ScrollHandling.h"

#include "gui/Layout.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical):
        adjHorizontal(adjHorizontal), adjVertical(adjVertical) {}


ScrollHandling::ScrollHandling(GtkScrollable* scrollable):
        ScrollHandling(gtk_scrollable_get_hadjustment(scrollable), gtk_scrollable_get_vadjustment(scrollable)) {}


ScrollHandling::~ScrollHandling() = default;

auto ScrollHandling::getHorizontal() -> GtkAdjustment* { return adjHorizontal; }

auto ScrollHandling::getVertical() -> GtkAdjustment* { return adjVertical; }

void ScrollHandling::init(GtkWidget* xournal, Layout* layout) {
    this->xournal = xournal;
    this->layout = layout;
}


#define TEMP_FIX true

void ScrollHandling::setLayoutSize(int width, int height) {
    // after a page has been inserted the layout size must be updated immediately,
    // otherwise it comes down to a race deciding if scrolling happens normally or not
#if TEMP_FIX
    // Todo(Fabian): remove that fix, since it is conceptually wrong behavior
    //               the adjustment is/must be changed implicitly by the layout and the resulting expose event
    if (gtk_adjustment_get_upper(getHorizontal()) != width) {
        gtk_adjustment_set_upper(getHorizontal(), width);
    }
    if (gtk_adjustment_get_upper(getVertical()) != height) {
        gtk_adjustment_set_upper(getVertical(), height);
    }
#endif
}

auto ScrollHandling::getPreferredWidth() -> int { return layout->getMinimalWidth(); }

auto ScrollHandling::getPreferredHeight() -> int { return layout->getMinimalHeight(); }
