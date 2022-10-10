#include "SelectionView.h"

#include <vector>

#include "control/tools/Selection.h"
#include "util/Color.h"
#include "util/LoopUtil.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

SelectionView::SelectionView(const Selection* selection, Repaintable* parent, Color selectionColor):
        OverlayView(parent), selection(selection), selectionColor(selectionColor) {
    this->registerToPool(selection->getViewPool());
}

SelectionView::~SelectionView() noexcept { this->unregisterFromPool(); }

void SelectionView::draw(cairo_t* cr) const {
    auto pts = this->selection->getBoundary();
    if (pts.size() < 3) {
        // To few points to draw
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(cr);  // cairo_save

    // set the line always the same size on display
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());

    Util::cairo_set_source_rgbi(cr, selectionColor);

    cairo_new_path(cr);
    for (auto& p: pts) {
        cairo_line_to(cr, p.x, p.y);
    }
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, selectionColor, FILLING_OPACITY);
    cairo_fill(cr);
}

bool SelectionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->selection; }

void SelectionView::on(SelectionView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void xoj::view::SelectionView::deleteOn(xoj::view::SelectionView::DeleteViewsRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->deleteOverlayView(this, rg);
}
