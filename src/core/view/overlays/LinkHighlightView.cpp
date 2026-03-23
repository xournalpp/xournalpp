#include "LinkHighlightView.h"

#include "control/tools/LinkHandler.h"
#include "model/Link.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"  // for Repaintable

using namespace xoj::view;

constexpr double LINE_WIDTH = 1.0;  // Should not exceed Link::PADDING
constexpr Color LINE_COLOR{Colors::red};

LinkHighlightView::LinkHighlightView(const LinkHandler* handler, Repaintable* parent):
        OverlayView(parent), handler(handler) {
    this->registerToPool(handler->getViewPool());
}

LinkHighlightView::~LinkHighlightView() { this->unregisterFromPool(); }

void LinkHighlightView::on(FlagDirtyRegionRequest, const Range& rg) { this->parent->flagDirtyRegion(rg); }

void LinkHighlightView::deleteOn(FinalizationRequest, const Range& rg) { this->parent->deleteOverlayView(this, rg); }

void LinkHighlightView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);
    for (const auto& optRect: {handler->getHighlightRect(), handler->getSelectRect()}) {
        if (optRect.has_value()) {
            auto rect = optRect.value();
            Util::cairo_set_source_rgbi(cr, LINE_COLOR);
            cairo_rectangle(cr, rect.x + LINE_WIDTH / 2, rect.y + LINE_WIDTH / 2, rect.width - LINE_WIDTH,
                            rect.height - LINE_WIDTH);
            cairo_set_line_width(cr, LINE_WIDTH);
            cairo_stroke(cr);
        }
    }
}

bool LinkHighlightView::isViewOf(const OverlayBase* overlay) const { return overlay == this->handler; }
