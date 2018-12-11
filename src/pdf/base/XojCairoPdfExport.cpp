#include "XojCairoPdfExport.h"

#include "view/DocumentView.h"

#include <i18n.h>
#include <Util.h>

#include <cairo/cairo-pdf.h>

XojCairoPdfExport::XojCairoPdfExport(Document* doc, ProgressListener* progressListener)
 : doc(doc),
   progressListener(progressListener),
   surface(NULL),
   cr(NULL)
{
    XOJ_INIT_TYPE(XojCairoPdfExport);
}

XojCairoPdfExport::~XojCairoPdfExport()
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	if (this->surface != NULL)
	{
		endPdf();
	}

    XOJ_RELEASE_TYPE(XojCairoPdfExport);
}

bool XojCairoPdfExport::startPdf(path file)
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	this->surface = cairo_pdf_surface_create(PATH_TO_CSTR(file), 0, 0);
	this->cr = cairo_create(surface);

	// Require Cairo 1.16
#ifdef CAIRO_PDF_METADATA_TITLE
//	cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_TITLE, doc->getFilename().c_str());
#endif

	return true;
}

void XojCairoPdfExport::endPdf()
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	cairo_destroy(this->cr);
	this->cr = NULL;
	cairo_surface_destroy(this->surface);
	this->surface = NULL;
}

void XojCairoPdfExport::exportPage(size_t page)
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	PageRef p = doc->getPage(page);

	cairo_pdf_surface_set_size(this->surface, p->getWidth(), p->getHeight());

	DocumentView view;

	if (p->getBackgroundType().isPdfPage())
	{
		int pgNo = p->getPdfPageNr();
		XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

		popplerPage->render(cr, true);
	}

	view.drawPage(p, this->cr, true /* dont render eraseable */);

	// next page
	cairo_show_page(this->cr);
}

bool XojCairoPdfExport::createPdf(path file, PageRangeVector& range)
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	if (range.size() == 0)
	{
		this->lastError = _("No pages to export!");
		return false;
	}

	if (!startPdf(file))
	{
		return false;
	}

	int count = 0;
	for (PageRangeEntry* e : range)
	{
		count += e->getLast() - e->getFirst() + 1;
	}

	if (this->progressListener)
	{
		this->progressListener->setMaximumState(count);
	}

	int c = 0;
	for (PageRangeEntry* e : range)
	{
		for (int i = e->getFirst(); i <= e->getLast(); i++)
		{
			int p = i - 1;
			if (p < 0 || p >= (int)doc->getPageCount())
			{
				continue;
			}

			exportPage(i);

			if (this->progressListener)
			{
				this->progressListener->setCurrentState(c++);
			}
		}
	}

	endPdf();
	return true;
}

bool XojCairoPdfExport::createPdf(path file)
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	if (doc->getPageCount() < 1)
	{
		lastError = _("No pages to export!");
		return false;
	}

	if (!startPdf(file))
	{
		return false;
	}

	int count = doc->getPageCount();
	if (this->progressListener)
	{
		this->progressListener->setMaximumState(count);
	}

	for (int i = 0; i < count; i++)
	{
		exportPage(i);

		if (this->progressListener)
		{
			this->progressListener->setCurrentState(i);
		}
	}

	endPdf();
	return true;
}

string XojCairoPdfExport::getLastError()
{
	XOJ_CHECK_TYPE(XojCairoPdfExport);

	return lastError;
}

