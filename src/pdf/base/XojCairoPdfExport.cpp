#include "XojCairoPdfExport.h"

#include "view/DocumentView.h"

#include <i18n.h>
#include <Util.h>

#include <cairo/cairo-pdf.h>

XojCairoPdfExport::XojCairoPdfExport(Document* doc, ProgressListener* progressListener)
 : doc(doc),
   progressListener(progressListener)
{
}

XojCairoPdfExport::~XojCairoPdfExport()
{
	if (this->surface != nullptr)
	{
		endPdf();
	}
}

/**
 * Export without background
 */
void XojCairoPdfExport::setNoBackgroundExport(bool noBackgroundExport)
{
	this->noBackgroundExport = noBackgroundExport;
}

bool XojCairoPdfExport::startPdf(Path file)
{
	this->surface = cairo_pdf_surface_create(file.c_str(), 0, 0);
	this->cr = cairo_create(surface);

	// Require Cairo 1.16
#ifdef CAIRO_PDF_METADATA_TITLE
//	cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_TITLE, doc->getFilename().c_str());
#endif

	return true;
}

void XojCairoPdfExport::endPdf()
{
	cairo_destroy(this->cr);
	this->cr = nullptr;
	cairo_surface_destroy(this->surface);
	this->surface = nullptr;
}

void XojCairoPdfExport::exportPage(size_t page)
{
	PageRef p = doc->getPage(page);

	cairo_pdf_surface_set_size(this->surface, p->getWidth(), p->getHeight());

	DocumentView view;

	if (p->getBackgroundType().isPdfPage() && !noBackgroundExport)
	{
		int pgNo = p->getPdfPageNr();
		XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

		popplerPage->render(cr, true);
	}

	view.drawPage(p, this->cr, true /* dont render eraseable */, noBackgroundExport);

	// next page
	cairo_show_page(this->cr);
}

bool XojCairoPdfExport::createPdf(Path file, PageRangeVector& range)
{
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
			if (i < 0 || i > (int)doc->getPageCount())
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

bool XojCairoPdfExport::createPdf(Path file)
{
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
	return lastError;
}

