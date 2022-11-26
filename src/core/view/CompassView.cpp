#include "CompassView.h"

#include <algorithm>  // for max, min
#include <cmath>      // for isnan, sqrt, cos, sin, atan2
#include <iomanip>
#include <sstream>
#include <utility>  // for move

#include <glib.h>  // for g_error, g_warning

#include "model/Compass.h"            // for Compass
#include "model/Stroke.h"             // for Stroke
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"         // for Repaintable
#include "view/View.h"                // for Context

#include "StrokeView.h"  // for StrokeView

using namespace xoj::view;

// all lengths are in centimeter
constexpr double FONT_SIZE = .2;
constexpr double CIRCLE_RAD = .3;
constexpr double TICK_SMALL = .1;
constexpr double TICK_LARGE = .2;
constexpr double RELATIVE_CIRCLE_POS = .8;
constexpr double RELATIVE_ANGULAR_CAPTION_POS = .3;
constexpr double OFFSET_CIRCLE_POS = .4;
constexpr const char* FONT_FAMILY = "Arial";

CompassView::CompassView(const Compass* compass, Repaintable* parent, ZoomControl* zoomControl):
        GeometryToolView(compass, parent, zoomControl) {
    this->registerToPool(compass->getViewPool());
}

CompassView::~CompassView() noexcept { this->unregisterFromPool(); };


void CompassView::on(FlagDirtyRegionRequest, const Range& rg) { this->parent->flagDirtyRegion(rg); }

void CompassView::on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) {
    assert(h > 0 && "Non-positive compass height");
    height = h;
    rotation = rot;
    matrix = m;
    circlePos = height * RELATIVE_CIRCLE_POS - OFFSET_CIRCLE_POS;
    angularCaptionPos = height * RELATIVE_ANGULAR_CAPTION_POS;
    maxHmark = static_cast<int>(std::round(height * 10.0));
    drawRotationDisplay = height >= 2.;
    drawRadialCaption = height >= 1.5;
    angularOffset = (height >= 2.) ? 1 : (height >= 1.2) ? 2 : (height >= 0.8) ? 5 : 10;
    angularCaptionOffset = (height >= 3.) ? 30 : (height >= 1.5) ? 45 : (height >= 1.) ? 90 : 360;
}

void CompassView::deleteOn(CompassView::FinalizationRequest, const Range& rg) {
    this->parent->drawAndDeleteToolView(this, rg);
}

void CompassView::drawGeometryTool(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_scale(cr, CM, CM);

    cairo_set_line_width(cr, LINE_WIDTH_IN_CM);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_select_font_face(cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);

    cairo_set_source_rgb(cr, 1, 0, 0);  // red
    drawOutline(cr);

    cairo_set_source_rgb(cr, .0, .0, 1.);  // blue
    drawHorizontalMarks(cr);

    cairo_set_source_rgb(cr, .5, 0, .5);  // violet
    drawAngularMarks(cr);
}

void CompassView::drawDisplays(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_transform(cr, &matrix);
    cairo_set_line_width(cr, LINE_WIDTH_IN_CM);
    cairo_select_font_face(cr, FONT_FAMILY, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);

    cairo_set_source_rgb(cr, 0, .5, .5);  // turquoise
    drawRotation(cr);
}

void CompassView::drawOutline(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_move_to(cr, .0, .0);
    cairo_arc(cr, .0, .0, this->height, .0, 2. * M_PI);
    cairo_stroke_preserve(cr);

    cairo_set_source_rgba(cr, .2, .2, .2, .1);  // transparent gray
    cairo_fill(cr);
}

void CompassView::drawRotation(cairo_t* cr) const {
    if (!drawRotationDisplay) {
        return;
    }
    xoj::util::CairoSaveGuard saveGuard(cr);
    // write the angle within a small circle
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::abs(std::remainder(deg(this->rotation), 180.));
    cairo_move_to(cr, .0, -this->circlePos);
    showTextCenteredAndRotated(cr, ss.str(), -deg(this->rotation));

    cairo_move_to(cr, .0, -this->circlePos);
    cairo_new_sub_path(cr);
    cairo_arc(cr, .0, -this->circlePos, CIRCLE_RAD, .0, 2. * M_PI);
    cairo_stroke(cr);
}

void CompassView::drawAngularMarks(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    for (int i = angularOffset; i < 360; i += angularOffset) {
        const double cs = std::cos(rad(i));
        const double si = std::sin(rad(i));
        const double tick = (i % 5 == 0) ? TICK_LARGE : TICK_SMALL;
        cairo_move_to(cr, this->height * cs, this->height * si);
        if (i % angularCaptionOffset == 0) {
            const double radTickEnd = (i == 270) ? (this->circlePos + 1.5 * CIRCLE_RAD) : (angularCaptionPos + 0.3);
            cairo_line_to(cr, radTickEnd * cs, radTickEnd * si);
            cairo_move_to(cr, angularCaptionPos * cs, angularCaptionPos * si);
            showTextCenteredAndRotated(cr, std::to_string(360 - i), i + 90);
        } else {
            cairo_rel_line_to(cr, -tick * cs, -tick * si);
        }
    }
    cairo_stroke(cr);
}

void CompassView::drawHorizontalMarks(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    // BEGIN: radial measuring marks

    for (int i = 0; i <= this->maxHmark; i++) {
        const double tick = (i % 5 == 0) ? TICK_LARGE : TICK_SMALL;
        // draw marks
        cairo_move_to(cr, static_cast<double>(i) / 10.0, .0);
        cairo_rel_line_to(cr, .0, tick);
        if (i % 10 == 0 && drawRadialCaption) {
            // draw numbers
            cairo_rel_move_to(cr, .0, FONT_SIZE / 2.);
            const auto text = std::to_string(std::abs(i / 10));
            showTextCenteredAndRotated(cr, text.c_str(), .0);
        }
    }
    cairo_stroke(cr);
    // END: radial measuring marks
}
