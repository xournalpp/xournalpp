#include "PdfElemSelection.h"

#include <algorithm>  // for max, min
#include <limits>     // for numeric_limits
#include <memory>     // for __shared_ptr_access
#include <utility>    // for move

#include <cairo.h>    // for cairo_line_to, cairo_region_destroy
#include <gdk/gdk.h>  // for GdkRGBA, gdk_cairo_set_source_rgba
#include <undo/GroupUndoAction.h>
#include <undo/InsertUndoAction.h>

#include "control/Control.h"      // for Control
#include "control/ToolHandler.h"  // for ToolHandler
#include "gui/PageView.h"         // for XojPageView
#include "gui/XournalView.h"      // for XournalView
#include "model/Document.h"       // for Document
#include "model/PageRef.h"        // for PageRef
#include "model/XojPage.h"        // for XojPage
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle, XojPdfPageSelectio...
#include "util/Assert.h"          // for xoj_assert
#include "util/safe_casts.h"      // for strict_cast, as_signed, as_si...
#include "view/overlays/PdfElementSelectionView.h"

static XojPdfPageSelectionStyle selectionStyleForToolType(ToolType type) {
    switch (type) {
        case ToolType::TOOL_SELECT_PDF_TEXT_RECT:
            return XojPdfPageSelectionStyle::Area;
        default:
            return XojPdfPageSelectionStyle::Linear;
    }
}

PdfElemSelection::PdfElemSelection(double x, double y, ToolHandler& th, PageRef page, Control* control):
        pdf(nullptr),
        xoppPage(std::move(page)),
        control(control),
        bounds({x, y, x, y}),
        style(selectionStyleForToolType(th.getToolType())),
        color(th.getColor()),
        highlighterAlpha(th.getSelectPDFTextMarkerOpacity()),
        finalized(false),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::PdfElementSelectionView>>()) {

    if (auto pNr = xoppPage->getPdfPageNr(); pNr != npos) {
        Document* doc = control->getDocument();
        doc->lock_shared();
        this->pdf = doc->getPdfPage(pNr);
        doc->unlock_shared();

        this->selectionPageNr = pNr;
    }
}

PdfElemSelection::~PdfElemSelection() {
    this->control->clipboardCutCopyEnabled(false);
    Range rg = getRegionBbox();
    this->viewPool->dispatchAndClear(xoj::view::PdfElementSelectionView::CANCEL_SELECTION_REQUEST, rg);
}

auto PdfElemSelection::finalizeSelectionAndRepaint() -> bool {
    Range rg = getRegionBbox();
    bool result = this->finalizeSelection();
    rg = rg.unite(getRegionBbox());
    if (!rg.empty()) {
        this->viewPool->dispatch(xoj::view::PdfElementSelectionView::FLAG_DIRTY_REGION_REQUEST, rg);
    }
    return result;
}

bool PdfElemSelection::finalizeSelection() {
    this->finalized = true;

    XojPdfPage::TextSelection selection = this->pdf->selectTextLines(this->bounds, this->style);
    this->selectedTextRegion = std::move(selection.region);
    this->selectedTextRects = std::move(selection.rects);
    this->selectedText = this->pdf->selectText(this->bounds, this->style);
    this->control->clipboardCutCopyEnabled(!this->selectedText.empty());
    // Informs the windowing system of the selection -- i.e. for accessibility purposes
    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), this->selectedText.c_str(),
                           strict_cast<gint>(this->selectedText.length()));
    return !this->selectedTextRects.empty();
}

void PdfElemSelection::changeStyle(XojPdfPageSelectionStyle style) { this->style = style; }

Range PdfElemSelection::getRegionBbox() const {
    if (this->selectedTextRegion && cairo_region_num_rectangles(this->selectedTextRegion.get()) > 0) {
        cairo_rectangle_int_t bbox{};
        cairo_region_get_extents(this->selectedTextRegion.get(), &bbox);
        return Range(bbox.x, bbox.y, bbox.x + bbox.width, bbox.y + bbox.height);
    }
    return Range();  // empty range
}

