#include "RulerView.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

RulerView::RulerView(const Ruler* ruler, Repaintable* parent, ZoomControl* zoomControl):
        GeometryToolView(ruler, parent, zoomControl) {
    registerToPool(ruler->getViewPool());
}

RulerView::~RulerView() noexcept { unregisterFromPool(); }

void RulerView::on(FlagDirtyRegionRequest, const Range& rg) { parent->flagDirtyRegion(rg); }

void RulerView::on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) {
    length = h;
    rotation = rot;
    matrix = m;
}

void RulerView::deleteOn(FinalizationRequest, const Range& rg) { parent->drawAndDeleteToolView(this, rg); }

void RulerView::drawGeometryTool(cairo_t* cr) const {
    xoj::util::CairoSaveGuard save(cr);
    cairo_scale(cr, CM, CM);
    constexpr double width = Ruler::THICKNESS;

    cairo_set_source_rgba(cr, 0.96, 0.93, 0.78, 0.9);
    cairo_rectangle(cr, -length / 2., 0., length, width);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 0.18, 0.18, 0.18, 0.8);
    cairo_set_line_width(cr, 0.025);
    cairo_stroke(cr);

    cairo_set_source_rgba(cr, 0.12, 0.12, 0.12, 0.9);
    for (int millimeter = static_cast<int>(-length * 5.); millimeter <= static_cast<int>(length * 5.);
         ++millimeter) {
        const double x = millimeter / 10.;
        const double tick = millimeter % 10 == 0 ? 0.7 : (millimeter % 5 == 0 ? 0.45 : 0.25);
        cairo_move_to(cr, x, 0.);
        cairo_line_to(cr, x, tick);
        if (millimeter % 10 == 0) {
            cairo_set_font_size(cr, 0.35);
            cairo_move_to(cr, x + 0.08, width - 0.25);
            cairo_show_text(cr, std::to_string(millimeter / 10).c_str());
        }
    }
    cairo_set_line_width(cr, 0.02);
    cairo_stroke(cr);
}

void RulerView::drawDisplays(cairo_t* cr) const {
    xoj::util::CairoSaveGuard save(cr);
    cairo_transform(cr, &matrix);
    cairo_set_source_rgba(cr, 0.08, 0.22, 0.55, 0.9);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 0.35 * CM);
    cairo_move_to(cr, -1.7 * CM, 1.35 * CM);
    cairo_show_text(cr, "TRAD");

    std::ostringstream text;
    text << std::fixed << std::setprecision(1) << std::abs(std::remainder(deg(rotation), 180.)) << " deg";
    cairo_set_font_size(cr, 0.25 * CM);
    cairo_move_to(cr, 0.2 * CM, 1.35 * CM);
    cairo_show_text(cr, text.str().c_str());
}
