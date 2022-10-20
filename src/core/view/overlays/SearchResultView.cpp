#include "SearchResultView.h"

#include "control/SearchControl.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

SearchResultView::SearchResultView(const SearchControl* searchControl, Repaintable* parent, Color frameColor):
        OverlayView(parent), searchControl(searchControl), frameColor(frameColor) {
    this->registerToPool(searchControl->getViewPool());
}

SearchResultView::~SearchResultView() noexcept { this->unregisterFromPool(); }

void SearchResultView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());

    for (const XojPdfRectangle& rect: this->searchControl->getResults()) {
        cairo_rectangle(cr, rect.x1, rect.y1, rect.x2 - rect.x1, rect.y2 - rect.y1);
        Util::cairo_set_source_rgbi(cr, frameColor);
        cairo_stroke_preserve(cr);
        Util::cairo_set_source_rgbi(cr, frameColor, BACKGROUND_OPACITY);
        cairo_fill(cr);
    }
}

bool SearchResultView::isViewOf(const OverlayBase* overlay) const { return overlay == this->searchControl; }

void SearchResultView::on(SearchResultView::SearchChangedNotification) {
    this->parent->flagDirtyRegion(this->parent->getVisiblePart());
}
