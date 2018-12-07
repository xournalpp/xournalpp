#include "PdfElementView.h"

#include "PdfPagesDialog.h"

#include "pdf/base/XojPdfPage.h"


PdfElementView::PdfElementView(int id, XojPdfPage* page, PdfPagesDialog* dlg)
 : BaseElementView(id, dlg),
   page(page),
   used(false)
{
	XOJ_INIT_TYPE(PdfElementView);
}

PdfElementView::~PdfElementView()
{
	XOJ_CHECK_TYPE(PdfElementView);

	XOJ_RELEASE_TYPE(PdfElementView);
}

bool PdfElementView::isUsed()
{
	XOJ_CHECK_TYPE(PdfElementView);

	return this->used;
}

void PdfElementView::setUsed(bool used)
{
	XOJ_CHECK_TYPE(PdfElementView);

	this->used = used;
}

void PdfElementView::setHideUnused()
{
	XOJ_CHECK_TYPE(PdfElementView);

	gtk_widget_set_visible(getWidget(), !isUsed());
}

void PdfElementView::paintContents(cairo_t* cr)
{
	XOJ_CHECK_TYPE(PdfElementView);

	double zoom = ((PdfPagesDialog*)dlg)->getZoom();
	cairo_scale(cr, zoom, zoom);
	page->render(cr);
}

int PdfElementView::getContentWidth()
{
	XOJ_CHECK_TYPE(PdfElementView);

	return page->getWidth() * ((PdfPagesDialog*)dlg)->getZoom();
}

int PdfElementView::getContentHeight()
{
	XOJ_CHECK_TYPE(PdfElementView);

	return page->getHeight() * ((PdfPagesDialog*)dlg)->getZoom();
}

