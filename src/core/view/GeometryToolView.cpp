#include "GeometryToolView.h"

#include "model/GeometryTool.h"       // for GeometryTool
#include "model/Stroke.h"             // for Stroke
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"         // for Repaintable
#include "view/View.h"                // for Context

#include "StrokeView.h"  // for StrokeView

using namespace xoj::view;

GeometryToolView::GeometryToolView(const GeometryTool* geometryTool, Repaintable* parent):
        ToolView(parent), geometryTool(geometryTool) {}

GeometryToolView::~GeometryToolView() = default;

void GeometryToolView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    this->drawGeometryTool(cr);
    this->drawTemporaryStroke(cr);
}

bool GeometryToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->geometryTool; }

void GeometryToolView::drawTemporaryStroke(cairo_t* cr) const {
    if (geometryTool->getStroke()) {
        auto context = xoj::view::Context::createDefault(cr);
        xoj::view::StrokeView strokeView(geometryTool->getStroke());
        strokeView.draw(context);
    }
}

void GeometryToolView::showTextCenteredAndRotated(cairo_t* cr, const std::string& text, double angle) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_text_extents_t te;
    cairo_text_extents(cr, text.c_str(), &te);
    const double dx = te.x_bearing + te.width / 2.0;
    const double dy = te.y_bearing + te.height / 2.0;

    cairo_rotate(cr, rad(angle));
    cairo_rel_move_to(cr, -dx, -dy);
    cairo_text_path(cr, text.c_str());
}
