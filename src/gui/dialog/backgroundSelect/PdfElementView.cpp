#include "PdfElementView.h"

PdfElementView::PdfElementView()
{
	XOJ_INIT_TYPE(PdfElementView);
}

PdfElementView::~PdfElementView()
{
	XOJ_CHECK_TYPE(PdfElementView);

	XOJ_RELEASE_TYPE(PdfElementView);
}

