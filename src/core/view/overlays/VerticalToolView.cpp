#include "VerticalToolView.h"

#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"
#include "util/raii/CairoWrappers.h"
#include "view/ElementContainerView.h"
#include "view/Repaintable.h"
#include "view/View.h"

using namespace xoj::view;

VerticalToolView::VerticalToolView(const VerticalToolHandler* handler, Repaintable* parent, ZoomControl* zoomControl,
                                   const Settings* settings):
        ToolView(parent), toolHandler(handler), zoomControl(zoomControl), aidColor(settings->getSelectionColor()) {
    this->registerToPool(handler->getViewPool());
    zoomControl->addZoomListener(this);
}

VerticalToolView::~VerticalToolView() noexcept {
    zoomControl->removeZoomListener(this);
    this->unregisterFromPool();
}

void VerticalToolView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    const double startY = toolHandler->getStartY();
    const double endY = toolHandler->getEndY();

    cairo_rectangle(cr, 0, startY, toolHandler->getPageWidth(), endY - startY);

    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    Util::cairo_set_source_rgbi(cr, this->aidColor);
    cairo_stroke_preserve(cr);

    Util::cairo_set_source_rgbi(cr, this->aidColor, BACKGROUND_OPACITY);
    cairo_fill(cr);

    this->drawWithoutDrawingAids(cr);
}

void VerticalToolView::drawWithoutDrawingAids(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    if (!mask.isInitialized()) {
        // Initialize the mask on first call, when changing sides or upon zoom change
        mask = createMask(cr);
        xoj::view::ElementContainerView v(this->toolHandler);
        v.draw(xoj::view::Context::createDefault(mask.get()));
    }

    cairo_translate(cr, 0, toolHandler->getEndY() - toolHandler->getStartY());

    mask.paintTo(cr);
}

bool VerticalToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->toolHandler; }

void VerticalToolView::on(VerticalToolView::SetVerticalShiftRequest, double shift) {
    Range rg = this->parent->getVisiblePart();
    auto side = this->toolHandler->getSide();

    // Padding for taking into account the drawing aid line width
    const double padding = 0.5 * BORDER_WIDTH_IN_PIXELS / this->parent->getZoom();
    if (side == VerticalToolHandler::Side::Above) {
        rg.maxY = std::min(std::max(shift, this->lastShift) + padding, rg.maxY);
    } else {
        rg.minY = std::max(std::min(shift, this->lastShift) - padding, rg.minY);
    }
    this->parent->flagDirtyRegion(rg);
    this->lastShift = shift;
}

void VerticalToolView::on(VerticalToolView::SwitchDirectionRequest) {
    mask.reset();
    this->parent->flagDirtyRegion(this->parent->getVisiblePart());
}

void VerticalToolView::deleteOn(VerticalToolView::FinalizationRequest) {
    double startY = toolHandler->getStartY();
    double endY = toolHandler->getEndY();
    auto [minY, maxY] = std::minmax(startY, endY);

    // Padding for taking into account the drawing aid line width
    const double padding = 0.5 * BORDER_WIDTH_IN_PIXELS / this->parent->getZoom();

    // Get a range containing exactly the drawing aid blue rectangle
    Range rg(std::numeric_limits<double>::lowest(), minY - padding, std::numeric_limits<double>::max(), maxY + padding);
    rg = rg.intersect(this->parent->getVisiblePart());

    this->parent->drawAndDeleteToolView(this, rg);
}

Mask VerticalToolView::createMask(cairo_t* tgtcr) const {
    auto side = toolHandler->getSide();
    auto startY = toolHandler->getStartY();
    /*
     * Create a mask that
     *      * starts at startY if side == Side::Below
     *      * ends at startY if side == Side::Above
     *      * has height and width those of the visible area
     * This mask is as small as possible, and still allows the user to see all elements go up or down, as long as the
     * cursor remains in the affected page.
     * TODO find a way to have the mask big enough for all situations:
     *              * If the previous page is partially visible, the mask is to short
     *              * If the cursor goes into the toolbar, the mask is to short
     *              * If several views are implemented, the mask size should depend on the possible cursor positions
     *                within the acted upon view, not the current view
     *                      (the entire page is not an option for infinite pages...)
     */
    const double zoom = this->parent->getZoom();
    Range range = this->parent->getVisiblePart();

    if (side == VerticalToolHandler::Side::Above) {
        range.translate(0, startY - range.maxY);
    } else {
        range.translate(0, startY - range.minY);
    }

    return Mask(cairo_get_target(tgtcr), range, zoom, CAIRO_CONTENT_COLOR_ALPHA);
}

void VerticalToolView::zoomChanged() {
    // If zooming in, the mask needs redrawing to get a suitable definition
    // If zooming out, the mask needs redrawing to cover a bigger part of the page
    mask.reset();
}
