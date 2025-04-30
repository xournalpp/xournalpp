#include "SelectorView.h"

#include <vector>

#include "control/tools/Selector.h"
#include "util/Color.h"
#include "util/LoopUtil.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

SelectorView::SelectorView(const Selector* selector, Repaintable* parent, Color selectorColor):
        OverlayView(parent), selector(selector), selectorColor(selectorColor) {
    this->registerToPool(selector->getViewPool());
}

SelectorView::~SelectorView() noexcept { this->unregisterFromPool(); }

void SelectorView::draw(cairo_t* cr) const {
    auto pts = this->selector->getBoundary();
    if (pts.size() < 3) {
        // To few points to draw
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(cr);  // cairo_save

    // set the line always the same size on display
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());

    Util::cairo_set_source_rgbi(cr, selectorColor);

    cairo_new_path(cr);
    for (auto& p: pts) {
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, selectorColor, FILLING_OPACITY);
    cairo_fill(cr);
}

bool SelectorView::isViewOf(const OverlayBase* overlay) const { return overlay == this->selector; }

void SelectorView::on(SelectorView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void xoj::view::SelectorView::deleteOn(xoj::view::SelectorView::DeleteViewsRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->deleteOverlayView(this, rg);
}
