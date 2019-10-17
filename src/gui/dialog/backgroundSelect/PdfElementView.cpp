#include "PdfElementView.h"

#include "PdfPagesDialog.h"

#include "pdf/base/XojPdfPage.h"


PdfElementView::PdfElementView(int id, XojPdfPageSPtr page, PdfPagesDialog* dlg)
 : BaseElementView(id, dlg),
   page(page)
{
}

PdfElementView::~PdfElementView()
{
}

bool PdfElementView::isUsed()
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
	double zoom = ((PdfPagesDialog*)dlg)->getZoom();
	cairo_scale(cr, zoom, zoom);
	page->render(cr);
}

int PdfElementView::getContentWidth()
{
	return page->getWidth() * ((PdfPagesDialog*)dlg)->getZoom();
}

int PdfElementView::getContentHeight()
{
	return page->getHeight() * ((PdfPagesDialog*)dlg)->getZoom();
}

