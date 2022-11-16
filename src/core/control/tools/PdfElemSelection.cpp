#include "PdfElemSelection.h"

#include <algorithm>  // for max, min
#include <cmath>
#include <limits>   // for numeric_limits
#include <memory>   // for __shared_ptr_access
#include <utility>  // for move

#include <cairo.h>    // for cairo_line_to, cairo_region_destroy
#include <gdk/gdk.h>  // for GdkRGBA, gdk_cairo_set_source_rgba
#include <glib.h>     // for g_assert, g_assert_nonnull

#include "control/Control.h"      // for Control
#include "control/ToolHandler.h"  // for ToolHandler
#include "gui/PageView.h"         // for XojPageView
#include "gui/XournalView.h"      // for XournalView
#include "model/Document.h"       // for Document
#include "model/PageRef.h"        // for PageRef
#include "model/XojPage.h"        // for XojPage
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle, XojPdfPageSelectio...
#include "view/overlays/PdfElementSelectionView.h"

PdfElemSelection::PdfElemSelection(double x, double y, Control* control):
        pdf(nullptr),
        bounds({x, y, x, y}),
        finalized(false),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::PdfElementSelectionView>>()) {

    if (auto pNr = control->getCurrentPage()->getPdfPageNr(); pNr != npos) {
        Document* doc = control->getDocument();
        doc->lock();
        this->pdf = doc->getPdfPage(pNr);
        doc->unlock();

        this->selectionPageNr = pNr;
    }

    this->toolType = control->getToolHandler()->getToolType();
}

PdfElemSelection::~PdfElemSelection() {
    Range rg = getRegionBbox();
    this->viewPool->dispatchAndClear(xoj::view::PdfElementSelectionView::CANCEL_SELECTION_REQUEST, rg);
}

auto PdfElemSelection::finalizeSelectionAndRepaint(XojPdfPageSelectionStyle style) -> bool {
    Range rg = getRegionBbox();
    bool result = this->finalizeSelection(style);
    rg = rg.unite(getRegionBbox());
    this->viewPool->dispatch(xoj::view::PdfElementSelectionView::FLAG_DIRTY_REGION_REQUEST, rg);
    return result;
}

bool PdfElemSelection::finalizeSelection(XojPdfPageSelectionStyle style) {
    this->finalized = true;

    XojPdfPage::TextSelection selection = this->pdf->selectTextLines(this->bounds, style);
    this->selectedTextRegion = std::move(selection.region);
    this->selectedTextRects = std::move(selection.rects);
    this->selectedText = this->pdf->selectText(this->bounds, style);
    return !this->selectedTextRects.empty();
}

XojPdfPageSelectionStyle PdfElemSelection::selectionStyleForToolType(ToolType type) {
    switch (type) {
        case ToolType::TOOL_SELECT_PDF_TEXT_RECT:
            return XojPdfPageSelectionStyle::Area;
        default:
            return XojPdfPageSelectionStyle::Linear;
    }
}

Range PdfElemSelection::getRegionBbox() const {
    if (this->selectedTextRegion && cairo_region_num_rectangles(this->selectedTextRegion.get()) > 0) {
        cairo_rectangle_int_t bbox{};
        cairo_region_get_extents(this->selectedTextRegion.get(), &bbox);
        return Range(bbox.x, bbox.y, bbox.x + bbox.width, bbox.y + bbox.height);
    }
    return Range();  // empty range
}

void PdfElemSelection::currentPos(double x, double y, XojPdfPageSelectionStyle style) {
    if (!this->pdf) {
        return;
    }

    // Update the end position
    this->bounds.x2 = x;
    this->bounds.y2 = y;
    Range rg = getRegionBbox();

    // Repaint the selected text area
    switch (style) {
        case XojPdfPageSelectionStyle::Linear:
        case XojPdfPageSelectionStyle::Word:
        case XojPdfPageSelectionStyle::Line:
            this->selectedTextRegion.reset(this->pdf->selectTextRegion(this->bounds, style), xoj::util::adopt);
            break;
        case XojPdfPageSelectionStyle::Area: {
            cairo_rectangle_int_t rect;
            rect.x = static_cast<int>(std::floor(std::min(bounds.x1, bounds.x2)));
            rect.width = static_cast<int>(std::ceil(std::max(bounds.x1, bounds.x2))) - rect.x;
            rect.y = static_cast<int>(std::floor(std::min(bounds.y1, bounds.y2)));
            rect.height = static_cast<int>(std::ceil(std::max(bounds.y1, bounds.y2))) - rect.y;
            this->selectedTextRegion.reset(cairo_region_create_rectangle(&rect), xoj::util::adopt);
        } break;
        default:
            g_assert(false && "Unreachable");
    }
    g_assert(this->selectedTextRegion);

    rg = rg.unite(getRegionBbox());
    this->viewPool->dispatch(xoj::view::PdfElementSelectionView::FLAG_DIRTY_REGION_REQUEST, rg);
}

auto PdfElemSelection::contains(double x, double y) -> bool {
    if (!this->selectedTextRegion) {
        return false;
    }

    return cairo_region_contains_point(this->selectedTextRegion.get(), static_cast<int>(x), static_cast<int>(y));
}

auto PdfElemSelection::getSelectedTextRects() const -> const std::vector<XojPdfRectangle>& { return selectedTextRects; }

auto PdfElemSelection::getSelectedText() const -> const std::string& { return this->selectedText; }

auto PdfElemSelection::getSelectionPageNr() const -> uint64_t { return selectionPageNr; }

auto PdfElemSelection::isFinalized() const -> bool { return this->finalized; }

void PdfElemSelection::setToolType(ToolType tType) { this->toolType = tType; }
