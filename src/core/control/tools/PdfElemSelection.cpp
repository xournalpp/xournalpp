#include "PdfElemSelection.h"

#include <limits>

#include <cairo.h>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "pdf/base/XojPdfPage.h"

PdfElemSelection::PdfElemSelection(double x, double y, XojPageView* view):
        view(view), pdf(nullptr), bounds({x, y, x, y}), finalized(false) {

    auto xournal = view->getXournal();
    if (auto pNr = this->view->getPage()->getPdfPageNr(); pNr != npos) {
        Document* doc = xournal->getControl()->getDocument();
        doc->lock();
        this->pdf = doc->getPdfPage(pNr);
        doc->unlock();

        this->selectionPageNr = pNr;
    }

    this->toolType = xournal->getControl()->getToolHandler()->getToolType();
}

PdfElemSelection::~PdfElemSelection() {
    this->selectedTextRects.clear();
    if (this->selectedTextRegion) {
        cairo_region_destroy(this->selectedTextRegion);
        this->selectedTextRegion = nullptr;
    }
}

auto PdfElemSelection::finalizeSelectionAndRepaint(XojPdfPageSelectionStyle style) -> bool {
    bool result = this->finalizeSelection(style);
    this->view->repaintPage();
    return result;
}

void PdfElemSelection::doublePress() { this->finalizeSelectionAndRepaint(XojPdfPageSelectionStyle::Word); }

void PdfElemSelection::triplePress() { this->finalizeSelectionAndRepaint(XojPdfPageSelectionStyle::Line); }

bool PdfElemSelection::finalizeSelection(XojPdfPageSelectionStyle style) {
    this->finalized = true;

    if (this->selectedTextRegion) {
        cairo_region_destroy(this->selectedTextRegion);
    }

    XojPdfPage::TextSelection selection = this->pdf->selectTextLines(this->bounds, style);
    this->selectedTextRegion = selection.region;
    this->selectedTextRects = std::move(selection.rects);
    this->selectedText = this->pdf->selectText(this->bounds, style);
    return !this->selectedTextRects.empty();
}

void PdfElemSelection::paint(cairo_t* cr, XojPdfPageSelectionStyle style) {
    if (!this->pdf)
        return;

    GdkRGBA selectionColor = view->getSelectionColor();
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};

    if (this->finalized || style != XojPdfPageSelectionStyle::Area) {
        if (!this->selectedTextRegion || cairo_region_is_empty(this->selectedTextRegion))
            return;

        gdk_cairo_region(cr, this->selectedTextRegion);
        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    } else {
        double aX = std::min(this->bounds.x1, this->bounds.x2);
        double bX = std::max(this->bounds.x1, this->bounds.x2);

        double aY = std::min(this->bounds.y1, this->bounds.y2);
        double bY = std::max(this->bounds.y1, this->bounds.y2);

        cairo_move_to(cr, aX, aY);
        cairo_line_to(cr, bX, aY);
        cairo_line_to(cr, bX, bY);
        cairo_line_to(cr, aX, bY);
        cairo_close_path(cr);

        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    }
}

XojPdfPageSelectionStyle PdfElemSelection::selectionStyleForToolType(ToolType type) {
    switch (type) {
        case ToolType::TOOL_SELECT_PDF_TEXT_RECT:
            return XojPdfPageSelectionStyle::Area;
        default:
            return XojPdfPageSelectionStyle::Linear;
    }
}

void PdfElemSelection::currentPos(double x, double y, XojPdfPageSelectionStyle style) {
    if (!this->pdf)
        return;

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::min();
    double minY = minX, maxY = maxX;

    // Update the end position
    this->bounds.x2 = x;
    this->bounds.y2 = y;

    // Repaint the selected text area
    switch (style) {
        case XojPdfPageSelectionStyle::Linear:
        case XojPdfPageSelectionStyle::Word:
        case XojPdfPageSelectionStyle::Line:
            // The text region may be split up across multiple lines, so redraw the
            // enclosing rectangle
            if (this->selectedTextRegion) {
                int count = cairo_region_num_rectangles(this->selectedTextRegion);
                if (count > 0) {
                    cairo_rectangle_int_t bbox;
                    cairo_region_get_extents(this->selectedTextRegion, &bbox);
                    minX = bbox.x;
                    minY = bbox.y;
                    maxX = bbox.x + bbox.width;
                    maxY = bbox.y + bbox.height;
                    this->view->repaintArea(minX - 20, minY - 20, maxX + 20, maxY + 20);
                }
            }

            this->selectTextRegion(style);
            break;
        case XojPdfPageSelectionStyle::Area:
            minX = std::min(this->bounds.x1, std::min(this->bounds.x2, x));
            minY = std::min(this->bounds.y1, std::min(this->bounds.y2, y));

            maxX = std::max(this->bounds.x1, std::max(this->bounds.x2, x));
            maxY = std::max(this->bounds.y1, std::max(this->bounds.y2, y));

            this->view->repaintArea(minX - 20, minY - 20, maxX + 20, maxY + 20);
            break;
        default:
            g_assert(false && "Unreachable");
    }
}

auto PdfElemSelection::contains(double x, double y) -> bool {
    if (!this->selectedTextRegion)
        return false;

    return cairo_region_contains_point(this->selectedTextRegion, static_cast<int>(x), static_cast<int>(y));
}

auto PdfElemSelection::selectTextRegion(XojPdfPageSelectionStyle style) -> bool {
    if (this->selectedTextRegion) {
        cairo_region_destroy(this->selectedTextRegion);
    }

    this->selectedTextRegion = this->pdf->selectTextRegion(this->bounds, style);
    g_assert_nonnull(this->selectedTextRegion);

    return !cairo_region_is_empty(this->selectedTextRegion);
}

auto PdfElemSelection::getSelectedTextRects() const -> const std::vector<XojPdfRectangle>& { return selectedTextRects; }

auto PdfElemSelection::getSelectedText() const -> const std::string& { return this->selectedText; }

auto PdfElemSelection::getPageView() const -> XojPageView* { return view; }

auto PdfElemSelection::getSelectionPageNr() const -> uint64_t { return selectionPageNr; }

auto PdfElemSelection::isFinalized() const -> bool { return this->finalized; }

void PdfElemSelection::setToolType(ToolType tType) { this->toolType = tType; }
