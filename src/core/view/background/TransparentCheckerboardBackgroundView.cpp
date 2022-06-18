#include "TransparentCheckerboardBackgroundView.h"

#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

TransparentCheckerboardBackgroundView::TransparentCheckerboardBackgroundView(double pageWidth, double pageHeight):
        BackgroundView(pageWidth, pageHeight), pattern(createPattern()) {}

TransparentCheckerboardBackgroundView::~TransparentCheckerboardBackgroundView() noexcept {
    cairo_pattern_destroy(pattern);
}

auto TransparentCheckerboardBackgroundView::createPattern() -> cairo_pattern_t* {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 2 * CHECKER_SIZE, 2 * CHECKER_SIZE);
    cairo_t* cr = cairo_create(surf);

    Util::cairo_set_source_rgbi(cr, LIGHT_GREY);
    cairo_paint(cr);

    Util::cairo_set_source_rgbi(cr, DARK_GREY);
    cairo_rectangle(cr, CHECKER_SIZE, 0, CHECKER_SIZE, CHECKER_SIZE);
    cairo_rectangle(cr, 0, CHECKER_SIZE, CHECKER_SIZE, CHECKER_SIZE);
    cairo_fill(cr);

    cairo_destroy(cr);

    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surf);

    cairo_surface_destroy(surf);  // The surface is now owned by the pattern

    cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

    return pattern;
}

void TransparentCheckerboardBackgroundView::draw(cairo_t* cr) const {
    cairo_save(cr);
    cairo_set_source(cr, this->pattern);
    cairo_paint(cr);
    cairo_restore(cr);
}
