#include "PdfElementView.h"

#include <utility>

#include "PdfPagesDialog.h"

#include "pdf/base/XojPdfPage.h"


PdfElementView::PdfElementView(int id, XojPdfPageSPtr page, PdfPagesDialog* dlg)
 : BaseElementView(id, dlg)
 , page(std::move(page))
{
}

PdfElementView::~PdfElementView() = default;

auto PdfElementView::isUsed() -> bool
{
	return this->used;
}

void PdfElementView::setUsed(bool used)
{
	this->used = used;
}

void PdfElementView::setHideUnused()
{
	gtk_widget_set_visible(getWidget(), !isUsed());
}

void PdfElementView::paintContents(cairo_t* cr)
{
	double zoom = (dynamic_cast<PdfPagesDialog*>(dlg))->getZoom();
	cairo_scale(cr, zoom, zoom);
	page->render(cr);
}

auto PdfElementView::getContentWidth() -> int
{
	return page->getWidth() * (dynamic_cast<PdfPagesDialog*>(dlg))->getZoom();
}

auto PdfElementView::getContentHeight() -> int
{
	return page->getHeight() * (dynamic_cast<PdfPagesDialog*>(dlg))->getZoom();
}

