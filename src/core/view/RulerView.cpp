#include "RulerView.h"

#include <algorithm>
#include <cmath>

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
            const std::string label = std::to_string(millimeter / 10);
            cairo_set_font_size(cr, 0.32);
            cairo_text_extents_t labelExtents;
            cairo_text_extents(cr, label.c_str(), &labelExtents);
            const double minLabelX = -length / 2. + 0.06 - labelExtents.x_bearing;
            const double maxLabelX = length / 2. - 0.06 - (labelExtents.x_bearing + labelExtents.width);
            const double labelX = std::clamp(x + 0.08, minLabelX, std::max(minLabelX, maxLabelX));
            cairo_move_to(cr, labelX, width - 0.15);
            cairo_show_text(cr, label.c_str());
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
    constexpr double displayY = 1.15;
    constexpr double brandX = -1.45;
    constexpr double badgeX = 0.5;
    constexpr double badgeRadius = 0.36;

    cairo_set_font_size(cr, 0.28);
    cairo_text_extents_t brandExtents;
    cairo_text_extents(cr, "TRAD", &brandExtents);
    cairo_set_source_rgba(cr, 0.96, 0.93, 0.78, 1.0);
    cairo_rectangle(cr, brandX - 0.1, displayY - 0.28, brandExtents.width + 0.2, 0.56);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 0.08, 0.22, 0.55, 1.0);
    cairo_move_to(cr, brandX, displayY - brandExtents.y_bearing - brandExtents.height / 2.);
    cairo_show_text(cr, "TRAD");

    const int angleDegrees = static_cast<int>(std::lround(std::abs(std::remainder(deg(rotation), 180.))));
    const std::string angle = std::to_string(angleDegrees) + "\xC2\xB0";

    cairo_set_source_rgba(cr, 0.96, 0.93, 0.78, 1.0);
    cairo_new_path(cr);
    cairo_arc(cr, badgeX, displayY, badgeRadius, 0., 2. * M_PI);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 0.08, 0.22, 0.55, 0.9);
    cairo_set_line_width(cr, 0.03);
    cairo_stroke(cr);

    cairo_set_font_size(cr, 0.26);
    cairo_text_extents_t angleExtents;
    cairo_text_extents(cr, angle.c_str(), &angleExtents);
    cairo_new_path(cr);
    cairo_move_to(cr, badgeX - angleExtents.x_bearing - angleExtents.width / 2.,
                  displayY - angleExtents.y_bearing - angleExtents.height / 2.);
    cairo_show_text(cr, angle.c_str());
}