void PdfElemSelection::currentPos(double x, double y) {
    if (!this->pdf) {
        return;
    }

    // Update the end position
    this->bounds.x2 = x;
    this->bounds.y2 = y;
    Range rg = getRegionBbox();

    // Repaint the selected text area
    switch (this->style) {
        case XojPdfPageSelectionStyle::Linear:
        case XojPdfPageSelectionStyle::Word:
        case XojPdfPageSelectionStyle::Line:
            this->selectedTextRegion.reset(this->pdf->selectTextRegion(this->bounds, this->style), xoj::util::adopt);
            break;
        case XojPdfPageSelectionStyle::Area: {
            cairo_rectangle_int_t rect;
            rect.x = floor_cast<int>(std::min(bounds.x1, bounds.x2));
            rect.width = ceil_cast<int>(std::max(bounds.x1, bounds.x2)) - rect.x;
            rect.y = floor_cast<int>(std::min(bounds.y1, bounds.y2));
            rect.height = ceil_cast<int>(std::max(bounds.y1, bounds.y2)) - rect.y;
            this->selectedTextRegion.reset(cairo_region_create_rectangle(&rect), xoj::util::adopt);
        } break;
        default:
            xoj_assert_message(false, "Unreachable");
    }
    xoj_assert(this->selectedTextRegion);

    rg = rg.unite(getRegionBbox());
    if (!rg.empty()) {
        this->viewPool->dispatch(xoj::view::PdfElementSelectionView::FLAG_DIRTY_REGION_REQUEST, rg);
    }
    this->finalized = false;
}

auto PdfElemSelection::contains(double x, double y) -> bool {
    if (!this->selectedTextRegion) {
        return false;
    }

    return cairo_region_contains_point(this->selectedTextRegion.get(), static_cast<int>(x), static_cast<int>(y));
}

auto PdfElemSelection::getSelectedTextRects() const -> const std::vector<XojPdfRectangle>& { return selectedTextRects; }

auto PdfElemSelection::getSelectedText() const -> const std::string& { return this->selectedText; }

auto PdfElemSelection::isFinalized() const -> bool { return this->finalized; }

void PdfElemSelection::highlight() {
    createStrokes(MarkerStyle::POS_TEXT_MIDDLE, MarkerStyle::WIDTH_TEXT_HEIGHT, highlighterAlpha);
}

void PdfElemSelection::underline() { createStrokes(MarkerStyle::POS_TEXT_BOTTOM, MarkerStyle::WIDTH_TEXT_LINE, 230); }

void PdfElemSelection::strikethrough() {
    createStrokes(MarkerStyle::POS_TEXT_MIDDLE, MarkerStyle::WIDTH_TEXT_LINE, 230);
}

void PdfElemSelection::createStrokes(MarkerStyle position, MarkerStyle width, int markerOpacity) {
    if (this->selectedTextRects.empty()) {
        return;
    }

    Range dirtyRange;
    std::vector<ElementPtr> strokes;
    for (XojPdfRectangle rect: this->selectedTextRects) {
        const double topOfLine = std::min(rect.y1, rect.y2);
        const double middleOfLine = (rect.y1 + rect.y2) / 2;
        const double bottomOfLine = std::max(rect.y1, rect.y2);
        const double rectWidth = std::abs(rect.y2 - rect.y1);

        // the center line position of stroke
        const double h = position == MarkerStyle::POS_TEXT_BOTTOM ? bottomOfLine :
                         position == MarkerStyle::POS_TEXT_MIDDLE ? middleOfLine :
                                                                    topOfLine;
        // the width of stroke
        const double w = width == MarkerStyle::WIDTH_TEXT_LINE ? 1 : rectWidth;

        auto stroke = std::make_unique<Stroke>();
        stroke->setColor(this->color);
        stroke->setFill(markerOpacity);
        stroke->setToolType(StrokeTool::HIGHLIGHTER);
        stroke->setWidth(w);
        stroke->addPoint(Point(rect.x1, h, -1));
        stroke->addPoint(Point(rect.x2, h, -1));
        stroke->setStrokeCapStyle(StrokeCapStyle::BUTT);

        dirtyRange.addPoint(rect.x1, h - 0.5 * w);
        dirtyRange.addPoint(rect.x2, h + 0.5 * w);

        strokes.push_back(std::move(stroke));
    }

    std::vector<const Element*> strokePtrs(strokes.size());
    std::transform(strokes.begin(), strokes.end(), strokePtrs.begin(), [](auto& e) { return e.get(); });

    auto* doc = this->control->getDocument();
    doc->lock();
    Layer* layer = xoppPage->getSelectedLayer();
    for (auto&& s: strokes) {
        layer->addElement(std::move(s));
    }
    doc->unlock();
    xoppPage->fireElementsChanged(strokePtrs, dirtyRange);

    auto undoAct = std::make_unique<GroupUndoAction>();
    for (auto* stroke: strokePtrs) {
        undoAct->addAction(std::make_unique<InsertUndoAction>(xoppPage, layer, stroke));
    }
    control->getUndoRedoHandler()->addUndoAction(std::move(undoAct));
}

bool PdfElemSelection::copy() {
    if (!this->selectedText.empty()) {
        this->control->getClipboardHandler().copyTextToClipboard(this->getSelectedText());
        return true;
    }
    return false;
}
