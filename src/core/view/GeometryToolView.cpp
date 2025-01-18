#include "GeometryToolView.h"

#include "control/zoom/ZoomControl.h"
#include "model/GeometryTool.h"       // for GeometryTool
#include "model/Stroke.h"             // for Stroke
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"         // for Repaintable
#include "view/View.h"                // for Context

#include "StrokeView.h"  // for StrokeView

using namespace xoj::view;

GeometryToolView::GeometryToolView(const GeometryTool* geometryTool, Repaintable* parent, ZoomControl* zoomControl):
        ToolView(parent), geometryTool(geometryTool), zoomControl(zoomControl) {
    zoomControl->addZoomListener(this);
}

GeometryToolView::~GeometryToolView() { zoomControl->removeZoomListener(this); }

void GeometryToolView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_save(cr);

    if (!mask.isInitialized()) {
        // Initialize the mask on first call, when the geometry tool changes size and when zooming
        mask = createMask(cr);
        this->drawGeometryTool(mask.get());
    }
    cairo_translate(cr, geometryTool->getTranslationX(), geometryTool->getTranslationY());
    cairo_rotate(cr, geometryTool->getRotation());
    mask.paintTo(cr);
    cairo_restore(cr);
    this->drawDisplays(cr);
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

auto GeometryToolView::createMask(cairo_t* targetCr) const -> Mask {
    const double zoom = this->parent->getZoom();
    Range rg = geometryTool->getToolRange(false);
    return Mask(cairo_get_target(targetCr), rg, zoom, CAIRO_CONTENT_COLOR_ALPHA);
}

void GeometryToolView::on(ResetMaskRequest) { mask.reset(); }

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

void GeometryToolView::zoomChanged() {
    // If zooming in, the mask needs redrawing. Otherwise it gets blurry.
    mask.reset();
}
