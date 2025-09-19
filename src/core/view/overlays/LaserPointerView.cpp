#include "LaserPointerView.h"

#include "control/tools/LaserPointerHandler.h"
#include "control/tools/StrokeHandler.h"
#include "model/Stroke.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"
#include "view/View.h"

#include "StrokeToolView.h"

using namespace xoj::view;

LaserPointerView::LaserPointerView(const LaserPointerHandler* handler, Repaintable* parent):
        OverlayView(parent), handler(handler) {
    this->registerToPool(handler->getViewPool());
}

LaserPointerView::~LaserPointerView() noexcept { this->unregisterFromPool(); }

void LaserPointerView::draw(cairo_t* cr) const {
    if (this->mask.isInitialized()) {
        xoj::util::CairoSaveGuard saveGuard(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        this->mask.paintToWithAlpha(cr, this->alpha);
    } else {
        this->mask = createMask(cr);
    }
    if (this->activeStrokeView) {
        this->activeStrokeView->draw(cr);
    }
}

bool LaserPointerView::isViewOf(const OverlayBase* overlay) const { return overlay == this->handler; }

void xoj::view::LaserPointerView::on(StartNewStrokeRequest, StrokeHandler* handler) {
    this->activeStrokeView = std::make_unique<StrokeToolView>(handler, *handler->getStroke(), parent);
    if (!this->extents.empty()) {
        on(SET_ALPHA_REQUEST, 255);
    }
}

void xoj::view::LaserPointerView::on(SetAlphaRequest, uint8_t alpha) {
    this->alpha = alpha;
    this->parent->flagDirtyRegion(this->extents);
}

void xoj::view::LaserPointerView::on(FinishStrokeRequest, const Range& strokeBox) {
    xoj_assert(this->activeStrokeView);
    if (this->mask.isInitialized()) {
        this->activeStrokeView->draw(this->mask.get());
    }
    this->activeStrokeView.reset();
    this->extents = this->extents.unite(strokeBox);
}

void xoj::view::LaserPointerView::on(InputCancellationRequest, const Range& rg) {
    this->activeStrokeView.reset();
    this->parent->flagDirtyRegion(rg);
}

void xoj::view::LaserPointerView::deleteOn(FinalizationRequest) {
    this->parent->deleteOverlayView(this, this->extents);
}


auto xoj::view::LaserPointerView::createMask(cairo_t* tgtcr) const -> Mask {
    const double zoom = this->parent->getZoom();
    Range visibleRange = this->parent->getVisiblePart();

    if (!visibleRange.isValid()) {
        /*
         * The user might be drawing on a page that is not visible at all:
         * e.g. https://github.com/xournalpp/xournalpp/pull/4158#issuecomment-1385954494
         */
        return Mask();
    }

    Mask mask(cairo_get_target(tgtcr), visibleRange, zoom, CAIRO_CONTENT_COLOR_ALPHA);
    cairo_t* cr = mask.get();

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    return mask;
}
