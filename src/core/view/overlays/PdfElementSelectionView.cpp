#include "PdfElementSelectionView.h"

#include "control/tools/PdfElemSelection.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

class Range;

using namespace xoj::view;

PdfElementSelectionView::PdfElementSelectionView(const PdfElemSelection* selection, Repaintable* parent,
                                                 Color selectionColor):
        OverlayView(parent), selection(selection), selectionColor(selectionColor) {
    this->registerToPool(selection->getViewPool());
}

PdfElementSelectionView::~PdfElementSelectionView() noexcept { this->unregisterFromPool(); }

void PdfElementSelectionView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    if (auto reg = selection->getSelectedRegion(); reg && !cairo_region_is_empty(reg)) {
        gdk_cairo_region(cr, reg);
        Util::cairo_set_source_rgbi(cr, selectionColor, SELECTION_OPACITY);
        cairo_fill(cr);
    }
}

bool PdfElementSelectionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->selection; }

void PdfElementSelectionView::on(PdfElementSelectionView::FlagDirtyRegionRequest, const Range& rg) const {
    this->parent->flagDirtyRegion(rg);
}

void PdfElementSelectionView::deleteOn(PdfElementSelectionView::CancelSelectionRequest, const Range& rg) {
    this->parent->deleteOverlayView(this, rg);
}
