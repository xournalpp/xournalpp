#include "PlainBackgroundView.h"

#include <algorithm>  // for max, min

#include "util/safe_casts.h"                 // for ceil_cast, floor_cast
#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

PlainBackgroundView::PlainBackgroundView(double pageWidth, double pageHeight, Color backgroundColor):
        BackgroundView(pageWidth, pageHeight), backgroundColor(backgroundColor) {}

void PlainBackgroundView::draw(cairo_t* cr) const {
    Util::cairo_set_source_rgbi(cr, this->backgroundColor);
    cairo_paint(cr);
}

std::pair<int, int> PlainBackgroundView::getIndexBounds(double min, double max, double step, double margin,
                                                        double length) {
    return {ceil_cast<int>(std::max(min, margin) / step), floor_cast<int>(std::min(max, margin + length) / step)};
}
