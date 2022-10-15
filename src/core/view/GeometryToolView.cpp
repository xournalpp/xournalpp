#include "GeometryToolView.h"

#include "model/GeometryTool.h"  // for GeometryTool
#include "model/Stroke.h"        // for Stroke
#include "view/Repaintable.h"    // for Repaintable
#include "view/View.h"           // for Context

#include "StrokeView.h"  // for StrokeView

using namespace xoj::view;

GeometryToolView::GeometryToolView(const GeometryTool* s, Repaintable* parent): ToolView(parent), s(s) {}

GeometryToolView::~GeometryToolView() = default;

void GeometryToolView::draw(cairo_t* cr) const {
    cairo_save(cr);
    this->drawGeometryTool(cr);
    this->drawTemporaryStroke(cr);
    cairo_restore(cr);
}

bool GeometryToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->s; }

void GeometryToolView::drawTemporaryStroke(cairo_t* cr) const {
    if (s->getStroke()) {
        auto context = xoj::view::Context::createDefault(cr);
        xoj::view::StrokeView strokeView(s->getStroke());
        strokeView.draw(context);
    }
}

void GeometryToolView::showTextCenteredAndRotated(cairo_t* cr, std::string text, double angle) const {
    cairo_save(cr);

    cairo_text_extents_t te;
    cairo_text_extents(cr, text.c_str(), &te);
    const double dx = te.x_bearing + te.width / 2.0;
    const double dy = te.y_bearing + te.height / 2.0;

    cairo_rotate(cr, rad(angle));
    cairo_rel_move_to(cr, -dx, -dy);
    cairo_text_path(cr, text.c_str());

    cairo_restore(cr);
}
