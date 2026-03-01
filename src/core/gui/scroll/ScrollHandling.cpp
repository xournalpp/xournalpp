#include "ScrollHandling.h"

#include "gui/Layout.h"
#include "util/Point.h"


ScrollHandling::ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical):
        adjHorizontal(adjHorizontal), adjVertical(adjVertical) {}


ScrollHandling::ScrollHandling(GtkScrolledWindow* win):
        ScrollHandling(gtk_scrolled_window_get_hadjustment(win), gtk_scrolled_window_get_vadjustment(win)) {}


ScrollHandling::~ScrollHandling() = default;

auto ScrollHandling::getHorizontal() -> GtkAdjustment* { return adjHorizontal; }

auto ScrollHandling::getVertical() -> GtkAdjustment* { return adjVertical; }

auto ScrollHandling::getPosition() const -> xoj::util::Point<double> {
    return {gtk_adjustment_get_value(adjHorizontal), gtk_adjustment_get_value(adjVertical)};
}
