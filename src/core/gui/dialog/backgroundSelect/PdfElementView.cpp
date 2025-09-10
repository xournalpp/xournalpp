#include "PdfElementView.h"

#include <memory>   // for __shared_pt...
#include <utility>  // for move

#include <gtk/gtk.h>  // for gtk_widget_...

#include "gui/dialog/backgroundSelect/BaseElementView.h"  // for BaseElement...
#include "pdf/base/XojPdfPage.h"                          // for XojPdfPageSPtr
#include "util/safe_casts.h"                              // for ceil_cast

#include "PdfPagesDialog.h"  // for PdfPagesDialog

PdfElementView::PdfElementView(size_t id, XojPdfPageSPtr page, PdfPagesDialog* dlg):
        BaseElementView(id, dlg), page(std::move(page)) {}

PdfElementView::~PdfElementView() = default;

auto PdfElementView::isUsed() const -> bool { return this->used; }

void PdfElementView::setUsed(bool used) { this->used = used; }

void PdfElementView::setHideIfUsed(bool hideIfUsed) { gtk_widget_set_visible(getWidget(), !(hideIfUsed && isUsed())); }

void PdfElementView::paintContents(cairo_t* cr) {
    cairo_scale(cr, PdfPagesDialog::ZOOM_VALUE, PdfPagesDialog::ZOOM_VALUE);
    page->render(cr);
}

auto PdfElementView::getContentWidth() -> int { return floor_cast<int>(page->getWidth() * PdfPagesDialog::ZOOM_VALUE); }

auto PdfElementView::getContentHeight() -> int {
    return floor_cast<int>(page->getHeight() * PdfPagesDialog::ZOOM_VALUE);
}
