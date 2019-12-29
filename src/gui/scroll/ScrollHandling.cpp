#include "ScrollHandling.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical):
        adjHorizontal(adjHorizontal), adjVertical(adjVertical) {}

ScrollHandling::~ScrollHandling() = default;

auto ScrollHandling::getHorizontal() -> GtkAdjustment* { return adjHorizontal; }

auto ScrollHandling::getVertical() -> GtkAdjustment* { return adjVertical; }

void ScrollHandling::init(GtkWidget* xournal, Layout* layout) {
    this->xournal = xournal;
    this->layout = layout;
}
