#include "PdfElementView.h"

#include <memory>   // for __shared_pt...
#include <utility>  // for move

#include <gtk/gtk.h>  // for gtk_widget_...

#include "gui/dialog/backgroundSelect/BaseElementView.h"  // for BaseElement...
#include "pdf/base/XojPdfPage.h"                          // for XojPdfPageSPtr
#include "util/safe_casts.h"                              // for ceil_cast

#include "PdfPagesDialog.h"  // for PdfPagesDialog

PdfElementView::PdfElementView(int id, XojPdfPageSPtr page, PdfPagesDialog* dlg):
        BaseElementView(id, dlg), page(std::move(page)) {}

PdfElementView::~PdfElementView() = default;

auto PdfElementView::isUsed() const -> bool { return this->used; }

void PdfElementView::setUsed(bool used) { this->used = used; }

void PdfElementView::setHideUnused() { gtk_widget_set_visible(getWidget(), !isUsed()); }

void PdfElementView::paintContents(cairo_t* cr) {
    double zoom = PdfPagesDialog::getZoom();
    cairo_scale(cr, zoom, zoom);
    page->render(cr);
}

auto PdfElementView::getContentWidth() -> int { return ceil_cast<int>(page->getWidth() * PdfPagesDialog::getZoom()); }

auto PdfElementView::getContentHeight() -> int { return ceil_cast<int>(page->getHeight() * PdfPagesDialog::getZoom()); }
