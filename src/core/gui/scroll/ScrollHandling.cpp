#include "ScrollHandling.h"

#include "gui/Layout.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical):
        adjHorizontal(adjHorizontal), adjVertical(adjVertical) {}


ScrollHandling::ScrollHandling(GtkScrolledWindow* win):
        ScrollHandling(gtk_scrolled_window_get_hadjustment(win), gtk_scrolled_window_get_vadjustment(win)) {}


ScrollHandling::~ScrollHandling() = default;

auto ScrollHandling::getHorizontal() -> GtkAdjustment* { return adjHorizontal; }

auto ScrollHandling::getVertical() -> GtkAdjustment* { return adjVertical; }

void ScrollHandling::init(GtkWidget* xournal, Layout* layout) {
    this->xournal = xournal;
    this->layout = layout;
}
