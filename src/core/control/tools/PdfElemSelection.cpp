#include "PdfElemSelection.h"

#include "control/Control.h"
#include "gui/XournalView.h"

PdfElemSelection::PdfElemSelection(double x, double y, XojPageView* view):
        sx(x),
        sy(y),
        ex(x),
        ey(y),

        view(view),
        isFinalized(false),
        isFinished(false) {

    auto xournal = view->getXournal();
    auto pNr = this->view->getPage()->getPdfPageNr();
    if (pNr != npos) {
        Document* doc = xournal->getControl()->getDocument();

        doc->lock();
        pdf = doc->getPdfPage(pNr);
        doc->unlock();
        this->pdf = std::move(pdf);

        this->selectionPageNr = pNr;
    }

    this->toolType = xournal->getControl()->getToolHandler()->getToolType();
}

PdfElemSelection::~PdfElemSelection() {
    this->selectedTextRects.clear();
    this->selectedText.clear();
    cairo_region_destroy(this->selectedTextRegion);
}

auto PdfElemSelection::finalize(PageRef page) -> bool {
    this->isFinalized = true;

    if (this->selectionStyle == XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_WORD) {
        doublePress();
    } else if (this->selectionStyle == XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_LINE) {
        triplePress();
    } else {
        this->selectFinally();
    }

    this->view->repaintPage();

    return !this->selectedTextRects.empty();
}

void PdfElemSelection::doublePress() {
    if (this->selectionStyle != XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_WORD) {
        this->isFinalized = false;
    }

    this->selectionStyle = XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_WORD;
    this->selectHeadTailFinally();
    // maybe we could do more here
}

void PdfElemSelection::triplePress() {
    if (this->selectionStyle != XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_LINE) {
        this->isFinalized = false;
    }

    this->selectionStyle = XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_LINE;
    this->selectHeadTailFinally();
    // maybe we could do more here
}

void PdfElemSelection::selectHeadTailFinally() {
    auto se = XojPdfRectangle{sx, sy, ex, ey};
    this->pdf->selectHeadTailFinally(se, &this->selectedTextRegion, &this->selectedTextRects, &this->selectedText,
                                     this->selectionStyle);
}

void PdfElemSelection::selectFinally() {
    if (toolType == ToolType::TOOL_SELECT_PDF_TEXT_RECT) {
        auto se = XojPdfRectangle{sx, sy, ex, ey};
        this->pdf->selectAreaFinally(se, &this->selectedTextRegion, &this->selectedTextRects, &this->selectedText);
    } else {
        this->selectHeadTailFinally();
    }
}

void PdfElemSelection::paint(cairo_t* cr, GdkRectangle* rect, double zoom) {
    if (!this->pdf)
        return;

    GdkRGBA selectionColor = view->getSelectionColor();
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};

    if (this->isFinalized || toolType == ToolType::TOOL_SELECT_PDF_TEXT_LINEAR) {
        if (!this->selectedTextRegion || cairo_region_is_empty(this->selectedTextRegion))
            return;

        gdk_cairo_region(cr, this->selectedTextRegion);
        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    } else {
        int aX = std::min(this->sx, this->ex);
        int bX = std::max(this->sx, this->ex);

        int aY = std::min(this->sy, this->ey);
        int bY = std::max(this->sy, this->ey);

        cairo_move_to(cr, aX, aY);
        cairo_line_to(cr, bX, aY);
        cairo_line_to(cr, bX, bY);
        cairo_line_to(cr, aX, bY);
        cairo_close_path(cr);

        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    }
}

void PdfElemSelection::currentPos(double x, double y) {
    if (!this->pdf)
        return;

    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    if (toolType == ToolType::TOOL_SELECT_PDF_TEXT_LINEAR) {
        if (this->selectedTextRegion) {
            int count = cairo_region_num_rectangles(this->selectedTextRegion);

            cairo_rectangle_int_t rect;
            for (int i = 0; i < count; ++i) {
                cairo_region_get_rectangle(this->selectedTextRegion, i, &rect);

                minX = std::min(minX, rect.x);
                minY = std::min(minY, rect.y);

                maxX = std::max(maxX, rect.x + rect.width);
                maxY = std::max(maxY, rect.y + rect.height);
            }

            if (count > 0) {
                this->view->repaintArea(minX - 20, minY - 20, maxX + 20, maxY + 20);
            }
        }

        this->ex = x;
        this->ey = y;

        cairo_region_destroy(this->selectedTextRegion);
        this->selectedTextRegion = nullptr;
        this->selectHeadTailTextRegion();
    } else if (toolType == ToolType::TOOL_SELECT_PDF_TEXT_RECT) {
        minX = std::min(this->sx, std::min(this->ex, x));
        minY = std::min(this->sy, std::min(this->ey, y));

        maxX = std::max(this->sx, std::max(this->ex, x));
        maxY = std::max(this->sy, std::max(this->ey, y));

        this->ex = x;
        this->ey = y;

        this->view->repaintArea(minX - 20, minY - 20, maxX + 20, maxY + 20);
    }
}

auto PdfElemSelection::contains(double x, double y) -> bool {
    if (!this->selectedTextRegion)
        return false;

    int count = cairo_region_num_rectangles(this->selectedTextRegion);
    if (count == 0)
        return false;

    cairo_rectangle_int_t rect;
    for (int i = 0; i < count; ++i) {
        cairo_region_get_rectangle(this->selectedTextRegion, i, &rect);
        if (rect.x <= x && (rect.x + rect.width) >= x && rect.y <= y && (rect.y + rect.height) >= y) {
            return true;
        }
    }
    return false;
}

auto PdfElemSelection::selectHeadTailTextRegion() -> bool {
    auto se = XojPdfRectangle{sx, sy, ex, ey};

    this->selectedTextRegion = this->pdf->selectHeadTailTextRegion(se, this->selectionStyle);

    return !cairo_region_is_empty(this->selectedTextRegion);
}

auto PdfElemSelection::getSelectedTextRects() const -> const std::vector<XojPdfRectangle>& { return selectedTextRects; }

auto PdfElemSelection::getSelectedText() const -> const std::string& { return selectedText; }

auto PdfElemSelection::getPageView() const -> XojPageView* { return view; }

auto PdfElemSelection::getSelectionPageNr() const -> uint64_t { return selectionPageNr; }

auto PdfElemSelection::getIsFinalized() const -> bool { return this->isFinalized; }

auto PdfElemSelection::getIsFinished() const -> bool { return this->isFinished; }

void PdfElemSelection::setToolType(ToolType tType) { this->toolType = tType; }
